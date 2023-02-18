#ifndef MATH_H
#define MATH_H

#define V2_EPSILON 0.000001f
#define V3_EPSILON 0.000001f
#define V4_EPSILON 0.000001f
#define MAT4_EPSILON 0.000001f
#define QUAT_EPSILON 0.000001f
#define DEG2RAD 0.0174533f
#define PI 3.14159265359f

inline f32
dot_product(const v2 &l, const v2 &r)
{
    return (l.x * r.x) + (l.y * r.y);
}

inline f32
dot_product(const v3 &l, const v3 &r)
{
    return (l.x * r.x) + (l.y * r.y) + (l.z * r.z);
}

inline v3
cross_product(const v3 &l, const v3 &r)
{
    return 
    {
        (l.y * r.z - l.z * r.y),
        (l.z * r.x - l.x * r.z),
        (l.x * r.y - l.y * r.x)
    };
}

inline v3
operator+(const v3 &l, const v3 &r)
{
    return {l.x + r.x, l.y + r.y, l.z + r.z};
}

inline void
operator+=(v3 &l, const v3 &r)
{
    l.x = l.x + r.x;
    l.y = l.y + r.y;
    l.z = l.z + r.z;
}

inline v3
operator-(const v3 &l, const v3 &r)
{
    return {l.x - r.x, l.y - r.y, l.z - r.z};
}

inline void
operator-=(v3 &l, const v3 &r)
{
    l.x = l.x - r.x;
    l.y = l.y - r.y;
    l.z = l.z - r.z;
}

inline v3
operator*(const v3 &l, const v3 &r)
{
    return {l.x * r.x, l.y * r.y, l.z * r.z};
}

inline v3
operator*(const v3 &v, float f)
{
    return {v.x * f, v.y * f, v.z * f};
}

inline void
operator*=(v3 &l, v3 &r)
{
    l.x *= r.x;
    l.y *= r.y;
    l.z *= r.z;
}

v3 operator*(const quat& q, const v3& v)
{
    return q.vector * 2.0f * dot_product(q.vector, v) + 
        v * (q.scalar * q.scalar - dot_product(q.vector, q.vector)) +
        cross_product(q.vector, v) * 2.0f * q.scalar;
}

inline v4
operator*(const v4 &l, const v4 &r)
{
    return { l.x * r.x, l.y * r.y, l.z * r.z, l.w * r.w };
}

inline quat
quat_multiply(const quat &l, const quat &r)
{
    return {
        r.x * l.w + r.y * l.z - r.z * l.y + r.w * l.x,
        -r.x * l.z + r.y * l.w + r.z * l.x + r.w * l.y,
        r.x * l.y - r.y * l.x + r.z * l.w + r.w * l.z,
        -r.x * l.x - r.y * l.y - r.z * l.z + r.w * l.w
    };
}

inline bool
operator==(const v3 &l, const v3 &r)
{
    if (l.x == r.x, l.y == r.y, l.z == r.z)
        return true;
    else
        return false;
}

inline bool
operator==(const v3 &v, float f)
{
    if (v.x == f, v.y == f, v.z == f)
        return true;
    else
        return false;
}

inline bool
operator==(const v4 &l, const v4 &r)
{
    if (l.x == r.x, l.y == r.y, l.z == r.z, l.w == r.w)
        return true;
    else
        return false;
}

inline real32
length_squared(const v2 &v)
{
    return (v.x * v.x) + (v.y * v.y);
}

