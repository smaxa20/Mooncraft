#include <iostream>
#include <cmath>
#include <iomanip>
#include <cassert>
#include <vector>
#include <algorithm>
using namespace std;

#include "Angel.h";
#include <GL/glew.h> // for OpenGL extensions
#include <GL/glut.h> // for Glut utility kit
#include "texture.h" // for the bitmap texture loader

// Global Projection Matrices
mat4 projection, modelview, translate;  

#include "GraphicsObject.h"
#include "SkyBox.h"
#include "Block.h"

// Movement other than jumping is heavily influenced by Cameron Rutherford - https://github.com/CameronRutherford/GraphicsGame
GLfloat mouseSensitivityX = 0.05;
GLfloat mouseSensitivityY = 0.0015;
bool aDown = false;
bool sDown = false;
bool dDown = false;
bool wDown = false;
bool jumping = false;
bool spaceDown = false;
bool lockedMouse = true;
bool stuck = false;
int accumDy = 0;

vec4 moveBackOrForward;
vec4 moveLeftOrRight;

// Gravity Physics
float g = 0.002;
float jumpSpeed = 0.05;
float moonJump = 0.1;
float vy = 0.0;

GLfloat  zoom = 0.0;         // Translation factor
vec4 view(0.0, 0.0, -2.0, 0.0);

// window size
int width = 1000;
int height = 1000;

GLfloat  fovy = 60.0;		 // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect = 1.0;       // Viewport aspect ratio
GLfloat  zNear = 0.01, zFar = 1000.0;

GLfloat dir = 1.0;
GLfloat theta[] = {0.0,0.0,0.0};
GLint axis = 1;

// The global SkyBox Object
SkyBox skybox;

// The one brick that I will redraw over and over
mat4 scale = Scale(0.5, 0.5, 0.5);
Block brick("Blocks\\brick.bmp", scale);
Block stone("Blocks\\stone.bmp", scale);
Block wood("Blocks\\wood.bmp", scale);
Block sand("Blocks\\sand.bmp", scale);
Block emerald("Blocks\\emerald.bmp", scale);
Block diamond("Blocks\\diamond.bmp", scale);

// blocks to help the user
Block transparent("Blocks\\placeholder.bmp", scale);

// used to switch between types of brick
enum BlockType { brickType, woodType, sandType, emeraldType, diamondType, stoneType };
BlockType currentType = brickType;

// The ground brick
Block ground("Blocks\\ground.bmp", Scale(200.0, 200.0, 200.0));

// The vector used to store every brick's location
vector<vec3> blockLocations;
vector<vec3> stoneLocations;
vector<vec3> brickLocations;
vector<vec3> woodLocations;
vector<vec3> sandLocations;
vector<vec3> emeraldLocations;
vector<vec3> diamondLocations;

// The randomly chosen maze
vector<string> chosenMaze;

point4  eye(-9.0, 0.1, -9.0, 1.0);
point4  at(-7.0, 0.0, -9.0, 1.0);
vec4    up(0.0, 1.0, 0.0, 0.0);

// Rounds a number to the nearest 0.5
double myRound(double num)
{
	double max = ceil(num);
	double mid = ceil(num) - 0.5;
	double min = floor(num);

	if (ceil(num) - num <= 0.5)
	{
		if (abs(max - num) < abs(mid - num))
			return max;
		else
			return mid;
	}
	else
	{
		if (abs(mid - num) < abs(min - num))
			return mid;
		else
			return min;
	}
}

bool myFind(vector<vec3> vec, vec3 location)
{
	for (vector<vec3>::iterator it = vec.begin(); it != vec.end(); ++it)
	{
		vec3 val = *it;
		if (location.x == val.x && location.y == val.y && location.z == val.z)
		{
			return true;
		}
	}
	return false;
}

void placeBlock()
{
	// if the proposed block wouldn't be hanging suspended in the air or underground then place it at 'location'
	vec3 location = vec3(myRound(at.x), myRound(at.y) - 0.25, myRound(at.z));
	if ((location.y == -0.25 ||																// the proposed block would be on the ground
		myFind(blockLocations, vec3(location.x + 0.5, location.y, location.z)) ||			// there's a block to the +x of the proposed block
		myFind(blockLocations, vec3(location.x - 0.5, location.y, location.z)) ||			// there's a block to the -x of the proposed block
		myFind(blockLocations, vec3(location.x, location.y + 0.5, location.z)) ||			// there's a block to the +y of the proposed block
		myFind(blockLocations, vec3(location.x, location.y - 0.5, location.z)) ||			// there's a block to the -y of the proposed block
		myFind(blockLocations, vec3(location.x, location.y, location.z + 0.5)) ||			// there's a block to the +z of the proposed block
		myFind(blockLocations, vec3(location.x, location.y, location.z - 0.5))) &&			// there's a block to the -z of the proposed block
		location.y >= -0.25)																// the proposed block must not be underground
	{
		switch (currentType)
		{
			case brickType:
				brickLocations.push_back(location);
				blockLocations.push_back(location);
				break;
			case stoneType:
				stoneLocations.push_back(location);
				blockLocations.push_back(location);
				break;
			case woodType:
				woodLocations.push_back(location);
				blockLocations.push_back(location);
				break;
			case sandType:
				sandLocations.push_back(location);
				blockLocations.push_back(location);
				break;
			case emeraldType:
				emeraldLocations.push_back(location);
				blockLocations.push_back(location);
				break;
			case diamondType:
				diamondLocations.push_back(location);
				blockLocations.push_back(location);
		}
	}
}

