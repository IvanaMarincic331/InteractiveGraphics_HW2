/**
    CSci-4611 Spring '14 Assignment #2 Pong 3D
**/

#ifndef App_h
#define App_h

#include <G3D/G3DAll.h>

class App : public GApp {
public:   
  App(const GApp::Settings& settings = GApp::Settings());
  virtual void onInit();
  virtual void onUserInput(UserInput *uinput); 
  virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt);
  virtual void onGraphics3D(RenderDevice* rd, Array< shared_ptr<Surface> >& surface);

  // Use these functions to access the current state of the paddle!
  Vector3 getPaddlePosition() { return paddleFrame.translation; }
  Vector3 getPaddleNormal() { return Vector3(0,0,-1); }
  Vector3 getPaddleVelocity() { return paddleVel; }
  
  // Functions for collision detection.
  virtual void detectCollisionTable();
  virtual void detectCollisionPaddle();

protected:
  // This CoordinateFrame stores position and rotation data for the paddle.
  CoordinateFrame paddleFrame;
  
  // This vector stores the paddle's current velocity.
  Vector3 paddleVel;
   
  virtual Vector3 ballPos(double time);
  double ballRadius;
  bool serve;

  static const double GRAVITY;
  double time;
    
  Vector3 position;
  Vector3 previousPosition;
  Vector3 velocity;
  Vector3 initialVel;

  bool tableCollision;
  bool paddleCollision;
  double paddleCollisionPos;
};

#endif
