#ifndef __COMMON__
#define __COMMON__

#include <cmath>
#include <iostream>
#include <vector>
#include "Cuttlebone/Cuttlebone.hpp"
#include "allocore/al_Allocore.hpp"
using namespace al;

// Common definition of application state
//
struct State {
  //  Vec3f vertex[14];
  Vec3f cursorPosition;
  Vec3f navPosition;
  Quatd navOrientation;
  //  vector<int> connections[14];
};

#endif
