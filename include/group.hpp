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
        for (auto obj : objectList)
            result |= obj->intersect(r, h, tmin);
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

private:
    std::vector<Object3D *> objectList;
};

#endif
