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
const double App::TABLE_FRICTION = 0.45; //https://prezi.com/lycm4jivbrc7/friction
const double App::PADDLE_FRICTION = 0.6; //https://prezi.com/lycm4jivbrc7/friction
const double App::RESTITUTION = 0.9; //www.ijee.ie/articles/Vol19-4/IJEE1433.pdf
const double App::AIR_DRAG = 0.47; //en.wikipedia.org/wiki/Drag_coefficient

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
	renderDevice->setColorClearValue(Color3(0.096863, 0.096863, 0.096863));
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
    
    initBallSpeed = 447.12; //MATH!
    initBallToTableAngle = toRadians(27.57); //MATH!
    
    resetBall();
    resetCollisions();
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
		serve = true;
	}
}

Vector3 App::updateBallPos(double time) {
	detectCollisionPaddle();
    detectCollisionTable();
    Vector3 newBallPosition(initBallVelocity.x*time, initBallVelocity.y*AIR_DRAG*time - GRAVITY*time*time*0.5 + 30, initBallVelocity.z*AIR_DRAG*time - 130);
    if(tableCollision) {
        newBallPosition.y *= -1;
    }
    if(paddleCollision) {
        newBallPosition.z = -newBallPosition.z + 2 * paddleCollisionPos;
    }
    return newBallPosition;
}

void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
	rdt *= 0.1; //slowed it down
    if(serve) {
        time += rdt; //start timing only while ball is in air
        ballVelocity = (ballPos - previousBallPos) / rdt;
    }
}

void App::detectCollisionTable() {
    if(ballPos.y <= BALL_RADIUS) {
        tableCollision = !tableCollision;
        if(ballPos.y >= -BALL_RADIUS) { //ball is on the table
        	initBallVelocity.y *= RESTITUTION; //reduce velocity due to bounciness friction
        	initBallVelocity.z *= TABLE_FRICTION; //reduce velocity due to table friction
        }
    }
}

void App::detectCollisionPaddle() {
    if( ballPos.x > (getPaddlePosition().x - 8) && ballPos.x < (getPaddlePosition().x + 8) &&
        ballPos.z > (getPaddlePosition().z - 8) && ballPos.z < (getPaddlePosition().z + 8) ) {
        paddleCollision = !paddleCollision;
        paddleCollisionPos = ballPos.z;
        initBallVelocity.z *= PADDLE_FRICTION; //reduce velocity due to friction from paddle
        /*Here we might assign some "swing force" to the paddle, to increase velocity each time*/
    }
}

/*this is called in OnGraphics3D*/
void App::game(RenderDevice* rd) {
    if (serve) {
		Sphere ball( ballPos, BALL_RADIUS);
		Draw::sphere( ball, rd, Color3(0.4,0.4,0.4));
        
        previousBallPos = ballPos; //will maybe need later
        ballPos = updateBallPos(time);
        
        if(ballPos.z > 160 || ballPos.y > 360) {
            resetBall();
        }
    }
}

void App::resetBall() {
	serve = false;
	time = 0.0;
	ballPos = Vector3(0,30,-130);
    initBallVelocity = Vector3(0,200,400);
}

void App::resetCollisions() {
	tableCollision = false;
	paddleCollision = false;
}

