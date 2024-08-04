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

static inline V3 
Vec3(R32 x, R32 y, R32 z)
{
    V3 v = {x, y, z};
    return v;
}

static inline V3 
V3Subtract(V3 a, V3 b)
{
    V3 result = { a.x - b.x, a.y - b.y, a.z - b.z };
    return result;
}

static inline V3 
V3Normalize(V3 v)
{
    R32 length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    V3 result = { v.x / length, v.y / length, v.z / length };
    return result;
}

static inline V3 
V3Cross(V3 a, V3 b)
{
    V3 result = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
    return result;
}

static inline R32 
V3Dot(V3 a, V3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Matrices are column major like OpenGL!!!!
typedef union M4 M4;
union M4
{
    R32 m[4][4];
    R32 elements[16];
};

// Matrix4

static inline M4 
MatMul(M4 a, M4 b)
{
    M4 result = {0};
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            result.m[i][j] = a.m[i][0] * b.m[0][j] + 
                             a.m[i][1] * b.m[1][j] + 
                             a.m[i][2] * b.m[2][j] + 
                             a.m[i][3] * b.m[3][j];
        }
    }
    return result;
}

static inline M4 
MatrixIdentity() 
{
    M4 matrix = 
        {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
    return matrix;
}

static inline M4 
MatrixPerspective(R32 fov, R32 width, R32 height, R32 near, R32 far) 
{
    R32 aspectRatio = width / height;
    R32 tan_half_fov = tanf(fov / 2.0f);
    M4 matrix;

    // Perspective projection matrix
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

static inline V3 
TransformToScreenSpace(V3 v, R32 width, R32 height) {
    V3 result;
    result.x = (v.x * width / 2.0f) + width / 2.0f;
    result.y = (v.y * height / 2.0f) + height / 2.0f;
    result.z = v.z;
    return result;
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

static V3 
TransformV3(M4* transform, V3 v) 
{
    V3 result;
    float w = v.x * transform->m[3][0] + v.y * transform->m[3][1] + v.z * transform->m[3][2] + transform->m[3][3];
    result.x = (v.x * transform->m[0][0] + v.y * transform->m[0][1] + v.z * transform->m[0][2] + transform->m[0][3]);
    result.y = (v.x * transform->m[1][0] + v.y * transform->m[1][1] + v.z * transform->m[1][2] + transform->m[1][3]);
    result.z = (v.x * transform->m[2][0] + v.y * transform->m[2][1] + v.z * transform->m[2][2] + transform->m[2][3]);

    if( w == 0.0f || w == 1.0f)
        return result;

    result.x /= w;
    result.y /= w;
    result.z /= w;
    
    return result;
}

