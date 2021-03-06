/**
Interactive Graphics
Spring '15 Assignment #2 Ping Pong 3D
Authors: Barbara and Ivana
**/

#include "App.h"
#include <iostream>
#include <string>
using namespace std;

// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

// all measurements in cm
const double App::GRAVITY = 981;
const double App::BALL_RADIUS = 2.0;
const double App::PADDLE_RADIUS = 8.0;
const double App::TABLE_FRICTION = 0.8;
const double App::RESTITUTION = 0.85;

int main(int argc, const char* argv[]) {
	(void)argc; (void)argv;
	GApp::Settings settings(argc, argv);
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
    /*
    createDeveloperHUD();
	debugWindow->setVisible(false);
	developerWindow->setVisible(false);
    setVisible(false);
	developerWindow->cameraControlWindow->setVisible(false);
    */
	showRenderingStats = false;

	// Setup the camera with a good initial position and view direction to see the table
	activeCamera()->setPosition(Vector3(0,100,250));
	activeCamera()->lookAt(Vector3(0,0,0), Vector3(0,1,0));
	activeCamera()->setFarPlaneZ(-1000);

	// set initial ball position
	x_pos = 0;
	y_pos = 30;
	z_pos = -130;
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
		netFlag = true;
	}
}

void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
	GApp::onSimulation(rdt, sdt, idt);
	rdt *= 0.5; // slow down time
    time += rdt;
}


/*==============================================================================================================================
    GAME
==============================================================================================================================*/
/*this is called in OnGraphics3D*/
void App::game(RenderDevice* rd) {
	lastBallPos = ballPos;
	ballPos = updateBallPos(time);
    //updateBallPos(time); // get next ball position and update ball position
    if ( ballPos.y > -60 ) { // draw ball only if it is above ground level
        Sphere ball( ballPos, BALL_RADIUS);
        Draw::sphere( ball, rd, Color3(0.922745, 0.922745, 0.922745), Color4::clear());
    }
}
/*==============================================================================================================================*/


/*==============================================================================================================================
    Update Ball Position
==============================================================================================================================*/
Vector3 App::updateBallPos(double time) {
	// calculate new ball position based on projectile motion formula
    Vector3 newBallPosition(initBallVelocity.x*time + x_pos, 
							initBallVelocity.y*time - GRAVITY*time*time*0.5 + y_pos,
							initBallVelocity.z*time + z_pos);
    ballPos = newBallPosition;
    
    // detect collisions
    detectCollisionNet();
	detectCollisionTable();
	detectCollisionPaddle();
    
	// make sure that the ball does not penetrate the table
	if((newBallPosition.y <=BALL_RADIUS) && isWithinTableBounds()) {
       newBallPosition.y = BALL_RADIUS;
    }

	// check for out of bounds: a) if hit by paddle, check that it didn't bounce off on the other side and that it's outside of the opponent's side of the table, b) if not hit by paddle, check that the bell went outside of the player's side of the table
    if((paddleCollision && !tableCollision && (ballPos.z < -137 || ballPos.x > 76.25 || ballPos.x < -76.25))
       || (!paddleCollision && (ballPos.z > 200 || ballPos.x > 76.25 || ballPos.x < -76.25))) {
        if(message == "") {
            message = "Out of bounds - opponent's point"; // show losing message
            messageColor = Color3(1,0,0);
            opponentScore++; // update opponent's score
        }
    }

	// check if ball hit the net
    if(netCollision) {
        message = "Hit the net - opponent's point"; // show losing message
        messageColor = Color3(1,0,0);
    }
    return newBallPosition;
}
/*==============================================================================================================================*/


/*==============================================================================================================================
    Check if ball is within the correct bounds
==============================================================================================================================*/
bool App::isWithinTableBounds() {
    return (ballPos.z >= -137) && (ballPos.z <= 137) &&
    (ballPos.x >= -76.25) && (ballPos.x <= 76.25);
}

