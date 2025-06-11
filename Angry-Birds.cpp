/*
CTIS-164-HW2
Name: Mohammed Ehsan Ullah Shareef
ehsan.shareef@ug.bilkent.edu.tr
Build Time - 48+ Hours

1. Game Overview
Title: Angry Birds - Hit the Target
Objective: Shoot cannonballs to hit flying Angry Birds (red and yellow) for points before time runs out.

2. Game States (Finite State Machine)
Implemented Finite state machine

MENU
Main screen with game title, buttons (START, CREDITS, EXIT).
Displays my name and instructions.
Background includes decorative wall patterns and a TV screen

PLAY
Active gameplay state with physics-based cannonball shooting.
Angry Birds spawn and fly upward.
Score, time, and controls are displayed.

CREDITS
Shows developer credits, university, and course info.
Option to return to the menu.

GAMEOVER
Displays final score, escaped birds, and total kills.
Options to restart or exit.

3. Core Gameplay Mechanics
A. Cannon & Projectile Physics
SlingShot Mechanics:
Adjustable position (UP/DOWN keys) and speed (LEFT/RIGHT keys).
Spacebar fires the cannonball.
Parabolic trajectory calculated using quadratic equations (fx(x)) aka aimer
Ball reloads after hitting a bird or going off-screen.

B. Targets (Angry Birds)
Two Types:
Red Birds (standard, 3 pts per hit).
Yellow Birds (bonus, 10 pts per hit, faster speed).

Spawn System:
Randomly generated at the bottom of the screen.
Move upward at varying speeds (TARGET_SPEED or FAST_SPEED).
Collision detection with cannonball (distance check).

C. Scoring & Penalties
Hits:
Red: +3 points.
Yellow: +10 points.
Miss Penalty: -5 points for wasted cannonballs.
Combo System: Visual feedback for consecutive hits/misses.

D. Time Limit
Initial Duration: 30 seconds.
Timer: Counts down every second till 0

4. Visual Elements
A. Graphics & Animations
Angry Birds:
Red: Detailed figure with feathers, eyes, beak, tail and eyebrows.
Yellow: Similar to red but with gold accents.

SlingShot:
Cloud-shaped base with a dynamic rubber band animation.
Cannonball:
Gray with white holes for texture.

Background:
Gradient sky (orange/yellow).
Decorative wall patterns (checkerboard style).
TV screen with aesthetic colors.

B. UI Components
Menu: Buttons, title, and hints.
Play Screen:
Top bar: Score, kills, time remaining.
Bottom bar: Controls (e.g., "F1: Pause").
Game Over Screen: Final stats and restart option.

C. Special Effects
Hit Responses:
Floating "+X pts" text (green for red birds, gold for yellow).
Fades out over time.
Miss Info Panel: Temporary warning about penalties.

5. Controls
Key	Action
F1	Pause/resume (PLAY) or restart (GAMEOVER).
ESC	Exit (MENU) or return to menu (CREDITS/GAMEOVER).
UP/DOWN	Adjust slingshot height.
LEFT/RIGHT	Decrease/increase cannonball speed.
SPACEBAR	Fire cannonball.
C	Open credits from the menu.
*/

#define _CRT_SECURE_NO_WARNINGS

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#define WINDOW_WIDTH  1200
#define WINDOW_HEIGHT 800

//constants
#define PI  3.14159265358

//initial duration of game
#define DURATION       30 
#define TIMER_PERIOD   15 

//Finite state machine modes
#define MENU       1
#define PLAY       2
#define CREDITS    3
#define GAMEOVER   4

#define PENALIZE_SCORE 5
#define HIT_SCORE_BONUS 3

#define MAX_SPAWN      5
#define TARGET_SPEED   4
#define FAST_SPEED     8
#define SPAWN_CHANCE   15 // percent chance per spawn frame
#define BONUS_SCORE    10

#define BALL_RADIUS 30
#define BIRD_RADIUS 50
#define BALL_RISE 75
#define BALL_DEPTH 35
#define SLING_INITIALX -480
#define SLING_INITIALY 0
#define HG 5.0
#define INFO_MISS_CONTAINER_TIME 2000
#define SLING_SPEED 15
#define SPEED_TOPUP 100

int  winWidth, winHeight;     // Current Window width and height
int gameState = MENU; // menu as default state

// Structure to store point properties
typedef struct {
	float x, y;
} point_t;

// Mouse position
point_t mouse;

typedef struct {
	float	x, y;
	float   speed;
	bool	active;
	bool	bonus;
} target_t;

target_t bird[MAX_SPAWN];

#define MAX_RESPONSE 20

typedef struct {
	float x, y;     // screen position
	float t;        // elapsed time
	bool active;
	bool bonus;
} response_t;

response_t response[MAX_RESPONSE];

static int missInfoTimeAccumulator = 0;

float Cx = 0, Cy = 0;      // Coordinates of the cloud
float Sx = SLING_INITIALX, Sy = 0;	// x and y coordinate of the sling shot
float cannonBallX = Sx - BALL_DEPTH;
float cannonBallY = Sy + BALL_RISE;
float c, h, k;                // Parameters of the quadratic equation
float ballRadius = BALL_RADIUS;
float birdRadius = BIRD_RADIUS;
float slingScale = 1.0;
int Vx = 900;                // Horizontal firing speed
int escapeCnt = 0;