void findAndErase(vector<vec3> &vec, vec3 location)
{
	// vector::find and checking if *it or val == location didn't work because of how equality is defined in vec3 (I think)
	// checking if the individual elements are equal works
	for (vector<vec3>::iterator it = vec.begin(); it != vec.end(); ++it)
	{
		vec3 val = *it;
		if (location.x == val.x && location.y == val.y && location.z == val.z)
		{
			vec.erase(it);
			break;
		}
	}
}

void destroyBlock()
{
	vec3 location = vec3(myRound(at.x), myRound(at.y) - 0.25, myRound(at.z));
	findAndErase(blockLocations, location);
	findAndErase(stoneLocations, location);
	findAndErase(brickLocations, location);
	findAndErase(woodLocations, location);
	findAndErase(sandLocations, location);
	findAndErase(emeraldLocations, location);
	findAndErase(diamondLocations, location);
}

void switchBlockType()
{
	switch (currentType)
	{
		case brickType: 
			currentType = stoneType;
			cout << "Block type: Stone" << endl;
			break;
		case stoneType:
			currentType = woodType;
			cout << "Block type: Wood" << endl;
			break;
		case woodType: 
			currentType = sandType; 
			cout << "Block type: Sand" << endl;
			break;
		case sandType: 
			currentType = emeraldType;
			cout << "Block type: Emerald" << endl;
			break;
		case emeraldType: 
			currentType = diamondType; 
			cout << "Block type: Diamond" << endl;
			break;
		case diamondType:
			currentType = brickType;
			cout << "Block type: Brick" << endl;
	}
}

bool collision(point4 newEye)
{
	// for each brick, check to see if 'eye' will be 0.9 units away from the brick's center
	//    at the position the user wants to move to
	//    if 'eye' is less than 0.9 units away, don't let the user move there
	for (int i = 0; i < blockLocations.size(); i++)
	{
		vec3 position = blockLocations[i];
		double radius = 0.9;
		double distance = sqrt(pow((newEye.x - position.x), 2) + pow((newEye.y - position.y), 2) + pow((newEye.z - position.z), 2));

		if (distance <= radius)
			return true;
	}
	return false;
}

void display( void )
{
	static float angle = 0.0;

	glClearColor(1.0,1.0,1.0,1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );  /*clear the window */

	projection = Perspective(fovy, aspect, zNear, zFar);
	modelview = Translate(0.0, 0.0, 1.0)*LookAt(eye, at, up);

	// draw the skybox, the ground, and all the blocks that have been placed
	skybox.draw(theta);

	ground.draw(theta, vec3(0.0, -100.5, 0.0), true);
	for (int i = 0; i < brickLocations.size(); i++)
		brick.draw(theta, brickLocations[i], true);
	for (int i = 0; i < stoneLocations.size(); i++)
		stone.draw(theta, stoneLocations[i], true);
	for (int i = 0; i < woodLocations.size(); i++)
		wood.draw(theta, woodLocations[i], true);
	for (int i = 0; i < sandLocations.size(); i++)
		sand.draw(theta, sandLocations[i], true);
	for (int i = 0; i < emeraldLocations.size(); i++)
		emerald.draw(theta, emeraldLocations[i], true);
	for (int i = 0; i < diamondLocations.size(); i++)
		diamond.draw(theta, diamondLocations[i], true);


	// shows the player where a block would be placed if they decide to place a block
	transparent.draw(theta, vec3(myRound(at.x), myRound(at.y) - 0.25, myRound(at.z)), false);

	angle += 0.5;
	if( angle > 360.0 ) angle -= 360.0;

	// New coordinates that the player would have moved to
	point4 newEye = eye;
	point4 newAt = at;

	// Figure out the velocity
	if (!(aDown && dDown))
	{
		if (aDown)
		{
			newEye += moveLeftOrRight;
			newAt += moveLeftOrRight;
		}
		else if (dDown)
		{
			newEye -= moveLeftOrRight;
			newAt -= moveLeftOrRight;
		}
	}
	if (!(wDown && sDown))
	{
		if (wDown)
		{
			newEye += moveBackOrForward;
			newAt += moveBackOrForward;
		}
		else if (sDown)
		{
			newEye -= moveBackOrForward;
			newAt -= moveBackOrForward;
		}
	}

	// Check to see if there would be a collision at the new point
	if (!collision(newEye))
	{
		eye = newEye;
		at = newAt;
	}

	// Jumping Physics
	if ( ((eye.y >= 0.1 || vy >= 0) && !collision(point4(eye.x, eye.y + vy, eye.z, eye.w))) || stuck)
	{
		eye.y += vy;
		vy -= g;
	}
	else
	{
		jumping = false;
		vy = 0.0;
	}
	if (eye.y <= 0.1)
	{
		jumping = false;
		eye.y = 0.1;
	}

	if ( collision(point4(eye.x, eye.y + 0.05, eye.z, eye.w)) && collision(point4(eye.x, eye.y + 0.1, eye.z, eye.w)) &&
		collision(eye + moveLeftOrRight) && collision(eye - moveLeftOrRight) &&
		collision(eye + moveBackOrForward) && collision(eye - moveBackOrForward) )
	{
		stuck = true;
	}
	else
	{
		stuck = false;
	}

	// swap the buffers
	glutSwapBuffers();

	glutPostRedisplay();
}

