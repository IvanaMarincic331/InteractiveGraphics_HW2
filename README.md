### Interactive Graphics HW2

## A One-Sided Ping Pong Game

*Barbara Borges Ribeiro*

*Ivana Marincic*

===========================

#### Purpose of Assignment
For this assignment, we created a 3D Ping Pong game using the G3D graphics engine. The game is "one-sided", because as of now, there exists one paddle that can be moved by the player. 

The purpose and learning objectives of this assignment that we were able to implement include:
* given an initial position and initial velocity, implement the projectile motion of a moving object, in this case, a ball
* update the ball's position correctly over time
* handling collisions between the ball and the paddle, and implement how these collisions impact the ball's velocity's magnitude and direction
* display text on the screen in G3D
* keep track of player's score, aka determine a winning or losing situation depending on the ball's position and whether it has been hit with the paddle

#### Running the Program
This program is run like most basic G3D programs.

#### How to Play
At the start of the game the player can move the paddle using a mouse or mouse pad.
To launch a ball, press the SPACEBAR. Even when a ball is mid-flight, hitting the SPACEBAR again will reset the ball's starting position and initial velocity.
Some things to try and look out for:
* align the paddle in the middle, and let it be stationary. The ball will jump off the paddle. Move the paddle further backwards, and the ball will do the same. Notice the relatively weak bounce of the ball.
* if the ball hits the net, it will jump off the net and make a small bounce on the table, to simulate a realisitc reflective jump when it hits the net (assuming the net is tight)
* launch the ball again, and this time swing the paddle to the side and hit the ball - the ball will reflect to that side
* do the same thing, but with a stronger or weaker swing - depending on how fast or slow we move the paddle, that's how much the ball will bounce
* this works when we move the paddle back and forth as well
* you win a point when you hit the ball with the paddle and the ball bounces on the opponent's side of the table
* you lose a point when you miss the ball, when it gets caught in the net, or when it doesn't bounce off the opponents' side of the table
* the scores are kept in the upper right corner. The change of color of the individual scores will signal to the player which score is theirs and which score is the opponent's
* the player is also notified of how the point is lost or won by a message in the middle of the display

### Implementation
#### Ball Motion
The paddle model and it's motion data are as given.
When the SPACEBAR is pressed, the ball's initial velocity and position are set, and a time count starts.
The time increments by half of the real time, and is used as a variable in the formula that dictates the ball's projectile motion. We assume that the initial speed and launch angle is captured by the initial velocity, and are therefore not used separately at any point.
#### Collisions
As the ball's next position is calculated as the time variable gets incremented, we check if that new position will cause a collision with either the table, paddle or the net. If a collision is detected, we reset the initial ball velocity (used in computing the projectile motion), the ball's position to the location of the collision and we also reset the time. This is such, because we treat each time a ball collides and has to change it's direction and velocity magnitude as a new projectile motion. 

How the direction and velocity of the ball is set, depends on what the ball collided with and the normal of that object. Since we reset the ball's motion on each collision, certain components of the ball's velocity will take on the respecitve components of the normals of the object the ball collided with. For instance, if the ball collides with the table, we want the ball to travel in the positive y direction again, which is the normal of the table. If the ball collides with the paddle, we want it's z component of the velocity to be negative, which is the normal of the paddle. 

We check for collisions by making sure that the ball's next position is within certain boundaries of the table, net and paddle.

For the table, this would mean checking that the ball is close to the 0 y value (since that is the y position of the table) and that it is within the x and z coordinates of the table. If all three cases are true, then we set a table collision flag to be true, which is used in determining the scores and the winning or losing states. 

Similarly, we check if the ball's position is close to the 0 z value and within the net's x and y coordinates when checking for the net collision. Here we set a net collision flag to be true for the first time to increment the opponent's score. 

As of the paddle, we want to make sure that the ball doesn't "go through" the paddle, even if the paddle is moving really fast. Therefore, we are computing the paddle's next position, and based on that, we create a cylinder object whose dimensions are the difference in the paddle's current and next position. All is left is to check whether the ball's next position will be within that cylinder. If yes, the paddle collision flag is set to true for calculating scores and the ball's new position and velocity are set accordingly.
