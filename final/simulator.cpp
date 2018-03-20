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

#include "Gamma/Envelope.h"
#include "Gamma/Filter.h"
#include "Gamma/Oscillator.h"
#include "allocore/io/al_App.hpp"

#include "GLV/glv.h"
#include "alloGLV/al_ControlGLV.hpp"
#include "alloutil/al_AlloSphereAudioSpatializer.hpp"
#include "alloutil/al_Simulator.hpp"

using namespace std;
using namespace al;

Vec3f midpoint(Vec3f a, Vec3f b, Vec3f c) {
  Vec3f middle;
  middle.x = (a.x + b.x + c.x) / 3;
  middle.y = (a.y + b.y + c.y) / 3;
  middle.z = (a.z + b.z + c.z) / 3;
  return middle;
}

float distance(Vec3f a, Vec3f b) {
  float d;
  d = sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2) + pow((a.z - b.z), 2));
  return d;
}

struct Node {
  Vec3f position = Vec3f(0, 0, 0);
  vector<int> connections;
  float frequency;
  float mixAmount = 1.0;
  Color active = {1, 0, 0, 1};
  Color inactive = {1, 1, 1, 1};

  Node() {}

  Node(Vec3f initPos, vector<int> initConnections, float initFreq) {
    position = initPos;
    connections = initConnections;
    frequency = initFreq;
  }

  void update(Vec3f cursorPos) {
    if (distance(position, cursorPos) < 0.1f)
      mixAmount = 0;
    else {
      mixAmount += 0.01f;
      if (mixAmount > 1.0f) mixAmount = 1.0f;
    }
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
  float counter;
  float increment;
  float currentFrequency;
  Node start;
  Node end;
  bool trigger;

  MyCursor() {
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
    position = startPos.lerp(endPos, counter);
    counter += increment;
    if (counter > 1) {
      trigger = true;
      unsigned i = rand() % end.connections.size();
      int next = end.connections[i];
      this->set(end, node[next]);
      counter -= 1;
    }
  }

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

  Strut(Node initStart, Node initEnd, Mesh& m) {
    start = initStart.position;
    end = initEnd.position;
    m.primitive(Graphics::LINES);
    m.stroke(2);
    m.vertex(start);
    m.vertex(end);
  }

  void set(Node setStart, Node setEnd, Mesh& m) {
    start = setStart.position;
    end = setEnd.position;
    m.primitive(Graphics::LINES);
    m.stroke(2);
    m.vertex(start);
    m.vertex(end);
  }

  void draw(Graphics& g, Mesh& m) {
    g.color(1, 1, 1, 1);
    g.draw(m);
  }
};

// struct AlloApp : App {
struct AlloApp : App, AlloSphereAudioSpatializer, InterfaceServerClient {
  Material material;
  Light light;
  Mesh sphere;
  Mesh line;

  GLVBinding gui;
  glv::Slider slider_speed;
  glv::Slider slider_attack;
  glv::Slider slider_decay;
  glv::Table layout;

  Vec3f origin = {0, 0, 0};

  Node node[14];
  Vec3f vertex[14];
  vector<Strut*> struts;
  vector<int> connections[14];
  float mixAmounts[14];

  float frequency[14];

  float fundamental = 200.0f;

  float a = 1;
  float b = 1.12;
  float c = 1.37;
  float d = 1.53;

  MyCursor myCursor;

  gam::Sine<> sine[8];
  float timbreFrequency[8] = {2.3, 3.8, 5.2, 5.8, 0, 0, 0, 0};
  float timbreAmplitude[8] = {0.28, 0.23, 0.16, 0.1, 0, 0, 0, 0};
  gam::AD<> env;
  float attack;
  float decay;

  State state;
  cuttlebone::Maker<State> maker;

