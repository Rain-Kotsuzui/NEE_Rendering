/**
 * @file main.cpp
 * @brief
 * @version 0.1
 * @date 2025-07-14
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <omp.h>

#include "camera.hpp"
#include "group.hpp"
#include "image.hpp"
#include "light.hpp"
#include "scene_parser.hpp"
#include "parameter.hpp"
#include "timer.hpp"
#include "render.hpp"

#include <string>

using namespace std;

int main(int argc, char *argv[])
{
  for (int argNum = 1; argNum < argc; ++argNum)
  {
    std::cout << "Argument " << argNum << " is: " << argv[argNum] << std::endl;
  }

  if (argc < 4)
  {
    std::cout << "Usage: ./bin/PA2 <input scene file> <output bmp file> <当前模式:basic_1/basic_2>"
              << std::endl;
    return 1;
  }
  string inputFile = argv[1];
  string outputFile = argv[2]; // only bmp is allowed.
  string mode = argv[3];

  if (mode != "basic_1" && mode != "basic_2" && mode != "NEE")
  {
    std::cout << "Invalid mode. Use 'basic_1' or 'basic_2' or 'NEE'." << std::endl;
    return 1;
  }
  // First, parse the scene using SceneParser.
  SceneParser sceneParser = SceneParser(inputFile.c_str());
  Camera *camera = sceneParser.getCamera();

  Image renderedImg(camera->getWidth(), camera->getHeight());
  Image gammaImg(camera->getWidth(), camera->getHeight());
  // Then loop over each pixel in the image, shooting a ray

  Timer timer(camera->getWidth() * camera->getHeight()); // 计时器

  // gamma矫正
  Vector3f r_max = Vector3f::ZERO;
  // OpenMP并行加速
  int numP = 16;
  omp_set_num_threads(numP);
  {
#pragma omp parallel for
    for (int index = 0; index < camera->getWidth() * camera->getHeight(); index++)
    {
      int u = index % camera->getWidth();
      int v = index / camera->getWidth();
      Vector3f color = Vector3f::ZERO;
      for (int i = 0; i < SAMPLE_COUNT; i++)
      {
        // TODO：体积光尘埃图
        Render render(mode, &sceneParser, camera);
        render.rend(u, v);
        color += render.getFinalColor();
      }
      timer.update();
      color = color * (1.0 / SAMPLE_COUNT); // 平均采样
      r_max = r_max.max(color);
      renderedImg.SetPixel(u, v, color);
    }
  }

  renderedImg.SaveImage(outputFile.c_str());

  omp_set_num_threads(numP);
  {
#pragma omp parallel for
    for (int index = 0; index < camera->getWidth() * camera->getHeight(); index++)
    {
      int u = index % camera->getWidth();
      int v = index / camera->getWidth();
      Vector3f color = renderedImg.GetPixel(u, v).pow(1.0f / GAMMA);
      gammaImg.SetPixel(u, v, color);
    }
  }
  gammaImg.SaveImage((outputFile.substr(0, outputFile.length() - 4) + "_gamma.bmp").c_str());

  std::cout << numP << " threads, " << timer.getTime() << " seconds cost";
  return 0;
}
