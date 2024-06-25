// CS 445 Prog 4 for Kyle Fitzpatrick
/*
	EXTRA CREDIT: 
		*3 one-pixel green lights on each long side of each fan blade
		*Fish scale increased by 0.1 after eating food

	Modified OpenGL445Setup.h
		Line 27: Changed the glOrtho arguments to glOrtho(-width_bound / 2.0, width_bound / 2.0, -height_bound / 2.0, height_bound / 2.0, 400, 2000.0);
		Line 48: Changed GLUT_SINGLE to GLUT_DOUBLE for double-buffered animation

	Architecture Statement:
		This program implements a simple game of a fish collecting food around its fish tank. Frame-by-frame animation
		and interactivity of this fish game is achieved using glut event callback.

		The function timer_func is the function registered as the glut timer callback. This function is registered
		every time_Delta (80, in this case) milliseconds. Every time the function is called, it updates the global
		state variables, including fish position, fish state, food position, to achieve a dynamic, interactive scene. 
		After timer_func ends, it calls the display callback to render the scene according to the new global states.

		The function display_func is the function registered as the glut display callback. This function
		completely redraws the canvas after every time timer_func is called, achieving frame-by-frame animation.
		All objects in the scene are drawn from their individual "drawX" functions called from inside display_func.
		Addiitonally, display_func includes functionality for double buffer swapping in order to support
		double-buffered animation.

		The function key_func is the function registered to glut keyboard callback, allowing for the game to
		support input from the user. Valid keypresses are processed in this function, and the corresponding
		program state variables (fish position) are updated accordingly. This function allows for state changes
		that affect program behavior in timer_func and display_func. Timer_func may change fish, food, and score states
		based on the new fish position state, and display_func will draw the fish in its new position based on its
		updated position state.

		The function initializeObjects is not registered to any glut event. However, it initializes some important
		program states, such as fish position, and includes code used to generate display lists for all of the
		objects in the scene. These display lists are later called every frame from the display_func to achieve
		frame-by-frame animation. The purpose of defining these objects ahead of time using display lists is
		to improve program efficiency/responsiveness.
*/
#include "pch.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "OpenGL445Setup.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <math.h>
#include <ctime>

using namespace std;

// All function, class, struct, enum forward declarations
struct Vector3;
enum FishState;
void initializeObjects();
void drawScore();
void drawTimer();
void drawFish();
void drawFans();
void drawTank();
void drawFood();
void display_func();
void timer_func(int ID);
void key_func(unsigned char key, int x, int y);
void initializeObjects();

#define canvas_Width 640 /* width of canvas - you may need to change this */
#define canvas_Height 640 /* height of canvas - you may need to change this */
#define time_Delta 80 // Time in ms between glut timer events (12.5 frames per second)
char canvas_Name[] = "Fish Bowl"; /* name in window top */
const float tankScale = 250.0;

