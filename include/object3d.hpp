#ifndef OBJECT3D_H
#define OBJECT3D_H

#include "hit.hpp"
#include "ray.hpp"
#include "material.hpp"

// Base class for all 3d entities.
class Object3D
{
public:
    Object3D() : material(nullptr) {}

    virtual ~Object3D() = default;

    explicit Object3D(Material *material)
    {
        this->material = material;
    }
    void setMaterial(Material *m) {
        material = m;
    }
    Material *getMaterial() const {
        return material;
    }
    // Intersect Ray with this object. If hit, store information in hit structure.
    virtual bool intersect(const Ray &r, Hit &h, float tmin) = 0;
    virtual void printInfo() = 0;
    virtual Object3D *getObj()const =0;
protected:
    Material *material;
};

#endif
