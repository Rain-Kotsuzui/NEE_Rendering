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
            hit.resetRemainMaterial(); // 每次反射前重置hit
            bool isIntersect = baseGroup->intersect(camRay, hit, EPS);
            Vector3f nowColor = Vector3f::ZERO;
            if (isIntersect)
            {

                for (int li = 0; li < sceneParser->getNumLights(); li++)
                {
                    Light *light = sceneParser->getLight(li);
                    if (light->isAreaLight())
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

                finalColor =nowColor * camRay.getKs() * camRay.getKr();
                camRay.miss();
            }
        }
        finalColor += ENV_LIGHT * camRay.getKr() * camRay.getKs() * ENV_LIGHT_INTENSITY; // 环境光
    }
    /*WRONG!!!!!
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

                Hit hit;
                Light *light = sceneParser->getLight(li);
                if (!light->isAreaLight())
                    continue; // Area light参与路径追踪
                float throught = 1;
                float curThrought = 1;

                float P_RR = 1;
                std::random_device rd;                          // 用于获取随机数种子
                std::mt19937 gen(rd());                         // 使用Mersenne Twister引擎
                std::uniform_real_distribution<> dis(0.0, 1.0); // 均匀分布，范围0.0到1.0
                double ksi = dis(gen);                          // 生成随机数
                // TODO颜色变灰问题
                float pdf_hemi = 1 / (2 * PI);
                for (int bounce = 0; bounce < MAX_BOUNCE; bounce++)
                {
                    P_RR = camRay.getThrought() * camRay.getKs() * camRay.getKr();
                    if (ksi > P_RR)
                    {
                        result = Vector3f::ZERO;
                        break;
                    }
                    ksi = dis(gen);
                    hit.resetRemainMaterial();
                    bool isIntersect = baseGroup->intersect(camRay, hit, EPS);
                    Hit lightHit;
                    bool isLightIntersect = light->intersect(camRay, lightHit, EPS);

                    if (!isIntersect)
                    {
                        if (isLightIntersect)
                        {
                            nowColor = light->getColor();
                            // curThrought = THROUGHT_CONST * clamp(-Vector3f::dot(light->getNormal(), camRay.getDirection()));
                            // result += nowColor * camRay.getKs() * camRay.getKr() * camRay.getThrought() * curThrought / pdf_hemi / P_RR;
                            result += nowColor * camRay.getKs() * camRay.getKr() * camRay.getThrought() / P_RR;

                            result *= light->getMaterial()->getIntensity();
                            break;
                        }
                        //result = Vector3f::ZERO;
                        break;
                    }
                    else
                    {
                        if (isLightIntersect) // 光线与光源相交
                        {
                            if (lightHit.getT() < hit.getT())
                            {
                                nowColor = light->getColor();
                                // curThrought = THROUGHT_CONST * clamp(-Vector3f::dot(light->getNormal(), camRay.getDirection()));
                                // result += nowColor * camRay.getKs() * camRay.getKr() * camRay.getThrought() * curThrought / pdf_hemi / P_RR;
                                result += nowColor * camRay.getKs() * camRay.getKr() * camRay.getThrought() / P_RR;
                                result *= light->getMaterial()->getIntensity();
                                break;
                            }
                        }

                        camRay.update(hit, pdf_hemi, P_RR); // 更新光线

                        nowColor = hit.getMaterial()->getDiffuseColor();
                        result += nowColor * camRay.getKs() * camRay.getKr() * camRay.getThrought(); // 计算光线衰减
                    }
                }

                finalColor += result;
            }
        }
    */
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

            Hit hit;
            Light *light = sceneParser->getLight(li);
            if (!light->isAreaLight())
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
                hit.resetRemainMaterial();
                bool isIntersect = baseGroup->intersect(camRay, hit, EPS);
                Hit lightHit;
                bool isLightIntersect = light->intersect(camRay, lightHit, EPS);

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
            nowColor = Vector3f::ZERO;
            result = Vector3f::ZERO;

            Hit hit;
            Light *light = sceneParser->getLight(li);
            if (!light->isAreaLight())
                continue; // Area light参与路径追踪
            Vector3f curThrought = 1;
            Vector3f L_dir = Vector3f::ZERO;
            Vector3f fr;
            float P_RR = 1;
            std::random_device rd;                          // 用于获取随机数种子
            std::mt19937 gen(rd());                         // 使用Mersenne Twister引擎
            std::uniform_real_distribution<> dis(0.0, 1.0); // 均匀分布，范围0.0到1.0
            double ksi;                                     // 生成随机数
            double pdf_hemi = 1 / (2 * PI);
            double pdf_light = 1 / light->getArea();
            for (int bounce = 0; bounce < MAX_BOUNCE; bounce++)
            {
                L_dir = Vector3f::ZERO;
                P_RR = camRay.getThrought().max();
                ksi = dis(gen);
                if (ksi > P_RR)
                    break;

                hit.resetRemainMaterial();
                bool isIntersect = baseGroup->intersect(camRay, hit, EPS);
                Hit lightHit;
                bool isLightIntersect = light->intersect(camRay, lightHit, EPS);

                if (!isIntersect)
                {
                    if (isLightIntersect) // 光线与光源相交
                    {
                        nowColor = light->getColor();
                        fr = nowColor * clamp(-Vector3f::dot(light->getNormal(), camRay.getDirection()));
                        result += fr * camRay.getThrought() * light->getMaterial()->getIntensity() / pdf_light;
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
                            fr = nowColor * clamp(-Vector3f::dot(light->getNormal(), camRay.getDirection()));
                            result += fr * camRay.getThrought() * light->getMaterial()->getIntensity() / pdf_light;
                            break;
                        }
                    }
                    Vector3f sample = light->getSample();

                    curThrought = camRay.getThrought();
                    camRay.update(hit, pdf_hemi, P_RR,sample); // 更新光线

                    Ray lightRay = Ray(camRay.getOrigin(), (sample - camRay.getOrigin()).normalized());
                    Hit shadowHit;
                    baseGroup->intersect(lightRay, shadowHit, EPS);
                    if ((camRay.getOrigin() - sample).length() < shadowHit.getT())
                    {
                        float cosTheta2 = clamp(Vector3f::dot(light->getNormal(), -lightRay.getDirection()));
                        float cosTheta1 = clamp(Vector3f::dot(hit.getNormal(), lightRay.getDirection()));
                        float coeff = cosTheta2 / (camRay.getOrigin() - sample).squaredLength();
                        L_dir = camRay.getLdirFr()* light->getMaterial()->getIntensity() * curThrought * cosTheta1 * coeff / pdf_light;
                    }
                    result += L_dir; // 计算最终颜色
                }
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
            if (!light->isAreaLight())
                continue; // Area light参与路径追踪
            float curThrought = 1;
            Vector3f L_dir = Vector3f::ZERO;
            float P_RR = 1;
            std::random_device rd;                          // 用于获取随机数种子
            std::mt19937 gen(rd());                         // 使用Mersenne Twister引擎
            std::uniform_real_distribution<> dis(0.0, 1.0); // 均匀分布，范围0.0到1.0
            double ksi;                                     // 生成随机数
            double pdf_hemi = 1 / (2 * PI);
            double pdf_light = 1 / light->getArea();
            for (int bounce = 0; bounce < MAX_BOUNCE; bounce++)
            {
                L_dir = Vector3f::ZERO;
                P_RR = camRay.getThrought_wrong() * camRay.getKs() * camRay.getKr();
                ksi = dis(gen);
                if (ksi > P_RR)
                    break;

                hit.resetRemainMaterial();
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
    string mode;
    SceneParser *sceneParser;
    Camera *camera;
    Vector3f nowColor;
    Vector3f finalColor;
};

#endif // RENDER_HPP