GLfloat light_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_diffuse[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat light_specular[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat light_position[] = { 0.0, 0.0, 0.0, 1.0 };

GLfloat mat_ambient[] = { 1.0, 1.0, 0.0, 1.0 };
GLfloat mat_diffuse[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat mat_specular[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat mat_shininess = 0.0;

// All global variables. These variables manage the many states
// of the game, such as inputs, scores, and positions of objects
Vector3* fishPos = NULL; // Position of the fish
Vector3* foodPos = NULL; // Position of the food (NULL when food has been eaten or despawns)
float fishScale = 1.0; // Fish scaling factor after eating

float fanRotation = 0.0; // Current rotation of the fan blades (degrees)
const float fanRotRate = 0.5; // Complete fan rotations per second

int gameTime = 30000; // Time since the game started running (not including menu)
int foodTime = 4000; // Time to spawn another food
int fishScaleTime = 0; // Timer for controlling fish size after eating
int score = 0;
FishState fishFlag;

const unsigned char scoreLabel[8] = "Score: ";	// Label for the fuel count in the top-left
const unsigned char timeLabel[7] = "Time: ";	// Label for the time in the top-right

int fishDisplayInd;	// Index for display list of the fish
int tankDisplayInd; // Index for display list of the fish tank
int fanDisplayInd; // Index for display list of one fan blade
int foodDisplayInd; // Index for display list of food

enum FishState {
	REGULAR,	// Default state of the fish
	GROWING,	// State of the fish after consuming food, before increasing in size
	LARGE		// State of the fish after consuming food, after increasing in size
};

// Vector3 struct; helps with storing and manipulating
// position, color, and vertices of scene objects
struct Vector3 {
	float x;
	float y;
	float z;

	Vector3() {
		x = 0;
		y = 0;
		z = 0;
	}

	Vector3(float xIn, float yIn, float zIn) {
		x = xIn;
		y = yIn;
		z = zIn;
	}

	// Useful operators (+, -, *) have been overloaded

	Vector3 operator+(Vector3 const& vec) {
		Vector3 out;
		out.x = x + vec.x;
		out.y = y + vec.y;
		out.z = z + vec.z;
		return out;
	}

	Vector3 operator-(Vector3 const& vec) {
		Vector3 out;
		out.x = x - vec.x;
		out.y = y - vec.y;
		out.z = z - vec.z;
		return out;
	}

	Vector3 operator*(int const& scale) {
		Vector3 out;
		out.x = x * scale;
		out.y = y * scale;
		out.z = z * scale;
		return out;
	}

	Vector3 operator*(float const& scale) {
		Vector3 out;
		out.x = x * scale;
		out.y = y * scale;
		out.z = z * scale;
		return out;
	}

	// Return the length of the vector
	float magnitude() {
		return sqrt(x * x + y * y + z * z);
	}

	// Return the magnitude of the difference between this vector and another vector
	float distance(Vector3 &rVec) {
		Vector3 temp(x - rVec.x, y - rVec.y, z - rVec.z); // Temp vector stores difference between the vectors
		return temp.magnitude(); // Return the magnitude of the difference
	}
};


// drawScore draws the score in the top left corner
// of the canvas.
void drawScore() {
	glPushMatrix();
	// Draw the "Score" text label
	glColor3f(1.0, 1.0, 1.0);
	glRasterPos3i(-300, 300, -500);
	glutBitmapString(GLUT_BITMAP_HELVETICA_12, scoreLabel);

	// Convert the score to characters and draw to the canvas
	string scoreText = to_string(score);
	for (int i = 0; i < scoreText.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, scoreText[i]);
	}
	glPopMatrix();
}

// drawTimer draws the timer in the top right corner
// of the canvas.
void drawTimer() {
	glPushMatrix();
	// Draw the "Time" text label
	glColor3f(1.0, 1.0, 1.0);
	glRasterPos3i(250, 300, -500);
	glutBitmapString(GLUT_BITMAP_HELVETICA_12, timeLabel);

	// Conver the time to characters and draw to the canvas
	string timeText = to_string(gameTime / 1000);
	for (int i = 0; i < timeText.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, timeText[i]);
	}
	glPopMatrix();
}

// drawFish draws the orange player-controlled fish to the screen
// This function is called within the display callback function
void drawFish() {
	glPushMatrix();
	glTranslatef(fishPos->x, fishPos->y, fishPos->z);
	glScalef(fishScale, fishScale, fishScale);	// The scale of the fish will change when the player collects food
	glCallList(fishDisplayInd);	// Call the display list for the fish as defined in the initializeObjects() function
	glPopMatrix();
}

// drawTank draws the yellow fish tank. This function
// is called within the display callback function.
void drawTank() {
	glPushMatrix();
	glCallList(tankDisplayInd); // Call the display list for the tank as defined in the initializeObjects() function
	glPopMatrix();
}

// drawTank draws the yellow fish tank fans. This function
// is called within the display callback function. This function
// supports frame-by-frame animation by drawing the fan blades
// offset by fanRotation degrees every frame.
void drawFans() {
	glPushMatrix();
	glTranslatef(0.0, 0.0, -50.0 - tankScale);	// Move fan to back wall
	glRotatef(fanRotation, 0.0, 0.0, 1.0);	// Set rotation using global fan rotation variable
	for (int i = 0; i < 6; i++) {
		glCallList(fanDisplayInd);	// Call the display list for a single fan blade once per blade, rotating
									// by 60 degrees for each blade drawn.
		glRotatef(60.0, 0.0, 0.0, 1.0);
	}
	glPopMatrix();
}

// drawFood draws food for the player to collect by moving the fish.
// Food is only drawn when it exists (when it has not been eaten,
// and when it has not waited for longer than three seconds without
// being eaten)
void drawFood() {
	if (foodPos) {	// Draw only if a food exists in the scene
		glPushMatrix();
		glTranslatef(foodPos->x, foodPos->y, foodPos->z);
		glCallList(foodDisplayInd);	// Call the display list for the food object as defined in initializeObjects()
		glPopMatrix();
	}
}

// glutDisplayFunc event handler function
// Supports frame-by-frame animation by drawing objects
// in the scene erasing the scene, and redrawing
// the objects in their new positions.
void display_func()
{
	// Lighting settings: Only has ambient light properties
	// Other light properties set to zero.
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHT0);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw score and timer before gluLookAt
	// to keep them "flat" on the canvas
	drawScore();
	drawTimer();
	gluLookAt(175, 320, 500, 0, 0, 0, 0, 1, 0);

	// Draw the fish, fans, tank, and food from
	// the repositioned camera.
	drawFish();
	drawFans();
	drawTank();
	drawFood();

	glutSwapBuffers();
	glFlush();
}

// glutTimerFunc event handler function. Using the timer event
// allows for precise frame times (roughly 30 ms) and 
// relative device independence. This function causes the
// changes in positions of the scene objects to be drawn by the
// display event handler. Furthermore, it checks the states of the
// up and down ('u', 'n') inputs and sets the player movement accordingly.
void timer_func(int ID) {
	// While the game is running, update the game timer, food timer,
	// and fan rotation every frame.
	if (gameTime > 0) {
		gameTime -= time_Delta;
		foodTime -= time_Delta;
		fanRotation += 360 * fanRotRate * time_Delta / 1000.0;
		glutTimerFunc(time_Delta, timer_func, 1); // Set the timer event handler for the next frame in 80 ms
	}

	// If the food's lifetime has expired, delete the current
	// food (if it has not been eaten) and spawn a new one
	if (foodTime < 0) {
		if (foodPos) {
			delete foodPos;
		}
		// New food at randomized coordinates with 200x200 region of the center of the front face
		foodPos = new Vector3(rand() / (float)RAND_MAX * 200 - 100, rand() / (float)RAND_MAX * 200 - 100, -51);
		foodTime = 4000; // Reset the food timer
	}
	// Delete the food once it has existed for three seconds, wait
	// for one more second before spawning more food
	else if (foodTime <= 1000 && foodPos) {
		delete foodPos;
		foodPos = NULL;
	}

	// Check for collision between the fish and the food.
	// Food is collected within 50 units of the center
	// of the fish.
	if (foodPos && fishPos->distance(*foodPos) <= 50) {
		foodTime = 1000; // Set food timer to one second, deleting the food
		score += 10;	// Increment score by 10
		fishScaleTime = 1000; // Set initial one second timer (before scale)
		fishFlag = GROWING;	// Set fish state to GROWING (state leading to increased fish scale)
		delete foodPos;	// Delete the food
		foodPos = NULL;
	}
	// If the fish has recently eaten food, manage the states associated with increasing
	// the scale of the fish
	if (fishFlag != REGULAR) {
		fishScaleTime -= time_Delta;

		if (fishScaleTime <= 0) { // Check for timer elapsed
			if (fishFlag == GROWING) { // Check for initial growing state
				fishScaleTime = 1000; // Re-set one second timer
				fishScale = 1.1; // Enlargen the fish by 0.1
				fishFlag = LARGE; // Set fish to next state, LARGE
			}
			else {	// Check for second timer elapsed (LARGE state after one second of scaling)
				fishScaleTime = -1;	// Disable the fish scale timer
				fishScale = 1.0;	// Reset the scale of the fish
				fishFlag = REGULAR;	// Reset the state of the fish to REGULAR
			}
		}
	}

	glutPostRedisplay();	// Draw this frame's changes to the scene objects
}


// key_func is the event handler for keyboard key presses. This event
// handler checks the currently pressed key and sets corresponding program states.
// Allows for interactivity: move the fish left and right with 'h' and 'j'
// respectively, and up and down with 'u' and 'n' respectively.
// Key presses modify the global state variable for the position
// of the fish, that controls the position of the fish in the
// display callback function.
void key_func(unsigned char key, int x, int y) { 
	switch (key) {

		// Move the fish to the left on 'h' press
	case 'h':
		fishPos->x -= 10;
		if (fishPos->x < -80) // Clamp fish position, prevent leaving the tank
			fishPos->x = -79;
		break;

		// Move the fish to the right on 'j' press
	case 'j':
		fishPos->x += 10;
		if (fishPos->x > 80)
			fishPos->x = 79;
		break;

		// Move the fish upwards on 'u' press
	case 'u':
		fishPos->y += 10;
		if (fishPos->y > 112.5)
			fishPos->y = 111;
		break;

		// Move the fish downwards on 'u' press
	case 'n':
		fishPos->y -= 10;
		if (fishPos->y < -112.5)
			fishPos->y = -111;
		break;


	}
}

// initializeObjects creates display lists for
// the various scene objects, seeds rand,
// and sets the initial position vector
// for the fish.
void initializeObjects() {
	srand(time(0));

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Fish starts in the middle of the front
	// face of the tank.
	fishPos = new Vector3(0.0, 0.0, -51.0);

	// Generate display list for the fish
	fishDisplayInd = glGenLists(1);
	glNewList(fishDisplayInd, GL_COMPILE);
	glColor3f(1.0, 0.65, 0.0);
	// Draw the body
	glPushMatrix();
	glTranslatef(-7.5, 0.0, 0.0);
	glScalef(75.0 / 2.0, 25.0 / 2.0, 25.0 / 2.0);
	glutWireOctahedron();
	glPopMatrix();
	glPushMatrix();
	// Draw the fin
	glBegin(GL_LINE_LOOP);
	glTranslatef(75.0 / 2 - 7.5, 0.0, 0.0);
	glVertex3f(30.0, 0.0, 0.0);
	glVertex3f(45.0, -25.0 / 2.0, 0);
	glVertex3f(45.0, 25.0 / 2.0, 0);
	glEnd();
	glPopMatrix();
	glEndList();

	// Generate display list for the yellow fish tank
	tankDisplayInd = glGenLists(1);
	glNewList(tankDisplayInd, GL_COMPILE);
	glTranslatef(0.0, 0.0, -50.0 - tankScale / 2.0);
	glColor3f(1.0, 1.0, 0.0);
	glutWireCube(tankScale);
	glEndList();

	// Generate display list for a yellow fan blade
	// with green lights. The fan blade is lit
	// using an ambient light.
	fanDisplayInd = glGenLists(1);
	glNewList(fanDisplayInd, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);
	glPushMatrix();
	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	glBegin(GL_TRIANGLES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 50.0, 0.0);
	glVertex3f(-25.0, 43.0, 0.0);
	glEnd();
	glDisable(GL_LIGHTING);
	// Green fan lights
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_POINTS);
	glVertex3f(0.0, 12.5, 0.0);
	glVertex3f(0.0, 25.0, 0.0);
	glVertex3f(0.0, 37.5, 0.0);
	glVertex3f(-6.25, 10.75, 0.0);
	glVertex3f(-12.5, 21.5, 0.0);
	glVertex3f(-18.75, 32.25, 0.0);
	glEnd();
	glPopMatrix();
	glEndList();
	
	// Generate display list for food
	foodDisplayInd = glGenLists(1);
	glNewList(foodDisplayInd, GL_COMPILE);
	glColor3f(1.0, 1.0, 1.0);
	glutWireSphere(5.0, 4, 4);
	glEndList();

}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	my_setup(canvas_Width, canvas_Height, canvas_Name);
	initializeObjects();// Initialize scene objects

	glutDisplayFunc(display_func); /* registers the display callback */
	glutKeyboardFunc(key_func);	// Registers the keyboard press callback
	glutTimerFunc(time_Delta, timer_func, 1); // Register the keyboard event callback

	glutMainLoop(); /* execute until killed */
	return 0;
}