#define WIN32_LEAN_AND_MEAN 

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// Yo wtf windows.h ... 
#undef far
#undef near

#define WIDTH 1280
#define HEIGHT 720

// My files
#include "util.h"
#include "letters.h"
#include "network.h"
#include "drawbuffer.h"
#include "linalg.h"

// Global variables
HINSTANCE app_instance;
char title[] = "FPS";
char window_class[] = "FPSWindowClass";
DrawBuffer* main_buffer;

const I32 CHAR_WIDTH = 12;
const I32 CHAR_HEIGHT = 16;

// Function prototypes if needed.
void DrawToBackBuffer(HDC hdc, DrawBuffer* buffer);

typedef struct Camera Camera;
struct Camera
{
    R32 phi;
    R32 theta;
    R32 radius;
};
static Camera camera;

LRESULT CALLBACK 
WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) 
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message) {
    case WM_KEYDOWN:
        R32 speed = 0.01f;
        R32 zoom = 1.01f;
    switch (wparam) {
        case 'W': camera.phi -= speed; break;
        case 'S': camera.phi += speed; break;
        case 'A': camera.theta -= speed; break;
        case 'D': camera.theta += speed; break;
        case 'X': camera.radius *= zoom; break;
        case 'Z': camera.radius /= zoom; break;
    }
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        DrawToBackBuffer(hdc, main_buffer);
        EndPaint(hwnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, message, wparam, lparam);
    }
    return 0;
}

void 
DrawToBackBuffer(HDC hdc, DrawBuffer* buffer) 
{
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = WIDTH;
    bmi.bmiHeader.biHeight = -HEIGHT;  // top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    StretchDIBits(hdc, 0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT,
                  buffer->pixels, &bmi, DIB_RGB_COLORS, SRCCOPY);
}

void 
TestTriangle(DrawBuffer* buffer, I32 x, I32 y, R32 angle, U32 color)
{
    R32 l = 20;
    I32 x0 = x + (I32)(l * sinf(angle));
    I32 y0 = y + (I32)(l * cosf(angle));
    I32 x1 = x + (I32)(l * sinf(angle + 2.0f * 3.14f / 3.0f));
    I32 y1 = y + (I32)(l * cosf(angle + 2.0f * 3.14f / 3.0f));
    I32 x2 = x + (I32)(l * sinf(angle + 4.0f * 3.14f / 3.0f));
    I32 y2 = y + (I32)(l * cosf(angle + 4.0f * 3.14f / 3.0f));
    FillTriangle(buffer, x0, y0, x1, y1, x2, y2, color);  
}

void 
BlitCharacter(DrawBuffer* buffer, I32 x, I32 y, char c, U32 color)
{
    const I32 BYTES_PER_ROW = (CHAR_WIDTH + 7) / 8; 

    I32 bitmap_offset = c * CHAR_HEIGHT * BYTES_PER_ROW;

    for(I32 yy = 0; yy < CHAR_HEIGHT; yy++) 
    for(I32 xx = 0; xx < CHAR_WIDTH; xx++) 
    {
        int byte_idx = bitmap_offset + yy * BYTES_PER_ROW + (xx / 8);
        int bit_idx = xx % 8;

        U8 val = (console_font_12x16[byte_idx] >> (7 - bit_idx)) & 1;

        if(val) 
        {
            SetPixelColor(buffer, x + xx, y + yy, color);
        }
    }
}

void 
BlitText(DrawBuffer* buffer, const char* text, I32 start_x, I32 start_y, U32 color)
{
    I32 x = start_x;
    I32 y = start_y;
    const I32 CHAR_WIDTH = 12;
    const I32 CHAR_HEIGHT = 16;

    while (*text) 
    {
        if (*text == '\n') 
        {
            x = start_x;
            y += CHAR_HEIGHT;
        } 
        else 
        {
            BlitCharacter(buffer, x, y, *text, color);
            x += CHAR_WIDTH;
            if (x + CHAR_WIDTH > WIDTH) {
                x = start_x;
                y += CHAR_HEIGHT;
            }
        }
        text++;
    }
}

