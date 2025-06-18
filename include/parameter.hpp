#ifndef PARAMETER_H
#define PARAMETER_H

#include <string>
#include <vecmath.h>
using std::string;
static const string WHITTED_STYLE = "basic_1"; // mode for whitted style ray tracing

static const int MAX_BOUNCE = 10; // maximum number of bounces for ray tracing
static const double DEFAULT_KS = 0.3; //反射系数
static const double DEFAULT_KR = 1; //折射系数
static const double INFINITY_LENGTH = 1e10; // infinity value for ray tracing
static const double EPS = 1e-4; // epsilon value for ray tracing
static const Vector3f ENV_LIGHT = Vector3f(1, 0.75, 0.24); // 环境光
#endif