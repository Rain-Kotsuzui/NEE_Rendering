#ifndef CAMERA_H
#define CAMERA_H

#include "ray.hpp"
#include <vecmath.h>
#include <float.h>
#include <cmath>
#include <random>

class Camera
{
public:
    Camera(const Vector3f &center, const Vector3f &direction, const Vector3f &up, int imgW, int imgH, float focus_dist, float lens_radius)
    {
        this->center = center;
        this->direction = direction.normalized();
        this->horizontal = Vector3f::cross(this->direction, up).normalized();
        this->up = Vector3f::cross(this->horizontal, this->direction);
        this->width = imgW;
        this->height = imgH;
        this->focus_dist = focus_dist;
        this->lens_radius = lens_radius;
    }

    // Generate rays for each screen-space coordinate
    virtual Ray generateRay(const Vector2f &point) = 0;
    virtual ~Camera() = default;

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    Vector3f getDirection() const{return direction;}
protected:
    // Extrinsic parameters
    Vector3f center;
    Vector3f direction;
    Vector3f up;
    Vector3f horizontal;
    float focus_dist;
    float lens_radius;
    // Intrinsic parameters
    int width;
    int height;
};

// You can add new functions or variables whenever needed.
class PerspectiveCamera : public Camera
{

public:
    PerspectiveCamera(const Vector3f &center, const Vector3f &direction,
                      const Vector3f &up, int imgW, int imgH, float angle, float focus_dist = 1, float lens_radius = 0)
        : Camera(center, direction, up, imgW, imgH, focus_dist, lens_radius),
          angle(angle) {}

    Vector3f random_unit_disk()
    {
        std::random_device rd;  // 用于获取随机数种子
        std::mt19937 gen(rd()); // 使用Mersenne Twister算法的随机数生成器
        // 定义一个0到1之间的均匀分布
        std::uniform_real_distribution<> dis(0.0, 1.0);
        double r = dis(gen);
        double phi = dis(gen);
        return Vector3f(sqrt(r) * cos(phi), sqrt(r) * sin(phi), 0);
    }
    const Vector2f random_Antialiasing()
    {
        std::random_device rd;  // 用于获取随机数种子
        std::mt19937 gen(rd()); // 使用Mersenne Twister算法的随机数生成器
        // 定义一个-1到1之间的均匀分布
        std::uniform_real_distribution<> dis(-1.0, 1.0);
        double x = dis(gen);
        double y = dis(gen);
        return Vector2f(x, y);
    }
    Ray generateRay(const Vector2f &point) override
    {
        Matrix3f R = Matrix3f(horizontal, -up, direction, 1);
        float fx = height / (2 * tan(angle / 2)), fy = height / (2 * tan(angle / 2));
        Vector3f offset = Vector3f::ZERO;
        if (lens_radius > 0.1)
            offset = random_unit_disk() * lens_radius;
        Vector2f aa = random_Antialiasing();
        return Ray(center + R * offset, R * (Vector3f((point.x() + aa.x() - 0.5 * width) / fx, (0.5 * height - point.y() - aa.y()) / fy, 1) * focus_dist - offset).normalized());
    }

private:
    float angle;
};

#endif // CAMERA_H
