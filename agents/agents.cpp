// Diarmid Flatley
// MAT 201B
// 2018 02 07
// diarmid@umail.ucsb.edu

// agents seek origin 
// collisions and seperation implemented
// TODO: alignment and cohesion

#include "allocore/io/al_App.hpp"
using namespace al;
using namespace std;

// some of these must be carefully balanced; i spent some time turning them.
// change them however you like, but make a note of these settings.
unsigned particleCount = 50;    // try 2, 5, 50, and 5000
float maximumAcceleration = 30;  // prevents explosion, loss of particles
float initialRadius = 50;        // initial condition
float initialSpeed = 50;         // initial condition
float gravityFactor = 1e6;       // see Gravitational Constant
float timeStep = 0.0625;         // keys change this value for effect
float scaleFactor = 0.1;         // resizes the entire scene
float sphereRadius = 10;  // increase this to make collisions more frequent
float freq = 0;
Vec3f target = Vec3f(0,0,0);
float desiredSeperation = 100;
float seperationForce = 30;

//Mesh sphere;  // global prototype; leave this alone
Mesh cube;

// helper function: makes a random vector
Vec3f r() { return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()); }

struct Particle {
  Pose pose;
  Vec3f velocity, acceleration;
  Color c;
  Particle() {
    pose.pos() = r() * initialRadius;
    velocity =
        // this will tend to spin stuff around the y axis
        Vec3f(0, 1, 0).cross(pose.pos()).normalize(initialSpeed);
    c = HSV(rnd::uniform(), 0.7, 1);
  }
  void draw(Graphics& g) {
    g.pushMatrix();
    g.translate(pose.pos());
    g.rotate(pose.quat());
    g.color(c);
    g.draw(cube);
    g.popMatrix();
  }
};

struct MyApp : App {
  Material material;
  Light light;
  bool simulate = true;

  vector<Particle> particle;

  MyApp() {
 //  addSphere(sphere, sphereRadius);
 //  sphere.generateNormals();
   addIcosahedron(cube, sphereRadius);
   cube.generateNormals(); 

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
        Vec3f difference = (b.pose.pos() - a.pose.pos());
        float d = difference.mag();
        if (d < 2 * sphereRadius) {
          cout << "COLLISION" << endl;
          freq = (rand() % 100) + 100; // if collision is detected randomly change saw frequency
         
          // create spring and calculate spring force
          Particle& a = particle[i];
          Particle& b = particle[j];
          // vector between the two particles 
          Vec3f difference = (b.pose.pos() - a.pose.pos());
          // distance
          float d = difference.mag();
          float stretch = d - (2 * sphereRadius);
          // Hooke's Law
          difference.normalize();
          Vec3f bounce = difference * (200000000.0f * stretch); 
          a.acceleration += bounce;
          b.acceleration -= bounce;
        } else {

        //seeking behavior   
        for (auto& p : particle) {
        Vec3f desired = (target - p.pose.pos());
        Vec3f steer = (desired - p.velocity);
        p.acceleration += steer;
        } 

        // seperation
        for (unsigned i = 0; i < particle.size(); ++i) {
          Vec3f adder = Vec3f(0,0,0);
          unsigned count=0;
          for (unsigned j = 1 + i; j < particle.size(); ++j) {
            Particle& a = particle[i];
            Particle& b = particle[j];
            Vec3f difference = (b.pose.pos() - a.pose.pos());
            float  d = difference.mag();
           // cout << d <<endl;
            if ((d < desiredSeperation))
             // difference.normalize();
              // cout << difference << endl;
              adder += difference;
              // cout << adder << endl;
              count++;
            }
            if (count > 0) {
              adder/count;
              // cout << adder <<endl;
              Vec3f steer = (adder - particle[i].velocity);
              particle[i].acceleration += (steer * seperationForce);  
            }
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
    for (auto& p : particle) p.pose.pos() += p.velocity * timeStep;
    for (auto& p : particle) p.velocity += p.acceleration * timeStep;
    for (auto& p : particle) { p.pose.faceToward(target,1.); } // fake orientation
    for (auto& p : particle) p.acceleration.zero();  // XXX zero accelerations
  }

  void onDraw(Graphics& g) {
    material();
    light();
    g.scale(scaleFactor);
    for (auto p : particle) p.draw(g);
  }


  void onSound(AudioIOData& io) {
   // io();
   // io.out(0) = 1;
   // io.out(1) = 1;
   
//   while(io()) {
//      saw.frequency(300+(freq),44100);
      
  //    io.out(0) = saw()*0.3;
    //  io.out(1) = saw()*0.3;
  //  }
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
