// Diarmid Flatley
// 2018-03-20
// MAT201B
// Final Project

// Pitch Lattice Sequencer
// Copyright (C) 2018 Diarmid Flatley

/*
This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "common.hpp"

#include "alloutil/al_OmniStereoGraphicsRenderer.hpp"

using namespace std;
using namespace al;

struct Node {
  Vec3f position = Vec3f(0, 0, 0);
  vector<int> connections;
  float mixAmount = 1.0f;
  Color active = {1, 0, 0, 1};
  Color inactive = {1, 1, 1, 1};

  Node() {}

  Node(Vec3f initPos, vector<int> initConnections) {
    position = initPos;
    connections = initConnections;
  }

  void draw(Graphics& g, Mesh& m) {
    g.pushMatrix();
    g.translate(position);
    g.color(active.mix(inactive, mixAmount));
    g.draw(m);
    g.popMatrix();
  }
};

struct MyCursor {
  Vec3f position;

  MyCursor() {}

  void draw(Graphics& g, Mesh& m) {
    g.pushMatrix();
    g.translate(position);
    g.color(1, 0, 0, 1);
    g.draw(m);
    g.popMatrix();
  }
};

struct Strut {
  Vec3f start, end;

  Strut() {}

  Strut(Node initStart, Node initEnd) {
    start = initStart.position;
    end = initEnd.position;
  }

  void set(Node setStart, Node setEnd) {
    start = setStart.position;
    end = setEnd.position;
  }

  void draw(Graphics& g, Mesh& m) {
    g.color(1, 1, 1, 1);
    m.reset();
    m.primitive(Graphics::LINES);
    m.stroke(2);
    m.vertex(start);
    m.vertex(end);
    g.draw(m);
  }
};

struct AlloApp : OmniStereoGraphicsRenderer {
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

  MyCursor myCursor;

  State state;
  cuttlebone::Taker<State> taker;

  AlloApp() {
    nav().pos(0, 0, 20);
    light.pos(0, 20, 0);

    lens().near(0.1);
    lens().far(1000);

    addSphereWithTexcoords(backgroundMesh);

    addSphere(sphere, 0.1);

    sphere.generateNormals();

    vertex[0] = {-1, 0, 0};
    vertex[1] = {0, 0, -1};
    vertex[2] = {0, 1, 0};
    vertex[3] = {1, 0, 0};
    vertex[4] = {0, 0, 1};
    vertex[5] = {0, -1, 0};
    vertex[6] = {-1, 1, -1};
    vertex[7] = {1, 1, -1};
    vertex[8] = {1, 1, 1};
    vertex[9] = {-1, 1, 1};
    vertex[10] = {-1, -1, -1};
    vertex[11] = {1, -1, -1};
    vertex[12] = {1, -1, 1};
    vertex[13] = {-1, -1, 1};

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
    connections[13] = {0, 4, 5};

    for (int i = 0; i < 14; i++) {
      node[i] = {vertex[i], connections[i]};
    }

    int strutCount = 0;
    for (int i = 0; i < 14; i++) {
      for (int j = 0; j < node[i].connections.size(); j++) {
        Strut* strut = new Strut;
        struts.push_back(strut);
        struts[strutCount]->set(node[i], node[node[i].connections[j]]);
        strutCount++;
      }
    }

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
    myCursor.position = state.cursorPosition;
    nav().pos(state.navPosition);
    nav().quat(state.navOrientation);
    pose = nav();
    for (int i = 0; i < 14; i++) {
      node[i].mixAmount = state.colorMix[i];
    }
  }

  void onDraw(Graphics& g) {
    // you may need these later
    shader().uniform("texture", 1.0);
    shader().uniform("lighting", 0.0);

    // g.lighting(false);
    g.depthMask(false);

    g.pushMatrix();
    g.translate(nav().pos());
    g.rotate(180, 0, 0, 1);
    backgroundTexture.bind();
    g.color(0, 0, 0);
    g.draw(backgroundMesh);
    backgroundTexture.unbind();
    g.popMatrix();

    g.depthMask(true);
    material();
    light();

    shader().uniform("texture", 0.0);
    shader().uniform("lighting", 0.5);

    for (unsigned i = 0; i < 14; i++) {
      node[i].draw(g, sphere);
    }

    for (unsigned i = 0; i < struts.size(); i++) {
      struts[i]->draw(g, line);
    }

    myCursor.draw(g, sphere);
  }
};

int main() {
  AlloApp app;
  app.taker.start();
  app.start();
}
