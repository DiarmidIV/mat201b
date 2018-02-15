// MAT201B
// Fall 2015
// Author: Karl Yerkes
//
// Cuttlebone "Laptop Graphics Renderer"
//

#include "Cuttlebone/Cuttlebone.hpp"
#include "allocore/io/al_App.hpp"

#include "common.hpp"

using namespace al;

unsigned agentCount = 30;
float sphereRadius = 10;
float scaleFactor = 0.1;

Mesh sphere; 

struct Agent {
  Pose pose;
  Color c;

  Agent() {
    c = HSV(rnd::uniform(), 0.7, 1);
  }

void draw(Graphics& g) {
  g.pushMatrix();
  g.translate(pose.pos());
  g.rotate(pose.quat());
  g.color(c);
  g.draw(sphere);
  g.popMatrix();
}
};

struct MyApp : App {
  Material material;
  Light light;
  bool simulate = true;

  State state;
  cuttlebone::Taker<State> taker;

  vector<Agent> agent;

  MyApp() {
    addSphere(sphere, sphereRadius);
    sphere.generateNormals();

    light.pos(0, 0, 0);              // place the light
    nav().pos(0, 0, 30);             // place the viewer
    lens().far(400);                 // set the far clipping plane
    agent.resize(agentCount);  // make all the particles
    background(Color(0.07));

    initWindow();
  }

  virtual void onAnimate(double dt) { taker.get(state); 

  for (unsigned i = 0; i < 30; ++i) {
    agent[i].pose.pos() = state.position[i];
    // cout << i << state.position[i] << endl;
  }
  }

  virtual void onDraw(Graphics& g, const Viewpoint& v) {
    material();
    light();
    g.scale(scaleFactor);
    for (auto a : agent) a.draw(g);
  }
};

int main() {
  MyApp app;
  app.taker.start();
  app.start();
}
