// Diarmid Flatley
// 2018-2-26
// MAT 201B
// tuning lattice


#include "allocore/io/al_App.hpp"
#include <cmath>
#include <vector>
#include <iostream>
#include "Gamma/Filter.h"
#include "Gamma/Oscillator.h"
#include "Gamma/Gen.h"
#include "allocore/io/al_App.hpp"
#include "Cuttlebone/Cuttlebone.hpp"
#include "common.hpp"

using namespace std;
using namespace al;

Vec3f midpoint (Vec3f a, Vec3f b, Vec3f c) {
    Vec3f middle;
  middle.x = (a.x + b.x + c.x) / 3;
  middle.y = (a.y + b.y + c.y) / 3;
  middle.z = (a.z + b.z + c.z) / 3;
  return middle;
}

struct Node {
  Vec3f position = Vec3f(0,0,0);
  vector<int>connections;
  float frequency;

  Node() {}

  Node(Vec3f initPos, vector<int> initConnections, float initFreq) {
   position = initPos;
   connections = initConnections;
   frequency = initFreq;
  } 

  void set(float x, float y, float z) {
    position = Vec3f(x,y,z);
  }

  void set(Vec3f setPos) {
    position = setPos;
  }

  void draw(Graphics& g, Mesh& m) {
    m.color(1,1,1);
    g.pushMatrix();
    g.translate(position);
    g.draw(m);
    g.popMatrix();
  }
};

struct Cursor {
  Vec3f position;
  float counter;
  float increment;
  float currentFrequency;
  Node start;
  Node end;
             
  Cursor() {
    increment = 0.05f;
    counter = 0.0f;
    currentFrequency = 0.0f;
  }

  void set(Node initStart, Node initEnd) {
    position = initStart.position;
    start = initStart;
    end = initEnd;
    currentFrequency = start.frequency;
  }

  void update(Node node[]) {
    Vec3f startPos = start.position;
    Vec3f endPos = end.position;
    counter += increment;
    if (counter > 1) {
      counter -= 1;
      unsigned i = rand() % end.connections.size();
      int next = end.connections[i];
      this->set(end, node[next]); 
    }
    position = startPos.lerp(endPos, counter);
  } 

  void draw(Graphics& g, Mesh& m) {
    m.color(0,0,1);
    g.pushMatrix();
    g.translate(position);
    g.draw(m);
    g.popMatrix();
  }
};

struct Strut {
  Vec3f start, end;
 
  Strut() {} 

  Strut (Node initStart, Node initEnd) {
    start = initStart.position;
    end = initEnd.position;
  }

  void set(Node setStart, Node setEnd) {
    start = setStart.position;
    end = setEnd.position;
  }

  void draw(Graphics& g, Mesh& m) {
    m.reset();
    m.color(1,1,1);
    m.primitive(Graphics::LINES);
    m.stroke(2);
    m.vertex(start);
    m.vertex(end);
    g.draw(m);
  }
};

struct AlloApp : App {
   Material material;
   Light light;
   Mesh sphere;
   Mesh line;
   Mesh sphere2;
   Mesh sphere3;

   Node node[14];
   Vec3f vertex[14];  
   vector<Strut*> struts;
   vector<int> connections[14];
   float frequency[14] = {350, 250, 218.75, 375, 262.5, 300, 
     291.66666, 312.5, 328.125, 214.285715, 
     200, 306.25, 225, 210};

   Cursor cursor;
   Cursor cursor2;

   gam::Sine<> sine;
   gam::Sine<> sine2;

   State state;
   cuttlebone::Maker<State> maker;

  AlloApp() : maker("127.0.0.1") {
    nav().pos(0, 0, 20);
    light.pos(0,0,0);

    addSphere(sphere, 0.1);
    addSphere(sphere2, 0.1);
    addSphere(sphere3, 0.1);
    sphere.generateNormals();
    sphere2.generateNormals();
    sphere3.generateNormals();

    vertex[0] = {-1,0,0};
    vertex[1] = {0,0,-1};
    vertex[2] = {0,1,0};
    vertex[3] = {1,0,0};
    vertex[4] = {0,0,1};
    vertex[5] = {0,-1,0};  
    vertex[6] = {-1,1,-1};
    vertex[7] = {1,1,-1};
    vertex[8] = {1,1,1};
    vertex[9] = {-1,1,1};
    vertex[10] = {-1,-1,-1};
    vertex[11] = {1,-1,-1};
    vertex[12] = {1,-1,1};
    vertex[13] = {-1,-1,1};

    connections[0] = {1, 2, 4, 5, 6, 9, 10, 13};
    connections[1] = {0, 2, 3, 5, 6, 7, 10, 11};
    connections[2] = {0, 1, 3, 4, 6, 7, 8, 9};
    connections[3] = {1, 2, 4, 5, 7, 8, 11, 12};
    connections[4] = {0, 2, 3, 5, 8, 9, 12, 13};
    connections[5] = {0, 1, 3, 4, 10, 11, 12, 13}; 
    connections[6] = {0, 1, 2};
    connections[7] = {1, 2, 3};
    connections[8] = {2, 3, 4};
    connections[9] = {0, 2, 4};
    connections[10] = {0, 1, 5};
    connections[11] = {1, 3, 5};
    connections[12] = {3, 4, 5};
    connections[13] = {0, 4 ,5};

    for (int i = 0; i < 14; i++) {
      node[i] = {vertex[i], connections[i], frequency[i]};
    //  state.vertex[i] = vertex[i];
    //  state.connections[i] = connections[i];
    } 

    int strutCount = 0;
    for (int i = 0; i < 14; i++) {
      for (int j = 0; j < node[i].connections.size(); j++) {
        Strut * strut = new Strut;
        struts.push_back(strut);
        struts[strutCount]->set(node[i],node[node[i].connections[j]]);
        strutCount++;
      }
    }

    cursor.set(node[0],node[2]);
    cursor2.set(node[1],node[0]);

    sine.freq(0);
    sine2.freq(0);

    initWindow();
    initAudio();
  }

  void onAnimate(double dt) {
    cursor.update(node);
    cursor2.update(node);
    state.cursorPosition = cursor.position;
    maker.set(state);
  }

  void onDraw(Graphics& g) {
   
    material();
    light();

    for (unsigned i = 0; i < 14; i++) {
    node[i].draw(g,sphere);
    }

    cursor.draw(g, sphere2);
    cursor2.draw(g, sphere3);
  
    for (unsigned i = 0; i < struts.size(); i++) { 
      struts[i]->draw(g, line);
    }
  }

  virtual void onSound(al::AudioIOData& io) {
    gam::Sync::master().spu(audioIO().fps());
    while (io()) {
      sine.freq(cursor.currentFrequency);
      sine2.freq(cursor2.currentFrequency * 2.0f);
      float s = 0;
      s = (sine() + sine2()) / 2.0f;
      io.out(0) = s;
      io.out(1) = s;
    }
  }
};

int main() { 
  AlloApp app; 
  app.maker.start();
  app.start();
}