void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) {
	rd->clear();

	Box wall( Vector3(-900, 0, -500), Vector3(900, 300, -500) );
	Box table( Vector3(-76.25, 0, -137), Vector3(76.25, -3, 137) );
	Box stand( Vector3(-66.25, -4, -117), Vector3(66.25, -60, 117) );
	Draw::box( wall, rd, Color3(0.317647, 0.317647, 0.317647), Color4::clear());
	Draw::box( table, rd, Color3(0, 0.3, 0.1), Color4::clear());
	Draw::box( stand, rd, Color3(0.762745, 0.762745, 0.762745), Color4::clear());

	LineSegment front_down = LineSegment::fromTwoPoints(Point3(-76.25, -3, 137), Point3(76.25, -3, 137));
	LineSegment front_up = LineSegment::fromTwoPoints(Point3(-76.25, 0, 137), Point3(76.25, 0, 137));
	LineSegment front_left = LineSegment::fromTwoPoints(Point3(-76.25, -3, 137), Point3(-76.25, 0, 137));
	LineSegment front_right = LineSegment::fromTwoPoints(Point3(76.25, -3, 137), Point3(76.25, 0, 137));
	Draw::lineSegment(front_down, rd, Color3(1, 1, 1));
	Draw::lineSegment(front_up, rd, Color3(1, 1, 1));
	Draw::lineSegment(front_left, rd, Color3(1, 1, 1));
	Draw::lineSegment(front_right, rd, Color3(1, 1, 1));

	LineSegment top_left = LineSegment::fromTwoPoints(Point3(-76.25, 0, 137), Point3(-76.25, 0, -137));
	LineSegment top_middle = LineSegment::fromTwoPoints(Point3(0, 0, 137), Point3(0, 0, -137));
	LineSegment top_right = LineSegment::fromTwoPoints(Point3(76.25, 0, 137), Point3(76.25, 0, -137));
	Draw::lineSegment(top_left, rd, Color3(1, 1, 1));
	Draw::lineSegment(top_middle, rd, Color3(1, 1, 1));
	Draw::lineSegment(top_right, rd, Color3(1, 1, 1));

	LineSegment back_top = LineSegment::fromTwoPoints(Point3(-76.25, 0, -137), Point3(76.25, 0, -137));
	Draw::lineSegment(back_top, rd, Color3(1, 1, 1));

	for( double x = -91.5; x <= 91.5; x += 2 ) {
		LineSegment horiz = LineSegment::fromTwoPoints(Point3(x, 0, 0), Point3(x, 15.25, 0));
		Draw::lineSegment(horiz, rd, Color3(0, 0, 0));
	}
	LineSegment left = LineSegment::fromTwoPoints(Point3(-91.5, 0, 0), Point3(-91.5, 15.25, 0));
	Draw::lineSegment(left, rd, Color3(1, 1, 1));
	LineSegment right = LineSegment::fromTwoPoints(Point3(91.5, 0, 0), Point3(91.5, 15.25, 0));
	Draw::lineSegment(right, rd, Color3(1, 1, 1));

	for( double y = 0; y <= 15.25; y += 2 ) {
		LineSegment vert = LineSegment::fromTwoPoints(Point3(-91.5, y, 0), Point3(91.25, y, 0));
		Draw::lineSegment(vert, rd, Color3(0, 0, 0));
	}
	LineSegment top = LineSegment::fromTwoPoints(Point3(-91.5, 15.25, 0), Point3(91.5, 15.25, 0));
	Draw::lineSegment(top, rd, Color3(1, 1, 1));
	LineSegment bottom_left = LineSegment::fromTwoPoints(Point3(-91.5, 0, 0), Point3(-76.25, 0, 0));
	Draw::lineSegment(bottom_left, rd, Color3(1, 1, 1));
	LineSegment bottom_right = LineSegment::fromTwoPoints(Point3(76.25, 0, 0), Point3(91.5, 0, 0));
	Draw::lineSegment(bottom_right, rd, Color3(1, 1, 1));

	game(rd);

    /*
    if ( serve == true ) {
	//SlowMesh mesh(PrimitiveType::TRIANGLE_FAN);
	//mesh.setColor(Color3::blue());
	//mesh.makeVertex(Vector2(400,400));
	//mesh.makeVertex(Vector2(800,400));
	//mesh.makeVertex(Vector2(800,800));
	//mesh.makeVertex(Vector2(400,800));
	//mesh.makeVertex(Vector2(400,400));
	//rd->push2D();
	//mesh.render(rd);
	//rd->pop2D();


	if ( serve == true ) {
		Sphere ball( position, ballRadius );
		Draw::sphere( ball, rd, Color3( 0.4, 0.4, 0.4 ));
        detectCollisionPaddle();
		detectCollisionTable();
        if( position.z > 160 ) {
          serve = false;
        }
	}*/


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


