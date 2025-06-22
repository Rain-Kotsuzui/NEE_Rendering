#ifndef LIGHT_H
#define LIGHT_H

#include <Vector3f.h>
#include "object3d.hpp"
#include "mesh.hpp"
#include "material.hpp"
#include <random>
class Light
{
public:
    Light() = default;

    virtual ~Light() = default;

    virtual void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const = 0;
    virtual const Vector3f getDirection(const Vector3f &p) const = 0;
    virtual const bool isShadowed(const float &T, const Vector3f &p) const = 0;
    virtual const int typeLight() const = 0;// 0 点光源，1面光源，2方向光

    inline virtual bool intersect(const Ray &r, Hit &h, float tmin) const = 0;
    virtual const Vector3f getColor() const = 0;
    virtual const Material *getMaterial() const = 0;
    virtual const Vector3f getNormal() const = 0;
    virtual const float getArea() const = 0;
    virtual const Vector3f getSample() const = 0;
};

class AreaLight : public Light
{
public:
    AreaLight() = delete;
    AreaLight(Mesh *me, Material *m, Vector3f col = Vector3f(1, 1, 1), Vector3f nor = Vector3f(0, 0, 0), float a = 1)
    {
        mesh = me;
        material = m;
        position = mesh->getCenter(); // 质心
        color = col;
        normal = nor;
        area = a;
    }

    ~AreaLight() override = default;
    const bool isShadowed(const float &T, const Vector3f &p) const override
    {
        return false;
    }
    const Vector3f getColor() const override { return color; }
    const Vector3f getDirection(const Vector3f &p) const override
    {
        // Area light does not have a specific direction, return a random point on the mesh
        Vector3f randomPoint = getSample();
        return (randomPoint - p).normalized();
    }
    const Vector3f getSample() const override
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        Vector3f result = Vector3f::ZERO;
        double weighth;
        double sum = 0;
        for (int i = 0; i < mesh->v.size(); i++)
        {
            weighth = dis(gen);
            sum += weighth;
            result += mesh->v[i] * weighth;
        }
        assert(mesh->v.size());
        result = result / sum;
        return result;
    }
    const float getArea() const override { return area; }
    const Material *getMaterial() const override { return material; }
    const Vector3f getNormal() const override { return normal; }
    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override
    { // 路径追踪中不会用到
    }
    bool intersect(const Ray &r, Hit &h, float tmin) const override
    {
        Hit hit;
        bool result = mesh->intersect(r, hit, tmin);
        if (result)
            h.set(hit.getT(), material, hit.getNormal());
        return result;
    }
    const int typeLight() const override { return 1; }

private:
    Mesh *mesh;
    Material *material;
    Vector3f position;
    Vector3f color;
    Vector3f normal;
    float area;
};


class DirectionalLight : public Light
{
public:
    DirectionalLight() = delete;

    DirectionalLight(const Vector3f &d, const Vector3f &c,Material *m)
    {
        direction = d.normalized();
        color = c;
        material = m;
    }

    ~DirectionalLight() override = default;

    const int typeLight() const override { return 2; }
    ///@param p unsed in this function
    ///@param distanceToLight not well defined because it's not a point light
    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override
    {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = -direction;
        col = color;
    }

    const Vector3f getDirection(const Vector3f &p) const override
    {
        return -direction.normalized();
    }
    const bool isShadowed(const float &T, const Vector3f &p) const override
    {
        // 光线在无穷远处
        return true;
    }

    bool intersect(const Ray &r, Hit &h, float tmin) const override {};
    const Vector3f getColor() const override {return color;};
    const Material *getMaterial() const override {return material;};
    const Vector3f getNormal() const override {};
    const Vector3f getSample() const override {};
    const float getArea() const override {}

private:
    Vector3f direction;
    Vector3f color;
    Material *material;
};

class PointLight : public Light
{
public:
    PointLight() = delete;

    PointLight(const Vector3f &p, const Vector3f &c)
    {
        position = p;
        color = c;
    }

    ~PointLight() override = default;

    const int typeLight() const override { return 0; }

    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override
    {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = (position - p);
        dir = dir / dir.length();
        col = color;
    }

    const Vector3f getDirection(const Vector3f &p) const override
    {
        return (position - p).normalized();
    }
    const bool isShadowed(const float &T, const Vector3f &p) const override
    {
        return (T < (position - p).length());
    }

    bool intersect(const Ray &r, Hit &h, float tmin) const override {};
    const Vector3f getColor() const override {};
    const Material *getMaterial() const override {};
    const Vector3f getNormal() const override {};
    const Vector3f getSample() const override {};
    const float getArea() const override {}

private:
    Vector3f position;
    Vector3f color;
};

//======================体积光=========================//

class VolPointLight : public Light
{
public:
    VolPointLight() = delete;
    VolPointLight(Mesh *me, Material *m, Vector3f col = Vector3f(1, 1, 1), Vector3f p=Vector3f::ZERO)
    {
        mesh = me;
        material = m;
        position = mesh->getCenter(); // 质心
        color = col;
        position =p;
    }

    ~VolPointLight() override = default;
    const bool isShadowed(const float &T, const Vector3f &p) const override
    {
        return false;
    }
    const Vector3f getColor() const override { return color; }
    const Vector3f getDirection(const Vector3f &p) const override
    {
        //线性光
        return (position-p).normalized();
    }
    const Vector3f getSample() const override
    {
        return position;
    }
    const float getArea() const override { }
    const Material *getMaterial() const override { return material; }
    const Vector3f getNormal() const override { }
    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override
    { // 不会用到
    }
    bool intersect(const Ray &r, Hit &h, float tmin) const override
    {   //非锥状光
        h.set((r.getOrigin()-position).length(), material, position);
        return true;
    }
    const int typeLight() const override { return 4; }

private:
    Mesh *mesh;
    Material *material;
    Vector3f position;
    Vector3f color;
};

class VolAreaLight : public Light
{
public:
    VolAreaLight() = delete;
    VolAreaLight(Mesh *me, Material *m, Vector3f col = Vector3f(1, 1, 1), Vector3f nor = Vector3f(0, 0, 0), float a = 1)
    {
        mesh = me;
        material = m;
        position = mesh->getCenter(); // 质心
        color = col;
        normal = nor;
        area = a;
    }

    ~VolAreaLight() override = default;
    const bool isShadowed(const float &T, const Vector3f &p) const override
    {
        return false;
    }
    const Vector3f getColor() const override { return color; }
    const Vector3f getDirection(const Vector3f &p) const override
    {
        //线性光
        return -normal;
    }
    const Vector3f getSample() const override
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        Vector3f result = Vector3f::ZERO;
        double weighth;
        double sum = 0;
        for (int i = 0; i < mesh->v.size(); i++)
        {
            weighth = dis(gen);
            sum += weighth;
            result += mesh->v[i] * weighth;
        }
        assert(mesh->v.size());
        result = result / sum;
        return result;
    }
    const float getArea() const override { return area; }
    const Material *getMaterial() const override { return material; }
    const Vector3f getNormal() const override { return normal; }
    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override
    { // 中不会用到
    }
    bool intersect(const Ray &r, Hit &h, float tmin) const override
    {
        Hit hit;
        bool result = mesh->intersect(r, hit, tmin);
        if (result)
            h.set(hit.getT(), material, hit.getNormal());
        return result;
    }
    const int typeLight() const override { return 4; }

private:
    Mesh *mesh;
    Material *material;
    Vector3f position;
    Vector3f color;
    Vector3f normal;
    float area;
};


#endif // LIGHT_H
