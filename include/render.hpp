#ifndef RENDER_HPP
#define RENDER_HPP

#include <vecmath.h>

#include "camera.hpp"
#include "group.hpp"
#include "hit.hpp"
#include "ray.hpp"
#include "scene_parser.hpp"
#include "light.hpp"
#include "parameter.hpp"
#include "material.hpp"
class Render
{
public:
    Render(string &mode, SceneParser *sceneParser, Camera *camera) : mode(mode), sceneParser(sceneParser), camera(camera), finalColor(Vector3f::ZERO), nowColor(Vector3f::ZERO) {};
    ~Render() = default;

    void rend(int &u, int &v)
    {
        if (mode == WHITTED_STYLE)
            rend_basic_1(u, v);
        else if (mode == PT)
            rend_basic_2(u, v);
        else
            rend_NEE(u, v);
    }

    void rend_basic_1(int &u, int &v)
    {
        nowColor = Vector3f::ZERO, finalColor = Vector3f::ZERO;
        Ray camRay = camera->generateRay(Vector2f(u, v));
        Group *baseGroup = sceneParser->getGroup();
        Hit hit;

        // 递归式光线追踪
        for (int bounce = 0; bounce < MAX_BOUNCE + 1 && camRay.getLength() < INFINITY_LENGTH; bounce++)
        {
            hit.resetRemainMaterial_Normal(); // 每次反射前重置hit
            bool isIntersect = baseGroup->intersect(camRay, hit, EPS);
            Vector3f nowColor = Vector3f::ZERO;
            if (isIntersect)
            {

                for (int li = 0; li < sceneParser->getNumLights(); li++)
                {
                    Light *light = sceneParser->getLight(li);
                    if (light->typeLight() != 0)
                        continue;
                    // 基础要求1.2-shadow ray
                    Ray shadowRay = Ray(camRay.pointAtParameter(hit.getT()), light->getDirection(camRay.pointAtParameter(hit.getT())));
                    Hit shadowHit;
                    bool isShadowed = baseGroup->shadow_intersect(shadowRay, shadowHit, EPS);
                    if (isShadowed && light->isShadowed(shadowHit.getT(), camRay.pointAtParameter(hit.getT())))
                        continue; // 光线被遮挡，跳过

                    Vector3f L, lightColor;
                    light->getIllumination(camRay.pointAtParameter(hit.getT()), L, lightColor);
                    nowColor += hit.getMaterial()->Shade(camRay.getDirection(), hit.getNormal(), L, lightColor);
                }
                finalColor += nowColor * camRay.getKs() * camRay.getKr();

                // 基础要求1.1-反射与折射二选一
                if (hit.getMaterial()->isReflective)
                    camRay.reflect(hit);
                else if (hit.getMaterial()->isRefractive)
                    camRay.refract(hit);
                else
                    camRay.miss();
            }
            else
            {
                nowColor = sceneParser->getBackgroundColor();

                finalColor = nowColor * camRay.getKs() * camRay.getKr();
                camRay.miss();
            }
        }
        finalColor += ENV_LIGHT * camRay.getKr() * camRay.getKs() * ENV_LIGHT_INTENSITY; // 环境光
    }
    void rend_basic_2(int &u, int &v)
    {
        Vector3f result = Vector3f::ZERO;
        finalColor = Vector3f::ZERO;
        Group *baseGroup = sceneParser->getGroup();

        auto clamp = [](float x)
        {
            return x > 0 ? x : 0;
        };
        for (int li = 0; li < sceneParser->getNumLights(); li++)
        {
            Ray camRay = camera->generateRay(Vector2f(u, v));
            nowColor = Vector3f::ZERO;
            result = Vector3f::ZERO;

            Hit hit = Hit(camRay.getDirection());
            Light *light = sceneParser->getLight(li);
            if (!light->typeLight())
                continue; // Area light参与路径追踪

            float P_RR = 1;
            std::random_device rd;                          // 用于获取随机数种子
            std::mt19937 gen(rd());                         // 使用Mersenne Twister引擎
            std::uniform_real_distribution<> dis(0.0, 1.0); // 均匀分布，范围0.0到1.0
            double ksi = dis(gen);                          // 生成随机数

            float pdf_hemi = 1 / (2 * PI);
            for (int bounce = 0; bounce < MAX_BOUNCE; bounce++)
            {
                P_RR = camRay.getThrought().max();
                if (ksi > P_RR)
                {
                    result = Vector3f::ZERO;
                    break;
                }
                ksi = dis(gen);

                hit.resetRemainMaterial_Normal();
                Hit lightHit;
                Vector3f n = hit.getNormal();

                bool isIntersect = baseGroup->intersect(camRay, hit, EPS);
                bool isLightIntersect = light->intersect(camRay, lightHit, EPS);

                if (isLightIntersect && (!isIntersect || lightHit.getT() < hit.getT()))
                {
                    float cosTheta1 = clamp(Vector3f::dot(n, camRay.getDirection()));
                    result = camRay.getLdirFr() * light->getMaterial()->getIntensity() * light->getColor() * camRay.getThrought() * cosTheta1 / pdf_hemi / P_RR;
                    break;
                }
                if (!isIntersect)
                    break;
                else
                    camRay.update(hit, pdf_hemi, P_RR); // 更新光
                                                        /*
                                                                        if (!isIntersect)
                                                                        {
                                                                            if (isLightIntersect)
                                                                            {
                                                                                nowColor = light->getColor();
                                                                                Vector3f fr = nowColor;
                                                                                // curThrought = THROUGHT_CONST * clamp(-Vector3f::dot(light->getNormal(), camRay.getDirection()));
                                                                                // result += nowColor * camRay.getKs() * camRay.getKr() * camRay.getThrought() * curThrought / pdf_hemi / P_RR;
                                                                                result = fr * camRay.getThrought() * clamp(-Vector3f::dot(light->getNormal(), camRay.getDirection())) / pdf_hemi / P_RR;
                                                                                result *= light->getMaterial()->getIntensity();
                                                                                break;
                                                                            }
                                                                            result *= 0;
                                                                            break;
                                                                        }
                                                                        else
                                                                        {
                                                                            if (isLightIntersect) // 光线与光源相交
                                                                            {
                                                                                if (lightHit.getT() < hit.getT())
                                                                                {
                                                                                    nowColor = light->getColor();
                                                                                    Vector3f fr = nowColor;
                                                                                    // curThrought = THROUGHT_CONST * clamp(-Vector3f::dot(light->getNormal(), camRay.getDirection()));
                                                                                    // result += nowColor * camRay.getKs() * camRay.getKr() * camRay.getThrought() * curThrought / pdf_hemi / P_RR;
                                                                                    result = fr * camRay.getThrought() * clamp(-Vector3f::dot(light->getNormal(), camRay.getDirection())) / pdf_hemi / P_RR;
                                                                                    result *= light->getMaterial()->getIntensity();
                                                                                    break;
                                                                                }
                                                                            }
                                        
                                                                            camRay.update(hit, pdf_hemi, P_RR); // 更新光线
                                                                        }
                                                                            */
            }

            finalColor += result;
        }
    }

