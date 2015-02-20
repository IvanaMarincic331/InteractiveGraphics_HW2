/**
Comp 394 S15 Assignment #2 Ping Pong 3D
Athors: Barbara and Ivana
**/

#include "App.h"
#include <iostream>
using namespace std;

// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

const double App::GRAVITY = 981;
const double App::BALL_RADIUS = 2.0;
const double App::PADDLE_RADIUS = 8.0;

int main(int argc, const char* argv[]) {
	(void)argc; (void)argv;
	GApp::Settings settings(argc, argv);

	// Change the window and other startup parameters by modifying the
	// settings class.  For example:
	settings.window.width       = 1280; 
	settings.window.height      = 720;

	return App(settings).run();
}


App::App(const GApp::Settings& settings) : GApp(settings) {
	renderDevice->setColorClearValue(Color3(0.2, 0.2, 0.2));
	renderDevice->setSwapBuffersAutomatically(true);
}


void App::onInit() {
	GApp::onInit();
	// Turn on the developer HUD
	createDeveloperHUD();
	debugWindow->setVisible(false);
	developerWindow->setVisible(false);
	developerWindow->cameraControlWindow->setVisible(false);
	showRenderingStats = false;

	// Setup the camera with a good initial position and view direction to see the table
	activeCamera()->setPosition(Vector3(0,100,250));
	activeCamera()->lookAt(Vector3(0,0,0), Vector3(0,1,0));
	activeCamera()->setFarPlaneZ(-1000);

	//
    time = 0.0;
	serve = false;
    
    initBallSpeed = 447.12; //MATH!
    initBallToTableAngle = toRadians(27.57); //MATH!
    
    initBallVelocity = Vector3(0,200,400);
    ballPos = Vector3(0,30,-130);
    
}


void App::onUserInput(UserInput *uinput) {

	// This block of code maps the 2D position of the mouse in screen space to a 3D position
	// 20 cm above the ping pong table.  It also rotates the paddle to make the handle move
	// in a cool way.  It also makes sure that the paddle does not cross the net and go onto
	// the opponent's side.
	Vector2 mouseXY = uinput->mouseXY();
	float xneg1to1 = mouseXY[0] / renderDevice->width() * 2.0 - 1.0;
	float y0to1 = mouseXY[1] / renderDevice->height();
	Matrix3 rotZ = Matrix3::fromAxisAngle(Vector3(0,0,1), aSin(-xneg1to1));    
	Vector3 lastPaddlePos = paddleFrame.translation;
	paddleFrame = CoordinateFrame(rotZ, Vector3(xneg1to1 * 100.0, 20.0, G3D::max(y0to1 * 137.0 + 20.0, 0.0)));
	Vector3 newPos = paddleFrame.translation;

	// This is a weighted average.  Update the velocity to be 10% the velocity calculated 
	// at the previous frame and 90% the velocity calculated at this frame.
	paddleVel = 0.1*paddleVel + 0.9*(newPos - lastPaddlePos);


	// This returns true if the SPACEBAR was pressed
	if (uinput->keyPressed(GKey(' '))) {
		// This is where you can "serve" a new ball from the opponent's side of the net 
		// toward you. I found that a good initial position for the ball is: (0, 30, -130).  
		// And, a good initial velocity is (0, 200, 400).  As usual for this program, all 
		// units are in cm.
        time = 0.0;
		serve = true;
        //initVelocity = Vector3(0,2,400);
        //ballPos = Vector3(0,30,-130);
	}
}

Vector3 App::updateBallPos(double time) {
    detectCollisionTable();
    return Vector3(0, initBallVelocity.y*time - GRAVITY*time*time*0.5 + 30, initBallVelocity.z*time - 130);
}



void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {

	// rdt is the change in time (dt) in seconds since the last call to onSimulation
	// So, you can slow down the simulation by half if you divide it by 2.
	rdt *= 0.1;
    //time += rdt;
    if(serve) {
        time += rdt;
        ballVelocity = (ballPos - previousBallPos) / rdt;
    }
    else {
        time = 0.0;
    }
	// Here are a few other values that you may find useful..
	// Radius of the ball = 2cm
	// Radius of the paddle = 8cm
	// Acceleration due to gravity = 981cm/s^2 in the negative Y direction
	// See the diagram in the assignment handout for the dimensions of the ping pong table



}

void App::detectCollisionTable() {
    if(ballPos.y <= BALL_RADIUS) {
        cout << "Table hit!\n";
        ballVelocity.y *= (-1);
        initBallVelocity.y *= (-1);
    }
}

void App::detectCollisionPaddle() {
    /*if((ballPos.x > (getPaddlePosition().x - 8) && ballPos.x < (getPaddlePosition().x + 8)) &&
       ballPos.z > (getPaddlePosition().z - ballRadius)) {
        
        Vector3 ballVector = Vector3(initVelocity.x*time, initVelocity.y*time, 5*time);

        initVelocity = ballVector - 2*(dot(ballVector, getPaddleNormal())*getPaddleNormal() );
    }*/
}

void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) {
	rd->clear();

	Box table( Vector3(-76.25, 0, -137), Vector3(76.25, -10, 137) );
	Box stand( Vector3(-66.25, -11, -117), Vector3(66.25, -65, 117) );
	Draw::box( table, rd, Color3(0,0.5,0), Color4::clear());
	Draw::box( stand, rd, Color3(1,1,1), Color4::clear());

	if (serve == true) {
		Sphere ball( ballPos, BALL_RADIUS);
		Draw::sphere( ball, rd, Color3(0.4,0.4,0.4));

        cout << ballPos << "\n";
        previousBallPos = ballPos;
        ballPos = updateBallPos(time);
        
        if(ballPos.z > 137) {
            serve = false;
            time = 0.0;
            ballPos = Vector3(0,30,-130);
        }
        //
	}


	// Draw the paddle using 2 cylinders
	rd->pushState();
	rd->setObjectToWorldMatrix(paddleFrame);
	Cylinder paddle(Vector3(0,0,-0.5), Vector3(0,0,0.5), 8.0);
	Cylinder handle(Vector3(0,-7.5,0), Vector3(0,-16,0), 1.5);
	Draw::cylinder(paddle, rd, Color3(0.5,0,0), Color4::clear());
	Draw::cylinder(handle, rd, Color3(0.3,0.4,0), Color4::clear());
	rd->popState();  

	// Call to make the GApp show the output of debugDraw
	drawDebugShapes();
}


