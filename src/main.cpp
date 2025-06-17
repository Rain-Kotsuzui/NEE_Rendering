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
  
  Render render(mode,&sceneParser,camera);
  Image renderedImg(camera->getWidth(), camera->getHeight());
  // Then loop over each pixel in the image, shooting a ray

  Timer timer(camera->getWidth() * camera->getHeight());//计时器

  for (int u = 0; u < camera->getWidth(); u++)
  {
    for (int v = 0; v < camera->getHeight(); v++)
    {
      render.rend(u,v);
      renderedImg.SetPixel(u, v, render.getFinalColor());
      timer.update();
    }
  }

  renderedImg.SaveImage(outputFile.c_str());
  std::cout << timer.getTime() << " seconds cost";
  return 0;
}