    void rend_NEE(int &u, int &v)
    {
        Vector3f result = Vector3f::ZERO;
        finalColor = Vector3f::ZERO;
        Group *baseGroup = sceneParser->getGroup();

        auto clamp = [](float x)
        {
            return x > 0 ? x : 0;
        };
        for (int li = 0; li < sceneParser->getNumLights(); li++)
        {
            Ray camRay = camera->generateRay(Vector2f(u, v));
            result = Vector3f::ZERO;

            Hit hit = Hit(camRay.getDirection());

            Light *light = sceneParser->getLight(li);
            if (light->typeLight() == 0)
                continue; // 点光源不参与路径追踪

            float P_RR = 1;
            std::random_device rd;                          // 用于获取随机数种子
            std::mt19937 gen(rd());                         // 使用Mersenne Twister引擎
            std::uniform_real_distribution<> dis(0.0, 1.0); // 均匀分布，范围0.0到1.0
            double ksi;                                     // 生成随机数
            const double pdf_hemi = 1 / (2 * PI);
            const double pdf_light = 1 / light->getArea();

            for (int bounce = 0; bounce < MAX_BOUNCE; bounce++)
            {

                P_RR = camRay.getThrought().max();
                ksi = dis(gen);
                if (ksi > P_RR)
                    break;
                Vector3f lastHit_N = hit.getNormal();
                hit.resetRemainMaterial_Normal();
                bool isIntersect = baseGroup->intersect(camRay, hit, EPS); // 更新hit

                Vector3f n;
                Ray sampleRay = Ray();
                if (light->typeLight() <= 3)
                    if (!NEE_getcolor(lastHit_N, isIntersect, baseGroup, hit, camRay, sampleRay, light, bounce, result, pdf_hemi, pdf_light))
                        break;
                if (light->typeLight() == 4)
                // 体积光
                {
                    Vector3f volLight = Vector3f::ZERO;
                    getVolLight(volLight, light, baseGroup, hit, camRay);
                    result += volLight * camRay.getThrought();
                    if (!isIntersect)
                        break;
                }
                camRay.update(hit, pdf_hemi, P_RR, sampleRay.getDirection()); // 更新光
            }

            finalColor += result;
        }
    }

