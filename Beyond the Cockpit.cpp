/* *******************************************************************************************************************************
CTIS - 164 - HW 1
PROJECT NAME : "BEYOND THE COCKPIT : CRYPTO VIEW" 
NAME: MOHAMMED EHSAN ULLAH SHAREEF
BUILD TIME : 30+ HOURS
MANDATORY CRITERIAS : ALL MET


ADDITIONAL FEATURES : 
1 : Enhanced Window/Cockpit Design:
Detailed GRADIENT window with a unique cockpit-like design
Decorative top insignia with light patterns
Custom wall pattern with textured appearance

2. Multiple Object Selection System:
You can switch between 3 objects: Ethereum, Bitcoin, and a Moon.
Keyboard: Press "1" for Ethereum, "2" for Bitcoin, "3" for Moon.
Mouse Clicks: Click inside the object: Left click for Ethereum, middle click for Bitcoin, right click for Moon.
The program checks if the mouse click is within a rectangle or circle that surrounds the object.
The objects are designed to look 3D.

3. Interactive Object Control:
Arrow keys to control object movement (up/down) and speed (left/right)
Reverse motion capability (negative speed)
on high speed mirage of multiple objects, or 3D spin
obj Speed display in the information area

4. Advanced Window Blind System:
Adjustable window blinds using W/S keys
Blinds that can be moved up and down within the window frame

5. Application State System:
Finite state machine with four states: START, TRANSITION, RUN, END_TRANSITION
Smooth transitions between states with animation effects
I asked you in the class for implimentation with one timmer but i couldn't figure it out so used 3  timmers
mathematical func to find currect blind position
Click areas for state transitions (rectangles at bottom of screen)
or click <Enter> to toggle between different states

6. User Interface Elements:
Informative text display showing current speed and controls
Visual feedback for pause/play states
Transition status indicators

7. Window Positioning:
Window centering on screen startup for better user experience
calculations made for ideal viewing experience, a bit above centre is final placement
Adjustable window dimensions

8.Smoothing:
Enhanced rendering features used

9. Pause/Resume Functionality:
Space bar to pause and resume animation
Visual indicator of pause state

10. Escape system:
press <Esc> to end program completely

Request : I couldn't attend the pop quiz as I was in Istanbul, writing another exam, 
if my extra efforts in this HW can be counted for the pop quiz, I would be greatful. 

******************************************************************************************************************************** */

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

// TEMPLATE CONSTANTS AND VARIABLES
#define WINDOW_WIDTH  1200
#define WINDOW_HEIGHT 800
#define PI  3.14159265358
#define TIME_PERIOD 20 // Animation update interval in milliseconds

//OBJECTS FOR MODE SELECTION
#define ETHERIUM 0
#define BITCOIN 1
#define MOON 2

// FINITE STATE MACHINE SELECTION
#define  START 0
#define  RUN   1
#define TRANSITION 2
#define END_TRANSITION 3  

// global variables for transition between START -> RUN
bool transitionActive = false;
int transitionDuration = 3000; // 3 seconds in milliseconds
int transitionSteps = 300; // Number of steps for the animation
int transitionStep = 0;
int initialBlindPos = 0; // Starting position for transition
int targetBlindPos = 0;  // Target position for transition
int initialRectPos = -270;
int targetRectPos = -300;
int currentRectPos = -270;

// global variables for the reverse transition
bool reverseTransitionActive = false;
int reverseTransitionStep = 0;
int reverseInitialBlindPos = 0;
int reverseTargetBlindPos = 0;
int reverseInitialRectPos = -300;
int reverseTargetRectPos = -270;
int reverseCurrentRectPos = -300;


// finite state machine global variables
int appState = START;

bool activeTimer = false;  // Controls animation state
bool up = false, down = false, right = false, left = false;  // Direction flags
int oX = 0, oY = 0;        // All object position coordinates [Etherium, Bitcoin, Moon]
int sY = 250;              // Initial position of window shade during run mode
int osY = -250;            // Initial position of window shade during start mode
int oSpeed = 4;            // Initial object speed

int mode = ETHERIUM;       // Initial object selection

// Function prototypes
void display();
void background();
void oSelection();
void etherium();
void bitcoin();
void moon();
void topInsignia();
void windows();
void display_start();
void display_run();
void blinds();
void onTimer(int v);
void onKeyDown(unsigned char key, int x, int y);
void onSpecialKeyDown(int key, int x, int y);
void onClick(int button, int state, int x, int y);
void onResize(int w, int h);
void Init();
void circle(int x, int y, int r);
void circle_wire(int x, int y, int r);
void vprint(int x, int y, void *font, const char *string, ...);
void wallPattern();
void lightPattern();
bool isInsideRect(int x, int y, int x1, int y1, int x2, int y2);
void onTransitionTimer(int v);
void startTransition();
void onReverseTransitionTimer(int v);
void startReverseTransition();

