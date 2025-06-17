#ifndef RAY_H
#define RAY_H

#include <cassert>
#include <iostream>
#include <Vector3f.h>
#include <math.h>
#include "parameter.hpp"
#include "hit.hpp"
#include "material.hpp"

class Material; // 前向声明，避免循环依赖
// Ray class mostly copied from Peter Shirley and Keith Morley
class Ray
{
public:
    Ray() = delete;
    Ray(const Vector3f &orig, const Vector3f &dir, double ks = 1,double Kr=1)
    {
        origin = orig;
        direction = dir;
        Ks = ks;
        Kr=Kr;
        length = 0;
        material =new Material(); // 默认折射率为真空
    }

    Ray(const Ray &r)
    {
        origin = r.origin;
        direction = r.direction;
        Ks = r.Ks;
        Kr = r.Kr;
        length = r.length;
        material = r.material;
    }
    ~Ray()
    {
        delete material; // 删除材质信息
    }

    const void reflect(const Hit &hit)
    {
        length += hit.getT();
        origin = pointAtParameter(hit.getT());
        direction = direction - 2 * (hit.getNormal() * direction) * hit.getNormal();
        Ks *= DEFAULT_KS;
    }
    const void refract(const Hit &hit)
    {   
        length += hit.getT();
        origin = pointAtParameter(hit.getT());

        float n1= material->getRefractiveIndex(); // 当前材质的折射率
        float n2 = hit.getMaterial()->getRefractiveIndex(); // 目标材质的折射率
        float cosTheta1 = -Vector3f::dot(direction, hit.getNormal());
        float cosTheata2 = sqrt(1 - (n1 / n2) * (n1 / n2) * (1 - cosTheta1 * cosTheta1));
        Vector3f refractedDir = (n1 / n2) * direction + (n1 / n2 * cosTheta1 - cosTheata2) * hit.getNormal();
        direction = refractedDir.normalized();
        material = hit.getMaterial(); // 更新材质信息
        Kr *= DEFAULT_KR;
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
    const float &getKr() const
    {
        return Kr;
    }
    const float &getLength() const
    {
        return length;
    }

    const Material *getMaterial() const
    {
        return material;
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
    void setKr(const float &kr)
    {
        assert(kr >= 0);
        Kr = kr;
    }
    void setLength(const float &len)
    {
        assert(len >= 0);
        length = len;
    }

    void setMaterial(Material *mat)
    {
        assert(mat != nullptr);
        material = mat;
    }

    Vector3f pointAtParameter(float t) const
    {
        return origin + direction * t;
    }

private:
    Vector3f origin;
    Vector3f direction;
    Material *material; // 折射需要的光线所在介质材质信息
    float Ks;     // 反射衰减系数
    float Kr;     // 折射衰减系数
    float length; // 长度
};

inline std::ostream &operator<<(std::ostream &os, const Ray &r)
{
    os << "Ray <" << r.getOrigin() << ", " << r.getDirection() << ">";
    return os;
}

#endif // RAY_H