void mouse_move(int x, int y)
{
	if (lockedMouse)
	{
		int centerX = int(width * 1/2);
		int centerY = int(height * 6/10);
		int dx = x - centerX;
		int dy = y - centerY;

		// Matrix that will rotate the various vectors
		mat4 Rotator = RotateY(-dx * mouseSensitivityX);

		// Change the view vector based on the mouse movement
		accumDy += dy;
		view = Rotator * view;
		at = eye + view;
		at.y -= accumDy * mouseSensitivityY;

		// Change the direction that you are moving in the same way
		moveBackOrForward = Rotator * moveBackOrForward;
		moveLeftOrRight = Rotator * moveLeftOrRight;

		// Move the cursor back to the middle of the screen and make it invisible again
		glutWarpPointer(centerX, centerY);
		glutSetCursor(GLUT_CURSOR_CROSSHAIR);
	}
	else
	{
		glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
	}
}

void myReshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
	aspect = GLfloat (w) / h;
}

void press_key(unsigned char key, int x, int y)
{
	if (key == 27) // escape key
		lockedMouse = !lockedMouse;
	if (key == 'w' && jumping == false)
		wDown = true;
	if (key == 'a' && jumping == false)
		aDown = true;
	if (key == 's' && jumping == false)
		sDown = true;
	if (key == 'd' && jumping == false)
		dDown = true;
	if (key == ' ' && jumping == false)
	{
		vy = jumpSpeed;
		jumping = true;
	}
	if (key == 'j' && jumping == false)
	{
		vy = moonJump;
		jumping = true;
	}
	if (key == 'i') // position info
		cout << "You are looking at " << at << endl;
	if (key == 'q')
		exit(0);
	glutPostRedisplay();
}
void release_key(unsigned char key, int x, int y)
{
	if (key == 'w')
		wDown = false;
	if (key == 'a')
		aDown = false;
	if (key == 's')
		sDown = false;
	if (key == 'd')
		dDown = false;
}

