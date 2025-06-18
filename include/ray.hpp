#ifndef RAY_H
#define RAY_H

#include <cassert>
#include <iostream>
#include <Vector3f.h>
#include <math.h>

#include "parameter.hpp"
#include "hit.hpp"
#include "material.hpp"
// Ray class mostly copied from Peter Shirley and Keith Morley
class Ray
{
public:
    Ray() = delete;
    Ray(const Vector3f &orig, const Vector3f &dir, double ks = 1, double kr = 1)
    {
        origin = orig;
        direction = dir;
        Ks = ks;
        Kr = kr;
        length = 0;
        material = new Material(); // 默认折射率为真空
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
    ~Ray() {}

    const void reflect(const Hit &hit)
    {

        length += hit.getT();
        origin = pointAtParameter(hit.getT());
        direction = direction - 2 * (hit.getNormal() * direction) * hit.getNormal();
        Kr *= hit.getMaterial()->getKr(); // 更新反射系数
    }
    const void refract(const Hit &hit)
    {

        origin = pointAtParameter(hit.getT());

        float n1;
        float n2;
        float ks;

        if (material == nullptr) // 如果没有材质信息，则默认为真空
            n1 = 1.0f;           // 真空的折射率为1,衰减为0
        else
            n1 = material->getRefractiveIndex(); // 当前材质的折射率

        if (hit.getMaterial() == nullptr) // 如果没有材质信息，则默认为真空
            n2 = 1.0f, ks = 1;            // 真空的折射率为1,衰减为0
        else
            n2 = hit.getMaterial()->getRefractiveIndex(), ks = hit.getMaterial()->getKs(); // 目标材质的折射率

        Vector3f n = hit.getNormal();
        float cosTheta1 = -Vector3f::dot(direction, n);
        if (n1 == n2 || cosTheta1 == 0) // 如果折射率相同或光线垂直于法线，则不折射
        {
            Ks *= ks; // 更新折射系数
            return;
        }
        if (cosTheta1 < 0) // 如果光线离开介质（向外为正法向）
        {
            cosTheta1 = -cosTheta1; // 法线方向相反
            n = -n;
        }
        if (cosTheta1 * cosTheta1 < 1 - (n2 / n1) * (n2 / n1)) // 大于临界角，反射
        {
            reflect(hit);
            return;
        }

        length += hit.getT();
        float cosTheata2 = sqrt(1 - (n1 / n2) * (n1 / n2) * (1 - cosTheta1 * cosTheta1));
        Vector3f refractedDir = (n1 / n2) * direction + (n1 / n2 * cosTheta1 - cosTheata2) * n;
        direction = refractedDir.normalized();
        material = hit.getMaterial(); // 更新材质信息
        Ks *= ks;                     // 更新折射系数
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
    float Ks;           // 反射衰减系数
    float Kr;           // 折射衰减系数
    float length;       // 长度
};

inline std::ostream &operator<<(std::ostream &os, const Ray &r)
{
    os << "Ray <" << r.getOrigin() << ", " << r.getDirection() << ">";
    return os;
}

#endif // RAY_H
