#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>

#include "hit.hpp"
#include "ray.hpp"
#include <iostream>

class Material
{
public:
    explicit Material(const Vector3f &d_color = Vector3f::ZERO, const Vector3f &s_color = Vector3f::ZERO, float s = 0, bool isReflective = true, bool isRefractive = false, float refractiveIndex = 1.0f)
        : // 初始化材质属性
          diffuseColor(d_color), specularColor(s_color), shininess(s), isReflective(isReflective), isRefractive(isRefractive), refractiveIndex(refractiveIndex)
    {
    }

    virtual ~Material() = default;

    virtual Vector3f getDiffuseColor() const
    {
        return diffuseColor;
    }

    Vector3f Shade(const Ray &ray, const Hit &hit, const Vector3f &dirToLight, const Vector3f &lightColor)
    {
        Vector3f shaded = Vector3f::ZERO;
        //
        auto clamp = [](float x)
        { return x > 0 ? x : 0; };
        Vector3f R = 2 * Vector3f::dot(hit.getNormal(), dirToLight) * hit.getNormal() - dirToLight;
        shaded = diffuseColor * clamp(Vector3f::dot(hit.getNormal(), dirToLight)) + specularColor * pow(clamp(Vector3f::dot(-ray.getDirection(), R)), shininess);
        shaded = shaded * lightColor;
        return shaded;
    }

    const float getRefractiveIndex() const
    {
        return refractiveIndex;
    }

    bool isReflective;           // 是否反射
    bool isRefractive;           // 是否折射
    float reflectiveCoefficient; // 反射占比

protected:
    Vector3f diffuseColor;
    Vector3f specularColor;
    float shininess;
    float refractiveIndex; // 折射率
};

#endif // MATERIAL_H
