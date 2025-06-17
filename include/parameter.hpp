#ifndef PARAMETER_H
#define PARAMETER_H

#include <string>
using std::string;
static const string WHITTED_STYLE = "basic_1"; // mode for whitted style ray tracing

static const int MAX_BOUNCE = 3; // maximum number of bounces for ray tracing
static const double DEFAULT_KS = 0.3; //反射系数
static const double DEFAULT_KR = 0.5; //折射系数
static const double INFINITY_LENGTH = 1e10; // infinity value for ray tracing
static const double EPS = 1e-2; // epsilon value for ray tracing
#endif