#include "allocore/io/al_App.hpp"
using namespace al;
using namespace std;

struct MyApp : App {
    // You might name these differently. These are just my suggestions. You could
    // probably get away with using just one mesh, but why? Always write a program
    // the simplest way first. Only try to improve a program that works. Never try
    // to improve a program that does not yet exists and work.
    //
    Mesh plane, cube, cylinder, current, target, last;
    
    bool toPlane = false;
    bool toCube = false;
    bool toCylinder = false;
    bool toWiggles = false;

    MyApp() {
      // Choose your own image
      //                       
      const char* filename = "mat201b/color_spaces/double-rainbow.jpg";
    
      // We're putting image in the constructor because we don't need it after we
      // extract the pixel colors and positions.
      //                                       
      Image image;
    
      if (!image.load(filename)) {
        cerr << "ERROR" << endl;
        exit(1);
      }
    
      Array& array(image.array());
      cout << "Array.print: " << endl << "   ";
      array.print();
      assert(array.type() == AlloUInt8Ty);

      Image::RGBAPix<uint8_t> pixel;

      int W = array.width();
      int H = array.height();
      for (size_t row = 0; row < H; ++row) {
        for (size_t col = 0; col < W; ++col) {
          array.read(&pixel.r, col, row);
          Color color(pixel.r / 256.0f, pixel.g / 256.0f, pixel.b / 256.0f, 0.6);
          current.color(color);
          current.vertex(col / (float)W, row / (float)H, 0);
          plane.vertex(col / (float)W, row / (float)H, 0);
          cube.vertex(pixel.r / 256.0f, pixel.g / 256.0f, pixel.b / 256.f);
          HSV convert;
          convert = HSV(color);
         // cylinder.vertex(convert.h,convert.s,convert.v);
          cylinder.vertex(convert.s * sin(2 * M_PI * convert.h), convert.v,
              convert.s * cos(2 * M_PI * convert.h));
          last.vertex(col / (float)W, row / (float)H, 0);
          target.vertex(col / (float)W, row / (float)H, 0);
        }
      }

      // place the viewer back a bit, so she can see the meshes
      nav().pos(0, 0, 7);
    }

    double t = 0;
    double interp = 0;
    double duration = 1;
    void onAnimate(double dt) {

      if (toPlane) {
        toPlane = false;
        toWiggles = false;
        duration = t + 1000; 
        for (unsigned v = 0; v < current.vertices().size(); ++v) {
          target.vertices()[v] = plane.vertices()[v];
          last.vertices()[v] = current.vertices()[v];
        }
      }

      if (toCube) {
        toCube = false;
        toWiggles = false;
        duration = t + 1000;
        for (unsigned v = 0; v < current.vertices().size(); ++v) {
          target.vertices()[v] = cube.vertices()[v];
          last.vertices()[v] = current.vertices()[v];
        }
      }

      if (toCylinder) {
        toCylinder = false;
        toWiggles = false;
        duration = t + 1000;
        for (unsigned v = 0; v < current.vertices().size(); ++v) {
          target.vertices()[v] = cylinder.vertices()[v];
          last.vertices()[v] = current.vertices()[v];
        }
      }
       
      if (toWiggles) {
        for (unsigned v = 0; v < current.vertices().size(); ++v)
          target.vertices()[v] = current.vertices()[v] +=
          Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * 0.007f;
         } 

      interp = t / duration;
      if (interp > 1) interp = 1;
      for (unsigned v = 0; v < current.vertices().size(); ++v)
      current.vertices()[v] = last.vertices()[v].lerp(target.vertices()[v], interp);

      t += dt;
    }
 
    void onDraw(Graphics& g) {
      g.draw(current);
    }
 
    void onKeyDown(const ViewpointWindow&, const Keyboard& k) {
  
      switch (k.key()) {
        default:  // always have a default case; this one "falls through" to '1'
        case '1':
          toPlane = true;
          break;
        case '2':
          toCube = true;
          break;
        case '3':
          toCylinder = true;
          break;
        case '4':
          toWiggles = true;
          break;
      }
    }
  };

int main() {
  MyApp app;
  app.initWindow();
  app.start();
}