void 
DrawGizmo(DrawBuffer* buffer, M4 transform, R32 width) {
    // Define vertices for the rectangles with the given width
    V3 xRect[4] = {
        {0.0f, width, width},
        {1.0f, width, width},
        {1.0f, -width, -width},
        {0.0f, -width, -width}
    };

    V3 yRect[4] = {
        {width, 0.0f, width},
        {width, 1.0f, width},
        {-width, 1.0f, -width},
        {-width, 0.0f, -width}
    };

    V3 zRect[4] = {
        {width, width, 0.0f},
        {width, width, 1.0f},
        {-width, -width, 1.0f},
        {-width, -width, 0.0f}
    };

    // Transform vertices
    for (int i = 0; i < 4; i++) {
        xRect[i] = TransformV3(&transform, xRect[i]);
        yRect[i] = TransformV3(&transform, yRect[i]);
        zRect[i] = TransformV3(&transform, zRect[i]);

        xRect[i] = TransformToScreenSpace(xRect[i], WIDTH, HEIGHT);
        yRect[i] = TransformToScreenSpace(yRect[i], WIDTH, HEIGHT);
        zRect[i] = TransformToScreenSpace(zRect[i], WIDTH, HEIGHT);
    }

    // Colors
    U32 red = 0xFF0000FF;   // Red
    U32 green = 0xFF00FF00; // Green
    U32 blue = 0xFFFF0000;  // Blue

    // Draw X axis rectangle (red)
    FillTriangleDepth(buffer, (I32)xRect[0].x, (I32)xRect[0].y, xRect[0].z, (I32)xRect[1].x, (I32)xRect[1].y, xRect[1].z, (I32)xRect[2].x, (I32)xRect[2].y, xRect[2].z, red);
    FillTriangleDepth(buffer, (I32)xRect[0].x, (I32)xRect[0].y, xRect[0].z, (I32)xRect[2].x, (I32)xRect[2].y, xRect[2].z, (I32)xRect[3].x, (I32)xRect[3].y, xRect[3].z, red);

    // Draw Y axis rectangle (green)
    FillTriangleDepth(buffer, (I32)yRect[0].x, (I32)yRect[0].y, yRect[0].z, (I32)yRect[1].x, (I32)yRect[1].y, yRect[1].z, (I32)yRect[2].x, (I32)yRect[2].y, yRect[2].z, green);
    FillTriangleDepth(buffer, (I32)yRect[0].x, (I32)yRect[0].y, yRect[0].z, (I32)yRect[2].x, (I32)yRect[2].y, yRect[2].z, (I32)yRect[3].x, (I32)yRect[3].y, yRect[3].z, green);

    // Draw Z axis rectangle (blue)
    FillTriangleDepth(buffer, (I32)zRect[0].x, (I32)zRect[0].y, zRect[0].z, (I32)zRect[1].x, (I32)zRect[1].y, zRect[1].z, (I32)zRect[2].x, (I32)zRect[2].y, zRect[2].z, blue);
    FillTriangleDepth(buffer, (I32)zRect[0].x, (I32)zRect[0].y, zRect[0].z, (I32)zRect[2].x, (I32)zRect[2].y, zRect[2].z, (I32)zRect[3].x, (I32)zRect[3].y, zRect[3].z, blue);
}

