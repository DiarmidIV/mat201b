// Diarmid Flatley
// 2018-2-26
// MAT 201B
// tuning lattice

#include "allocore/io/al_App.hpp"
#include <cmath>
using namespace al;


Vec3f midpoint (Vec3f a, Vec3f b) {
    Vec3f middle;
  middle.x = (a.x - b.x) / 2;
  middle.y = (a.y - b.y) / 2;
  middle.z = (a.z - b.z) / 2;
  return middle;
}

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

struct Cursor {
  Vec3f position;
  float increment;
  float speed;
  Vec3f start;
  Vec3f end;
             
  Cursor() {
    increment = 0;
    speed = 0.01;
  }

  void set(Node init) {
    position = init.position;
  }

  void update(Node setStart, Node setEnd) {
    start = setStart.position;
    end = setEnd.position;

    increment += speed;
    if (increment > 1) {
      //  start = end;
      //      end =
      increment -= 1;
    }
    position = start.lerp(end, increment);
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
  float radians;
  Vec3f XYZ;

  Strut() {}

  void set(Vec3f setPosition, float setAngle, Vec3f setXYZ) {
    Vec3f position = setPosition;
    float radians = setAngle;// (setAngle * M_PI)/180;
    Vec3f XYZ = setXYZ;
  }

  void draw(Graphics& g, Mesh& m) {
    g.pushMatrix();
    g.translate(position);
    g.rotate(radians,XYZ);
    g.draw(m);
    g.popMatrix();
  }
};

struct Lines {
  void draw(Graphics& g, Mesh& m, Vec3f vertex[]) {
    m.primitive(Graphics::LINE_LOOP);
    m.stroke(2);

    m.vertex(vertex[1]);
    m.vertex(vertex[2]);
    m.vertex(vertex[5]);
    m.vertex(vertex[0]);
    m.vertex(vertex[2]);
    m.vertex(vertex[4]);
    m.vertex(vertex[0]);
    m.vertex(vertex[3]);
    m.vertex(vertex[4]);
    m.vertex(vertex[1]);
    m.vertex(vertex[3]);
    m.vertex(vertex[5]);
    g.draw(m);
  }
};


struct AlloApp : App {
   Material material;
   Light light;
   Mesh sphere;
   Mesh line;
   Mesh cylinder;

   Vec3f center;

   Node node[6];
   Vec3f vertex[6];  
   Strut strut[12]; 
  
   Lines lines;

   Cursor cursor;

   double time;

  AlloApp() {
    nav().pos(0, 0, 20);
    light.pos(0,0,0);

    addSphere(sphere, 0.1);
    addCylinder(cylinder, 0.01, 1);

    sphere.generateNormals();
    cylinder.generateNormals();
    
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

    cursor.set(node[1]);

    time = 0.0;

    initWindow();
  }

  void onAnimate(double dt) {
    time += dt;
    cursor.update(node[1],node[2]);
  }

  void onDraw(Graphics& g) {
   
    material();
    light();

    for (unsigned i = 0; i < 6; i++) {
    node[i].draw(g,sphere);
    }    
    /*  
    node[0].draw(g,sphere);
    node[1].draw(g,sphere);
    node[2].draw(g,sphere);
    node[3].draw(g,sphere);
    node[4].draw(g,sphere);
    node[5].draw(g,sphere);
    */

    strut[0].draw(g,cylinder);

    cursor.draw(g, sphere);
/*
    line.primitive(Graphics::LINE_LOOP);
    line.stroke(2);

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
*/
    lines.draw(g,line,vertex);
  }
};

int main() { AlloApp().start(); }
