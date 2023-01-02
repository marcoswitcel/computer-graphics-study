#ifndef GEOMETRY_CPP
#define GEOMETRY_CPP

typedef struct Vec3f {
    float x, y, z;

    inline Vec3f operator -(const Vec3f &rhs)
    {
        return Vec3f { .x = x - rhs.x, .y = y - rhs.y, .z = z - rhs.z, };
    }
    inline Vec3f operator ^(const Vec3f &rhs)
    {
        return Vec3f { 
            .x = y*rhs.z-z*rhs.y,
            .y = z*rhs.x-x*rhs.z,
            .z = x*rhs.y-y*rhs.x,
         };
    }
} Vec3f;

typedef struct Vec2i {
    int x, y;
} Vec2i;

#endif // GEOMETRY_CPP