I32 APIENTRY 
WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, LPSTR lpcmdline, I32 ncmdshow) 
{
    WNDCLASSEX wcex;
    HWND window_handle;
    MSG msg;

    // Create a buffer to draw to
    DrawBuffer* buffer = CreateBuffer(WIDTH, HEIGHT);

    // TODO: Change this such that we have double buffering. 
    main_buffer = buffer;

    app_instance = hinstance;

    // Allocate a console for the application
    AllocConsole();
    freopen("CONOUT$", "w", stdout);

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hinstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = window_class;
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    printf("2Now starting application \n");

    RegisterClassEx(&wcex);

    window_handle = CreateWindow(window_class, title, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, 0, WIDTH, HEIGHT, NULL, NULL, app_instance, NULL);

    if (!window_handle) 
    {
        return FALSE;
    }

    ShowWindow(window_handle, ncmdshow);
    UpdateWindow(window_handle);

    #define N_TRIANGLES 10000
    float x[N_TRIANGLES];
    float y[N_TRIANGLES];
    U32 c[N_TRIANGLES];

    for(int i = 0; i < N_TRIANGLES; i++)
    {
        x[i] = RandomR32(0, WIDTH);
        y[i] = RandomR32(0, HEIGHT);
        U8 r = RandomU32(0, 255);
        U8 g = RandomU32(0, 255);
        U8 b = RandomU32(0, 255);
        U32 color = CreateColor(r, g, b, 255);
        c[i] = color;
    }

    camera.radius = 14.0f;
    camera.phi = 3.14f/4.0f;
    
    R32 t = 0;
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        t += 0.01f;

        V3 eye;
        eye.x = camera.radius * sinf(camera.phi) * cosf(camera.theta);
        eye.y = camera.radius * sinf(camera.phi) * sinf(camera.theta);
        eye.z = camera.radius * cosf(camera.phi);

        printf("%f %f %f\n" , eye.x, eye.y, eye.z);

        // Clear buffer
        ClearBuffer(buffer);

        // Camera setup
        V3 center = {0.0f, 0.0f, 0.0f}; // Look at point
        V3 up = {0.0f, 0.0f, 1.0f};    // Up direction
        M4 view = MatrixLookAt(eye, center, up);

        // Perspective matrix
        M4 perspective = MatrixPerspective(1.2f, WIDTH, HEIGHT, 1.0f, 10.0f);

        // Draw a test 3D triangle
        V3 v0 = {-0.5f, 0.5f, 0.0f}; // bottom left
        V3 v1 = {0.5f, -0.14f, 0.0f};  // bottom right
        V3 v2 = {0.0f, 0.0f, 2.0f+sinf(t)};   // top center

        // Combine transformations
        M4 transform = MatMul(perspective, view);

        // Transform
#if 1
        v0 = TransformV3(&transform, v0);
        v1 = TransformV3(&transform, v1);
        v2 = TransformV3(&transform, v2);
#endif

        v0 = TransformToScreenSpace(v0, WIDTH, HEIGHT);
        v1 = TransformToScreenSpace(v1, WIDTH, HEIGHT);
        v2 = TransformToScreenSpace(v2, WIDTH, HEIGHT);

        DrawGizmo(buffer, transform, 0.1f); 
#if 0
        // Triangle stress test
        for(I32 i = 0; i < 200; i++)
        {
            U32 color = RandomU32(0, UINT32_MAX);
            TestTriangle(buffer, x[i], y[i], i+t, c[i]);
        }
#endif

        BlitCharacter(buffer, 0, 0, 'X', 0xffffffff);
        BlitCharacter(buffer, 20, 20, 'X', 0xffffffff);
        BlitCharacter(buffer, 60, 20, 'A', 0xffffffff);
        BlitCharacter(buffer, 80, 20, 's', 0xffffffff);
        BlitCharacter(buffer, 100, 20, 'T', 0xffffffff);

        U8 r = 255;
        U8 g = 0;
        U8 b = 255;
        U32 color = CreateColor(r, g, b, 255);
        BlitText(buffer, "Hello!! Hello I am tim!", 20, HEIGHT/2, color);

        r = 0;
        g = 255;
        b = 255;
        color = CreateColor(r, g, b, 255);
        BlitText(buffer, "Still tim", 180, HEIGHT/2+100, color);

        r = 255;
        g = 255;
        b = 255;
        color = CreateColor(r, g, b, 255);
        BlitText(buffer, "Helloooo I am tim and i am happpy to be here.", 20, 20, color);
        BlitText(buffer, "Hellooo i am happy to be here", 20, 40, color);
        BlitText(buffer, "Helloo i am happy", 20, 60, color);
        BlitText(buffer, "hlleoosdfj", 20, 80, color);
        BlitText(buffer, ":)", 20, 100, color);

        InvalidateRect(window_handle, NULL, FALSE);
    }

    return (I32)msg.wParam;
}

