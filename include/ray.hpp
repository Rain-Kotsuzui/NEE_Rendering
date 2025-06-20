#ifndef RAY_H
#define RAY_H

#include <cassert>
#include <iostream>
#include <Vector3f.h>
#include <math.h>
#include <random>

#include "parameter.hpp"
#include "hit.hpp"
#include "material.hpp"
// Ray class mostly copied from Peter Shirley and Keith Morley
class Ray
{
public:
    Ray() = delete;
    Ray(const Vector3f &orig, const Vector3f &dir, double ks = 1, double kr = 1)
    {
        origin = orig;
        direction = dir;
        Ks = ks;
        Kr = kr;
        length = 0;
        material = VOID; // 默认折射率为真空
        throught = 1;
    }

    Ray(const Ray &r)
    {
        origin = r.origin;
        direction = r.direction;
        Ks = r.Ks;
        Kr = r.Kr;
        length = r.length;
        material = r.material;
        throught = r.throught;
    }
    ~Ray() {}

    const void update(const Hit &hit,const float &pdf_hemi,const float &P_RR)
    {
        Vector3f wi=direction;
        Vector3f wo;
        Vector3f fr;//BRDF计算

        std::random_device rd;                          // 用于获取随机数种子
        std::mt19937 gen(rd());                         // 使用Mersenne Twister引擎
        std::uniform_real_distribution<> dis(0.0, 1.0); // 均匀分布，范围0.0到1.0
        double randomValue;
        // 基础要求2-反射,折射,漫反射三选一
        randomValue = dis(gen);                                                  // 生成一个0到1之间的随机数
        if (0 < randomValue && randomValue < hit.getMaterial()->refractiveRatio) // 折射
            refract(hit);
        else if (hit.getMaterial()->refractiveRatio < randomValue && randomValue < hit.getMaterial()->refractiveRatio + hit.getMaterial()->reflectiveRatio) // 反射
            reflect(hit);
        else // 漫反射
        {
            Vector3f randomDir = Vector3f::ZERO;
            double cosTheta, phi;
            do
            {
                // 球面均匀分布
                cosTheta = 1 - 2 * dis(gen);
                phi = 2 * PI * dis(gen);
                randomDir = Vector3f(cos(phi) * sqrt(1 - cosTheta * cosTheta), sin(phi) * sqrt(1 - cosTheta * cosTheta), cosTheta);
            } while (Vector3f::dot(randomDir, hit.getNormal()) < 0); // 确保随机方向与法线方向一致
            diffuseReflect(hit, randomDir);

            auto clamp = [](float x)
            {
                return x > 0 ? x : 0;
            };
            
            float cosTheta1 = clamp(Vector3f::dot(direction, hit.getNormal()));
//wrong but cool

            float curThrought_wrong = THROUGHT_CONST * cosTheta1 / pdf_hemi / P_RR;
            throught_wrong *=curThrought_wrong;
            
//
            wo = direction;
            //fr=hit.getMaterial()->sample_f(wi,wo,hit.getNormal());//brdf
            fr=hit.getMaterial()->getDiffuseColor()/PI;
            throught = throught*fr*cosTheta1/pdf_hemi/P_RR;
        }
    }
    const void reflect(const Hit &hit)
    {

        length += hit.getT();
        origin = pointAtParameter(hit.getT());
        direction = direction - 2 * (Vector3f::dot(hit.getNormal(),direction)) * hit.getNormal();
        Ks *= hit.getMaterial()->getKs(); // 更新反射系数

        throught *=hit.getMaterial()->getKs();
    }
    const void diffuseReflect(const Hit &hit, Vector3f &randomDir)
    {

        length += hit.getT();
        origin = pointAtParameter(hit.getT());
        direction = randomDir;
        Ks *=hit.getMaterial()->getKs();

        throught = throught*Ks;
    }
    const void refract(const Hit &hit)
    {

        origin = pointAtParameter(hit.getT());

        float n1;
        float n2;
        float kr;

            n1 = material->getRefractiveIndex(); // 当前材质的折射率
            n2 = hit.getMaterial()->getRefractiveIndex(), kr = hit.getMaterial()->getKr(); // 目标材质的折射率

        Vector3f n = hit.getNormal();
        float cosTheta1 = -Vector3f::dot(direction, n);
        if (n1 == n2 || cosTheta1 == 0) // 如果折射率相同或光线垂直于法线，则不折射
        {
            Kr *= kr; // 更新折射系数
            return;
        }
        if (cosTheta1 < 0) // 如果光线离开介质（向外为正法向）
        {
            cosTheta1 = -cosTheta1; // 法线方向相反
            n = -n;
        }
        if (cosTheta1 * cosTheta1 < 1 - (n2 / n1) * (n2 / n1)) // 大于临界角，反射
        {
            reflect(hit);
            return;
        }

        length += hit.getT();
        float cosTheata2 = sqrt(1 - (n1 / n2) * (n1 / n2) * (1 - cosTheta1 * cosTheta1));
        Vector3f refractedDir = (n1 / n2) * direction + (n1 / n2 * cosTheta1 - cosTheata2) * n;
        direction = refractedDir.normalized();
        material = hit.getMaterial(); // 更新材质信息
        Kr *= kr;                     // 更新折射系数

        throught = throught*Kr;
    }

    const void miss()
    {
        length += INFINITY_LENGTH;
        Ks *= DEFAULT_KS;
    }

    const Vector3f &getOrigin() const
    {
        return origin;
    }

    const Vector3f &getDirection() const
    {
        return direction;
    }

    const float &getKs() const
    {
        return Ks;
    }
    const float &getKr() const
    {
        return Kr;
    }
    const float &getLength() const
    {
        return length;
    }

    const Material *getMaterial() const
    {
        return material;
    }
    const Vector3f getThrought() const
    {
        return throught;
    }
    const float getThrought_wrong() const
    {
        return throught_wrong;
    }
    void setOrigin(const Vector3f &orig)
    {
        origin = orig;
    }
    void setDirection(const Vector3f &dir)
    {
        direction = dir.normalized();
    }
    void setKs(const float &ks)
    {
        assert(ks >= 0);
        Ks = ks;
    }
    void setKr(const float &kr)
    {
        assert(kr >= 0);
        Kr = kr;
    }
    void setLength(const float &len)
    {
        assert(len >= 0);
        length = len;
    }

    void setMaterial(Material *mat)
    {
        material = mat;
    }
    void setThrought(const float &thr)
    {
        throught = thr;
    }

    Vector3f pointAtParameter(float t) const
    {
        return origin + direction * t;
    }

private:
    Vector3f origin;
    Vector3f direction;
    Material *material; // 折射需要的光线所在介质材质信息
    float Ks;           // 反射衰减系数
    float Kr;           // 折射衰减系数
    float length;       // 长度
    Vector3f throught;     // RGB各通量
    float throught_wrong;
};

inline std::ostream &operator<<(std::ostream &os, const Ray &r)
{
    os << "Ray <" << r.getOrigin() << ", " << r.getDirection() << ">";
    return os;
}

#endif // RAY_H
