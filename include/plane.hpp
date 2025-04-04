#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// function: ax+by+cz=d
// choose your representation , add more fields and fill in the functions

class Plane : public Object3D
{
public:
    Plane()
    {
        throw "Plane not defined!";
    }

    Plane(const Vector3f &normal, float d, Material *m) : Object3D(m), normal(normal), d(d)
    {
    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override
    {
        float t1=Vector3f::dot(r.getDirection(), normal);
        if (abs(t1) < 1e-6)
            return false;
        float t = (d-Vector3f::dot(normal,r.getOrigin()))/t1;
        if (t > tmin && t < h.getT())
        {
            h.set(t, material, normal);
            return true;
        }
        else
            return false;
    }
    void printInfo() override {}

protected:
    Vector3f normal;
    float d;
};

#endif // PLANE_H
