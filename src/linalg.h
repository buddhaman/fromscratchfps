#include "util.h"

typedef union V3 V3;
union V3
{
    struct 
    {
        R32 x;
        R32 y;
        R32 z;
    };
};

// Matrices are column major like OpenGL!!!!
typedef union M4 M4;
union M4
{
    struct 
    {
        R32 m[4][4];
    };
    
    struct 
    {
        R32 Elements[16];
    };
};

static M4 MatrixIdentity() 
{
    M4 matrix = 
    {
        .m = {
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}
        }
    };
    return matrix;
}

static M4 MatrixPerspective(float fov, float aspectRatio, float near, float far) 
{
    float tanHalfFov = tanf(fov / 2.0f);
    M4 matrix;

    matrix.m[0][0] = 1 / (aspectRatio * tanHalfFov);
    matrix.m[0][1] = 0;
    matrix.m[0][2] = 0;
    matrix.m[0][3] = 0;

    matrix.m[1][0] = 0;
    matrix.m[1][1] = 1 / tanHalfFov;
    matrix.m[1][2] = 0;
    matrix.m[1][3] = 0;

    matrix.m[2][0] = 0;
    matrix.m[2][1] = 0;
    matrix.m[2][2] = -(far + near) / (far - near);
    matrix.m[2][3] = -(2 * far * near) / (far - near);

    matrix.m[3][0] = 0;
    matrix.m[3][1] = 0;
    matrix.m[3][2] = -1;
    matrix.m[3][3] = 0;

    return matrix;
}
