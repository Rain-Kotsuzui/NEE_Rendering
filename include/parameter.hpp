#ifndef PARAMETER_H
#define PARAMETER_H

#include <string>
#include <vecmath.h>
using std::string;
static const string WHITTED_STYLE = "basic_1"; // mode for whitted style ray tracing
static const string PT = "basic_2";
static const string NEE ="NEE";
static const int MAX_BOUNCE = 10;                          // maximum number of bounces for ray tracing
static const int SAMPLE_COUNT = 70;                               // 采样次数
static const int VOLLIGHT_SAMPLE_COUNT = 20;                               // 采样次数

static const double DEFAULT_KS = 0.6;                      // 反射系数
static const double DEFAULT_KR = 1;                        // 折射系数
static const double INFINITY_LENGTH = 1e10;                // infinity value for ray tracing
static const double EPS = 1e-4;                            // epsilon value for ray tracing
static const double THROUGHT_CONST = 1.8/(2*3.1415);              // 光线衰减常数
static const double PI =3.1415926535;
static const Vector3f ENV_LIGHT = Vector3f(1, 1, 1)*0.1; // 环境光
static const float ENV_LIGHT_INTENSITY=1;
static const float GAMMA=2.2;
#endif