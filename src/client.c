#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#define WIDTH 800
#define HEIGHT 600

#define true 1
#define false 0

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef int8_t I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;

typedef float R32;
typedef double R64;

#define ArraySize(array) (sizeof(arr) / sizeof(array[0]))

inline R32 RandomR32(R32 min, R32 max) 
{
    R32 scale = rand() / (R32) RAND_MAX; 
    return min + scale * (max - min); 
}

inline U32 RandomU32(U32 min, U32 max) 
{
    return (rand() % (max-min)) + min;
}

inline U32 CreateColor(U8 r, U8 g, U8 b, U8 a) 
{
    return ((U32)a << 24) | ((U32)r << 16) | ((U32)g << 8) | (U32)b;
}

// Global variables
HINSTANCE app_instance;
char title[] = "FPS";
char window_class[] = "FPSWindowClass";
U32 pixel_buffer[WIDTH * HEIGHT];

// Function prototypes if needed.
void DrawBuffer(HDC hdc);

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) 
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message) {
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        DrawBuffer(hdc);
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

void ClearBuffer() 
{
    memset(pixel_buffer, 0, sizeof(pixel_buffer));
}

void SetPixelColor(I32 x, I32 y, U32 color) 
{
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) 
    {
        pixel_buffer[y * WIDTH + x] = color;
    }
}

void DrawBuffer(HDC hdc) 
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
                  pixel_buffer, &bmi, DIB_RGB_COLORS, SRCCOPY);
}

void DrawLine(I32 x0, I32 y0, I32 x1, I32 y1, U32 color) 
{
    I32 dx = abs(x1 - x0);
    I32 dy = abs(y1 - y0);
    I32 sx = (x0 < x1) ? 1 : -1;
    I32 sy = (y0 < y1) ? 1 : -1;
    I32 err = dx - dy;

    while (true) 
    {
        SetPixelColor(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        I32 e2 = 2 * err;
        if (e2 > -dy) 
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) 
        {
            err += dx;
            y0 += sy;
        }
    }
}

void FillBottomFlatTriangle(I32 x0, I32 y0, I32 x1, I32 y1, I32 x2, I32 y2, U32 color)
{
    float invslope1 = (float)(x1 - x0) / (y1 - y0);
    float invslope2 = (float)(x2 - x0) / (y2 - y0);

    float curx1 = x0;
    float curx2 = x0;

    for (I32 scanlineY = y0; scanlineY <= y1; scanlineY++) 
    {
        DrawLine((I32)curx1, scanlineY, (I32)curx2, scanlineY, color);
        curx1 += invslope1;
        curx2 += invslope2;
    }
}

void FillTopFlatTriangle(I32 x0, I32 y0, I32 x1, I32 y1, I32 x2, I32 y2, U32 color)
{
    float invslope1 = (float)(x2 - x0) / (y2 - y0);
    float invslope2 = (float)(x2 - x1) / (y2 - y1);

    float curx1 = x2;
    float curx2 = x2;

    for (I32 scanlineY = y2; scanlineY > y0; scanlineY--) 
    {
        DrawLine((I32)curx1, scanlineY, (I32)curx2, scanlineY, color);
        curx1 -= invslope1;
        curx2 -= invslope2;
    }
}

void FillTriangle(I32 x0, I32 y0, I32 x1, I32 y1, I32 x2, I32 y2, U32 color)
{
    // Sort vertices by y-coordinate ascending (y0 <= y1 <= y2)
    if (y0 > y1) { I32 tmp = y0; y0 = y1; y1 = tmp; tmp = x0; x0 = x1; x1 = tmp; }
    if (y1 > y2) { I32 tmp = y1; y1 = y2; y2 = tmp; tmp = x1; x1 = x2; x2 = tmp; }
    if (y0 > y1) { I32 tmp = y0; y0 = y1; y1 = tmp; tmp = x0; x0 = x1; x1 = tmp; }

    if (y1 == y2) 
    {
        // Bottom-flat triangle
        FillBottomFlatTriangle(x0, y0, x1, y1, x2, y2, color);
    } 
    else if (y0 == y1) 
    {
        // Top-flat triangle
        FillTopFlatTriangle(x0, y0, x1, y1, x2, y2, color);
    } 
    else 
    {
        // General triangle
        I32 x3 = (I32)(x0 + ((float)(y1 - y0) / (y2 - y0)) * (x2 - x0));
        I32 y3 = y1;
        FillBottomFlatTriangle(x0, y0, x1, y1, x3, y3, color);
        FillTopFlatTriangle(x1, y1, x3, y3, x2, y2, color);
    }
}

void TestTriangle(I32 x, I32 y, R32 angle, U32 color)
{
    // Animate triangle vertices
    R32 l = 10;
    I32 x0 = x + (I32)(l * sinf(angle));
    I32 y0 = y + (I32)(l * cosf(angle));
    I32 x1 = x + (I32)(l * sinf(angle + 2.0f * 3.14f / 3.0f));
    I32 y1 = y + (I32)(l * cosf(angle + 2.0f * 3.14f / 3.0f));
    I32 x2 = x + (I32)(l * sinf(angle + 4.0f * 3.14f / 3.0f));
    I32 y2 = y + (I32)(l * cosf(angle + 4.0f * 3.14f / 3.0f));
    FillTriangle(x0, y0, x1, y1, x2, y2, color);  // Green triangle
}

I32 APIENTRY WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, LPSTR lpcmdline, I32 ncmdshow) 
{
    WNDCLASSEX wcex;
    HWND window_handle;
    MSG msg;

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

    printf("Now starting application \n");

    printf("NOw starting application \n");

    RegisterClassEx(&wcex);

    window_handle = CreateWindow(window_class, title, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, 0, WIDTH, HEIGHT, NULL, NULL, app_instance, NULL);

    if (!window_handle) 
    {
        return FALSE;
    }

    ShowWindow(window_handle, ncmdshow);
    UpdateWindow(window_handle);

    // Clear the buffer and draw some pixels
    ClearBuffer();

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
    
    R32 t = 0;
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        t += 0.01f;

        // Clear buffer
        ClearBuffer();

        // Draw a triangle
        for(I32 i = 0; i < N_TRIANGLES; i++)
        {
            U32 color = RandomU32(0, UINT32_MAX);
            TestTriangle(x[i], y[i], i+t, c[i]);
        }

        InvalidateRect(window_handle, NULL, FALSE);
    }

    return (I32)msg.wParam;
}