bool isPaused = false;
bool  animation = false;      // Flag to show if the sling is fired
bool  gameover = false;       // Flag for game checking game status
int remainingTime = DURATION;
int targetHits = 0;
int bonusHits = 0;

bool targetHit = false;
int scoreCnt = 0;
bool showMissInfo = false;
int missComboCount = 0;


//prototypes 
void restartGame();
void giveHitBonus();
void penalizeMiss();


void vprint(int x, int y, void *font, const char *string, ...) {
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);

	int len, i;
	glRasterPos2f((GLfloat)x, (GLfloat)y);
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
		glutBitmapCharacter(font, str[i]);
}

void vprint2(float x, float y, float size, const char *string, ...) {
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);
	glPushMatrix();
	glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)0);
	glScalef(size, size, 1);

	int len, i;
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
	}
	glPopMatrix();
}

void drawRectangle(int x1, int x2, int y1, int y2, float t) {
	glColor4f(0.4f, 0.4f, 0.4f, t);
	glRectf((GLfloat)x1, (GLfloat)y1, (GLfloat)x2, (GLfloat)y2);
}


void circle(float x, float y, float r) {
	float angle;
	glBegin(GL_POLYGON);
	for (int i = 0; i < 100; i++) {
		angle = 2 * (float)PI*i / 100;
		glVertex2f(x + r * (float)cos(angle), y + r * (float)sin(angle));
	}
	glEnd();
}

void drawLine(float x1, float y1, float x2, float y2) {
	glBegin(GL_LINES);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}


void showCursorPosition() {
	// Show mouse coordinate at top right region
	glColor3f(1, 1, 1);
	vprint(345, -385, GLUT_BITMAP_8_BY_13, "(%.0f, %.0f)", mouse.x, mouse.y);
}