bool App::isWithinNetBounds() {
    return (ballPos.z < 6.5 && ballPos.z > -1 && //tight z bound
    ballPos.y < 16.25 &&
    ballPos.x > -76.25 && ballPos.x < 76.25);
}
/*==============================================================================================================================*/

/*==============================================================================================================================
    Detect all collisions with ball
==============================================================================================================================*/
void App::detectCollisionTable() {
	if ((ballPos.y <= BALL_RADIUS*2) && isWithinTableBounds() ) {

		// reset new launch position
		if (!netCollision) {
			if (netFlag) { // only run this code if there wasn't been a net collision in the past
				time = 0; // reset time
				z_pos = ballPos.z;
				x_pos = ballPos.x;
				y_pos = BALL_RADIUS*4;
			}
			initBallVelocity.y *= RESTITUTION; // slow y-speed due to energy lost during table collision 
	        initBallVelocity.z *= TABLE_FRICTION; // slow z-speed due to friction when hitting table
		}

        tableCollision = true;

		// check if table collision happens on opponent's side AFTER a paddle collision on user's side
        if(paddleCollision && ballPos.z < -10) {
            if(message == "") {
                message = "Nice shot - your point!"; // show winning message
                messageColor = Color3(0,1,0);
                playerScore++; // update user's score
            }
        }
	}
    else tableCollision = false;
}

void App::detectCollisionPaddle() {
	// run this code only if there hasn't yet been a paddle collision (prevents multiple collisios
	// within a short span of time)
	if (!paddleCollision) {

		// check paddle collision for the whole ball (not just its center)
		for (int x = ballPos.x - BALL_RADIUS; x <= ballPos.x + BALL_RADIUS; x++) {
			for (int z = ballPos.z; z <= ballPos.z + BALL_RADIUS*2; z++) {

				// create a cylinder between the last and the current paddle position
				// (give it a 1cm thickness in case the two positions are the same)
				paddlePosCylinder = Cylinder( lastPaddlePos + Vector3(0,0,-0.5),
											  newPaddlePos + Vector3(0,0,0.5),
											  PADDLE_RADIUS);

				// check if any of the ball's points in within the cylinder
				paddleCollision = paddlePosCylinder.contains(  Vector3(x, 20, z) );
				if (paddleCollision) {
					time = 0; // reset time

					// reset new launch position
					x_pos = ballPos.x;
					y_pos = ballPos.y;
					z_pos = ballPos.z - 5;

					// change the ball's x-velocity, so that it "inherits" from the paddle's velocity multiplied by an arbitrary constant
					initBallVelocity.x = getPaddleVelocity().x * 20;
                    
					// reverse z-velocity and make it inherit from the paddle's z-velocity multiplied by an arbitrary constant as well
					initBallVelocity.z *= getPaddleNormal().z + 0.5*getPaddleVelocity().z;
					return;
				}
			}
		}
	}
}

void App::detectCollisionNet() {
    if (isWithinNetBounds() || !netFlag) {
        netCollision = true;

		// update score, reset time and invert z-velocity only for the first time that we detect a net collision
		if (netFlag) {
			time = 0; // reset time
			
			// reset new launch position
			x_pos = ballPos.x;
			y_pos = 0;
			z_pos = 2;

			opponentScore++; // update opponent's score
			initBallVelocity.z *= -1;
			initBallVelocity.x *= 0; // remove x-velocity
			netFlag = false;
		}

		initBallVelocity.z *= 0.85; // remove z-velocity
        
		// check if the ball is going back towards the net (because initBallVelocity has decreased so much)
        if (lastBallPos.z > ballPos.z && !isWithinNetBounds()) {
            time = 0;
            z_pos = ballPos.z;
            initBallVelocity.z = 0; // remove z-velocity
            initBallVelocity.y *=0.5; //for additional small jump at the end
        }
    }
}
/*==============================================================================================================================*/


