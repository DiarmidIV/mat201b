// MAT201B
// Fall 2015
// Author(s): Karl Yerkes
//
// Shows how to:
// - Use a Mesh to draw something simple
//

#include "allocore/io/al_App.hpp"
using namespace al;

struct Node {
  Vec3f position = Vec3f(0,0,0);
  
  Node() {}

  void set(float x, float y, float z) {
    position = Vec3f(x,y,z);
  }

  void set(Vec3f setPos) {
    position = setPos;
  }

  void draw(Graphics& g, Mesh& m) {
    g.pushMatrix();
    g.translate(position);
    g.draw(m);
    g.popMatrix();
  }
};

struct Strut {
  Vec3f position;
  float angle;
  Vec3f XYZ;

  Strut() {}

  void set(Vec3f setPosition, float setAngle, Vec3f setXYZ) {
    position = setPosition;
    angle = setAngle;
    XYZ = setXYZ;
  }

  void draw(Graphics& g, Mesh& m) {
    g.pushMatrix();
    g.translate(position);
    g.rotate(angle,XYZ);
    g.draw(m);
    g.popMatrix();
  }
};

struct AlloApp : App {
   Material material;
   Light light;
   Mesh sphere;
   Mesh line;

   Node node[6];
   Vec3f vertex[6];  
   Strut strut[12]; 
    
  AlloApp() {
    nav().pos(0, 0, 20);
    light.pos(0,0,0);

    addSphere(sphere, 0.1);

    sphere.generateNormals();
    
    vertex[0].set(1,0,0);
    vertex[1].set(-1,0,0);
    vertex[2].set(0,1,0);
    vertex[3].set(0,-1,0);
    vertex[4].set(0,0,1);
    vertex[5].set(0,0,-1);
   
    node[0].set(vertex[0]);
    node[1].set(vertex[1]);
    node[2].set(vertex[2]);
    node[3].set(vertex[3]);
    node[4].set(vertex[4]);
    node[5].set(vertex[5]);
     
    initWindow();
  }



  void onDraw(Graphics& g) {
   

    material();
    light();

    node[0].draw(g,sphere);
    node[1].draw(g,sphere);
    node[2].draw(g,sphere);
    node[3].draw(g,sphere);
    node[4].draw(g,sphere);
    node[5].draw(g,sphere);
  
    line.primitive(Graphics::LINE_LOOP);

    line.vertex(vertex[1]);
    line.vertex(vertex[2]);
    line.vertex(vertex[5]);
    line.vertex(vertex[0]);
    line.vertex(vertex[2]);
    line.vertex(vertex[4]);
    line.vertex(vertex[0]);
    line.vertex(vertex[3]);
    line.vertex(vertex[4]);
    line.vertex(vertex[1]);
    line.vertex(vertex[3]);
    line.vertex(vertex[5]);
    g.draw(line);

//    strut[0].draw(g,line);
  }
};

int main() { AlloApp().start(); }