void topInsignia(void) {

	// DRAW a POLYGON
	glColor3f((GLfloat)0.4, (GLfloat)0.4, (GLfloat)1);
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

	glColor3f((GLfloat)0.9, (GLfloat)0.9, (GLfloat)0.9);
	glBegin(GL_POLYGON);
	glVertex2f(-90, 450);
	glVertex2f(-37, 302);
	glVertex2f(0, 292);
	glVertex2f(37, 302);
	glVertex2f(90, 450);
	glEnd();


	glLineWidth(3);
	// DRAW A LINE STRIP (CONNECTED LINES)
	glColor3f((GLfloat)1.0, (GLfloat)0.9, (GLfloat)0.8);
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

void wallPattern(void)
{
	int s = 10;
	int px = -595, py = 435;
	for (int r = 0; r < 44; r++)
	{
		for (int c = 0; c < 6; c++)
		{
			glColor3ub(255, 229, 204);
			glRectf((GLfloat)px, (GLfloat)py, (GLfloat)(px + s), (GLfloat)(py - s));
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
			glRectf((GLfloat)px, (GLfloat)py, (GLfloat)(px + s), (GLfloat)(py - s));
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
			glRectf((GLfloat)px, (GLfloat)py, (GLfloat)(px + s), (GLfloat)(py - s));
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
			glRectf((GLfloat)px, (GLfloat)py, (GLfloat)(px + s), (GLfloat)(py - s));
			px -= 20;
		}
		py -= 20;
		px = 465;
	}

}

void windows(void) {
	glColor3ub(40, 40, 60); //window border outermost blue
	glRectf(-470, 270, 470, -270);

	glColor3ub(255, 229, 204); //window border innermost cream
	glRectf(-460, 260, 460, -260);

	glBegin(GL_QUADS); //cockpit window
	glColor3f((GLfloat)0.2, (GLfloat) 0.3, (GLfloat) 0.5); // blueish
	glVertex2f(-450, 250);    // Top-left corner
	glColor3f((GLfloat)0.2, (GLfloat)0.4, (GLfloat)0.4); //light aurora colour
	glVertex2f(450, 250);     // Top-right corner
	glColor3f((GLfloat)0.2, (GLfloat) 0.2, (GLfloat)0.3); // dark blue
	glVertex2f(450, -250);    // Bottom-right corner
	glVertex2f(-450, -250);   // Bottom-left corner
	glEnd();

}

void menu_background(void) {

	wallPattern();
	windows();
	showCursorPosition();
}

void menu_text(void) {

	glColor3f(1, 1, 0);
	glLineWidth(3);
	vprint2((float)-127, (float)100, (float)0.3, "ANGRY BIRDS");
	drawLine(-130, 80, 135, 80);

	//Hints inside the window
	glColor3ub(0, 255, 0);
	vprint(285, -185, GLUT_BITMAP_8_BY_13, "<F1>     : start");
	vprint(285, -205, GLUT_BITMAP_8_BY_13, "<C>      : credit");
	vprint(285, -225, GLUT_BITMAP_8_BY_13, "<ESC>    : exit");

	glColor3f(1, 1, 1); // Buttons
	vprint2(-50, 10, 0.25, "START");
	vprint2(-65, -50, 0.25, "CREDITS");
	vprint2(-35, -110, 0.25, "EXIT");

	// bottom rectangle gradient
	glBegin(GL_QUADS);
	glColor3ub(255, 229, 204); // cream
	glVertex2f(-165, -275);
	glColor3f((GLfloat)0.4, (GLfloat)0.4, (GLfloat)1);  //bluish same as insignia
	glVertex2f(-165, -360);
	glColor3f((GLfloat)0.4, (GLfloat)0.4, (GLfloat)1);  //bluish same as insignia
	glVertex2f(165, -360);
	glColor3ub(255, 229, 204); // cream
	glVertex2f(165, -275);
	glEnd();

	// bottom display
	glColor3f(1, 1, 1);
	vprint(-115, -200, GLUT_BITMAP_8_BY_13, "Mohammed Ehsan Ullah Shareef");
	vprint(-45, -220, GLUT_BITMAP_8_BY_13, "-Homework 2-");
}

void display_menu() {
	glClear(GL_COLOR_BUFFER_BIT);
	menu_background();
	menu_text();

}

void play_gradient(void) {

	glBegin(GL_QUADS);
	// Top-left (yellow)
	glColor3ub(255, 200, 40);
	glVertex2f(-600, 400);

	// Bottom-left (yellow)
	glColor3ub(255, 200, 40);
	glVertex2f(-600, -400);

	// Bottom-right (orange)
	glColor3ub(255, 102, 10);
	glVertex2f(600, -400);

	// Top-right (orange)
	glColor3ub(255, 102, 10);
	glVertex2f(600, 400);
	glEnd();


}

void drawRedAngryBird(float x, float y, float scale = 1.0) {
	birdRadius = birdRadius * scale;

	// Head Feathers (2 red triangles) 
	glColor3ub(255, 0, 0);
	glBegin(GL_TRIANGLES);
	glVertex2f(x - (-6) * scale, y + 49 * scale);
	glVertex2f(x - 11 * scale, y + 47 * scale);
	glVertex2f(x - 9 * scale, y + 62 * scale);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex2f(x - 15 * scale, y + 47 * scale);
	glVertex2f(x - 30 * scale, y + 39 * scale);
	glVertex2f(x - 27 * scale, y + 55 * scale);
	glEnd();

	// Tail (3 black rectangles)
	glColor3ub(0, 0, 0);
	// top
	glBegin(GL_TRIANGLES);
	glVertex2f(x + 17 * scale, y - 34 * scale);
	glVertex2f(x + 53 * scale, y - 35 * scale);
	glVertex2f(x + 46 * scale, y - 41 * scale);
	glEnd();

	//middle
	glBegin(GL_TRIANGLES);
	glVertex2f(x + 20 * scale, y - 26 * scale);
	glVertex2f(x + 37 * scale, y - 53 * scale);
	glVertex2f(x + 43 * scale, y - 49 * scale);
	glEnd();

	//bottom
	glBegin(GL_TRIANGLES);
	glVertex2f(x + 29 * scale, y - 27 * scale);
	glVertex2f(x + 28 * scale, y - 57 * scale);
	glVertex2f(x + 23 * scale, y - 57 * scale);
	glEnd();


	// Body 
	glColor3ub(255, 0, 0); // red
	circle(x, y, birdRadius);

	// Eyes
	float eyeOffsetX = 15 * scale;
	float eyeOffsetY = 10 * scale;
	float eyeRadius = 12 * scale;

	glColor3ub(255, 255, 255); // white
	circle(x - eyeOffsetX, y + eyeOffsetY, eyeRadius);
	circle(x + eyeOffsetX, y + eyeOffsetY, eyeRadius);

	// Pupils
	float pupilRadius = (float) 5.0 * scale;
	glColor3ub(0, 0, 0); // black
	circle(x - eyeOffsetX, y + eyeOffsetY, pupilRadius);
	circle(x + eyeOffsetX, y + eyeOffsetY, pupilRadius);

	// Belly (grey polygon)
	glColor3ub(200, 200, 200);
	glBegin(GL_POLYGON);
	glVertex2f(x - 11 * scale, y - 20 * scale); // left top
	glVertex2f(x + 11 * scale, y - 20 * scale); // right top
	glVertex2f(x + 31 * scale, y - 40 * scale); // right bottom
	glVertex2f(x, y - 55 * scale);              // tip bottom
	glVertex2f(x - 31 * scale, y - 40 * scale); // left bottom
	glEnd();

	// Beak 
	glColor3ub(255, 204, 0); // yellow
	glBegin(GL_TRIANGLES);
	glVertex2f(x, y);
	glVertex2f(x - 10 * scale, y - 15 * scale);
	glVertex2f(x + 10 * scale, y - 15 * scale);
	glEnd();

	// Eyebrows
	glColor3ub(0, 0, 0); // black
	glLineWidth(5);
	drawLine(x - 25 * scale, y + 25 * scale, x - 5 * scale, y + 20 * scale);
	drawLine(x + 5 * scale, y + 20 * scale, x + 25 * scale, y + 25 * scale);
}

void drawYellowAngryBird(float x, float y, float scale = 1.0) {
	birdRadius = birdRadius * scale;

	// Head Feathers (2 red triangles) 
	glColor3ub(255, 221, 0); // yellow
	glBegin(GL_TRIANGLES);
	glVertex2f(x + 6 * scale, y + 49 * scale);
	glVertex2f(x - 11 * scale, y + 47 * scale);
	glVertex2f(x - 9 * scale, y + 62 * scale);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex2f(x - 15 * scale, y + 47 * scale);
	glVertex2f(x - 30 * scale, y + 39 * scale);
	glVertex2f(x - 27 * scale, y + 55 * scale);
	glEnd();

	// Tail (3 black rectangles)
	glColor3ub(0, 0, 0);
	// top
	glBegin(GL_TRIANGLES);
	glVertex2f(x + 17 * scale, y - 34 * scale);
	glVertex2f(x + 53 * scale, y - 35 * scale);
	glVertex2f(x + 46 * scale, y - 41 * scale);
	glEnd();

	//middle
	glBegin(GL_TRIANGLES);
	glVertex2f(x + 20 * scale, y - 26 * scale);
	glVertex2f(x + 37 * scale, y - 53 * scale);
	glVertex2f(x + 43 * scale, y - 49 * scale);
	glEnd();

	//bottom
	glBegin(GL_TRIANGLES);
	glVertex2f(x + 29 * scale, y - 27 * scale);
	glVertex2f(x + 28 * scale, y - 57 * scale);
	glVertex2f(x + 23 * scale, y - 57 * scale);
	glEnd();


	// Body 
	glColor3ub(255, 221, 0); // yellow
	circle(x, y, birdRadius);

	// Eyes
	float eyeOffsetX = 15 * scale;
	float eyeOffsetY = 10 * scale;
	float eyeRadius = 12 * scale;

	glColor3ub(255, 255, 255); // white
	circle(x - eyeOffsetX, y + eyeOffsetY, eyeRadius);
	circle(x + eyeOffsetX, y + eyeOffsetY, eyeRadius);

	// Pupils
	float pupilRadius = (float)5.0 * scale;
	glColor3ub(0, 0, 0); // black
	circle(x - eyeOffsetX, y + eyeOffsetY, pupilRadius);
	circle(x + eyeOffsetX, y + eyeOffsetY, pupilRadius);

	// Belly (cream polygon)
	glColor3ub(255, 255, 204);
	glBegin(GL_POLYGON);
	glVertex2f(x - 11 * scale, y - 20 * scale); // left top
	glVertex2f(x + 11 * scale, y - 20 * scale); // right top
	glVertex2f(x + 31 * scale, y - 40 * scale); // right bottom
	glVertex2f(x, y - 55 * scale);              // tip bottom
	glVertex2f(x - 31 * scale, y - 40 * scale); // left bottom
	glEnd();

	// Beak 
	glColor3ub(255, 153, 0); // orange - yellow
	glBegin(GL_TRIANGLES);
	glVertex2f(x, y);
	glVertex2f(x - 10 * scale, y - 15 * scale);
	glVertex2f(x + 10 * scale, y - 15 * scale);
	glEnd();

	// Eyebrows
	glColor3ub(0, 0, 0); // black
	glLineWidth(5);
	drawLine(x - 25 * scale, y + 25 * scale, x - 5 * scale, y + 20 * scale);
	drawLine(x + 5 * scale, y + 20 * scale, x + 25 * scale, y + 25 * scale);
}

void drawSlingBall(float x, float y, float scale = 1.0) {
	// Main Ball 
	glColor3ub(60, 60, 60); // Dark grey
	circle(x, y, ballRadius * scale);

	// 3 black small circles
	glColor3ub(255, 255, 255); // White
	float holeRadius = (float)4.5 * scale;

	circle((float)x - 7.5f * scale, (float)y + 7.5f * scale, (float)holeRadius); // top-left
	circle((float)x + 7.5f * scale, (float)y + 7.5f * scale, (float)holeRadius); // top-right
	circle((float)x, (float)y - 7.5f * scale, (float)holeRadius);               // bottom-center

}

void drawCloud(float x, float y, float scale = 1.0) {
	glColor3f(1, 1, 1); // White

	circle(x + 0 * scale, y + 0 * scale, 30 * scale);
	circle(x - 25 * scale, y + 0 * scale, 20 * scale);
	circle(x + 25 * scale, y - 2 * scale, 20 * scale);
	circle(x + 21 * scale, y - 19 * scale, 10 * scale);
}

void drawSlingShot(float x, float y, float scale = 1.0) {

	drawCloud(x + 0.0f * scale, y - 100.0f * scale, 1.5f * scale);

	glColor3ub(100, 100, 100); // Dark gray
	circle(x - 0 * scale, y - 75 * scale, 25 * scale);

	glColor3ub(200, 200, 200); // light gray

	// base
	glRectf(x - 15 * scale, y + 75 * scale, x + 15 * scale, y - 75 * scale);


	glColor3ub(150, 150, 150); // Dark gray
	// horizontal
	glRectf(x - 15 * scale, y + 70 * scale, x + 75 * scale, y + 90 * scale);
	//vertical
	glRectf(x - 15 * scale, y + 70 * scale, x + 5 * scale, y + 160 * scale);

	glColor3ub(150, 150, 150); // Dark gray
	glLineWidth((GLfloat)5.0*scale);      // strip width

	if (animation)
	{
		glBegin(GL_LINE_STRIP);
		glVertex2f(x - 5, y *scale + 155 * scale);
		glVertex2f(x - 95 * scale * slingScale, y + 165 * scale);
		glVertex2f(x + 70 * scale, y + 80 * scale);
		glEnd();
	}
	else
	{
		glBegin(GL_LINE_STRIP);
		glVertex2f(x - 5, y *scale + 155 * scale);
		glVertex2f(x - 100 * scale, y + 30 * scale);
		glVertex2f(x + 70 * scale, y + 80 * scale);
		glEnd();
	}

	glColor3ub(200, 200, 200); // light gray
	circle(x - 5 * scale, y + 160 * scale, 10 * scale);
	circle(x + 75 * scale, y + 80 * scale, 10 * scale);

}

void addResponse(float x, float y, bool bonus) {
	for (int i = 0; i < MAX_RESPONSE; ++i) {
		if (!response[i].active) {
			response[i].active = true;
			response[i].x = x;
			response[i].y = y;
			response[i].t = 0.0f;
			response[i].bonus = bonus;
			break;
		}
	}
}

void bonusGrant() {
	scoreCnt = scoreCnt + BONUS_SCORE;
	glutPostRedisplay();
}

void targetHitCheck() {

	for (int i = 0; i < MAX_SPAWN; i++) {
		if (!bird[i].active) continue;


		float birdX = bird[i].x;
		float birdY = bird[i].y;


		float dx = birdX - cannonBallX;
		float dy = birdY - cannonBallY;
		float distanceSquared = dx * dx + dy * dy;


		float radiusSum = ballRadius + birdRadius;
		if (distanceSquared <= radiusSum * radiusSum) {
			// Bird hit!
			targetHits++;
			bird[i].active = false;
			showMissInfo = false;
			targetHit = true;

			if (bird[i].bonus) {
				bonusHits++;
				bonusGrant();
			}
			else {
				giveHitBonus();
			}

			addResponse(birdX, birdY, bird[i].bonus);
			glutPostRedisplay();
		}
	}
}



void spawnBird() {
	int activeCount = 0;
	for (int i = 0; i < MAX_SPAWN; i++)
		if (bird[i].active) ++activeCount;
	if (activeCount >= MAX_SPAWN) return;

	if (rand() % 100 > 3) return;

	for (int i = 0; i < MAX_SPAWN; ++i) {
		if (!bird[i].active) {
			float xMin = 0;
			float xMax = (WINDOW_WIDTH / 2.0f) - birdRadius;

			// try to find a position
			float targetX;
			bool gotSpot = false;
			for (int attempt = 0; attempt < 25; ++attempt) { // 25 attempts to find position
				targetX = xMin + (rand() / (float)RAND_MAX) * (xMax - xMin);

				// check distance to all other active birds
				bool collides = false;
				for (int j = 0; j < MAX_SPAWN; ++j) {
					if (!bird[j].active) continue;
					float dx = targetX - bird[j].x;
					// since all start at same y, only x matters here
					if (fabs(dx) < 1.75 * birdRadius) {
						collides = true;
						break;
					}
				}
				if (!collides) {
					gotSpot = true;
					break;
				}
			}

			// if we failed to find a non-colliding spot, skip spawn
			if (!gotSpot) return;

			// activate and initialize
			bird[i].active = true;
			bird[i].x = targetX;
			// using constant window size here because when resized the angry birds will still come out from the drawn region only
			bird[i].y = -(WINDOW_HEIGHT / 2.0f) + birdRadius; // Start from the screen bottom

			// decide if bonus bird (yellow)
			if ((rand() % 100) < SPAWN_CHANCE) { // aprox 15% chance for bonus bird
				bird[i].bonus = true;
				bird[i].speed = FAST_SPEED;
			}
			else {
				bird[i].bonus = false;
				bird[i].speed = TARGET_SPEED;
			}
			break;
		}
	}
}

void updateBirds() {
	float off = birdRadius + 10.0f;  // bird radius + small buffer
	for (int i = 0; i < MAX_SPAWN; i++) {
		if (!bird[i].active) continue;
		// move up animation
		bird[i].y += bird[i].speed;
		// if passed top edge, deactivate


		if (bird[i].y > (WINDOW_HEIGHT / 2.0f) - off)
		{
			bird[i].active = false;
			escapeCnt++;
		}
	}
	spawnBird();
}



void angryBirdSelection(float x, float y, bool bonus) {
	if (bonus)
		drawYellowAngryBird(x, y, 1);
	else
		drawRedAngryBird(x, y, 1);
}

void AngryBirdsRelease() {
	for (int i = 0; i < MAX_SPAWN; i++)
		if (bird[i].active)
			angryBirdSelection(bird[i].x, bird[i].y, bird[i].bonus);
}

void penalizeMiss() {
	scoreCnt = scoreCnt - PENALIZE_SCORE;
	glutPostRedisplay();
}

void giveHitBonus() {
	scoreCnt = scoreCnt + HIT_SCORE_BONUS;
	glutPostRedisplay();
}

void drawMissInfoPanel() {
	drawRectangle(-400, 400, -350, -300, 0.8f);

	glColor3f(1.0f, 0.7f, 0.0f);
	vprint(-170, -330, GLUT_BITMAP_9_BY_15, "Penalty %d points if Canon Ball wasted!", PENALIZE_SCORE);

}

void gamingInfoText() {
	int topY = 350;

	glColor3f(1.0f, 1.0f, 1.0f);
	vprint(-480, topY, GLUT_BITMAP_9_BY_15, "Score: %d", scoreCnt);
	vprint(-300, topY, GLUT_BITMAP_9_BY_15, "Killed Red Angry Bird: %d", targetHits);
	vprint(50, topY, GLUT_BITMAP_9_BY_15, "Killed Yellow Angry Bird: %d", bonusHits);
	vprint(-480, topY - 20, GLUT_BITMAP_9_BY_15, "Sling Speed: %d", Vx / 100);

	if (remainingTime < 0)
		remainingTime = 0;

	vprint(350, topY, GLUT_BITMAP_9_BY_15, "Time Left: %d sec", remainingTime);

}

void print(int x, int y, const char* string, void* font) {
	int len, i;

	glRasterPos2f((GLfloat)x, (GLfloat)y);
	len = (int)strlen(string);
	for (i = 0; i < len; i++)
		glutBitmapCharacter(font, string[i]);
}

void instructionsInfoText() {
	glColor3f(1.0f, 1.0f, 1.0f);
	if (!gameover)
		print(-350, -370, "SpaceBar: Fire                  UP/DOWN: Sling Location               F1: Pause/Resume", GLUT_BITMAP_8_BY_13);
	print(-350, -390, "                                LEFT/RIGHT: Sling Speed               <esc>: end page", GLUT_BITMAP_8_BY_13);

}

void play_top_bottom() {

	glColor3ub(40, 40, 40); //charcoal grey
	glBegin(GL_QUADS);
	glVertex2f(-600, 400);
	glVertex2f(-600, 300);
	glVertex2f(600, 300);
	glVertex2f(600, 400);
	glEnd();

	glBegin(GL_QUADS);
	glVertex2f(-600, -300);
	glVertex2f(-600, -400);
	glVertex2f(600, -400);
	glVertex2f(600, -300);
	glEnd();

	gamingInfoText();
	instructionsInfoText();
}



void drawRectangle() {
	drawRectangle(-150, 150, 100, -75, 0.5);
}

void drawPausedContainer() {
	drawRectangle();

	glColor4f(1, 1, 1, 0.5);
	vprint(-80, 25, GLUT_BITMAP_TIMES_ROMAN_24, "GAME PAUSED!");
	vprint(-82, 0, GLUT_BITMAP_9_BY_15, "Press F1 to continue");
}

void constructEquation() {
	c = (float)(-HG / ((Vx * Vx) / 36.0));
	h = cannonBallX;
	k = cannonBallY;
}

float fx(float x) {
	return c * (x - h) * (x - h) + k;
}


void drawResponse() {
	const int RESPONSE_LIMIT = 30;
	const float MAX_RISE = 50.0f;
	const int TEXT_OFFSET_X = 60;
	const int TEXT_OFFSET_Y = 18;
	const int WINDOW_MARGIN = 550;

	for (int i = 0; i < RESPONSE_LIMIT; ++i) {
		if (!response[i].active) {
			continue;
		}

		float progress = response[i].t;
		float currentRise = MAX_RISE * progress;
		float alphaValue = 1.0f - progress;


		if (response[i].bonus) {
			glColor4f(1.0f, 0.84f, 0.0f, alphaValue); // Gold color for bonus
		}
		else {
			glColor4f(0.0f, 1.0f, 0.0f, alphaValue);  // Green color for regular
		}

		// Determine score bonus based on type
		int displayedScore = response[i].bonus ? BONUS_SCORE : HIT_SCORE_BONUS;


		if (response[i].x + WINDOW_MARGIN >= WINDOW_WIDTH) {
			response[i].x = WINDOW_WIDTH - WINDOW_MARGIN;
		}

		// Draw the score text
		vprint((int)response[i].x + TEXT_OFFSET_X,
			(int)(response[i].y + 2 * currentRise + TEXT_OFFSET_Y),
			GLUT_BITMAP_HELVETICA_18,
			"+%d pts",
			displayedScore);
	}
}

void initializeResponse() {
	for (int i = 0; i < 30; ++i)
		response[i].active = false;
}

void drawPath(float startX, float startY, float Vx, float Vy) {
	const float g = 9.8f; // Gravity
	float x, y;

	float c = -g / (2.0f * Vx * Vx);
	float b = Vy / Vx;
	float a = startY;

	glColor3f(0.2f, 0.8f, 1.0f); // Sky blue path
	glLineWidth(2);
	glBegin(GL_LINE_STRIP);

	for (x = startX; x < 400; x += 3) {
		float relX = x - startX;
		y = c * relX * relX + b * relX + a;
		if (y < -200) break;
		glVertex2f(x, y);
	}
	glEnd();

}


void display_play() {

	glClear(GL_COLOR_BUFFER_BIT);
	play_gradient();
	drawSlingShot(Sx, Sy, 1);
	drawSlingBall(cannonBallX, cannonBallY, 1);
	AngryBirdsRelease();
	play_top_bottom();
	drawResponse();

	drawMissInfoPanel();

	if (isPaused && !gameover)
		drawPausedContainer();

	if (gameover) {
		gameState = GAMEOVER;
	}

	if (!animation) drawPath(cannonBallX, cannonBallY, Vx / 10, 0.0f); //vy to account vertical velocity 

	showCursorPosition();
}

void credits_texts(void) {

	glColor3f(1, 1, 0);
	vprint(-50, 100, GLUT_BITMAP_TIMES_ROMAN_24, "CREDITS");
	drawLine(-120, 85, 130, 85);

	glColor3f(1, 1, 1);
	vprint(-71, 50, GLUT_BITMAP_HELVETICA_18, "Bilkent University");
	vprint(-40, 20, GLUT_BITMAP_HELVETICA_18, "CTIS 164");
	vprint(-160, -10, GLUT_BITMAP_HELVETICA_18, "Technical Mathematics with Programming");
	vprint(-135, -70, GLUT_BITMAP_HELVETICA_18, "- Mohammed Ehsan Ullah Shareef - ");

	// bottom rectangle gradient
	glBegin(GL_QUADS);
	glColor3ub(255, 229, 204); // cream
	glVertex2f(-165, -275);
	glColor3f((GLfloat) 0.4, (GLfloat) 0.4, (GLfloat)1);  //bluish same as insignia
	glVertex2f(-165, -360);
	glColor3f((GLfloat)0.4, (GLfloat)0.4, (GLfloat)1);  //bluish same as insignia
	glVertex2f(165, -360);
	glColor3ub(255, 229, 204); // cream
	glVertex2f(165, -275);
	glEnd();

	glColor3ub(0, 255, 0);
	vprint(285, -225, GLUT_BITMAP_8_BY_13, "<ESC>    : menu");

}

void display_credits() {
	glClear(GL_COLOR_BUFFER_BIT);
	topInsignia();
	wallPattern();
	windows();
	credits_texts();
	showCursorPosition();
}

void ballRelod() {
	animation = false;
	cannonBallX = Sx - BALL_DEPTH;
	cannonBallY = Sy + BALL_RISE;

	if (!targetHit)
		penalizeMiss();
	else
		targetHit = false;
}

void resetPositions() {

	// resent sling and ball position
	Sx = SLING_INITIALX;
	Sy = SLING_INITIALY;
	cannonBallX = Sx - BALL_DEPTH;
	cannonBallY = Sy + BALL_RISE;

}

void resetTimer() {
	isPaused = false;
	gameover = false;
	remainingTime = DURATION;
}

void initializeBirds() {
	srand((unsigned)time(NULL));
	for (int i = 0; i < MAX_SPAWN; i++) {
		bird[i].active = false;
		bird[i].bonus = false;
	}
}

void resetScore() {
	scoreCnt = 0;
	targetHits = 0;
	bonusHits = 0;
	missComboCount = 0;
	targetHit = false;
	showMissInfo = false;
}

void restartGame() {

	ballRelod();
	resetPositions();
	resetTimer();
	initializeResponse();
	initializeBirds();
	resetScore();
}

void display_gameover() {
	glClear(GL_COLOR_BUFFER_BIT);
	topInsignia();
	wallPattern();
	windows();
	showCursorPosition();

	drawRectangle();
	glColor3f(1.0f, 0.85f, 0.4f);

	vprint(-70, 20, GLUT_BITMAP_9_BY_15, "Score: %d points", scoreCnt);
	vprint(-75, -5, GLUT_BITMAP_9_BY_15, "Birds escaped: %d", escapeCnt);
	vprint(-70, -30, GLUT_BITMAP_9_BY_15, "Total kills: %d", targetHits + bonusHits);


	//Hints inside the window

	if (gameover)
	{
		glColor3f(1.0f, 0.85f, 0.4f);
		vprint(-80, 50, GLUT_BITMAP_TIMES_ROMAN_24, "GAME OVER!!");
		glColor3ub(0, 255, 0);
		vprint(285, -185, GLUT_BITMAP_8_BY_13, "<F1>     : restart");
		vprint(285, -205, GLUT_BITMAP_8_BY_13, "<ESC>    : exit");
	}
	else
	{
		glColor3ub(0, 255, 0);
		vprint(285, -185, GLUT_BITMAP_8_BY_13, "<F1>     : continue");
		vprint(285, -205, GLUT_BITMAP_8_BY_13, "<ESC>    : exit");
	}

}


void display() {
	// Clear window to black
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	switch (gameState) {
	case MENU: display_menu(); break;
	case PLAY: display_play(); break;
	case CREDITS: display_credits(); break;
	case GAMEOVER: display_gameover(); break;
	}
	glutSwapBuffers();

}

void onKeyDown(unsigned char key, int x, int y) {


	if (gameState == MENU) {
		if (key == 27)
			exit(0);

		if (key == 'c' || key == 'C')
			gameState = CREDITS;
	}

	else if (gameState == PLAY) {

		if (key == 27)
		{
			gameState = GAMEOVER;
			if (!isPaused)
				isPaused = !isPaused;
		}

		if (key == ' ' && !isPaused)
			animation = true;


	}
	else if (gameState == CREDITS) {
		if (key == 27)
			gameState = MENU;
	}
	else
		if (key == 27)
			exit(0);

	glutPostRedisplay();
}


void onSpecialKeyDown(int key, int x, int y) {

	if (gameState == PLAY && key == GLUT_KEY_F1 && !gameover)
	{
		isPaused = !isPaused;
	}


	if (gameState == PLAY && !isPaused && !gameover && !animation) {
		switch (key) {
		case GLUT_KEY_UP:
			Sy += SLING_SPEED;
			if (Sy > 130)
				Sy = 130;
			cannonBallY = Sy + BALL_RISE;
			break;
		case GLUT_KEY_DOWN:
			Sy -= SLING_SPEED;
			if (Sy < -150)
				Sy = -150;
			cannonBallY = Sy + BALL_RISE;
			break;
		case GLUT_KEY_LEFT:
			Vx -= SPEED_TOPUP;
			if (Vx <= 300)
				Vx = 300;
			break;
		case GLUT_KEY_RIGHT:
			Vx += SPEED_TOPUP;
			if (Vx >= 2000)
				Vx = 2000;
			break;
		}

		constructEquation();
	}


}

void onSpecialKeyUp(int key, int x, int y) {
	if (gameState == MENU && key == GLUT_KEY_F1) {
		restartGame();
		gameState = PLAY;
	}

	if (gameState == GAMEOVER && key == GLUT_KEY_F1)
		if (gameover)
		{
			restartGame();
			gameState = PLAY;
		}
		else
		{
			gameState = PLAY;
			if (isPaused)
				isPaused = !isPaused;
		}

}


void updateResponse(float dt) {
	for (int i = 0; i < 30; ++i) {
		if (!response[i].active) continue;
		response[i].t += dt;
		if (response[i].t >= 1.0f)
			response[i].active = false;
	}
}


void onTimer(int v) {
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
	float dt = TIMER_PERIOD / 1000.0f;

	static int timeAccumulator = 0;
	if (!gameover && !isPaused) {
		timeAccumulator += TIMER_PERIOD;

		if (timeAccumulator >= 1000) {
			remainingTime--;
			timeAccumulator = 0;
		}

		if (remainingTime <= 0)
			gameover = true;

		if (showMissInfo && !targetHit) {
			missInfoTimeAccumulator += TIMER_PERIOD;
			if (missInfoTimeAccumulator >= INFO_MISS_CONTAINER_TIME) {
				showMissInfo = false;
				missInfoTimeAccumulator = 0;
				missComboCount = 0;
			}
		}
		else
			missInfoTimeAccumulator = 0;

		if (animation) {
			// move sling ball:
			cannonBallX += Vx * TIMER_PERIOD / 1000.0f;
			cannonBallY = fx(cannonBallX);

			slingScale = TIMER_PERIOD / 1000.0f;
			targetHitCheck();

			// reset cannon ball if it goes off screen
			if (cannonBallY < -WINDOW_HEIGHT / 2 || cannonBallX > WINDOW_WIDTH / 2)
				ballRelod();
		}

		// move birds & spawn new ones
		updateBirds();
	}

	if (!gameover && !isPaused) {
		updateResponse(dt);
	}
	glutPostRedisplay();
}

void onMove(int x, int y) {
	mouse.x = x - winWidth / 2.0f;
	mouse.y = winHeight / 2.0f - y;

	// To refresh the window it calls display() function
	glutPostRedisplay();
}

void onResize(int w, int h) {
	winWidth = w;
	winHeight = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	display();
}

void init() {
	// Smoothing shapes
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	display_menu();
	display_play();
	display_credits();
	display_gameover();
	constructEquation();
	initializeBirds();
	initializeResponse();
}

int main(int argc, char* argv[]) {
	//Window Created
	glutInit(&argc, argv); // Initialize GLUT
	 // Set the window's initial width & height
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	// Get the width of the screen
	int screenWidth = glutGet(GLUT_SCREEN_WIDTH);
	// Get the height of the screen
	int screenHeight = glutGet(GLUT_SCREEN_HEIGHT);
	// Position the window initially in center of every screen and moved up a bit for better experience
	glutInitWindowPosition((screenWidth - WINDOW_WIDTH) / 2, (screenHeight - WINDOW_HEIGHT) / 2 - 60);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("Angry Birds - Hit the target : Ehsan");

	glutDisplayFunc(display);
	glutReshapeFunc(onResize);

	// Mouse Events
	glutPassiveMotionFunc(onMove);

	// Keyboard Events
	glutSpecialUpFunc(onSpecialKeyUp);

	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecialKeyDown);

	// Timer Event
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);

	init();
	glutMainLoop();
	return 0;
}
