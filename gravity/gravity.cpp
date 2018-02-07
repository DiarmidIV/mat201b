// Diarmid Flatley
// MAT 201B
// 2018 02 07
// diarmid@umail.ucsb.edu

#include "allocore/io/al_App.hpp"
using namespace al;
using namespace std;

// some of these must be carefully balanced; i spent some time turning them.
// change them however you like, but make a note of these settings.
unsigned particleCount = 10;    // try 2, 5, 50, and 5000
float maximumAcceleration = 30;  // prevents explosion, loss of particles
float initialRadius = 50;        // initial condition
float initialSpeed = 50;         // initial condition
float gravityFactor = 1e6;       // see Gravitational Constant
float timeStep = 0.0625;         // keys change this value for effect
float scaleFactor = 0.1;         // resizes the entire scene
float sphereRadius = 20;  // increase this to make collisions more frequent
bool collision = false;
float freq = 0;

Mesh sphere;  // global prototype; leave this alone


// rudimentary sound stuff
struct Phasor {
  float phase = 0, increment = 0.001;
  void frequency(float hz, float sampleRate) { increment = hz / sampleRate; }
  float getNextSample() {
    float returnValue = phase;
    phase += increment;
    if (phase > 1) phase -= 1;
      return returnValue;
  }
  float operator()() { return getNextSample(); }
};

struct Sawtooth : Phasor {
  float getNextSample() { return 2 * Phasor::getNextSample() - 1; }
  float operator()() { return getNextSample(); }
};


// helper function: makes a random vector
Vec3f r() { return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()); }

struct Particle {
  Vec3f position, velocity, acceleration;
  Color c;
  Particle() {
    position = r() * initialRadius;
    velocity =
        // this will tend to spin stuff around the y axis
        Vec3f(0, 1, 0).cross(position).normalize(initialSpeed);
    c = HSV(rnd::uniform(), 0.7, 1);
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
  bool simulate = true;

  vector<Particle> particle;

  MyApp() {
    addSphere(sphere, sphereRadius);
    sphere.generateNormals();
    light.pos(0, 0, 0);              // place the light
    nav().pos(0, 0, 30);             // place the viewer
    lens().far(400);                 // set the far clipping plane
    particle.resize(particleCount);  // make all the particles
    background(Color(0.07));

    initWindow();
    initAudio();
  }

  void onAnimate(double dt) {
    if (!simulate)
      // skip the rest of this function
      return;

    // check for collisions between particles
    for (unsigned i = 0; i < particle.size(); ++i)
      for (unsigned j = 1 + i; j < particle.size(); ++j) {
        Particle& a = particle[i];
        Particle& b = particle[j];
        Vec3f difference = (b.position - a.position);
        float d = difference.mag();
        if (d < 2 * sphereRadius) {
          cout << "COLLISION" << endl;
          freq = (rand() % 100) + 100; // if collision is detected randomly change saw frequency
         
          // create spring and calculate spring force
          Particle& a = particle[i];
          Particle& b = particle[j];
          // vector between the two particles 
          Vec3f difference = (b.position - a.position);
          // distance
          float d = difference.mag();
          float stretch = d - (2 * sphereRadius);
          // Hooke's Law
          difference.normalize();
          Vec3f bounce = difference * (200000000.0f * stretch); 
          a.acceleration += bounce;
          b.acceleration -= bounce;
        } else {

    for (unsigned i = 0; i < particle.size(); ++i)
      for (unsigned j = 1 + i; j < particle.size(); ++j) {
        Particle& a = particle[i];
        Particle& b = particle[j];
        Vec3f difference = (b.position - a.position);
        float d = difference.mag();
        // F = ma where m=1
        Vec3f acceleration = difference / (d * d * d) * gravityFactor;
        // equal and opposite force (symmetrical)
        a.acceleration += acceleration;
        b.acceleration -= acceleration;
      }
        }
      }

    // Limit acceleration
    unsigned limitCount = 0;
    for (auto& p : particle)
      if (p.acceleration.mag() > maximumAcceleration) {
        p.acceleration.normalize(maximumAcceleration);
        limitCount++;
      }
    printf("%u of %u\n", limitCount, particle.size());

    // Euler's Method; Keep the time step small
    for (auto& p : particle) p.position += p.velocity * timeStep;
    for (auto& p : particle) p.velocity += p.acceleration * timeStep;
    for (auto& p : particle) p.acceleration.zero();  // XXX zero accelerations
  }

  void onDraw(Graphics& g) {
    material();
    light();
    g.scale(scaleFactor);
    for (auto p : particle) p.draw(g);
  }

Sawtooth saw;

  void onSound(AudioIOData& io) {
   // io();
   // io.out(0) = 1;
   // io.out(1) = 1;
   
   while(io()) {
      saw.frequency(300+(freq),44100);
      
      io.out(0) = saw()*0.3;
      io.out(1) = saw()*0.3;
    }
      }
  


  void onKeyDown(const ViewpointWindow&, const Keyboard& k) {
    switch (k.key()) {
      default:
      case '1':
        // reverse time
        timeStep *= -1;
        break;
      case '2':
        // speed up time
        if (timeStep < 1) timeStep *= 2;
        break;
      case '3':
        // slow down time
        if (timeStep > 0.0005) timeStep /= 2;
        break;
      case '4':
        // pause the simulation
        simulate = !simulate;
        break;
    }
  }
};

int main() { MyApp().start(); }
