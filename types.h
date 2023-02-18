#ifndef TYPES_H
#define TYPES_H

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef intptr_t intptr;
typedef uintptr_t uintptr;

typedef size_t memory_index;

typedef float real32;
typedef double real64;

typedef int8 s8;
typedef int8 s08;
typedef int8 b8;

typedef int16 s16;
typedef int32 s32;
typedef int64 s64;
typedef bool32 b32;

typedef uint8 u8;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef real32 r32;
typedef real64 r64;
typedef real32 f32;
typedef real64 f64;

#define function static
#define local_persist static
#define global_variable static
#define array_count(n) (sizeof(n) / sizeof(n[0]))

union v2
{
    struct
    {
        real32 x, y;
    };
    struct
    {
        real32 u, v;
    };
    struct
    {
        real32 Width, Height;
    };
    real32 E[2];
};

union v2s
{
    struct
    {
        s32 x, y;
    };
    struct
    {
        s32 u, v;
    };
    struct
    {
        s32 Width, Height;
    };
    s32 E[2];
};

union v3
{
    struct
    {
        real32 x, y, z;
    };
    struct
    {
        real32 r, g, b;
    };
    f32 E[3];
};

union v4
{
    struct
    {
        real32 x, y, z, w;
    };
    struct
    {
        v3 vector;
        f32 scalar;
    };
    f32 E[4];
};
typedef v4 quat;

struct m4x4
{
    f32 E[4][4]; // E[ROW][COLUMN]
};

#endif //TYPES_H