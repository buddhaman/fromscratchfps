#include <math.h>

#include "util.h"

// Vector3

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

static V3 V3Subtract(V3 a, V3 b)
{
    V3 result = { a.x - b.x, a.y - b.y, a.z - b.z };
    return result;
}

static V3 V3Normalize(V3 v)
{
    float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    V3 result = { v.x / length, v.y / length, v.z / length };
    return result;
}

static V3 V3Cross(V3 a, V3 b)
{
    V3 result = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
    return result;
}

static R32 V3Dot(V3 a, V3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

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
        R32 elements[16];
    };
};

// Matrix4

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
    float tan_half_fov = tanf(fov / 2.0f);
    M4 matrix;

    matrix.m[0][0] = 1 / (aspectRatio * tan_half_fov);
    matrix.m[0][1] = 0;
    matrix.m[0][2] = 0;
    matrix.m[0][3] = 0;

    matrix.m[1][0] = 0;
    matrix.m[1][1] = 1 / tan_half_fov;
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

V3 TransformPoint(const V3* point, const M4* matrix) 
{
    V3 result;
    result.x = point->x * matrix->m[0][0] + point->y * matrix->m[0][1] + point->z * matrix->m[0][2] + matrix->m[0][3];
    result.y = point->x * matrix->m[1][0] + point->y * matrix->m[1][1] + point->z * matrix->m[1][2] + matrix->m[1][3];
    result.z = point->x * matrix->m[2][0] + point->y * matrix->m[2][1] + point->z * matrix->m[2][2] + matrix->m[2][3];
    return result;
}
