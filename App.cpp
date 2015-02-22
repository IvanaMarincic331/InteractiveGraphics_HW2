/**
Comp 394 S15 Assignment #2 Ping Pong 3D
Athors: Barbara and Ivana
**/

#include "App.h"
#include <iostream>
#include <string>
using namespace std;

// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

const double App::GRAVITY = 981;
const double App::BALL_RADIUS = 2.0;
const double App::PADDLE_RADIUS = 8.0;
const double App::PADDLE_FRICTION = 0.75;
const double App::RESTITUTION = 0.85;

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
    //renderDevice->setColorClearValue(Color3(1, 1, 1));
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

	x_pos = 0;
	y_pos = 30;
	z_pos = -130;
	table_col = Color3(0, 0.3, 0.1);
	serve = false;
    resetCollisions();
    resetMessage();
    resetScores();
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
	lastPaddlePos = paddleFrame.translation;
	paddleFrame = CoordinateFrame(rotZ, Vector3(xneg1to1 * 100.0, 20.0, G3D::max(y0to1 * 137.0 + 20.0, 0.0)));
	newPaddlePos = paddleFrame.translation;

	// This is a weighted average.  Update the velocity to be 10% the velocity calculated 
	// at the previous frame and 90% the velocity calculated at this frame.
	paddleVel = 0.1*paddleVel + 0.9*(newPaddlePos - lastPaddlePos);


	// This returns true if the SPACEBAR was pressed
	if (uinput->keyPressed(GKey(' '))) {
		resetBall();
	}
}

Vector3 App::updateBallPos(double time) {
    Vector3 newBallPosition(initBallVelocity.x*time + x_pos, 
							initBallVelocity.y*time - GRAVITY*time*time*0.5 + y_pos,
							initBallVelocity.z*time + z_pos);
	detectCollisionPaddle();
    detectCollisionTable();
    detectCollisionNet();
    if((newBallPosition.y <=BALL_RADIUS) && (ballPos.z >= -137) && (ballPos.z <= 137) &&
                                             (ballPos.x >= -76.25) && (ballPos.x <= 76.25)) {
       newBallPosition.y = BALL_RADIUS;
    }
    if((paddleCollision && !tableCollision && (ballPos.z < -137 || ballPos.x > 76.25 || ballPos.x < -76.25))
       || (!paddleCollision && (ballPos.z > 200 || ballPos.x > 76.25 || ballPos.x < -76.25))) {
        if(message == "") {
            message = "Out of bounds - opponent's point";
            messageColor = Color3(1,0,0);
            opponentScore++;
        }
    }
    if(netCollision) {
        message = "Hit the net - opponent's point";
        messageColor = Color3(1,0,0);
        opponentScore++;
    }
    return newBallPosition;
}

void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
	GApp::onSimulation(rdt, sdt, idt);
	rdt *= 0.5; //slowed it down
    time += rdt; //start timing only while ball is in air
    ballVelocity = (ballPos - lastBallPos) / rdt;
}

void App::detectCollisionTable() {
	if ((ballPos.y <= BALL_RADIUS*2) && 
		(ballPos.z >= -137) && (ballPos.z <= 137) &&
		(ballPos.x >= -76.25) && (ballPos.x <= 76.25) ) {
		time = 0;
		x_pos = ballPos.x;
		y_pos = BALL_RADIUS*4;
		z_pos = ballPos.z;
		initBallVelocity.y *= RESTITUTION;
        tableCollision = true;
        if(paddleCollision && ballPos.z < -0.5 && !netCollision ) {
            if(message == "") {
                message = "Nice shot - your point!";
                messageColor = Color3(0,1,0);
                playerScore++;
            }
        }
	}
    else tableCollision = false;
}

void App::detectCollisionPaddle() {
	if (!paddleCollision) {
		for (int x = ballPos.x - BALL_RADIUS; x <= ballPos.x + BALL_RADIUS; x++) {
			for (int z = ballPos.z; z <= ballPos.z + BALL_RADIUS*2; z++) {
				paddlePosCylinder = Cylinder( lastPaddlePos + Vector3(0,0,-0.5),
											  newPaddlePos + Vector3(0,0,0.5),
											  PADDLE_RADIUS);
				paddleCollision = paddlePosCylinder.contains(  Vector3(x, 20, z) );
				if (paddleCollision) {
					time = 0;
					x_pos = ballPos.x;
					y_pos = ballPos.y;
					z_pos = ballPos.z - 10;
					initBallVelocity.x = getPaddleVelocity().x*20;
					initBallVelocity.y *= 1.1;
					initBallVelocity.z *= -1;
					return;
				}
			}
		}
	}
}

