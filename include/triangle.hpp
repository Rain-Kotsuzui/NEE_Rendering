#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>
using namespace std;

// TODO: implement this class and add more fields as necessary,
class Triangle : public Object3D
{

public:
	Triangle() = delete;

	// a b c are three vertex positions of the triangle
	Triangle(const Vector3f &a, const Vector3f &b, const Vector3f &c, Material *m) : Object3D(m), vertices{a, b, c}
	{
	}

	bool intersect(const Ray &ray, Hit &hit, float tmin) override
	{
		Vector3f E1 = vertices[0] - vertices[1], E2 = vertices[0] - vertices[2], S = vertices[0] - ray.getOrigin();
		float t1 = Matrix3f(ray.getDirection(), E1, E2, 1).determinant();
		if (abs(t1) < 1e-6)
			return false;
        float t=Matrix3f(S, E1, E2, 1).determinant()/t1;
		float beta=Matrix3f(ray.getDirection(), S, E2, 1).determinant()/t1;
		float gamma=Matrix3f(ray.getDirection(), E1, S, 1).determinant()/t1;
		if(t>tmin&&beta>=0&&beta<=1&&gamma>=0&&gamma<=1&&beta+gamma<=1&&t<hit.getT())
		{
            Vector3f norm=(Vector3f::cross(E1,E2)).normalized();
            hit.set(t, material, norm);
			return true;
		}
		return false;
	}
	void printInfo() override {}
	Vector3f normal;
	Vector3f vertices[3];

protected:
};

#endif // TRIANGLE_H