void onResize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// To draw a filled circle, centered at (x,y) with radius r
void circle(int x, int y, int r) {
	float angle;
	glBegin(GL_POLYGON);
	for (int i = 0; i < 100; i++) {
		angle = 2 * PI*i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

// To draw a hollow circle, centered at (x,y) with radius r
void circle_wire(int x, int y, int r) {
	float angle;

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 100; i++) {
		angle = 2 * PI*i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

// Enables smooth rendering of shapes
void Init() {
	//Smoothing shapes
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
}

// function to display text in the window
void vprint(int x, int y, void *font, const char *string, ...) {
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);

	int len, i;
	glRasterPos2f(x, y);
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
		glutBitmapCharacter(font, str[i]);
}

//Creates a pattern of yellow lights
void lightPattern(void)
{
	int s = 5;
	int px = -32, py = 445;
	for (int r = 0; r < 14; r++)
	{
		for (int c = 0; c < 7; c++)
		{
			glColor3ub(255, 255, 0);
			glRectf(px, py, (px + s), (py - s));
			px += 10;
		}
		py -= 10;
		px = -32;
	}
}

//Draws textured walls for the cockpit
void wallPattern(void)
{
	int s = 10;
	int px = -595, py = 435;
	for (int r = 0; r < 44; r++)
	{
		for (int c = 0; c < 6; c++)
		{
			glColor3ub(255, 229, 204);
			glRectf(px, py, (px + s), (py - s));
			px += 20;
		}
		py -= 20;
		px = -595;
	}

	px = 585;
	py = 435;
	for (int r = 0; r < 44; r++)
	{
		for (int c = 0; c < 6; c++)
		{
			glColor3ub(255, 229, 204);
			glRectf(px, py, (px + s), (py - s));
			px -= 20;
		}
		py -= 20;
		px = 585;
	}

	px = -475, py = 435;
	for (int r = 0; r < 8; r++)
	{
		for (int c = 0; c < 15; c++)
		{
			glColor3ub(255, 229, 204);
			glRectf(px, py, (px + s), (py - s));
			px += 20;
		}
		py -= 20;
		px = -475;
	}

	px = 465;
	py = 435;
	for (int r = 0; r < 8; r++)
	{
		for (int c = 0; c < 15; c++)
		{
			glColor3ub(255, 229, 204);
			glRectf(px, py, (px + s), (py - s));
			px -= 20;
		}
		py -= 20;
		px = 465;
	}

}

//Draws the decorative top element
void topInsignia(void) {

	// DRAW a POLYGON
	glColor3f(0.4, 0.4, 1);
	glBegin(GL_POLYGON);
	glVertex2f(-110, 450);
	glVertex2f(-57, 292);
	glVertex2f(0, 280);
	glVertex2f(57, 292);
	glVertex2f(110, 450);
	glVertex2f(-110, 450);
	glEnd();
	// DRAW INNER POLYGON
	//window border innermost cream

	glColor3f(0.9, 0.9, 0.9);
	glBegin(GL_POLYGON);
	glVertex2f(-90, 450);
	glVertex2f(-37, 302);
	glVertex2f(0, 292);
	glVertex2f(37, 302);
	glVertex2f(90, 450);
	glEnd();

	lightPattern();

	glLineWidth(3);
	// DRAW A LINE STRIP (CONNECTED LINES)
	glColor3f(1.0, 0.9, 0.8);
	glBegin(GL_LINE_STRIP);
	glVertex2f(-110, 450);
	glVertex2f(-57, 292);
	glVertex2f(0, 280);
	glVertex2f(57, 292);
	glVertex2f(110, 450);
	glEnd();

	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_LINE_STRIP);
	glVertex2f(-90, 450);
	glVertex2f(-37, 302);
	glVertex2f(0, 292);
	glVertex2f(37, 302);
	glVertex2f(90, 450);
	glEnd();

}

// raws the main viewport window
void windows(void) {
	glColor3ub(40, 40, 60); //window border outermost blue
	glRectf(-470, 270, 470, -270);

	glColor3ub(255, 229, 204); //window border innermost cream
	glRectf(-460, 260, 460, -260);

	glBegin(GL_QUADS); //cockpit window
	glColor3f(0.2, 0.3, 0.5); // blueish
	glVertex2f(-450, 250);    // Top-left corner
	glColor3f(0.2, 0.4, 0.4); //light aurora colour
	glVertex2f(450, 250);     // Top-right corner
	glColor3f(0.2, 0.2, 0.3); // dark blue
	glVertex2f(450, -250);    // Bottom-right corner
	glVertex2f(-450, -250);   // Bottom-left corner
	glEnd();

}

//Writes the commands area with text
void bottom(void) {
	glColor3f(1, 1, 1);
	if (!activeTimer)
		vprint(-30, -220, GLUT_BITMAP_HELVETICA_12, "Press <F1>");
	else if (oSpeed == 0)
		vprint(-45, -220, GLUT_BITMAP_HELVETICA_12, "Play <Space>");
	else if (oSpeed > 0)
	{
		vprint(-30, -200, GLUT_BITMAP_HELVETICA_12, "Speed : %d", oSpeed);
		vprint(-45, -220, GLUT_BITMAP_HELVETICA_12, "Pause <Space>");
	}
	else if (oSpeed < 0)
	{
		vprint(-35, -200, GLUT_BITMAP_HELVETICA_12, "Reverse : %d", -oSpeed);
		vprint(-45, -220, GLUT_BITMAP_HELVETICA_12, "Pause <Space>");
	}
}

//Draw the three different objects
void etherium(void) {

	glColor3ub(255, 255, 0);
	circle(oX + 0, oY + 0, 125);
	glColor3ub(225, 225, 0);
	circle(oX + 0, oY + 0, 112.5);


	glBegin(GL_TRIANGLES);
	//Centre-right corner
	glColor3f(0.4, 0.4, 0.4);
	glVertex2f(oX + 0, oY + 20);
	glVertex2f(oX + 0, oY + -20);
	glVertex2f(oX + 41, oY + 0);

	//Centre-left corner
	glColor3f(0.3, 0.3, 0.3);
	glVertex2f(oX + 0, oY + 20);
	glVertex2f(oX + 0, oY + -20);
	glVertex2f(oX + -41, oY + 0);

	//Top-right corner
	glColor3f(0.5, 0.5, 0.5);
	glVertex2f(oX + 3, oY + 70);
	glVertex2f(oX + 0, oY + 24);
	glVertex2f(oX + 42, oY + 0);

	//Top-left corner
	glColor3f(0.6, 0.6, 0.6);
	glVertex2f(oX + 3, oY + 70);
	glVertex2f(oX + 0, oY + 24);
	glVertex2f(oX + -42, oY + 0);

	//Bottom-right corner
	glBegin(GL_TRIANGLES);
	glColor3f(0.6, 0.6, 0.6);
	glVertex2f(oX + 0, oY + -53);
	glVertex2f(oX + 0, oY + -24);
	glVertex2f(oX + 42, oY + -3);

	//Bottom-left corner
	glColor3f(0.5, 0.5, 0.5);
	glVertex2f(oX + 0, oY + -53);
	glVertex2f(oX + 0, oY + -24);
	glVertex2f(oX + -42, oY + -3);
	glEnd();

	glLineWidth(3);
	glBegin(GL_LINES);
	// Line 1
	glColor3f(0.3, 0.3, 0.3);
	glVertex2f(oX + 0, oY + -52);
	glVertex2f(oX + 0, oY + -23);
	glEnd();

	glLineWidth(3);
	// LINE STRIPS 
	//BOTTOM CONCAVE

	glColor3f(0.3, 0.3, 0.3);
	glBegin(GL_LINE_STRIP);
	glVertex2f(oX + -41, oY + -3);
	glVertex2f(oX + 0, oY + -24);
	glVertex2f(oX + 41, oY + -3);
	glEnd();

	//TOP CONCAVE
	glColor3f(0.3, 0.3, 0.3);
	glBegin(GL_LINE_STRIP);
	glVertex2f(oX + -42, oY + 0);
	glVertex2f(oX + 0, oY + 24);
	glVertex2f(oX + 42, oY + 0);
	glEnd();

	//TOP COVERING
	glColor3f(0.3, 0.3, 0.3);
	glBegin(GL_LINE_STRIP);
	glVertex2f(oX + -41, oY + 3);
	glVertex2f(oX + 0, oY + 70);
	glVertex2f(oX + 41, oY + 3);
	glEnd();

	//BOTTOM COVERING
	glColor3f(0.3, 0.3, 0.3);
	glBegin(GL_LINE_STRIP);
	glVertex2f(oX + -41, oY + -3);
	glVertex2f(oX + 0, oY + -53);
	glVertex2f(oX + 41, oY + -3);
	glEnd();



}
void bitcoin(void) {
	glColor3ub(255, 255, 0);
	circle(oX + 0, oY + 0, 125); //outer gold circle
	glColor3f(0.8, 0.8, 0.8);
	circle(oX + 0, oY + 0, 112.5); //inner silver circle

	glColor3ub(255, 255, 0);
	circle(oX + 0, oY + 30, 37.5); //upper gold B

	glColor3ub(255, 255, 0);
	circle(oX + 0, oY - 30, 37.5); //lower gold B

	glColor3f(0.7, 0.7, 0.7);
	circle(oX - 5, oY + 28, 22.5); //upper silver B

	glColor3f(0.7, 0.7, 0.7);
	circle(oX - 5, oY - 28, 22.5); //upper silver B

	glColor3ub(255, 255, 0);
	glRectf(oX + -37.5, oY + 83, oX + -17.5, oY + -83); // Draw a long gold rectangle

	glColor3ub(255, 255, 0);
	glRectf(oX + -6, oY + 83, oX + 7, oY + 65); // Draw short top gold rectangle

	glColor3ub(255, 255, 0);
	glRectf(oX + -6, oY + -83, oX + 7, oY + -65); // Draw short bottom gold rectangle

	glColor3ub(255, 255, 0);
	glRectf(oX + -52, oY + 54, oX + -37.5, oY + 40); // Draw short top-left gold rectangle

	glColor3ub(255, 255, 0);
	glRectf(oX + -52, oY + -54, oX + -37.5, oY + -40); // Draw short bottom-left gold rectangle
}
void moon(void) {
	glColor3f(0.8, 0.8, 0.8);
	circle(oX + 0, oY + 0, 125); //outer silver circle

	glColor3f(0.7, 0.7, 0.7);
	circle(oX + 40, oY + -75, 20); //outer lighter silver circle


	glColor3f(0.6, 0.6, 0.6);
	circle(oX + 40, oY + -75, 16); //inner darker silver circle


	glColor3f(0.7, 0.7, 0.7);
	circle(oX + 20, oY + -25, 15); //outer lighter silver circle


	glColor3f(0.6, 0.6, 0.6);
	circle(oX + 20, oY + -25, 12); //inner darker silver circle

	glColor3f(0.7, 0.7, 0.7);
	circle(oX + -20, oY + -25, 10); //outer lighter silver circle


	glColor3f(0.6, 0.6, 0.6);
	circle(oX + -20, oY + -25, 7); //inner darker silver circle

	glColor3f(0.7, 0.7, 0.7);
	circle(oX + -10, oY + -65, 12); //outer lighter silver circle


	glColor3f(0.6, 0.6, 0.6);
	circle(oX + -10, oY + -65, 7); //inner darker silver circle

	glColor3f(0.7, 0.7, 0.7);
	circle(oX + -40, oY + -75, 12); //outer lighter silver circle

	glColor3f(0.6, 0.6, 0.6);
	circle(oX + -40, oY + -5, 7); //inner darker silver circle

	glColor3f(0.6, 0.6, 0.6);
	circle(oX + 40, oY + 25, 7); //inner darker silver circle

	glColor3f(0.7, 0.7, 0.7);
	circle(oX + 40, oY + 25, 12); //outer lighter silver circle

	glColor3f(0.6, 0.6, 0.6);
	circle(oX + 60, oY + 60, 7); //inner darker silver circle

	glColor3f(0.7, 0.7, 0.7);
	circle(oX + -60, oY + 75, 20); //outer lighter silver circle


	glColor3f(0.6, 0.6, 0.6);
	circle(oX + -60, oY + 75, 16); //inner darker silver circle


	glColor3f(0.7, 0.7, 0.7);
	circle(oX + 0, oY + 90, 15); //outer lighter silver circle


	glColor3f(0.6, 0.6, 0.6);
	circle(oX + 0, oY + 90, 12); //inner darker silver circle

	glLineWidth(3);
	glColor3f(0.3, 0.3, 0.3);
	circle_wire(oX + 0, oY + 0, 125); //hollow circle covering 

}

//Draws adjustable window blinds in Run mode & reverse transition
void blinds(void) {
	// Draw the blind
	glColor3f(0.9, 0.9, 0.9); //off white
	glBegin(GL_QUADS);
	glVertex2f(-450, 250);
	glColor3f(0.9, 0.9, 0.7); //aurora tint
	glVertex2f(450, 250);
	glColor3f(0.3, 0.3, 0.3); //charcoal grey
	glVertex2f(450, sY);
	glVertex2f(-450, sY);
	glEnd();
	glutPostRedisplay();
}

// Draws adjustable window blinds in start mode & transition
void blinds_off(void) {
	// Draw the blind
	glColor3f(0.9, 0.9, 0.9); //off white
	glBegin(GL_QUADS);
	glVertex2f(-450, 250);
	glColor3f(0.9, 0.9, 0.7); //aurora tint
	glVertex2f(450, 250);
	glColor3f(0.3, 0.3, 0.3); //charcoal grey
	glVertex2f(450, osY);
	glVertex2f(-450, osY);
	glEnd();
	glutPostRedisplay();
}

void background() {
	glClearColor(0.3, 0.3, 0.3, 1.0); // charcoal grey background
	glClear(GL_COLOR_BUFFER_BIT);
	wallPattern(); // calling all static patterns
	topInsignia();
	windows();
}

//object selection menu [Etherium, Bitcoin, Moon)
void oSelection() {
	switch (mode) {
	case ETHERIUM:
		etherium();
		break;
	case BITCOIN:
		bitcoin();
		break;
	case MOON:
		moon();
		break;
	}
}

// Controls object animation, moving it across the screen and respawning on other side
void onTimer(int v) {
	glutTimerFunc(TIME_PERIOD, onTimer, 0);

	if (activeTimer) {
		oX += oSpeed;
		if (oX > 330) //limiting
			oX = -330; //respawning
		if (oX < -330) //limiting 
			oX = 330; //respawning
	}

	glutPostRedisplay();
}


// Function to handle transition timer
void onTransitionTimer(int v) {
	if (transitionActive) {
		transitionStep++;
		float progress = (float)transitionStep / transitionSteps;

		// Calculate the current positions
		osY = initialBlindPos + (targetBlindPos - initialBlindPos) * progress;
		currentRectPos = initialRectPos + (targetRectPos - initialRectPos) * progress;

		// Checker to see if transition complete
		if (transitionStep >= transitionSteps) {
			transitionActive = false;
			appState = RUN;  // change to RUN mode
			sY = osY;        // Syncing the 2 blind positions
		}
		else {
			glutTimerFunc(transitionDuration / transitionSteps, onTransitionTimer, 0);
		}
	}
	glutPostRedisplay();
}

// Function to start the transition
void startTransition() {
	if (appState == START) {
		appState = TRANSITION;
		transitionActive = true;
		transitionStep = 0;

		// Initialize blind transition parameters
		initialBlindPos = osY;
		targetBlindPos = 250; // Move blinds until window top

		// rectangle transition parameters
		initialRectPos = -270;
		targetRectPos = -300;
		currentRectPos = initialRectPos;

		// calling timer to animate
		glutTimerFunc(transitionDuration / transitionSteps, onTransitionTimer, 0);
	}
}

// Function to handle reverse transition timer
void onReverseTransitionTimer(int v) {
	if (reverseTransitionActive) {
		reverseTransitionStep++;
		float progress = (float)reverseTransitionStep / transitionSteps;

		// Calculate the current positions
		sY = reverseInitialBlindPos + (reverseTargetBlindPos - reverseInitialBlindPos) * progress;
		reverseCurrentRectPos = reverseInitialRectPos + (reverseTargetRectPos - reverseInitialRectPos) * progress;

		// checker to see if reverse transition done
		if (reverseTransitionStep >= transitionSteps) {
			reverseTransitionActive = false;
			appState = START;  // change to START mode
			osY = sY;         // Syncing the blind positions
		}
		else {
			glutTimerFunc(transitionDuration / transitionSteps, onReverseTransitionTimer, 0);
		}
	}
	glutPostRedisplay();
}

// Function to start reverse transition
void startReverseTransition() {
	if (appState == RUN) {
		appState = END_TRANSITION;
		reverseTransitionActive = true;
		reverseTransitionStep = 0;

		// blind transition parameters
		reverseInitialBlindPos = sY;
		reverseTargetBlindPos = -250; // Move blind till end of window

		// rectangle transition parameters
		reverseInitialRectPos = -300;
		reverseTargetRectPos = -270;
		reverseCurrentRectPos = reverseInitialRectPos;

		// calling reverse transition timer
		glutTimerFunc(transitionDuration / transitionSteps, onReverseTransitionTimer, 0);
	}
}


// Handles regular key presses:
// - ESC to exit
// - Space to pause/resume animation
// - 1,2,3 to switch between objects
// - W/S to move window shade up/down
// - Enter to toggle between app states
void onKeyDown(unsigned char key, int x, int y) {


	if (key == ' ' && appState == RUN) {
		if (oSpeed != 0)
			oSpeed = 0;
		else
			oSpeed = 4;
	}
	if (key == 27) {   // 27 is Escape key
		printf("End of program.\n");
		exit(0);
	}

	if (key == '1') {
		mode = ETHERIUM;  // mode to Ethereum
	}
	else if (key == '2') {
		mode = BITCOIN;   //  mode to Bitcoin
	}
	else if (key == '3') {
		mode = MOON;      //  mode to Moon
	}
	if (key == 'w' || key == 'W')
	{
		if (sY <= 240) //control to not exit window
			sY += 10; // move shade up
		if (osY <= 240)
			osY += 10;
	}
	else if (key == 's' || key == 'S')
	{
		if (sY >= -240) //control to not exit window
			sY -= 10; //move shade down
		if (osY >= -240)
			osY -= 10;
	}
	if (key == 13) {  // 13 is enter key
		if (appState == START)
			startTransition(); // Start transition from START to RUN
		else if (appState == RUN)
			startReverseTransition(); // Start transition from RUN to START
	}

	glutPostRedisplay();
}

// Handles special keys:
// GLUT_KEY_DOWN, GLUT_KEY_F1
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_LEFT, GLUT_KEY_RIGHT
	// - F1 to activate animation
	// - Arrow keys to move object and change speed
void onSpecialKeyDown(int key, int x, int y) {
	if (key == GLUT_KEY_F1)
		activeTimer = true;
	switch (key) {
	case GLUT_KEY_UP:
		up = true;
		if (oY < 121) //control
			oY += 10; // to move up
		break;
	case GLUT_KEY_DOWN:
		down = true;
		if (oY > -130) //control
			oY -= 10; //move down
		break;
	case GLUT_KEY_LEFT:
		left = true;
		oSpeed -= 1;//to increase speed -ve
		break;
	case GLUT_KEY_RIGHT:
		right = true;
		oSpeed += 1; //to increase speed +ve
		break;
	}

	// To refresh the window it calls display() function
	glutPostRedisplay();

}

// display during START state
void display_start() {
	activeTimer = false; //stopping timer each time as we move from RUN -> START. To stop object moving in START state.
	blinds_off();

	glBegin(GL_QUADS);
	glColor3f(0.3, 0.3, 0.3);	// Dark Gray
	glVertex2f(-165, -270);
	glColor3ub(255, 229, 204); //cream
	glVertex2f(-165, -360);
	glColor3ub(255, 229, 204); //cream
	glVertex2f(165, -360);
	glColor3f(0.3, 0.3, 0.3);	// Dark Gray
	glVertex2f(165, -270);
	glEnd();

	glColor3f(1, 1, 1);
	vprint(-160, -325, GLUT_BITMAP_HELVETICA_18, "Beyond the Cockpit: Crypto Encounter");
	vprint(-145, -350, GLUT_BITMAP_HELVETICA_18, "ID: Mohammed Ehsan Ullah Shareef");

	glBegin(GL_QUADS);
	glColor3f(0.3, 0.3, 0.3);	// Dark Gray
	glVertex2f(-360, -270);
	glColor3ub(255, 229, 204); //cream
	glVertex2f(-360, -360);
	glColor3ub(255, 229, 204); //cream
	glVertex2f(-250, -360);
	glColor3f(0.3, 0.3, 0.3);	// Dark Gray
	glVertex2f(-250, -270);

	glColor3f(0.3, 0.3, 0.3);	// Dark Gray
	glVertex2f(280, -270);
	glColor3ub(255, 229, 204); //cream
	glVertex2f(280, -360);
	glColor3ub(255, 229, 204); // cream
	glVertex2f(390, -360);
	glColor3f(0.3, 0.3, 0.3);	// Dark Gray
	glVertex2f(390, -270);
	glEnd();

	glColor3f(1, 1, 1); //white
	vprint(-343, -325, GLUT_BITMAP_HELVETICA_12, "Click to Enter");
	vprint(-330, -340, GLUT_BITMAP_HELVETICA_12, "<Enter>");
	vprint(297, -325, GLUT_BITMAP_HELVETICA_12, "Click to Enter");
	vprint(310, -340, GLUT_BITMAP_HELVETICA_12, "<Enter>");

	glutPostRedisplay();
}

// display during RUN state
void display_run() {


	glBegin(GL_QUADS);

	glColor3ub(255, 229, 204); //cream
	glVertex2f(-165, -300);
	glVertex2f(-165, -360);
	glVertex2f(165, -360);
	glVertex2f(165, -300);
	glEnd();

	glColor3f(0.3, 0.3, 0.3); //charcoal grey
	vprint(-160, -325, GLUT_BITMAP_HELVETICA_18, "Beyond the Cockpit: Crypto Encounter");
	vprint(-145, -350, GLUT_BITMAP_HELVETICA_18, "ID: Mohammed Ehsan Ullah Shareef");

	glutSpecialFunc(onSpecialKeyDown);
	oSelection();
	bottom();
	blinds();

	glBegin(GL_QUADS);
	glColor3ub(255, 229, 204); //cream
	glVertex2f(-360, -300);
	glVertex2f(-360, -360);
	glVertex2f(-250, -360);
	glVertex2f(-250, -300);

	glColor3ub(255, 229, 204); //cream
	glVertex2f(280, -300);
	glVertex2f(280, -360);
	glVertex2f(390, -360);
	glVertex2f(390, -300);
	glEnd();

	glColor3f(0.3, 0.3, 0.3); //charcoal grey
	vprint(-350, -325, GLUT_BITMAP_HELVETICA_12, "Click to Escape");
	vprint(-330, -340, GLUT_BITMAP_HELVETICA_12, "<Enter>");
	vprint(292, -325, GLUT_BITMAP_HELVETICA_12, "Click to Escape");
	vprint(310, -340, GLUT_BITMAP_HELVETICA_12, "<Enter>");

	glutPostRedisplay();
}

// display during TRANSITION state
void display_transition() {
	blinds_off();


	glBegin(GL_QUADS);
	glColor3f(0.3, 0.3, 0.3);	// Dark Gray
	glVertex2f(-165, currentRectPos);
	glColor3ub(255, 229, 204); // cream
	glVertex2f(-165, -360);
	glColor3ub(255, 229, 204); // cream
	glVertex2f(165, -360);
	glColor3f(0.3, 0.3, 0.3);	// Dark Gray
	glVertex2f(165, currentRectPos);
	glEnd();

	glColor3f(1, 1, 1);
	vprint(-160, -325, GLUT_BITMAP_HELVETICA_18, "Beyond the Cockpit: Crypto Encounter");
	vprint(-145, -350, GLUT_BITMAP_HELVETICA_18, "ID: Mohammed Ehsan Ullah Shareef");

	glBegin(GL_QUADS);
	glColor3f(0.3, 0.3, 0.3);	// Dark Gray
	glVertex2f(-360, currentRectPos);
	glColor3ub(255, 229, 204); //cream
	glVertex2f(-360, -360);
	glColor3ub(255, 229, 204); //cream
	glVertex2f(-250, -360);
	glColor3f(0.3, 0.3, 0.3);	// Dark Gray
	glVertex2f(-250, currentRectPos);

	glColor3f(0.3, 0.3, 0.3);	// Dark Gray
	glVertex2f(280, currentRectPos);
	glColor3ub(255, 229, 204); //cream
	glVertex2f(280, -360);
	glColor3ub(255, 229, 204); //cream
	glVertex2f(390, -360);
	glColor3f(0.3, 0.3, 0.3);	// Dark Gray
	glVertex2f(390, currentRectPos);
	glEnd();


	glColor3f(1, 1, 1); // white
	vprint(-340, -325, GLUT_BITMAP_HELVETICA_12, "Transitioning...");
	vprint(297, -325, GLUT_BITMAP_HELVETICA_12, "Transitioning...");

	glutPostRedisplay();
}

//display during REVERSE TRANSITION state
void display_reverse_transition() {
	
	activeTimer = false; //stopping timer each time as we move from RUN -> START. To stop object moving in START state.

	glBegin(GL_QUADS);
	glColor3f(0.3, 0.3, 0.3);  // Dark Gray
	glVertex2f(-165, reverseCurrentRectPos);
	glColor3ub(255, 229, 204); //  cream
	glVertex2f(-165, -360);
	glColor3ub(255, 229, 204); // cream
	glVertex2f(165, -360);
	glColor3f(0.3, 0.3, 0.3);  // Dark Gray
	glVertex2f(165, reverseCurrentRectPos);
	glEnd();

	glColor3f(1, 1, 1); // white
	vprint(-160, -325, GLUT_BITMAP_HELVETICA_18, "Beyond the Cockpit: Crypto Encounter");
	vprint(-145, -350, GLUT_BITMAP_HELVETICA_18, "ID: Mohammed Ehsan Ullah Shareef");

	glutSpecialFunc(onSpecialKeyDown);
	oSelection();
	bottom();
	blinds();

	
	glBegin(GL_QUADS);
	glColor3f(0.3, 0.3, 0.3);  // Dark Gray
	glVertex2f(-360, reverseCurrentRectPos);
	glColor3ub(255, 229, 204); //  cream
	glVertex2f(-360, -360);
	glColor3ub(255, 229, 204); //  cream
	glVertex2f(-250, -360);
	glColor3f(0.3, 0.3, 0.3);  // Dark Gray
	glVertex2f(-250, reverseCurrentRectPos);

	glColor3f(0.3, 0.3, 0.3);  // Dark Gray
	glVertex2f(280, reverseCurrentRectPos);
	glColor3ub(255, 229, 204); //  cream
	glVertex2f(280, -360);
	glColor3ub(255, 229, 204); //  cream
	glVertex2f(390, -360);
	glColor3f(0.3, 0.3, 0.3);  // Dark Gray
	glVertex2f(390, reverseCurrentRectPos);
	glEnd();

	glColor3f(1, 1, 1); // white
	vprint(-340, -325, GLUT_BITMAP_HELVETICA_12, "Returning...");
	vprint(300, -325, GLUT_BITMAP_HELVETICA_12, "Returning...");

	glutPostRedisplay();
}

// app state selection menu (START -> TRANSITION -> RUN -> END_TRANSITION -> START)
void fSelection() {
	switch (appState) {
	case START: display_start(); break;
	case TRANSITION: display_transition(); break;
	case RUN: display_run(); break;
	case END_TRANSITION: display_reverse_transition(); break;
	}
}

// Function to check if a point (x,y) is inside a rectangle
bool isInsideRect(int x, int y, int x1, int y1, int x2, int y2) {
	// Convert glut coordinates to OpenGL coordinate system
	int winX = x - WINDOW_WIDTH / 2;
	int winY = WINDOW_HEIGHT / 2 - y;

	// Check if the point is inside the rectangle
	return (winX >= x1 && winX <= x2 && winY >= y1 && winY <= y2);
}

// button : GLUT_LEFT_BUTTON, GLUT_RIGHT_BUTTON, GLUT_MIDDLE_BUTTON
// state  : GLUT_DOWN, GLUT_UP
 // Detects clicks on objects:
	// - Left click for Ethereum
	// - Middle click for Bitcoin
	// - Right click for Moon
	// Only registers if clicking within the object radius
    // Only registers if click is within the animation status rectangle
void onClick(int button, int state, int x, int y) {


	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (isInsideRect(x, y, -360, -360, -250, -270) || isInsideRect(x, y, 280, -360, 390, -270)) {
			if (appState == START) {
				startTransition(); // Start transition from START to RUN
			}
			else if (appState == RUN) {
				startReverseTransition(); // Start transition from RUN to START
			}
		}
	}


	if (appState == RUN)
	{
		// Convert glut coordinates to open gl coordinate system
		int windowX = x - WINDOW_WIDTH / 2;
		int windowY = WINDOW_HEIGHT / 2 - y;

		// Calculate direct distance from click to object center
		//distance = sqrt(x2-x1)^2 + (y2-y1)^2) [eucledian formula]
		float distance = sqrt(pow(windowX - oX, 2) + pow(windowY - oY, 2));

		// Only process clicks within the object's radius
		if (state == GLUT_DOWN && distance <= 125) {
			if (button == GLUT_LEFT_BUTTON) {
				mode = ETHERIUM;  // Left click displays Ethereum
			}
			else if (button == GLUT_MIDDLE_BUTTON) {
				mode = BITCOIN;   // Middle click displays Bitcoin
			}
			else if (button == GLUT_RIGHT_BUTTON) {
				mode = MOON;      // Right click displays Moon
			}
			glutPostRedisplay();
		}

	}
	glutPostRedisplay();
}

