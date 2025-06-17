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
class Render
{
public:
    Render(string &mode, SceneParser *sceneParser, Camera *camera) : mode(mode), sceneParser(sceneParser), camera(camera), finalColor(Vector3f::ZERO), nowColor(Vector3f::ZERO) {};
    ~Render() = default;

    void rend(int &u, int &v)
    {
        if (mode == WHITTED_STYLE)
            rend_basic_1(u, v);
        else
        // TODO
        {
        }
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
            hit = Hit(); // 每次反射前重置hit
            bool isIntersect = baseGroup->intersect(camRay, hit, EPS);
            Vector3f nowColor = Vector3f::ZERO;
            if (isIntersect)
            {
                // 基础要求1.1-反射与折射
                if (hit.getMaterial()->isReflective)
                    camRay.reflect(hit);
                else if (hit.getMaterial()->isRefractive)
                    camRay.refract(hit);
                else
                    camRay.miss();

                for (int li = 0; li < sceneParser->getNumLights(); li++)
                {
                    Light *light = sceneParser->getLight(li);
                    // 基础要求1.2-shadow ray
                    Ray shadowRay = Ray(camRay.pointAtParameter(hit.getT()), light->getDirection(camRay.pointAtParameter(hit.getT())));
                    Hit shadowHit;
                    bool isShadowed = baseGroup->intersect(shadowRay, shadowHit, EPS);
                    if (isShadowed && light->isShadowed(shadowHit.getT(), camRay.pointAtParameter(hit.getT())))
                        continue; // 光线被遮挡，跳过

                    Vector3f L, lightColor;
                    light->getIllumination(camRay.pointAtParameter(hit.getT()), L, lightColor);
                    nowColor = hit.getMaterial()->Shade(camRay, hit, L, lightColor);
                }
            }
            else
            {
                nowColor = sceneParser->getBackgroundColor();
                camRay.miss();
            }
            // accumulate the color
            finalColor += nowColor * camRay.getKs()*camRay.getKr(); // 第n次反射，ray的衰减系数为Ks^(n-1)
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