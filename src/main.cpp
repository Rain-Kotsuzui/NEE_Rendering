#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>

#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"

#include <string>

#include <chrono>
#include <iomanip>

using namespace std;

int main(int argc, char *argv[])
{
    for (int argNum = 1; argNum < argc; ++argNum)
    {
        std::cout << "Argument " << argNum << " is: " << argv[argNum] << std::endl;
    }

    if (argc != 3)
    {
        std::cout << "Usage: ./bin/PA1 <input scene file> <output bmp file>" << std::endl;
        return 1;
    }
    string inputFile = argv[1];
    string outputFile = argv[2]; // only bmp is allowed.

    // First, parse the scene using SceneParser.
    SceneParser sceneParser = SceneParser(inputFile.c_str());
    Camera *camera = sceneParser.getCamera();

    Image renderedImg(camera->getWidth(), camera->getHeight());
    // Then loop over each pixel in the image, shooting a ray

    
        long tot = camera->getWidth() * camera->getHeight();
        long now = 0;
        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        float time;
    
    for (int u = 0; u < camera->getWidth(); u++)
    {
        for (int v = 0; v < camera->getHeight(); v++)
        {
            Ray camRay = camera->generateRay(Vector2f(u, v));
            Group *baseGroup = sceneParser.getGroup();
            Hit hit;
            bool isIntersect = baseGroup->intersect(camRay, hit, 0);

            if (isIntersect)
            {
                Vector3f finalColor = Vector3f::ZERO;
                for (int li = 0; li < sceneParser.getNumLights(); li++)
                {
                    Light *light = sceneParser.getLight(li);
                    Vector3f L, lightColor;

                    light->getIllumination(camRay.pointAtParameter(hit.getT()), L, lightColor);
                    finalColor += hit.getMaterial()->Shade(camRay, hit, L, lightColor);
                }
                // finalColor+=ambientLight*Ka;
                renderedImg.SetPixel(u, v, finalColor);
            }
            else
                renderedImg.SetPixel(u, v, sceneParser.getBackgroundColor());
                
             now++;
             end = std::chrono::high_resolution_clock::now();
             duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
             time = duration.count() / 1e6;
             std::cout << std::fixed << std::setprecision(2) << time * (tot * 1.0 / now - 1) << " seconds remain" << std::endl;
             std::cout << "\033[2J\033[1;1H";
        }
    }
    renderedImg.SaveImage(outputFile.c_str());
    // through that pixel and finding its intersection with
    // the scene.  Write the color at the intersection to that
    // pixel in your output image.
    // std::cout << "Hello! Computer Graphics!" << endl;
    std::cout << time << " seconds cost";
    return 0;
}