void mouse(int btn, int state, int x, int y)
{
	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		placeBlock();
	else if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
		destroyBlock();
	else if (btn == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
		switchBlockType();
	glutPostRedisplay();
}

void init_gl()
{
	glEnable(GL_DEPTH_TEST);
}

void init()
{   
	init_gl();			    // Setup general OpenGL stuff of the object //could do all of this by creating  skybox.init function that does all 5 things

	skybox.init_data();	        // Setup the data for the this object
	skybox.init_VAO();           // Initialize the vertex array object for this object
	skybox.init_VBO();			// Initialize the data buffers for this object
	skybox.init_shader();		// Initialize the shader objects and textures for skybox
	skybox.init_texture_map();	// Initialize the texture map for this object

	brick.init_data();		// Setup the data for the this object
	brick.init_VAO();		// Initialize the vertex array object for this object
	brick.init_VBO();		// Initialize the data buffers for this object
	brick.init_shader();		// Initialize the shader objects and textures for skybox
	brick.init_texture_map();// Initialize the texture map for this object

	stone.init_data();		// Setup the data for the this object
	stone.init_VAO();		// Initialize the vertex array object for this object
	stone.init_VBO();		// Initialize the data buffers for this object
	stone.init_shader();		// Initialize the shader objects and textures for skybox
	stone.init_texture_map();// Initialize the texture map for this object

	wood.init_data();		// Setup the data for the this object
	wood.init_VAO();		// Initialize the vertex array object for this object
	wood.init_VBO();		// Initialize the data buffers for this object
	wood.init_shader();		// Initialize the shader objects and textures for skybox
	wood.init_texture_map();// Initialize the texture map for this object

	sand.init_data();		// Setup the data for the this object
	sand.init_VAO();		// Initialize the vertex array object for this object
	sand.init_VBO();		// Initialize the data buffers for this object
	sand.init_shader();		// Initialize the shader objects and textures for skybox
	sand.init_texture_map();// Initialize the texture map for this object

	emerald.init_data();		// Setup the data for the this object
	emerald.init_VAO();		// Initialize the vertex array object for this object
	emerald.init_VBO();		// Initialize the data buffers for this object
	emerald.init_shader();		// Initialize the shader objects and textures for skybox
	emerald.init_texture_map();// Initialize the texture map for this object

	diamond.init_data();		// Setup the data for the this object
	diamond.init_VAO();		// Initialize the vertex array object for this object
	diamond.init_VBO();		// Initialize the data buffers for this object
	diamond.init_shader();		// Initialize the shader objects and textures for skybox
	diamond.init_texture_map();// Initialize the texture map for this object

	transparent.init_data();		// Setup the data for the this object
	transparent.init_VAO();		// Initialize the vertex array object for this object
	transparent.init_VBO();		// Initialize the data buffers for this object
	transparent.init_shader();		// Initialize the shader objects and textures for skybox
	transparent.init_texture_map();// Initialize the texture map for this object

	ground.init_data();		        // Setup the data for the this object
	ground.init_VAO();				// Initialize the vertex array object for this object
	ground.init_VBO();				// Initialize the data buffers for this object
	ground.init_shader();			// Initialize the shader objects and textures for skybox
	ground.init_texture_map();		// Initialize the texture map for this object


	// make sure I'm looking thre right way to start
	view = RotateY(-85) * view;//rotate eye -85 degrees
	at = eye + view;

	float movementSensitivity = 0.09f;
	moveBackOrForward = normalize(at - eye) * movementSensitivity;
	moveLeftOrRight = -normalize(cross(moveBackOrForward, up)) * movementSensitivity;

	GL_CHECK_ERRORS
}

void OnShutdown()
{
	skybox.cleanup(); // release the textures on the graphics card
}

void checkGlew()
{
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)	{
		cerr<<"Error: " << glewGetErrorString(err)<<endl;
	} else {
		if (GLEW_VERSION_3_3)
		{
			cout<<"Driver supports OpenGL 3.3\nDetails:"<<endl;
		}
	}
	cout<<"Using GLEW "<<glewGetString(GLEW_VERSION)<<endl;
	cout<<"Vendor: "<<glGetString (GL_VENDOR)<<endl;
	cout<<"Renderer: "<<glGetString (GL_RENDERER)<<endl;
	cout<<"Version: "<<glGetString (GL_VERSION)<<endl;
	cout<<"GLSL: "<<glGetString (GL_SHADING_LANGUAGE_VERSION)<<endl;
}

int main(int argc, char **argv)
{
	atexit(OnShutdown);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(width, height);
    glutCreateWindow("MoonCraft");


	checkGlew();
    init();
    glutReshapeFunc(myReshape);
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutPassiveMotionFunc(mouse_move);
    glutKeyboardFunc(press_key);
	glutKeyboardUpFunc(release_key);

	cout << "Welcome to Mooncraft!" << endl;
	cout << endl;
	cout << "*****************************************************" << endl;
	cout << "*   w moves forward" << endl;
	cout << "*   s moves backward" << endl;
	cout << "*   a moves left" << endl;
	cout << "*   d moves right" << endl;
	cout << "*   space bar jumps" << endl;
	cout << "*   j does a moon jump" << endl;
	cout << "*   move the mouse around the screen to move your view" << endl;
	cout << "*   left click places a brick where you're looking" << endl;
	cout << "*   right click destroys the brick you're looking at" << endl;
	cout << "*   clicking the mouse scroller switches brick types" << endl;
	cout << "*   i gives you information about your positioning" << endl;
	cout << "*   escape key unlocks and locks the cursor to the center of the screen" << endl;
	cout << "*   q quits the game" << endl;
	cout << "*****************************************************" << endl;
	cout << endl;
	cout << "Now create to your heart's content!" << endl;
	cout << endl << endl;

    glutMainLoop();

    return 0;
}