inline real32
length_squared(const v3 &v)
{
    return (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
}

inline real32
length_squared(const v4 &v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

inline void
normalize(v3 &v)
{
    real32 len_sq = length_squared(v);
    if (len_sq < V3_EPSILON)
        return;
    else
    {
        real32 inverse_length = 1.0f / sqrtf(len_sq);
        v.x *= inverse_length;
        v.y *= inverse_length;
        v.z *= inverse_length;
    }
}

inline v2
normalized(const v2 &v)
{
    real32 len_sq = length_squared(v);
    if (len_sq < V2_EPSILON)
        return v;
    else
    {
        real32 inverse_length = 1.0f / sqrtf(len_sq);
        return {v.x * inverse_length, v.y * inverse_length };
    }
}

inline v3
normalized(const v3 &v)
{
    real32 len_sq = length_squared(v);
    if (len_sq < V3_EPSILON)
        return v;
    else
    {
        real32 inverse_length = 1.0f / sqrtf(len_sq);
        return {v.x * inverse_length, v.y * inverse_length, v.z * inverse_length};
    }
}

inline v4
normalized(const v4 &v)
{
    real32 len_sq = length_squared(v);
    if (len_sq < V4_EPSILON)
        return { 0, 0, 0, 1 };
    else
    {
        real32 inverse_length = 1.0f / sqrtf(len_sq);
        return {v.x * inverse_length, v.y * inverse_length, v.z * inverse_length, v.w * inverse_length};
    }
}

inline m4x4
get_frustum(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f)
{
    if (l == r || t == b || n == f)
    {
        error("Invalid frustum");
        return {};
    }
    
    return
    {
        (2.0f * n) / (r - l), 0, 0, 0,
        0, (2.0f * n) / (t - b), 0, 0,
        (r + l) / (r - l), (t + b) / (t - b), (-(f + n)) / (f - n), -1,
        0, 0, (-2 * f * n) / (f - n), 0
    };
}

inline m4x4
perspective_projection(real32 fov, real32 aspect_ratio, real32 n, real32 f)
{
    real32 y_max = n * tanf(fov * PI / 360.0f);
    real32 x_max = y_max * aspect_ratio;
    return get_frustum(-x_max, x_max, -y_max, y_max, n, f);
}

inline m4x4
orthographic_projection(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f)
{
    if (l == r || t == b || n == f)
    {
        error("orthographic_projection() Invalid arguments");
        return {};
    }
    return
    {
        2.0f / (r - l), 0, 0, 0,
        0, 2.0f / (t - b), 0, 0,
        0, 0, -2.0f / (f - n), 0,
        -((r+l)/(r-l)),-((t+b)/(t-b)),-((f+n)/(f-n)), 1
    };
}

inline m4x4
identity_m4x4()
{
    return
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

inline m4x4
look_at(const v3 &position, const v3 &target, const v3 &up)
{
    v3 f = normalized(target - position) * -1.0f;
    v3 r = cross_product(up, f);
    if (r == 0)
        return identity_m4x4();
    normalize(r);
    v3 u = normalized(cross_product(f, r));
    v3 t = {-dot_product(r, position), -dot_product(u, position), -dot_product(f, position)};
    
    return
    {
        r.x, u.x, f.x, 0,
        r.y, u.y, f.y, 0,
        r.z, u.z, f.z, 0,
        t.x, t.y, t.z, 1
    };
}

quat get_rotation(f32 angle, const v3& axis)
{
    v3 norm = normalized(axis);
    f32 s = sinf(angle * 0.5f);
    return { norm.x * s, norm.y * s, norm.z * s, cosf(angle * 0.5f) };
}

inline m4x4 
create_transform_m4x4(v3 position, quat rotation, v3 scale)
{
    v3 x = {1, 0, 0};
    v3 y = {0, 1, 0};
    v3 z = {0, 0, 1};
    
    x = rotation * x;
    y = rotation * y;
    z = rotation * z;
    
    x = x * scale.x;
    y = y * scale.y;
    z = z * scale.z;
    
    return
    {
        x.x, x.y, x.z, 0,
        y.x, y.y, y.z, 0,
        z.x, z.y, z.z, 0,
        position.x, position.y, position.z, 1
    };
}

inline void
print_m4x4(m4x4 matrix)
{
    for (int i = 0; i < 16; i++)
    {
        s32 row = i / 4;
        s32 column = i - (row * 4);
        printf("%f ", matrix.E[row][column]);
        if ((i + 1) % 4 == 0)
            printf("\n");
    }
}

// Returns a quat which contains the rotation between two vectors.
// The two vectors are treated like they are points in the same sphere.
v4 from_to(const v3& from, const v3& to)
{
    v3 f = normalized(from);
    v3 t = normalized(to);
    if (f == t)
    {
        return { 0, 0, 0, 1 };
    }
    else if (f == t * -1.0f)
    {
        v3 ortho = { 1, 0, 0 };
        if (fabsf(f.y) < fabsf(f.x))
            ortho = { 0, 1, 0 };
        if (fabsf(f.z) < fabs(f.y) && fabs(f.z) < fabsf(f.x))
            ortho = { 0, 0, 1 };
        v3 axis = normalized(cross_product(f, ortho));
        return { axis.x, axis.y, axis.z, 0.0f };
    }
    v3 half = normalized(f + t);
    v3 axis = cross_product(f, half);
    return { axis.x, axis.y, axis.z, dot_product(f, half) };
}

v4 get_rotation_to_direction(const v3& direction, const v3& up)
{
    // Find orthonormal basis vectors
    v3 forward = normalized(direction);
    v3 norm_up = normalized(up);
    v3 right = cross_product(norm_up, forward);
    norm_up = cross_product(forward, right);
    
    v4 world_to_object = from_to({ 0, 0, 1 }, forward); // From world forward to object forward
    v3 object_up = { 0, 1, 0 };
    object_up = world_to_object * object_up; // What direction is the new object up?
    v4 u_to_u = from_to(object_up, norm_up); // From object up to desired up
    v4 result = quat_multiply(world_to_object, u_to_u); // Rotate to forward direction then twist to correct up
    
    return normalized(result);
}

#endif //MATH_H
