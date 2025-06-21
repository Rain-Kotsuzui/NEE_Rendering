#ifndef GROUP_H
#define GROUP_H

#include "object3d.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include <iostream>
#include <vector>

class Group : public Object3D
{

public:
    Group()
    {
    }

    explicit Group(int num_objects)
    {
        objectList.resize(num_objects);
    }

    ~Group() override
    {
    }

    bool intersect(const Ray &r, Hit &h, float tmin) override
    {
        bool result = false;
        Material *hitMaterial = h.getMaterial();
        for (auto obj : objectList)
            result |= obj->intersect(r, h, tmin);
        if (result && h.getMaterial()->isRefractive && hitMaterial == h.getMaterial())
            h.setMaterial(VOID); // 折射出射
        return result;
    }
    bool shadow_intersect(const Ray &r, Hit &h, float tmin)
    {
        bool result = false;
        Material *hitMaterial = h.getMaterial();
        for (auto obj : objectList)
        {
            Object3D *temp=obj;
            while(temp->getObj()!=nullptr)
                temp = temp->getObj();
            if( temp->getMaterial()->isRefractive) // 如果材质是折射材质，则不计算阴影
                continue;
            result |= obj->intersect(r, h, tmin);
        }
        return result;
    }

    void addObject(int index, Object3D *obj)
    {
        objectList[index] = obj;
    }

    int getGroupSize()
    {
        return objectList.size();
    }
    void printInfo() override
    {
        for (int i = 0; i < getGroupSize(); i++)
        {
            std::cout << "Object: " << i << " ";
            objectList[i]->printInfo();
        }
    }
    
    Object3D *getObj()const override{ }

private:
    std::vector<Object3D *> objectList;
};

#endif
