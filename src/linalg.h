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
        {
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

static M4 
MatrixLookAt(V3 eye, V3 center, V3 up)
{
    V3 f = V3Normalize(V3Subtract(center, eye));
    V3 s = V3Normalize(V3Cross(f, up));
    V3 u = V3Cross(s, f);

    M4 result = {0};
    result.m[0][0] = s.x;
    result.m[0][1] = u.x;
    result.m[0][2] = -f.x;
    result.m[0][3] = 0.0f;

    result.m[1][0] = s.y;
    result.m[1][1] = u.y;
    result.m[1][2] = -f.y;
    result.m[1][3] = 0.0f;

    result.m[2][0] = s.z;
    result.m[2][1] = u.z;
    result.m[2][2] = -f.z;
    result.m[2][3] = 0.0f;

    result.m[3][0] = -V3Dot(s, eye);
    result.m[3][1] = -V3Dot(u, eye);
    result.m[3][2] = V3Dot(f, eye);
    result.m[3][3] = 1.0f;

    return result;
}

static inline M4 
MatMul(M4 a, M4 b)
{
    M4 result;
    for(I32 j = 0; j < 4; j++)
    for(I32 i = 0; i < 4; i++)
    {
        result.m[i][j]  = a.m[i][0]*b.m[0][j] + a.m[i][1]*b.m[1][j] 
                        + a.m[i][2]*b.m[2][j] + a.m[i][3]*b.m[3][j];
    }
    return result;
}

static V3 
TransformPoint(const V3* point, const M4* matrix) 
{
    V3 result;
    result.x = point->x * matrix->m[0][0] + point->y * matrix->m[0][1] + point->z * matrix->m[0][2] + matrix->m[0][3];
    result.y = point->x * matrix->m[1][0] + point->y * matrix->m[1][1] + point->z * matrix->m[1][2] + matrix->m[1][3];
    result.z = point->x * matrix->m[2][0] + point->y * matrix->m[2][1] + point->z * matrix->m[2][2] + matrix->m[2][3];
    return result;
}