// Main rendering function that draws everything
// Calls all other drawing functions in proper order
void display() {

	background();
	fSelection();

	glutSwapBuffers();
}


// Initializes the window and sets up all event handlers
// Positions window in center of screen
// Starts the main loop that keeps program running
int main(int argc, char* argv[]) {

	//Window Created
	glutInit(&argc, argv); // Initialize GLUT
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);  // Set the window's initial width & height
	int screenWidth = glutGet(GLUT_SCREEN_WIDTH);  // Get the width of the screen
	int screenHeight = glutGet(GLUT_SCREEN_HEIGHT); // Get the height of the screen
	glutInitWindowPosition((screenWidth - WINDOW_WIDTH) / 2, (screenHeight - WINDOW_HEIGHT) / 2 - 60); // Position the window initially in center of every screen and moved up a bit for better experience
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("Beyond the Cockpit: Crypto Encounter - By Ehsan");

	// STEP #2: Register the Events
	glutDisplayFunc(display);  // Event = DISPLAY, call display
	glutReshapeFunc(onResize); // Event = RESIZE,  call onResize

	// Keyboard Related Events
	glutKeyboardFunc(onKeyDown);


	//Mouse releated events
	glutMouseFunc(onClick);    // Event = CLICK,   call onClick

	// Timer Event
	glutTimerFunc(1000, onTimer, 0);

	Init(); // smoothning

	// Do this in an infinite loop
	glutMainLoop();
	return 0;
}
