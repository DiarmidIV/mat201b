#ifndef __COMMON__
#define __COMMON__

#include "allocore/io/al_App.hpp"
using namespace al;

// Common definition of application state
//
struct State {

  Vec3f vertex[14];
  Vec3f cursorPosition;
  vector<int> connections[14];

};

#endif