    void rend_wrong(int &u, int &v)
    {
        Vector3f result = Vector3f::ZERO;
        finalColor = Vector3f::ZERO;
        Group *baseGroup = sceneParser->getGroup();

        auto clamp = [](float x)
        {
            return x > 0 ? x : 0;
        };
        for (int li = 0; li < sceneParser->getNumLights(); li++)
        {
            Ray camRay = camera->generateRay(Vector2f(u, v));
            nowColor = Vector3f::ZERO;
            result = Vector3f::ZERO;

            Hit hit;
            Light *light = sceneParser->getLight(li);
            if (!light->typeLight() != 1)
                continue; // Area light参与路径追踪
            float curThrought = 1;
            float P_RR = 1;
            std::random_device rd;                          // 用于获取随机数种子
            std::mt19937 gen(rd());                         // 使用Mersenne Twister引擎
            std::uniform_real_distribution<> dis(0.0, 1.0); // 均匀分布，范围0.0到1.0
            double ksi;                                     // 生成随机数
            double pdf_hemi = 1 / (2 * PI);
            double pdf_light = 1 / light->getArea();
            for (int bounce = 0; bounce < MAX_BOUNCE; bounce++)
            {

                P_RR = camRay.getThrought_wrong() * camRay.getKs() * camRay.getKr();
                ksi = dis(gen);
                if (ksi > P_RR)
                    break;
                Vector3f L_dir = Vector3f::ZERO;
                hit.resetRemainMaterial_Normal();
                bool isIntersect = baseGroup->intersect(camRay, hit, EPS);
                Hit lightHit;
                bool isLightIntersect = light->intersect(camRay, lightHit, EPS);

                if (!isIntersect)
                {
                    if (isLightIntersect) // 光线与光源相交
                    {
                        nowColor = light->getColor();
                        curThrought = THROUGHT_CONST * clamp(-Vector3f::dot(light->getNormal(), camRay.getDirection()));
                        result += nowColor * camRay.getKs() * camRay.getKr() * camRay.getThrought_wrong() * curThrought * light->getMaterial()->getIntensity() / pdf_hemi / P_RR;
                    }
                    break;
                }
                else
                {
                    if (isLightIntersect) // 光线与光源相交
                    {
                        if (lightHit.getT() < hit.getT())
                        {
                            nowColor = light->getColor();
                            curThrought = THROUGHT_CONST * clamp(-Vector3f::dot(light->getNormal(), camRay.getDirection()));
                            result += nowColor * camRay.getKs() * camRay.getKr() * camRay.getThrought_wrong() * curThrought * light->getMaterial()->getIntensity() / pdf_hemi / P_RR;
                            break;
                        }
                    }
                    nowColor = hit.getMaterial()->getDiffuseColor();
                    Vector3f sample = light->getSample();

                    curThrought = camRay.getThrought_wrong();
                    camRay.update(hit, pdf_hemi, P_RR); // 更新光线

                    Ray lightRay = Ray(camRay.pointAtParameter(hit.getT()), (sample - camRay.pointAtParameter(hit.getT())).normalized());
                    Hit shadowHit;
                    baseGroup->intersect(lightRay, shadowHit, EPS);
                    if ((camRay.pointAtParameter(hit.getT()) - sample).length() < shadowHit.getT())
                    {
                        float cosTheta2 = clamp(Vector3f::dot(light->getNormal(), -lightRay.getDirection()));
                        float cosTheta1 = clamp(Vector3f::dot(hit.getNormal(), lightRay.getDirection()));
                        float coeff = cosTheta2 * THROUGHT_CONST / (camRay.pointAtParameter(hit.getT()) - sample).squaredLength();
                        L_dir = nowColor * light->getMaterial()->getIntensity() * curThrought * cosTheta1 * coeff / pdf_light;
                    }
                    result += L_dir + nowColor * camRay.getKs() * camRay.getKr() * camRay.getThrought_wrong(); // 计算最终颜色
                }
            }

            finalColor += result;
        }
    }
    Vector3f getFinalColor() const
    {
        return finalColor;
    }

private:
    void getVolLight(Vector3f &volLight, Light *light, Group *baseGroup, const Hit &hit, Ray &camRay)
    {
        std::random_device rd1;                          // 用于获取随机数种子
        std::mt19937 gen1(rd1());                        // 使用Mersenne Twister引擎
        std::uniform_real_distribution<> dis1(0.0, 1.0); // 均匀分布，范围0.0到1.0
        double t;
        float T = hit.getT();
        float pdf_L = 1 / T;
        for (int count = 0; count < VOLLIGHT_SAMPLE_COUNT; count++)
        {
            t = dis1(gen1) * T;
            Vector3f o = camRay.pointAtParameter(t);
            Ray shadowRay = Ray(o, light->getDirection(o));
            Hit shadowHit;
            Hit lightHit;
            bool isShadowed = baseGroup->intersect(shadowRay, shadowHit, EPS);
            bool isLighted = light->intersect(shadowRay, lightHit, EPS);
            if (isLighted && (lightHit.getT() < shadowHit.getT()))
                volLight += light->getColor() * light->getMaterial()->getIntensity() / (lightHit.getT() * lightHit.getT()) / pdf_L;
        }
        volLight = volLight / SAMPLE_COUNT;
    }
    bool NEE_getcolor(Vector3f &n, bool isIntersect, Group *baseGroup, const Hit &hit, Ray &camRay, Ray &sampleRay, Light *light, int &bounce, Vector3f &result, const float &pdf_hemi, const float &pdf_light)
    {
        auto clamp = [](float x)
        {
            return x > 0 ? x : 0;
        };
        Hit lightHit;
        bool isLightIntersect = light->intersect(camRay, lightHit, EPS);

        if (light->typeLight() == 2) // 方向光
        {
            Hit sHit;
            Ray sRay = Ray(camRay.getOrigin(), light->getDirection(camRay.getOrigin()));
            bool isShadowed = baseGroup->intersect(sRay, sHit, EPS);
            if (bounce && !isShadowed)
            {
                float cosTheta1 = clamp(Vector3f::dot(n, camRay.getDirection()));
                result = camRay.getLdirFr() * light->getMaterial()->getIntensity() * light->getColor() * camRay.getThrought() * cosTheta1 / pdf_hemi;
                return false;
            }
            if (!isIntersect)
                return false;
            return true;
        }

        Vector3f L_dir = Vector3f::ZERO;
        Vector3f sample = light->getSample();
        sampleRay = Ray(camRay.getOrigin(), (sample - camRay.getOrigin()).normalized());
        Hit shadowHit;
        baseGroup->intersect(sampleRay, shadowHit, EPS);

        if (isLightIntersect && (!isIntersect || lightHit.getT() < hit.getT()))
        {
            float cosTheta1 = clamp(Vector3f::dot(n, camRay.getDirection()));
            L_dir = camRay.getLdirFr() * light->getMaterial()->getIntensity() * light->getColor() * camRay.getThrought() * cosTheta1;
            result += L_dir;
            return false;
        }
        else if (bounce && (sampleRay.getOrigin() - sample).length() < shadowHit.getT())
        {
            float cosTheta2 = clamp(Vector3f::dot(light->getNormal(), -sampleRay.getDirection()));
            float cosTheta1 = clamp(Vector3f::dot(n, sampleRay.getDirection()));
            float coeff = cosTheta2 / (sampleRay.getOrigin() - sample).squaredLength();
            L_dir = camRay.getLdirFr() * light->getMaterial()->getIntensity() * light->getColor() * camRay.getThrought() * cosTheta1 * coeff / pdf_light;
        }
        result += L_dir; // 计算最终颜色
        return true;
    }

    string mode;
    SceneParser *sceneParser;
    Camera *camera;
    Vector3f nowColor;
    Vector3f finalColor;
};

#endif // RENDER_HPP