/*==============================================================================================================================
    Draw winning/losing message
==============================================================================================================================*/
void App::drawMessage(RenderDevice* rd) {
    rd->push2D();
	
	// create 2D coordinate frame onto which we display win/lose messages
    CoordinateFrame cframe(Vector3(0,50,0));
    rd->setObjectToWorldMatrix(cframe);
    
    // center align the messages properly
    int messageSpace = 200;
    if ( message == "Nice shot - your point!" ) messageSpace = 290;
    else if ( message == "Out of bounds - opponent's point" ) messageSpace = 120;
    
	// draw win/lose message
    debugFont->draw2D(rd,message, Vector2(messageSpace, 70), 42, messageColor); // draw message
    
	// convert scores to strings and display them appropriately
    const G3D::String playerScoreDisplay = string(to_string(playerScore)).c_str();
    const G3D::String opponentScoreDisplay = string(to_string(opponentScore)).c_str();
    String dash = "-";
    
    int dashSpace = 110;
    int opponentSpace = 140;
    
    if (playerScore > 9) {
        dashSpace = 120;
        opponentSpace = 150;
    }
    
	// change player's score color if she has just won
    Color3 playerColor;
    if(messageColor == Color3(0,1,0)) playerColor = messageColor;
    else playerColor = Color3::yellow();
    
	// change computer's score color if it has just won
    Color3 opponentColor;
    if(messageColor == Color3(1,0,0)) opponentColor = messageColor;
    else opponentColor = Color3::yellow();
    
	// draw scoreboard
    debugFont->draw2D( rd, playerScoreDisplay, Vector2(80, 0), 20, playerColor);
    debugFont->draw2D( rd, dash, Vector2(dashSpace, 0), 20, Color3::yellow());
    debugFont->draw2D( rd, opponentScoreDisplay, Vector2(opponentSpace, 0), 20, opponentColor);
    
    rd->pop2D();
}
/*==============================================================================================================================*/


/*==============================================================================================================================
    Reset everything to inital values
==============================================================================================================================*/
void App::resetBall() {
	time = 0; // reset time
	ballPos = Vector3(0,30,-130); // reset position to original values
    initBallVelocity = Vector3(0,200,400); // reset velocity to original values
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
/*==============================================================================================================================*/


/*==============================================================================================================================
    OnGraphics3D
==============================================================================================================================*/
void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) {
	rd->clear();
    
    // Draw the table, the stand under it and the wall in the back
	Box wall( Vector3(-900, 0, -500), Vector3(900, 300, -500) );
	Box table( Vector3(-76.25, 0, -137), Vector3(76.25, -3, 137) );
	Box stand( Vector3(-66.25, -4, -117), Vector3(66.25, -60, 117) );
	Draw::box( wall, rd, Color3(0.317647, 0.317647, 0.317647), Color4::clear());
	Draw::box( table, rd, Color3(0, 0.3, 0.1), Color4::clear());
	Draw::box( stand, rd, Color3(0.562745, 0.562745, 0.562745), Color4::clear());

    /******************************************TABLE STRIPES****************************************/
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
    /*************************************************************************************/
    
    /*******************************************NET******************************************/
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
    /************************************************************************************/

	/*******************************************PADDLE SHADOW******************************************/
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
    /*************************************************************************************/

    /*******************************************BALL SHADOW******************************************/
	if (isWithinTableBounds() && serve) {
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
    /*************************************************************************************/
    

	// Draw the paddle using 2 cylinders
	rd->pushState();
	rd->setObjectToWorldMatrix(paddleFrame);
	Cylinder paddle(Vector3(0,0,-0.5), Vector3(0,0,0.5), 8.0);
	Cylinder handle(Vector3(0,-7.5,0), Vector3(0,-16,0), 1.5);
	Draw::cylinder(paddle, rd, Color3(0.5,0,0), Color4::clear());
	Draw::cylinder(handle, rd, Color3(0.3,0.4,0), Color4::clear());
	rd->popState();
    
    /**************GAME**************/
    if (serve) {
		game(rd);
	}
    /******************************/

	drawMessage(rd);

	// Call to make the GApp show the output of debugDraw
	drawDebugShapes();
}
/*==============================================================================================================================*/


