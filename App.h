/**
    CSci-4611 Spring '14 Assignment #2 Pong 3D
Authors: Barbara and Ivana
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
    

protected:
    
    // Functions for collision detection.
    virtual void detectCollisionTable();
    virtual void detectCollisionPaddle();
    virtual void detectCollisionNet();
    
    bool isWithinTableBounds();
    bool isWithinNetBounds();
    
    virtual void game(RenderDevice* rd);
    virtual Vector3 updateBallPos(double time);
    
    virtual void resetBall();
    virtual void resetCollisions();
    virtual void resetMessage();
    virtual void resetScores();
    
    virtual void drawMessage(RenderDevice* rd);
    
    // This CoordinateFrame stores position and rotation data for the paddle.
    CoordinateFrame paddleFrame;
  
    // This vector stores the paddle's current velocity.
    Vector3 paddleVel;
    
    static const double GRAVITY;
    static const double BALL_RADIUS;
    static const double PADDLE_RADIUS;
    static const double TABLE_FRICTION;
    static const double RESTITUTION;
    static const double PADDLE_FRICTION;
    
    int playerScore;
    int opponentScore;
    String message;
    Color3 messageColor;
    
    
    double time;
    //do we need a total travel time? Say, if the paddle misses the ball or once it gets hit back
    
    Vector3 ballPos;
    Vector3 initBallVelocity;
    double x_pos;
	double y_pos;
	double z_pos;
    
    double initBallSpeed;
    double initBallToTableAngle;
    
    bool serve;
    
    Vector3 previousPosition;
    
    bool tableCollision;
    bool paddleCollision;
    bool netCollision;
    
    double paddleCollisionPos;
	Vector3 tableCollisionPos;
	double timeCollision;


	Color3 table_col;

	Vector3 newPaddlePos;
	Vector3 lastPaddlePos;

	Cylinder paddlePosCylinder;
    
};

#endif
