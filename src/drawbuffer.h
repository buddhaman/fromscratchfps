#include "util.h"

typedef struct DrawBuffer DrawBuffer;
struct DrawBuffer
{
    I32 width;
    I32 height;
    U32* pixels;

    // This is optional
    R32* depth;
};

DrawBuffer* 
CreateBuffer(I32 width, I32 height)
{
    DrawBuffer* buffer = (DrawBuffer*)malloc(sizeof(*buffer) + width*height*sizeof(U32));
    buffer->pixels = (U32*)(buffer+1);
    buffer->width = width;
    buffer->height = height;
    return buffer;
}

void 
ClearBuffer(DrawBuffer* buffer) 
{
    memset(buffer->pixels, 0, buffer->width*buffer->height*sizeof(U32));
}

inline static void 
SetPixelColor(DrawBuffer* buffer, I32 x, I32 y, U32 color) 
{
    if (x >= 0 && x < buffer->width && y >= 0 && y < buffer->height) 
    {
        buffer->pixels[y * buffer->width + x] = color;
    }
}

inline static void 
DrawLine(DrawBuffer* buffer, I32 x0, I32 y0, I32 x1, I32 y1, U32 color) 
{
    I32 dx = abs(x1 - x0);
    I32 dy = abs(y1 - y0);
    I32 sx = (x0 < x1) ? 1 : -1;
    I32 sy = (y0 < y1) ? 1 : -1;
    I32 err = dx - dy;

    while (true) 
    {
        SetPixelColor(buffer, x0, y0, color);
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

void 
FillBottomFlatTriangle(DrawBuffer* buffer, I32 x0, I32 y0, I32 x1, I32 y1, I32 x2, I32 y2, U32 color)
{
    float invslope1 = (float)(x1 - x0) / (y1 - y0);
    float invslope2 = (float)(x2 - x0) / (y2 - y0);

    float curx1 = x0;
    float curx2 = x0;

    for (I32 scanlineY = y0; scanlineY <= y1; scanlineY++) 
    {
        DrawLine(buffer, (I32)curx1, scanlineY, (I32)curx2, scanlineY, color);
        curx1 += invslope1;
        curx2 += invslope2;
    }
}

void 
FillTopFlatTriangle(DrawBuffer* buffer, I32 x0, I32 y0, I32 x1, I32 y1, I32 x2, I32 y2, U32 color)
{
    float invslope1 = (float)(x2 - x0) / (y2 - y0);
    float invslope2 = (float)(x2 - x1) / (y2 - y1);

    float curx1 = x2;
    float curx2 = x2;

    for (I32 scanlineY = y2; scanlineY > y0; scanlineY--) 
    {
        DrawLine(buffer, (I32)curx1, scanlineY, (I32)curx2, scanlineY, color);
        curx1 -= invslope1;
        curx2 -= invslope2;
    }
}

void 
FillTriangle(DrawBuffer* buffer, I32 x0, I32 y0, I32 x1, I32 y1, I32 x2, I32 y2, U32 color)
{
    // Sort vertices by y-coordinate ascending (y0 <= y1 <= y2)
    if (y0 > y1) { I32 tmp = y0; y0 = y1; y1 = tmp; tmp = x0; x0 = x1; x1 = tmp; }
    if (y1 > y2) { I32 tmp = y1; y1 = y2; y2 = tmp; tmp = x1; x1 = x2; x2 = tmp; }
    if (y0 > y1) { I32 tmp = y0; y0 = y1; y1 = tmp; tmp = x0; x0 = x1; x1 = tmp; }

    if (y1 == y2) 
    {
        // Bottom-flat triangle
        FillBottomFlatTriangle(buffer, x0, y0, x1, y1, x2, y2, color);
    } 
    else if (y0 == y1) 
    {
        // Top-flat triangle
        FillTopFlatTriangle(buffer, x0, y0, x1, y1, x2, y2, color);
    } 
    else 
    {
        // General triangle
        I32 x3 = (I32)(x0 + ((float)(y1 - y0) / (y2 - y0)) * (x2 - x0));
        I32 y3 = y1;
        FillBottomFlatTriangle(buffer, x0, y0, x1, y1, x3, y3, color);
        FillTopFlatTriangle(buffer, x1, y1, x3, y3, x2, y2, color);
    }
}