void App::detectCollisionNet() {
    if (ballPos.z < 6 && ballPos.z > -4 &&
        ballPos.y < 17.25 &&
        ballPos.x > -76.25 && ballPos.x < 76.25) {
        netCollision = true;
        time = 0;
        x_pos = ballPos.x;
        y_pos = ballPos.y;
        z_pos = 3;
        initBallVelocity.x *= 0;
        initBallVelocity.y *= 0.7;
        initBallVelocity.z *= 0;
    } else {
        netCollision = false;
    }
    
}

/*this is called in OnGraphics3D*/
void App::game(RenderDevice* rd) {
	lastBallPos = ballPos;
	ballPos = updateBallPos(time);
    if(ballPos.y > -60){ //
        Sphere ball( ballPos, BALL_RADIUS);
        Draw::sphere( ball, rd, Color3(0.922745, 0.922745, 0.922745), Color4::clear());
    }
}

void App::resetBall() {
	time = 0.0;
	ballPos = Vector3(0,30,-130);
    initBallVelocity = Vector3(0,200,400);
	serve = true;
	x_pos = 0;
	y_pos = 30;
	z_pos = -130;
    resetCollisions();
    resetMessage();
    
}

void App::resetCollisions() {
    paddleCollision = false;
    tableCollision = false;
    netCollision = false;
}

void App::resetMessage() {
    message = "";
    messageColor = Color3(0,0,0);
}

void App::resetScores() {
    playerScore = 0;
    opponentScore = 0;
}

void App::drawMessage(RenderDevice* rd) {
    rd->push2D();
    CoordinateFrame cframe(Vector3(0,50,0));
    rd->setObjectToWorldMatrix(cframe);
    debugFont->draw2D(rd,message, Vector2(200, 70), 42, messageColor);
    
    
    //string m = to_string(playerScore).c_str();//+" - "+to_string(opponentScore);
    //cout << m;
    //debugFont->draw2D(rd,m,Vector2(100, 0), 20, Color3::yellow());*/ //failing at displaying the scores here
    rd->pop2D();
}

void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) {
	rd->clear();
    

	Box wall( Vector3(-900, 0, -500), Vector3(900, 300, -500) );
	Box table( Vector3(-76.25, 0, -137), Vector3(76.25, -3, 137) );
	Box stand( Vector3(-66.25, -4, -117), Vector3(66.25, -60, 117) );
	Draw::box( wall, rd, Color3(0.317647, 0.317647, 0.317647), Color4::clear());
	Draw::box( table, rd, table_col, Color4::clear());
	Draw::box( stand, rd, Color3(0.562745, 0.562745, 0.562745), Color4::clear());

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

    /*Net*/
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

	if (serve) {
		game(rd);
	}

	
	if ((getPaddlePosition().z <= 137) &&
		(getPaddlePosition().x >= -76.25) && (getPaddlePosition().x <= 76.25) ) {
		for ( int i = 0; i < 20; i++ ) {
			Vector3 paddleShadowVector1 = getPaddlePosition();
			paddleShadowVector1.y = i/20 + 1;
			Vector3 paddleShadowVector2 = getPaddlePosition();
			paddleShadowVector2.y = i/20 + 1.1;
			Cylinder paddleShadow( paddleShadowVector1, paddleShadowVector2, PADDLE_RADIUS*((20-i)/20.0) );
			float color = 0.1 / (i+1);
			Draw::cylinder(paddleShadow, rd, Color4(color, color, color, 0.03), Color4::clear());
		}
	}

	if ((ballPos.z >= -137) && (ballPos.z <= 137) &&
		(ballPos.x >= -76.25) && (ballPos.x <= 76.25) && serve) {
		for ( int i = 0; i < 20; i++ ) {
			Vector3 ballShadowVector1 = ballPos;
			ballShadowVector1.y = i/20 + 1;
			Vector3 ballShadowVector2 = ballPos;
			ballShadowVector2.y = i/20 + 1.1;
			Cylinder ballShadow( ballShadowVector1, ballShadowVector2, BALL_RADIUS*((20-i)/20.0) );
			float color = 0.1 / (i+1);
			Draw::cylinder(ballShadow, rd, Color4(color, color, color, 0.03), Color4::clear());
		}
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
    drawMessage(rd);

}


