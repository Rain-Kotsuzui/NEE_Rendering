#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>

#include "ray.hpp"
#include "hit.hpp"
#include <iostream>

class Material
{
public:
    explicit Material(const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO, float s = 0, bool isReflective = true) : diffuseColor(d_color), specularColor(s_color), shininess(s), isReflective(isReflective)
    {
    }

    virtual ~Material() = default;

    virtual Vector3f getDiffuseColor() const
    {
        return diffuseColor;
    }

    Vector3f Shade(const Ray &ray, const Hit &hit,
                   const Vector3f &dirToLight, const Vector3f &lightColor)
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

    bool isReflective; // 是否反射

protected:
    Vector3f diffuseColor;
    Vector3f specularColor;
    float shininess;
};

#endif // MATERIAL_H
