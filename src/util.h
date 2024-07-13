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
