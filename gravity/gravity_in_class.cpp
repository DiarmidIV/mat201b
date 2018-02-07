#include "allocore/io/al_App.hpp"
using namespace al;
using namespace std;

Mesh sphere;

Vec3f r() {
      return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
        }

struct Particle {
  Vec3f position, velocity, acceleration;
  Color c;

Particle() {
  position = r() * 2;
  velocity = r() * 0.003;
  c = HSV(rnd::uniformS(),1,1);
}  
  void draw(Graphics& g) {
    g.pushMatrix();
    g.translate(position);
    g.color(c);
    g.draw(sphere);
    g.popMatrix();
  }
};


struct MyApp : App {
  Material material;
  Light light;

  vector<Particle> particle;

  MyApp() {
    nav().pos(0,0,7);
    addSphere(sphere);
    sphere.generateNormals();
    light.pos(5,5,5);

    particle.resize(10);
  }
  double timeStep = 0.01;
  double t = 0;
  void onAnimate(double dt) {
    t += dt;

    for (unsigned i = 0; i < particle.size(); ++i)
      for(unsigned j = 1 + i; j < particle.size(); ++j) {
        Particle& a = particle[i];
        Particle& b = particle[j];
        Vec3f direction = (b.position - a.position);
        float distance = (direction.mag());
        direction.normalize();
        Vec3f acceleration = direction / (distance * distance) * timeStep;
        a.acceleration += acceleration;
        b.acceleration -= acceleration;
      }
    float max = 0.001;
    for (auto& p : particle) 
      if (p.acceleration.mag() > max) p.acceleration.normalize(max);

    for (auto& p : particle) p.position += p.velocity;
    for (auto& p : particle) p.velocity += p.acceleration;
  }


  void onDraw(Graphics& g) {
    material();
    light();
    for (auto p : particle) 
      p.draw(g);
  }
};

int main() {
  MyApp app;
  app.initWindow();
  app.start();
}
