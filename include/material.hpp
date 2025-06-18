#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>
#include <iostream>
#include "parameter.hpp"
class Ray;
class Hit;

class Material
{
public:
    explicit Material(const Vector3f &d_color = Vector3f::ZERO, const Vector3f &s_color = Vector3f::ZERO, float s = 0, bool isReflective = false, bool isRefractive = true, float refractiveIndex = 1.0f, float Kr = DEFAULT_KR, float Ks = DEFAULT_KS)
        : // 初始化材质属性
          diffuseColor(d_color), specularColor(s_color), shininess(s), isReflective(isReflective), isRefractive(isRefractive), refractiveIndex(refractiveIndex), Kr(Kr), Ks(Ks)
    {
    }

    virtual ~Material() = default;

    virtual Vector3f getDiffuseColor() const
    {
        return diffuseColor;
    }

    Vector3f Shade(const Vector3f &ray, const Vector3f &n, const Vector3f &dirToLight, const Vector3f &lightColor)
    {
        Vector3f shaded = Vector3f::ZERO;
        //
        auto clamp = [](float x)
        { return x > 0 ? x : 0; };
        Vector3f R = 2 * Vector3f::dot(n, dirToLight) * n - dirToLight;
        shaded = diffuseColor * clamp(Vector3f::dot(n, dirToLight)) + specularColor * pow(clamp(Vector3f::dot(-ray, R)), shininess);
        shaded = shaded * lightColor;
        return shaded;
    }

    const float getRefractiveIndex() const
    {
        return refractiveIndex;
    }
    const float getKr() const
    {
        return Kr;
    }
    const float getKs() const
    {
        return Ks;
    }

    bool isReflective;           // 是否反射
    bool isRefractive;           // 是否折射
    float reflectiveCoefficient; // 反射占比

protected:
    Vector3f diffuseColor;
    Vector3f specularColor;
    float shininess;
    float refractiveIndex; // 折射率
    float Kr;              // 反射衰减
    float Ks;              // 折射衰减
};
static Material *VOID = new Material(Vector3f::ZERO, Vector3f::ZERO, 0, false, true, 1.0f, 1, 1); // 空材质
#endif // MATERIAL_H