  AlloApp()
      : maker(Simulator::defaultBroadcastIP()),
        InterfaceServerClient(Simulator::defaultInterfaceServerIP()) {
    nav().pos(0, 0, 20);
    light.pos(0, 20, 0);

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

    frequency[0] = a * d;
    frequency[1] = a * b;
    frequency[2] = b * d;
    frequency[3] = b * c;
    frequency[4] = c * d;
    frequency[5] = a * c;
    frequency[6] = (a * b * d) / c;
    frequency[7] = b * b;
    frequency[8] = (b * c * d) / a;
    frequency[9] = d * d;
    frequency[10] = a * a;
    frequency[11] = (a * b * c) / d;
    frequency[12] = c * c;
    frequency[13] = (a * c * d) / b;

    for (int i = 0; i < 14; i++) {
      node[i] = {vertex[i], connections[i], frequency[i]};
    }

    int strutCount = 0;
    for (int i = 0; i < 14; i++) {
      for (int j = 0; j < node[i].connections.size(); j++) {
        Strut* strut = new Strut;
        struts.push_back(strut);
        struts[strutCount]->set(node[i], node[node[i].connections[j]], line);
        strutCount++;
      }
    }

    myCursor.set(node[0], node[2]);

    initWindow();

    gui.bindTo(window());
    gui.style().color.set(glv::Color(0.7), 0.5);
    layout.arrangement("x");

    slider_speed.setValue(0.1);
    layout << slider_speed;
    layout << new glv::Label("cursor speed");

    slider_attack.setValue(0.01);
    layout << slider_attack;
    layout << new glv::Label("attack");

    slider_decay.setValue(0.3);
    layout << slider_decay;
    layout << new glv::Label("decay");

    layout.arrange();
    gui << layout;

    // audio
    AlloSphereAudioSpatializer::initAudio();
    AlloSphereAudioSpatializer::initSpatialization();
    // if gamma
    gam::Sync::master().spu(AlloSphereAudioSpatializer::audioIO().fps());
    scene()->addSource(aSoundSource);
    aSoundSource.dopplerType(DOPPLER_NONE);
    // scene()->usePerSampleProcessing(true);
    scene()->usePerSampleProcessing(false);
  }

  void onAnimate(double dt) {
    while (InterfaceServerClient::oscRecv().recv())
      ;  // XXX

    myCursor.increment = slider_speed.getValue() * slider_speed.getValue();
    attack = slider_attack.getValue() * slider_attack.getValue();
    decay = slider_decay.getValue() * slider_decay.getValue();
    myCursor.update(node);

    for (unsigned i = 0; i < 14; i++) {
      node[i].update(myCursor.position);
      state.colorMix[i] = node[i].mixAmount;
    }

    state.cursorPosition = myCursor.position;

    state.navPosition = nav().pos();
    state.navOrientation = nav().quat();
    maker.set(state);
  }

  void onDraw(Graphics& g) {
    material();
    light();

    for (unsigned i = 0; i < 14; i++) {
      node[i].draw(g, sphere);
    }

    myCursor.draw(g, sphere);

    for (unsigned i = 0; i < struts.size(); i++) {
      struts[i]->draw(g, line);
    }
  }

  SoundSource aSoundSource;
  virtual void onSound(al::AudioIOData& io) {
    aSoundSource.pose().pos(myCursor.position);
    while (io()) {
      if (myCursor.trigger == true) {
        env.attack(attack);
        env.decay(decay);
        env.amp(1.0);
        env.reset();
        myCursor.trigger = false;
      }

      float s = 0;

      for (int i = 0; i < 8; i++) {
        sine[i].freq(myCursor.currentFrequency * fundamental *
                     timbreFrequency[i]);
        s += sine[i]() * timbreAmplitude[i];
      }

      s *= env() / 8.0f;
      // s *= 1 / (distance(nav().pos(), origin) * distance(nav().pos(),
      // origin));

      // XXX -- this is broken the line below should work, but it sounds
      // terrible
      aSoundSource.writeSample(s);
      //
      // these two lines should go onces the lien above works
      // io.out(0) = s;
      // io.out(1) = s;
    }
    listener()->pose(nav());
    scene()->render(io);
  }
};

int main() {
  AlloApp app;
  app.AlloSphereAudioSpatializer::audioIO().start();
  app.InterfaceServerClient::connect();
  app.maker.start();
  app.start();
}
