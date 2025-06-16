#ifndef RAY_H
#define RAY_H

#include <cassert>
#include <iostream>
#include <Vector3f.h>
#include "parameter.hpp"
#include "hit.hpp"

// Ray class mostly copied from Peter Shirley and Keith Morley
class Ray
{
public:
    Ray() = delete;
    Ray(const Vector3f &orig, const Vector3f &dir, double ks = 1)
    {
        origin = orig;
        direction = dir;
        Ks = ks;
        length = 0;
    }

    Ray(const Ray &r)
    {
        origin = r.origin;
        direction = r.direction;
        Ks = r.Ks;
        length = r.length;
    }

    const void reflect(const Hit &hit)
    {
        length += hit.getT();
        origin = pointAtParameter(hit.getT());
        direction = direction - 2 * (hit.getNormal() * direction) * hit.getNormal();
        Ks *= DEFAULT_KS;
    }

    const void miss()
    {
        length += INFINITY_LENGTH;
        Ks *= DEFAULT_KS;
    }

    const Vector3f &getOrigin() const
    {
        return origin;
    }

    const Vector3f &getDirection() const
    {
        return direction;
    }

    const float &getKs() const
    {
        return Ks;
    }
    const float &getLength() const
    {
        return length;
    }

    void setOrigin(const Vector3f &orig)
    {
        origin = orig;
    }
    void setDirection(const Vector3f &dir)
    {
        direction = dir.normalized();
    }
    void setKs(const float &ks)
    {
        assert(ks >= 0);
        Ks = ks;
    }
    void setLength(const float &len)
    {
        assert(len >= 0);
        length = len;
    }

    Vector3f pointAtParameter(float t) const
    {
        return origin + direction * t;
    }

private:
    Vector3f origin;
    Vector3f direction;
    float Ks;     // 衰减系数
    float length; // 长度
};

inline std::ostream &operator<<(std::ostream &os, const Ray &r)
{
    os << "Ray <" << r.getOrigin() << ", " << r.getDirection() << ">";
    return os;
}

#endif // RAY_H
