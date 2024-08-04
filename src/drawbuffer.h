#include "util.h"

typedef struct DrawBuffer DrawBuffer;
struct DrawBuffer
{
    I32 width;
    I32 height;

    U32* pixels;
    R32* depth;
};

DrawBuffer* 
CreateBuffer(I32 width, I32 height)
{
    // All bufffers are 4 bytes per pixel.
    size_t n_pixels = width*height;
    size_t buffer_size = 4 * n_pixels;
    DrawBuffer* buffer = (DrawBuffer*)malloc(sizeof(*buffer) + buffer_size*2);
    buffer->pixels = (U32*)(buffer+1);
    buffer->depth = (R32*)(buffer->pixels+n_pixels);
    buffer->width = width;
    buffer->height = height;
    return buffer;
}

void 
ClearBuffer(DrawBuffer* buffer) 
{
    memset(buffer->pixels, 0, buffer->width*buffer->height*sizeof(U32));
    I32 n_pixels = buffer->width*buffer->height;
    for(I32 i = 0; i < n_pixels; i++)
    {
        buffer->depth[i] = R32_MAX;
    }
}

inline static void 
SetPixelColor(DrawBuffer* buffer, I32 x, I32 y, U32 color) 
{
    if (x >= 0 && x < buffer->width && y >= 0 && y < buffer->height) 
    {
        buffer->pixels[y * buffer->width + x] = color;
    }
}

static inline B8 
InRange(DrawBuffer* buffer, I32 x, I32 y)
{
    return (x >= 0 && x < buffer->width && y >= 0 && y < buffer->height);
}

inline static void 
SetPixelColorDepth(DrawBuffer* buffer, I32 x, I32 y, U32 color, R32 depth)
{
    if (x >= 0 && x < buffer->width && y >= 0 && y < buffer->height) 
    {
        I32 index = y * buffer->width + x;
        if(depth < buffer->depth[index])
        {
            buffer->pixels[index] = color;
            buffer->depth[index] = depth;
        }
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

inline static void 
DrawLineDepth(DrawBuffer* buffer, I32 x0, I32 y0, R32 z0, I32 x1, I32 y1, R32 z1, U32 color) 
{
    I32 dx = abs(x1 - x0);
    I32 dy = abs(y1 - y0);
    I32 sx = (x0 < x1) ? 1 : -1;
    I32 sy = (y0 < y1) ? 1 : -1;
    I32 err = dx - dy;
    R32 dz = (dx > dy) ? (z1 - z0) / dx : (z1 - z0) / dy;

    while (true) 
    {
        SetPixelColorDepth(buffer, x0, y0, color, z0);
        if (x0 == x1 && y0 == y1) break;
        I32 e2 = 2 * err;
        if (e2 > -dy) 
        {
            err -= dy;
            x0 += sx;
            z0 += dz * sx;
        }
        if (e2 < dx) 
        {
            err += dx;
            y0 += sy;
            z0 += dz * sy;
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
FillBottomFlatTriangleDepth(DrawBuffer* buffer, I32 x0, I32 y0, R32 z0, I32 x1, I32 y1, R32 z1, I32 x2, I32 y2, R32 z2, U32 color)
{
    float invslope1 = (float)(x1 - x0) / (y1 - y0);
    float invslope2 = (float)(x2 - x0) / (y2 - y0);
    float invz1 = (float)(z1 - z0) / (y1 - y0);
    float invz2 = (float)(z2 - z0) / (y2 - y0);

    float curx1 = x0;
    float curx2 = x0;
    float curz1 = z0;
    float curz2 = z0;

    for (I32 scanlineY = y0; scanlineY <= y1; scanlineY++) 
    {
        DrawLineDepth(buffer, (I32)curx1, scanlineY, curz1, (I32)curx2, scanlineY, curz2, color);
        curx1 += invslope1;
        curx2 += invslope2;
        curz1 += invz1;
        curz2 += invz2;
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
FillTopFlatTriangleDepth(DrawBuffer* buffer, I32 x0, I32 y0, R32 z0, I32 x1, I32 y1, R32 z1, I32 x2, I32 y2, R32 z2, U32 color)
{
    float invslope1 = (float)(x2 - x0) / (y2 - y0);
    float invslope2 = (float)(x2 - x1) / (y2 - y1);
    float invz1 = (float)(z2 - z0) / (y2 - y0);
    float invz2 = (float)(z2 - z1) / (y2 - y1);

    float curx1 = x2;
    float curx2 = x2;
    float curz1 = z2;
    float curz2 = z2;

    for (I32 scanlineY = y2; scanlineY > y0; scanlineY--) 
    {
        DrawLineDepth(buffer, (I32)curx1, scanlineY, curz1, (I32)curx2, scanlineY, curz2, color);
        curx1 -= invslope1;
        curx2 -= invslope2;
        curz1 -= invz1;
        curz2 -= invz2;
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
        FillBottomFlatTriangle(buffer, x0, y0, x1, y1, x2, y2, color);
    } 
    else if (y0 == y1) 
    {
        FillTopFlatTriangle(buffer, x0, y0, x1, y1, x2, y2, color);
    } 
    else 
    {
        I32 x3 = (I32)(x0 + ((float)(y1 - y0) / (y2 - y0)) * (x2 - x0));
        I32 y3 = y1;
        FillBottomFlatTriangle(buffer, x0, y0, x1, y1, x3, y3, color);
        FillTopFlatTriangle(buffer, x1, y1, x3, y3, x2, y2, color);
    }
}

void 
FillTriangleDepth(DrawBuffer* buffer, I32 x0, I32 y0, R32 z0, I32 x1, I32 y1, R32 z1, I32 x2, I32 y2, R32 z2, U32 color)
{
    // Sort vertices by y-coordinate ascending (y0 <= y1 <= y2)
    if (y0 > y1) { I32 tmp = y0; y0 = y1; y1 = tmp; tmp = x0; x0 = x1; x1 = tmp; tmp = z0; z0 = z1; z1 = tmp; }
    if (y1 > y2) { I32 tmp = y1; y1 = y2; y2 = tmp; tmp = x1; x1 = x2; x2 = tmp; tmp = z1; z1 = z2; z2 = tmp; }
    if (y0 > y1) { I32 tmp = y0; y0 = y1; y1 = tmp; tmp = x0; x0 = x1; x1 = tmp; tmp = z0; z0 = z1; z1 = tmp; }

    if (y1 == y2) 
    {
        FillBottomFlatTriangleDepth(buffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, color);
    } 
    else if (y0 == y1) 
    {
        FillTopFlatTriangleDepth(buffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, color);
    } 
    else 
    {
        I32 x3 = (I32)(x0 + ((float)(y1 - y0) / (y2 - y0)) * (x2 - x0));
        I32 y3 = y1;
        R32 z3 = z0 + ((float)(y1 - y0) / (y2 - y0)) * (z2 - z0);
        FillBottomFlatTriangleDepth(buffer, x0, y0, z0, x1, y1, z1, x3, y3, z3, color);
        FillTopFlatTriangleDepth(buffer, x1, y1, z1, x3, y3, z3, x2, y2, z2, color);
    }
}
