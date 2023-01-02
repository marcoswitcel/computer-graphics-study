#ifndef GEOMETRY_CPP
#define GEOMETRY_CPP

typedef struct Vec3f {
    float x, y, z;

    inline Vec3f operator -(const Vec3f &rhs)
    const {
        return Vec3f { .x = x - rhs.x, .y = y - rhs.y, .z = z - rhs.z, };
    }
    inline Vec3f operator ^(const Vec3f &rhs)
    const {
        return Vec3f { 
            .x = y*rhs.z-z*rhs.y,
            .y = z*rhs.x-x*rhs.z,
            .z = x*rhs.y-y*rhs.x,
         };
    }

    inline Vec3f operator *(float rhs)
    const {
        return Vec3f { .x = x * rhs, .y = y * rhs, .z = z * rhs, };
    }

    inline float operator *(const Vec3f &rhs)
    const {
        return x*rhs.x + y*rhs.y + z*rhs.z;
    }

    float norm() const { return std::sqrt(x*x + y*y + z*z);  }

    Vec3f& normalize(float length = 1.0)
    {
        *this = (*this) * (length/norm());
        return *this;
    }
} Vec3f;

typedef struct Vec2i {
    int x, y;
} Vec2i;

#endif // GEOMETRY_CPP
