// Diarmid Flatley
// MAT 201B
// 2018 02 07
// diarmid@umail.ucsb.edu

// agents seek origin 
// collisions and seperation implemented
// TODO: alignment and cohesion

#include "allocore/io/al_App.hpp"
#include "Cuttlebone/Cuttlebone.hpp"
#include "common.hpp"

using namespace al;
using namespace std;

// some of these must be carefully balanced; i spent some time turning them.
// change them however you like, but make a note of these settings.
unsigned agentCount = 30;    // try 2, 5, 50, and 5000
float initialRadius = 50;        // initial condition
float initialSpeed = 50;         // initial condition
float timeStep = 0.005;         // keys change this value for effect
float scaleFactor = 0.1;         // resizes the entire scene
float sphereRadius = 10;  // increase this to make collisions more frequent
float freq = 0;
Vec3f target = Vec3f(0,0,0);
float seekForce = 1;
float desiredSeparation = 60;
float seperationForce = 1;
float alignForce = 1;
float cohesionForce = 1;
float maxforce = 0.7;
float maxspeed = 20;
float neighbordist = 50;


//Vec3f position [30];

Mesh sphere;  // global prototype; leave this alone

// helper function: makes a random vector
Vec3f r() { return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()); }

struct Agent {
  Pose pose;
  Vec3f velocity, acceleration;
  Color c;

  Agent() {
    pose.pos() = r() * initialRadius;
    velocity =
        // this will tend to spin stuff around the y axis
        Vec3f(0, 1, 0).cross(pose.pos()).normalize(initialSpeed);
    c = HSV(rnd::uniform(), 0.7, 1);
  }

  void update() {
    velocity += acceleration * timeStep;
    if (velocity.mag() > maxspeed) velocity.normalize(maxspeed);
    pose.pos() += velocity * timeStep;
    acceleration.zero();
  }

  void applyForce(Vec3f force) {
    acceleration += force;
  }

  Vec3f seek(Vec3f target) {
    Vec3f desired = (target - pose.pos());
    desired.normalize();
    desired *= maxspeed;
    Vec3f steer = (desired - velocity);
    if (steer.mag() > maxforce) steer.normalize(maxforce);
    return steer;
  }

  Vec3f separate (vector<Agent> agents) {
    Vec3f steer = Vec3f(0,0,0);
    int count = 0;
    for (Agent other : agents) {
      float d = (pose.pos() - other.pose.pos()).mag();
      if ((d > 0) && (d < desiredSeparation)) {
        Vec3f diff = pose.pos() - other.pose.pos();
        diff.normalize();
        diff /= d;
        steer += diff;
        count ++;
      }
    }
    if (count > 0) {
      steer /= (float)count;
    }
    if (steer.mag() > 0) {
      steer.normalize();
      steer *= maxspeed;
      steer -= velocity;
      if (steer.mag() > maxforce) steer.normalize(maxforce);
        }
    return steer;
  }

  Vec3f align (vector<Agent> agents) {
    Vec3f sum = Vec3f(0,0,0);
    int count = 0;
    for (Agent other : agents) {
      float d = (pose.pos() - other.pose.pos()).mag();
      if ((d > 0) && (d < neighbordist)) {
        sum += other.velocity;
        count++;
      }
    }
    if (count > 0) {  
      sum /= (float)count;
      sum.normalize();
      sum *= maxspeed;
      Vec3f steer = sum - velocity;
      if (steer.mag() > maxforce) steer.normalize(maxforce);
      return steer;
    } else {
      Vec3f noSteer = Vec3f(0,0,0);
      return noSteer;
    }
  } 

  Vec3f cohesion (vector<Agent> agents) {
    Vec3f sum(0,0,0);
    int count = 0;
    for (Agent other : agents) {
      float d = (pose.pos() - other.pose.pos()).mag();
      if ((d > 0) && (d < neighbordist)) {
        sum += other.pose.pos();
        count++;
      }
    }
    if (count > 0) {
      sum /= count;
      return seek(sum);
    } else {
      Vec3f noSteer = Vec3f(0,0,0);
      return noSteer;
    }
  }
           
void applyBehaviors(vector<Agent> agents) {
   Vec3f se = seek(target);
   Vec3f sep = separate(agents);
   Vec3f ali = align(agents);
   Vec3f co = cohesion(agents);
   
   se *= seekForce;
   sep *= seperationForce;
   ali *= alignForce;
   co *= cohesionForce;  

   applyForce(se);
   applyForce(sep);
   applyForce(ali);
   applyForce(co);
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
  cuttlebone::Maker<State> maker;

  vector<Agent> agent;

  MyApp() : maker("127.0.0.1") {
    addSphere(sphere, sphereRadius);
    sphere.generateNormals();

    light.pos(0, 0, 0);              // place the light
    nav().pos(0, 0, 30);             // place the viewer
    lens().far(400);                 // set the far clipping plane
    agent.resize(agentCount);  // make all the particles
    background(Color(0.07));

    initWindow();
    initAudio();
  }

  void onAnimate(double dt) {
    maker.set(state);
    
    if (!simulate)
      // skip the rest of this function
      return;

    // check for collisions between particles
    
    for (unsigned i = 0; i < agent.size(); ++i)
      for (unsigned j = 1 + i; j < agent.size(); ++j) {
        Agent& a = agent[i];
        Agent& b = agent[j];
        float d = (b.pose.pos() - a.pose.pos()).mag();
        if (d < 2 * sphereRadius) {
          cout << "COLLISION" << endl;
          // create spring and calculate spring force
          Agent& a = agent[i];
          Agent& b = agent[j];
          // vector between the two particles 
          Vec3f difference = (b.pose.pos() - a.pose.pos());
          // distance
          float d = difference.mag();
          float stretch = d - (2 * sphereRadius);
          // Hooke's Law
          difference.normalize();
          difference *= maxspeed;
          Vec3f bounce = difference * (0.8f * stretch); 
         // if (bounce.mag() > maxforce) bounce.normalize(maxforce);
          a.acceleration += (bounce);
          b.acceleration -= (bounce);
        } else {

          for (auto& a : agent) {
           a.applyBehaviors(agent);
           a.update();
          }
        }
      }
   // Vec3f position [agentCount];    
    for (unsigned i = 0; i < agent.size(); ++i) {
        state.position[i] = agent[i].pose.vec();   
    }
  }

  void onDraw(Graphics& g) {
    material();
    light();
    g.scale(scaleFactor);
    for (auto a : agent) a.draw(g);
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

int main() { 
  MyApp app;
  // MyApp().start();
  app.maker.start();
  app.start();
}
