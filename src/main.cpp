#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "camera.hpp"
#include "group.hpp"
#include "image.hpp"
#include "light.hpp"
#include "scene_parser.hpp"
#include "parameter.hpp"

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

  if (argc != 4)
  {
    std::cout << "Usage: ./bin/PA2 <input scene file> <output bmp file> <当前模式:basic_1/basic_2>"
              << std::endl;
    return 1;
  }
  string inputFile = argv[1];
  string outputFile = argv[2]; // only bmp is allowed.
  string mode = argv[3];
  if (mode != "basic_1" && mode != "basic_2")
  {
    std::cout << "Invalid mode. Use 'basic_1' or 'basic_2'." << std::endl;
    return 1;
  }
  // First, parse the scene using SceneParser.
  SceneParser sceneParser = SceneParser(inputFile.c_str());
  Camera *camera = sceneParser.getCamera();

  Image renderedImg(camera->getWidth(), camera->getHeight());
  // Then loop over each pixel in the image, shooting a ray

  long tot = camera->getWidth() * camera->getHeight();
  long now = 0;
  auto start = std::chrono::high_resolution_clock::now();
  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  float time;

  for (int u = 0; u < camera->getWidth(); u++)
  {
    for (int v = 0; v < camera->getHeight(); v++)
    {
      Ray camRay = camera->generateRay(Vector2f(u, v));
      Group *baseGroup = sceneParser.getGroup();
      Hit hit;
      Vector3f finalColor = Vector3f::ZERO;

      // 递归式光线追踪
      for (int bounce = 0; bounce < MAX_BOUNCE+1 && camRay.getLength() < INFINITY_LENGTH; bounce++)
      {
        hit = Hit();
        bool isIntersect = baseGroup->intersect(camRay, hit, EPS);
        Vector3f nowColor = Vector3f::ZERO;
        if (isIntersect)
        {
          for (int li = 0; li < sceneParser.getNumLights(); li++)
          {
            Light *light = sceneParser.getLight(li);
            Vector3f L, lightColor;
            light->getIllumination(camRay.pointAtParameter(hit.getT()), L, lightColor);
            nowColor = hit.getMaterial()->Shade(camRay, hit, L, lightColor);
          }

if (mode == WHITTED_STYLE)
{
            // 基础要求1-光线不会在漫反射表面反弹
            if (hit.getMaterial()->isReflective)
              {printf("reflect\n");
                camRay.reflect(hit);}
            else
              camRay.miss();
}
else
{
          // 基础要求2-光线会在漫反射表面反弹
          // TODO
}

        }
        else
        {
          nowColor = sceneParser.getBackgroundColor();
          camRay.miss();
        }
        // accumulate the color
        finalColor += nowColor * camRay.getKs()/DEFAULT_KS; // 第n次反射，ray的衰减系数为Ks^(n-1)
      }

      renderedImg.SetPixel(u, v, finalColor);
      now++;
      end = std::chrono::high_resolution_clock::now();
      duration =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      time = duration.count() / 1e6;
      std::cout << std::fixed << std::setprecision(2)
                << time * (tot * 1.0 / now - 1) << " seconds remain"
                << std::endl;
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
