// Diarmid Flatley
// 2018-2-26
// MAT 201B
// tuning lattice


#include "allocore/io/al_App.hpp"
#include <cmath>
#include <vector>
#include <iostream>
#include "allocore/io/al_App.hpp"
#include "Cuttlebone/Cuttlebone.hpp"
 #include "common.hpp"

using namespace std;
using namespace al;

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
    increment = 0.1f;
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
   
   Mesh backgroundMesh;
   Texture backgroundTexture;

   Mesh sphere;
   Mesh line;
   Node node[14];
   Vec3f vertex[14];  
   vector<Strut*> struts;
   vector<int> connections[14];
   float frequency[14] = {350, 250, 218.75, 375, 262.5, 300, 
     291.66666, 312.5, 328.125, 214.285715, 
     200, 306.25, 225, 210};

   Cursor cursor;

   State state;
   cuttlebone::Taker<State> taker;

  AlloApp() {
    nav().pos(0, 0, 20);
    light.pos(0,0,0);

    lens().near(0.1);
    lens().far(1000);

    addSphereWithTexcoords(backgroundMesh);

    addSphere(sphere, 0.1);

    sphere.generateNormals();
    
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
     // state.vertex[i] = vertex[i];
     // state.connections[i] = connections[i];
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

    initWindow();
    initAudio();

    Image image;
    SearchPaths searchPaths;
    searchPaths.addSearchPath("./");
    string filename = searchPaths.find("blue.jpg").filepath();
    if (image.load(filename)) {
      cout << "Read image from " << filename << endl;
    } else {
      cout << "Failed to read image from " << filename << "!!!" << endl;
      exit(-1);
    }
    
    
    backgroundTexture.allocate(image.array());
  }

  void onAnimate(double dt) {
    taker.get(state);
    cursor.position = state.cursorPosition;
    nav().pos(state.navPosition);
    nav().quat(state.navOrientation);
  }

  void onDraw(Graphics& g) {
    g.lighting(false);
    g.depthMask(false);
    
    g.pushMatrix();
    g.translate(nav().pos());
    g.rotate(180, 0, 0, 1);
    backgroundTexture.bind();
    g.color(1, 1, 1);
    g.draw(backgroundMesh);
    backgroundTexture.unbind();
    g.popMatrix();

    g.depthMask(true);
    material();
    light();
    
    g.color(1, 1, 1);

    for (unsigned i = 0; i < 14; i++) {
      node[i].draw(g,sphere);
    }

    for (unsigned i = 0; i < struts.size(); i++) {
      struts[i]->draw(g, line);
    }
    
    g.color(1, 0, 0);
    cursor.draw(g, sphere);
  
  }

};

int main() { 
  AlloApp app; 
  app.taker.start();
  app.start();
}
