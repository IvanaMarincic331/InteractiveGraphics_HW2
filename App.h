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

	// Functions for collision detection.
    virtual void detectCollisionTable();
    virtual void detectCollisionPaddle();
    virtual void detectCollisionNet();
    
	// Functions for detecting if the ball is within the table and net bounds
    bool isWithinTableBounds();
    bool isWithinNetBounds();
    
    virtual void game(RenderDevice* rd); // renders the ball
    virtual Vector3 updateBallPos(double time); // return ball position based on time
    
	// Reset the game
    virtual void resetBall();
    virtual void resetCollisions();
    virtual void resetMessage();
    virtual void resetScores();
    
	// Draw win/lose message on the screen
    virtual void drawMessage(RenderDevice* rd);
   

protected: 
	// Numerical constants
    static const double GRAVITY;
    static const double BALL_RADIUS;
    static const double PADDLE_RADIUS;
    static const double TABLE_FRICTION;
    static const double RESTITUTION;
    static const double PADDLE_FRICTION;
    
    double time; // game time (slower than real time)

	Vector3 ballPos; // current ball position
    Vector3 initBallVelocity; // intial ball velocity
    double x_pos; // the initial launch x position
	double y_pos; // the initial launch y position
	double z_pos; // the initial launch z position
  
    CoordinateFrame paddleFrame; // stores position and rotation data for the paddle.1
    Vector3 paddleVel; // paddle's current velocity.
    
	bool serve; // is the ball in play
	bool tableCollision; // has the ball collided with the table
    bool paddleCollision; // has the ball collided with the table
    bool netCollision; // has the ball collided with the net

	Vector3 newPaddlePos; // the current paddle position
	Vector3 lastPaddlePos; // the last paddle position
	Cylinder paddlePosCylinder; // a cylinder between the last and the current paddle position

    int playerScore; // player/user's score
    int opponentScore; // opponent/computer's score
    String message; // win/lose message to be printed in the screen
    Color3 messageColor;
};

#endif
