//Includes
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stack>
#include "glut.h"

using namespace std;

#define GRIDSIZE 50 //Grid size in the x and in the y
//#define ENEMYCOUNT 9 //Number of Enemies
#define MAZEWIDTH 16 //Maze width
#define MAZEHEIGHT 16 //Maze Height
//#define GOALCOUNT 8 //Number of goals

const char WALL = '#';

int enemyCounter, goalCounter, wallCounter;

float cameraX = GRIDSIZE / 2; //Camera x position
float cameraY = GRIDSIZE / 2; //Camera y position
float cameraZ = GRIDSIZE / 4; //Camera z position
float totalAngleX = 0.0;
float totalAngleY = 0.0;

typedef struct rgbtag {
	float r; //Red value
	float g; //Green value
	float b; //Blue value
}rgb;

typedef struct entities
{
	float influence; //Influence of the point
	float range; //Range of influence of the point
	int x; //x coordinate of the point
	int y; //y coordinate of the point
	int speedX; //speed of the point if required
	int speedY; //speed of the point if required
}entity;

char loadGrid[GRIDSIZE*GRIDSIZE]; //Holds the 1 dimensional values of the influence map being loaded in as chars
char loadGrid2d[GRIDSIZE][GRIDSIZE]; //Holds the 2 dimensional values of the influence map being loaded in as chars
rgb grid[GRIDSIZE][GRIDSIZE]; //Hold the rgb values for each point on the map
rgb tempGrid[GRIDSIZE][GRIDSIZE]; //Temporarily holds the new rgb values to be saved to the grid

entity *walls; //Holds the data for the walls
entity *goal; //Holds the data for the goals
entity *enemy; //Holds the data for the enemies
entity player; //Holds the data for the player

//function prototypes
void init(void);
void display(void);
void drawBox(float w, float h, float l);
void reshape(int w, int h);
void setInfluence(void);
void playerAI(void);
void updateGrid(void);
void readMapIn(void);

void init()
// Initialize
//************************************************************************
// desc: initializes OpenGL
{
	readMapIn();
	player.influence = 0.1; //Set the player's influence (The value that alters the last position the player was at

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);		// clears colour buffer 
	glEnable(GL_DEPTH_TEST);					// hidden surface removal
	glEnable(GL_CULL_FACE);						// do not calculate inside of poly's
	glFrontFace(GL_CCW);						// counter clock-wise polygons are out

	int i, j, k;

	//Loop through for the GRIDSIZE
	for (i = 0; i < GRIDSIZE; i++) 
	{
		//Loop through for the GRIDSIZE
		for (j = 0; j < GRIDSIZE; j++) 
		{
			//Set the whole grid to black
			grid[i][j].r = 0;
			grid[i][j].g = 0;
			grid[i][j].b = 0;

			//Loop through for ever enemy
			for (k = 0; k < enemyCounter; k++)
			{
				//Check if i and j match the coordinates of the enemy
				if (i == enemy[k].x && j == enemy[k].y)
				{
					grid[i][j].r = 1.0; //Set the enemy's position to red
				}
			}

			//Loop through for every goal
			for (k = 0; k < goalCounter; k++)
			{
				//Check if i and j match the coordinates of the goal
				if (i == goal[k].x && j == goal[k].y)
				{
					grid[i][j].g = 1.0; //Set the goal's position to green
				}
			}

			//Check if i and j match the coordinates of the player AI
			if (i == player.x && j == player.y)
			{
				grid[i][j].g = 1.0; //Set the player AI's position to green
			}
		}
	}

	setInfluence(); //Call the function that sets all the initial influences
}

void setInfluence()
{
	float distance; //Hold the calculated distance from one source on the map to another point on the map
	float highestInfluenceEnemy = 0.0;
	float highestInfluenceGoal = 0.0;

	//Loop through for the GRIDSIZE
	for (int i = 0; i < GRIDSIZE; i++)
	{
		//Loop through for the GRIDSIZE
		for (int j = 0; j < GRIDSIZE; j++)
		{
			//Loop through for the number of enemies
			for (int k = 0; k < enemyCounter; k++)
			{
				//Check if the current point being looked at is the position of the enemy
				if (i == enemy[k].x && j == enemy[k].y)
					tempGrid[i][j].r = 1.0; //Set the current point being looked at to red
				else
				{
					//Calculate the distance from the source point to the current point being looked at on the map
					distance = sqrt(((enemy[k].x - i) * (enemy[k].x - i)) + ((enemy[k].y - j) * (enemy[k].y - j)));

					float tempRValue = enemy[k].influence / (exp(distance - enemy[k].range) + 1);
					//Calculate the influence the source has on the point in the map
					tempGrid[i][j].r += tempRValue;

					if (tempRValue > highestInfluenceEnemy)
						highestInfluenceEnemy = tempRValue;
				}
			}
			//Loop through for the number of goals
			for (int k = 0; k < goalCounter; k++)
			{
				//Check if the current point being looked at is the position of the goal
				if (i == goal[k].x && j == goal[k].y)
					tempGrid[i][j].g = 1.0; //Set the current point being looked at to green
				else
				{
					//Calculate the distance from the source point to the current point being looked at on the map
					distance = sqrt((goal[k].x - i) * (goal[k].x - i) + (goal[k].y - j) * (goal[k].y - j));

					float tempGValue = goal[k].influence / (exp(distance - goal[k].range) + 1);

					//Calculate the influence the source has on the point in the map
					tempGrid[i][j].g += tempGValue;

					if (tempGValue > highestInfluenceGoal)
						highestInfluenceGoal = tempGValue;
				}
			}
		}
	}
	//player.y++;
	//Loop through for the GRIDSIZE
	for (int i = 0; i < GRIDSIZE; i++)
	{
		//Loop through for the GRIDSIZE
		for (int j = 0; j < GRIDSIZE; j++)
		{
			tempGrid[i][j].r /= highestInfluenceEnemy; //Normalise the red values
			tempGrid[i][j].g /= highestInfluenceGoal; //Normalise the green value

			//Loop through for the GRIDSIZE
			for (int k = 0; k < wallCounter; k++)
			{
				//Create the white wall surrounding the scene so the player AI doesn't leave the area
				//Create white walls on the left
				if (i == walls[k].x && j == walls[k].y)
				{
					tempGrid[i][j].r = 1.0;
					//tempGrid[i][j].g = 1.0;
					//tempGrid[i][j].b = 1.0;
				}
			}

			//Check if i and j match the coordinates of the player AI
			if (i == player.x && j == player.y)
			{
				tempGrid[i][j].g = 1.0; //Set the position of the player in the map to green
			}
		}
	}

	//copy calculated array into original
	for (int i = 0; i < GRIDSIZE; i++)
	{
		for (int j = 0; j < GRIDSIZE; j++)
		{
			grid[i][j] = tempGrid[i][j];
		}
	}
}

void drawBox(float w, float h, float l)
//drawBox
//***************************************************************************
//This function first translates a further x,y,z units then draws a box of
//width w, height h, length l
{
	glPushMatrix();       //save modelview 
	w = w / 2.0; h = h / 2.0; l = l / 2.0; //adjust values so centre is in middle of box
										   //draw faces of box
	glBegin(GL_POLYGON);
	glVertex3f(w, l, h);
	glVertex3f(w, l, -h);
	glVertex3f(-w, l, -h);
	glVertex3f(-w, l, h);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(w, l, h);
	glVertex3f(-w, l, h);
	glVertex3f(-w, -l, h);
	glVertex3f(w, -l, h);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(w, l, h);
	glVertex3f(w, -l, h);
	glVertex3f(w, -l, -h);
	glVertex3f(w, l, -h);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(-w, l, h);
	glVertex3f(-w, l, -h);
	glVertex3f(-w, -l, -h);
	glVertex3f(-w, -l, h);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(-w, -l, h);
	glVertex3f(-w, -l, -h);
	glVertex3f(w, -l, -h);
	glVertex3f(w, -l, h);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(w, -l, -h);
	glVertex3f(-w, -l, -h);
	glVertex3f(-w, l, -h);
	glVertex3f(w, l, -h);
	glEnd();
	glPopMatrix();   //restore previous modelview matrix so leaving things as you found them 
}

// display
//************************************************************************
// desc: handles drawing of scene
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// clear screen and depth buffer
	glLoadIdentity();										// reset modelview matrix


	glRotatef(totalAngleX, 0, 1, 0); //Rotate around the y
	glRotatef(totalAngleY, 1, 0, 0); //Rotate around the x

	gluLookAt(cameraX, cameraY, cameraZ, cameraX, cameraY, cameraZ - 1, 0.0, 1.0, 0.0); //reset camera position
	glColor3f(0, 0, 0);  //set colour
	glTranslatef(0.5, 0.5, 0.5);  //move by x,y and z units

	//Loop through for the GRIDSIZE
	for (int i = 0; i < GRIDSIZE; i++) 
	{
		//Loop through for the GRIDSIZE
		for (int j = 0; j < GRIDSIZE; j++) 
		{
			if (grid[i][j].r > grid[i][j].b)
			{
				glColor3f(grid[i][j].r, 0, 0); //set the colour to red
			}
			else
			{
				glColor3f(0, 0, grid[i][j].b);  //set colour to blue
			}

			if (grid[i][j].r > grid[i][j].g)
			{
				glColor3f(grid[i][j].r, 0, 0); //set colour to red
			}
			else
			{
				glColor3f(0, grid[i][j].g, 0); //set colour to green
			}
			if (grid[i][j].g != 0)
			{
				glColor3f(grid[i][j].r, grid[i][j].g, grid[i][j].b);
			}

			glPushMatrix(); //save current position/orientation, etc.
			glTranslatef(i, j, 0);
			drawBox(1, 1, 1);  //draw 
			glPopMatrix(); //restore previous "current" position/orientation etc.
		}
	}


	glFlush();  //force drawing

	glutSwapBuffers();			// bring backbuffer to foreground
								//any errors then display error codes 
	GLenum errCode;
	const GLubyte *errString;
	if ((errCode = glGetError()) != GL_NO_ERROR)
	{
		errString = gluErrorString(errCode);
		fprintf(stderr, "OpenGL error : %s\n", errString);
	}

}

//Handles the user inputs
void keyboardread(unsigned char key, int x, int y)
{
	//Values used to alter the camera
	float vx = 0, vy = 0, vz = 0, xAngle = 0, yAngle = 0;

	switch (key) 
	{
	case 'w':
	{
		vy += 1; //Move Camera up
		break;
	}
	case 'd':
	{
		vx += 1; //Move Camera right
		break;
	}
	case 's':
	{
		vy -= 1; //Move Camera down
		break;
	}
	case 'a':
	{
		vx -= 1; //Move Camera left
		break;
	}
	case 'j':
	{
		xAngle += 10; //Turn image to the left
		break;
	}
	case 'l':
	{
		xAngle -= 10; //Turn image to the right
		break;
	}
	case 'i':
	{
		vz -= 1; //Move camera forwards
		break;
	}
	case 'k':
	{
		vz += 1; //Move camera backwards
		break;
	}
	case 'u':
		break;
	case ' ':

		//Reset the tempGrid to all black except for the blue value
		for (int i = 0; i < GRIDSIZE; i++)
		{
			for (int j = 0; j < GRIDSIZE; j++)
			{
				tempGrid[i][j].r = 0.0;
				tempGrid[i][j].g = 0.0;
			}
		}

		playerAI(); //Run the playerAI function
		
		int oldPlayerX = player.x; //Holds player's last x position
		int oldPlayerY = player.y; //Holds player's last y position

		int oldEnemyX[GRIDSIZE * GRIDSIZE]; //Holds enemy's last x position
		int oldEnemyY[GRIDSIZE * GRIDSIZE]; //Holds enemy's last y position

		//Loop through for as many enemies as there are
		for (int k = 0; k < enemyCounter; k++)
		{
			oldEnemyX[k] = enemy[k].x; //Save the enemy's last x position
			oldEnemyY[k] = enemy[k].y; //Save the enemy's last y position

			//Check if the enemy has hit a wall and if so reverse their speed in the axis
			for (int l = 0; l < wallCounter; l++)
			{
				if (enemy[k].x == walls[l].x && enemy[k].y + 1 == walls[l].y || enemy[k].x == walls[l].x && enemy[k].y - 1 == walls[l].y)
					enemy[k].speedY *= -1;

				if (enemy[k].x + 1 == walls[l].x && enemy[k].y == walls[l].y || enemy[k].x - 1 == walls[l].x && enemy[k].y== walls[l].y)
					enemy[k].speedX *= -1;
			}

			enemy[k].x += enemy[k].speedX;
			enemy[k].y += enemy[k].speedY;
		}

		//Loop through for the number of goals or green targets on the map
		for (int i = 0; i < goalCounter; i++)
		{
			//Check if the player has landed on a goal
			if (player.x == goal[i].x && player.y == goal[i].y)
			{
				goal[i].x = -10;
				goal[i].y = -10;
				//tempGrid[goal[i].x][goal[i].y].g = 0.0;
				//goal[i].influence = 0.0; //Set the goal's influence to 0.0
			}

		}
		//Loop through for the GRIDSIZE
		for (int i = 0; i < GRIDSIZE; i++)
		{
			//Loop through for the GRIDSIZE
			for (int j = 0; j < GRIDSIZE; j++)
			{
				//Loop through for the number of enemies
				//for (int k = 0; k < enemyCounter; k++)
				//{
					////Check if i and j match the coordinate points of the current enemy position being looked at
					//if (i == enemy[k]f.x && j == enemy[k].y)
					//	tempGrid[i][j].r = 1.0; //Set the r value of the current enemy position to 1.0

					////Check if i and j match the coordinate points of the last position of the enemy being looked at
					//if (i == oldEnemyX[k] && j == oldEnemyY[k])
					//	tempGrid[i][j].r = 0.0; //Set the r value of the last position the enemy was at to 0.0 
				//}
				//Check if i and j match the coordinate points of the last player AI position
				if (i == oldPlayerX && j == oldPlayerY)
				{
					tempGrid[i][j].b += player.influence; //Add the player influence to b value in the last location of the player AI
					tempGrid[i][j].g = 0.0; //Set the g value in the last location of the player AI to 0.0
				}
				//Check if i and j match the coordinate points of the current player AI position
				if (i == player.x && j == player.y)
				{
					tempGrid[i][j].g = 1.0; //Set the g value in the current player position to 1.0
				}
			}
		}
		updateGrid(); //Call the upgradeGrid function
		break;
	}

	cameraX += vx; //Change the cameraX value, making the camera move in the x
	cameraY += vy; //Change the cameraY value, making the camera move in the y
	cameraZ += vz; //Change the cameraZ value, making the camera move in the z

	totalAngleX += xAngle; //Change the totalAngleX value, making the camera rotate around the y
	totalAngleY += yAngle; //Change the totalAngleY value, making the camera rotate around the z

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glutPostRedisplay();
}

//Updates the grid
void updateGrid()
{
	float distance; //Hold the calculated distance from one source on the map to another point on the map
	float highestInfluenceEnemy = 0.0;
	float highestInfluenceGoal = 0.0;

	//Loop through for the GRIDSIZE
	for (int i = 0; i < GRIDSIZE; i++)
	{
		//Loop through for the GRIDSIZE
		for (int j = 0; j < GRIDSIZE; j++)
		{
			//Loop through for the number of enemies
			for (int k = 0; k < enemyCounter; k++)
			{
				//Check if the current point being looked at is the position of the enemy
				if (i == enemy[k].x && j == enemy[k].y)
					tempGrid[i][j].r = 1.0; //Set the current point being looked at to red
				else
				{
					//Calculate the distance from the source point to the current point being looked at on the map
					distance = sqrt(((enemy[k].x - i) * (enemy[k].x - i)) + ((enemy[k].y - j) * (enemy[k].y - j)));

					float tempRValue = enemy[k].influence / (exp(distance - enemy[k].range) + 1);
					//Calculate the influence the source has on the point in the map
					tempGrid[i][j].r += tempRValue;

					if (tempRValue > highestInfluenceEnemy)
						highestInfluenceEnemy = tempRValue;
				}
			}
			//Loop through for the number of goals
			for (int k = 0; k < goalCounter; k++)
			{
				//Check if the current point being looked at is the position of the goal
				if (i == goal[k].x && j == goal[k].y)
					tempGrid[i][j].g = 1.0; //Set the current point being looked at to green
				else
				{
					//Calculate the distance from the source point to the current point being looked at on the map
					distance = sqrt((goal[k].x - i) * (goal[k].x - i) + (goal[k].y - j) * (goal[k].y - j));

					float tempGValue = goal[k].influence / (exp(distance - goal[k].range) + 1);

					//Calculate the influence the source has on the point in the map
					tempGrid[i][j].g += tempGValue;

					if (tempGValue > highestInfluenceGoal)
						highestInfluenceGoal = tempGValue;
				}
			}
		}
	}

	//Loop through for the GRIDSIZE
	for (int i = 0; i < GRIDSIZE; i++)
	{
		//Loop through for the GRIDSIZE
		for (int j = 0; j < GRIDSIZE; j++)
		{
			tempGrid[i][j].r /= highestInfluenceEnemy;
			tempGrid[i][j].g /= highestInfluenceGoal;

			//Loop through for the GRIDSIZE
			for (int k = 0; k < wallCounter; k++)
			{
				//Create the white wall surrounding the scene so the player AI doesn't leave the area
				//Create white walls on the left
				if (i == walls[k].x && j == walls[k].y)
				{
					tempGrid[i][j].r = 1.0;
					//tempGrid[i][j].g = 1.0;
					//tempGrid[i][j].b = 1.0;
				}
			}
		}
	}

	//copy calculated array into original
	for (int i = 0; i < GRIDSIZE; i++)
	{
		for (int j = 0; j < GRIDSIZE; j++)
		{
			grid[i][j] = tempGrid[i][j];
		}
	}
}

void reshape(int w, int h)
//reshape
//************************************************************************
//OpenGL reshape callback is called when the window is resized
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h); //respecify size of view port to match new w and h
	glMatrixMode(GL_PROJECTION);  //go into projection mode
	glLoadIdentity();   //reset projection matrix
	//glOrtho(0.0, GRIDSIZE, 0.0, GRIDSIZE, 0.0, 100.0); //defines orthoganal projection (parrallel
													   // viewing volume
													   // window limits of display
													   // left =0.0, right =2.0,
													   // bottom = 0.0, top = 2.0
													   // near =0.0, far = 100.0
	gluPerspective(500.0,(GLfloat)w/(GLfloat)h, 0.1, 100.0);  //redefine projection matrix to match changes in w and h
	glMatrixMode(GL_MODELVIEW);  //switch back to modelview mode
	glLoadIdentity(); //reset modelview matrix
	gluLookAt(GRIDSIZE / 2, GRIDSIZE / 2, 7.0f, GRIDSIZE / 2, GRIDSIZE / 2, 6.0f, 0.0, 1.0, 0.0); //reset camera position
}

void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboardread);
	glutMainLoop();
}

void playerAI()
{
	int movementCheck = 0; //0 = player moves up, 1 = player moves right, 2 = player moves down, 3 = player moves left

	float upR, rightR, downR, leftR; //Holeds the altered rgb(influence) values

	//Grab the red value and multiply by 2 because you don't want the player to move towards red
	//then add the blue value because you don't want the player to move towards blue, 
	//but don't muliply it as you want the player to move to blue instead of red
	//then subtract the green value from it because you want the player to move towards green
	//Do this for above the player, to the right of the player, below the player, and to the left of the player
	upR = (grid[player.x][player.y + 1].r) * 2 + (grid[player.x][player.y + 1].b) - (grid[player.x][player.y + 1].g);
	rightR = (grid[player.x + 1][player.y].r) * 2 + (grid[player.x + 1][player.y].b) - (grid[player.x + 1][player.y].g);
	downR = (grid[player.x][player.y - 1].r) * 2 + (grid[player.x][player.y - 1].b) - (grid[player.x][player.y - 1].g);
	leftR = (grid[player.x - 1][player.y].r) * 2 + (grid[player.x - 1][player.y].b) - (grid[player.x - 1][player.y].g);

	if (player.y + 1 >= GRIDSIZE - 1)
		upR += rightR + downR + leftR + upR;
	if (player.y - 1 <= 0)
		downR += upR + rightR + leftR + downR;
	if (player.x + 1 >= GRIDSIZE - 1)
		rightR += upR + downR + leftR + rightR;
	if (player.x - 1 <= 0)
		leftR += upR + downR + rightR + leftR;

	//Check if right has the smallest influence
	if (rightR < upR && rightR < downR && rightR < leftR)
	{
		movementCheck = 1;
	}
	//Check if down has the smallest influence
	if (downR < upR && downR < rightR && downR < leftR)
	{
		movementCheck = 2;
	}
	//Check if left has the smallest influence
	if (leftR < upR && leftR < downR && leftR < rightR)
	{
		movementCheck = 3;
	}
	//Check if up has the smallest influence
	if (upR < leftR && upR < downR && upR < rightR)
	{
		movementCheck = 0;
	}

	//Move the player differently depending on the value of movementCheck
	switch (movementCheck)
	{
	case 0:
	{
		player.y++; //Move the player up
		break;
	}
	case 1:
	{
		player.x++; //Move the player right
		break;
	}
	case 2:
	{
		player.y--; //Move the player down
		break;
	}
	case 3:
	{
		player.x--; //Move the player left
		break;
	}
	default:
		break;
	}
}

void readMapIn()
{
	string line;
	string infMapFile;

	ifstream infMapList;

	int count;
	char choice[32];
	
	do
	{
		count = 0;
		infMapList.open("Influence Map List.txt"); //Opens the file containing the list of influence maps

		cout << "Enter the influence map you want to load in by entering the corresponding number" << endl;

		//Read the entire file and print out the options for the user
		while (getline(infMapList, line))
		{
			count++;
			cout << count << ": " << line << endl;
		}

		cin >> choice; //User inputs the influence map file they wish to load

		infMapList.close(); //close the file

		infMapList.open("Influence Map List.txt"); //Opens the file containing the list of influence maps

		count = 0;

		//Read the entire file and check if the choice the user input was valid
		while (getline(infMapList, line))
		{
			count++;

			//Check if the input was valid
			if (count == atoi(choice))
			{
				infMapFile = line; //Grab the name of the file that the user chose
			}

		}

		infMapList.close(); //Close file

	} while (infMapFile.empty()); //Loop until the input is valid and so infMapFile isn't empty



	ifstream infMap(infMapFile); //Opens the file containing the maze

	count = 0;

	//Loop through until the entire file has been read
	while (getline(infMap, line))
	{
		line.copy(&loadGrid[GRIDSIZE*count], GRIDSIZE, 0); //Read in the maze and add it to the maze variable
		count++;
	}

	infMap.close(); //Close the file

	int enemyCount = 0, goalCount = 0, wallCount = 0;

	//Count the total number of enemies, goals, and walls
	for (int i = 0; i < GRIDSIZE; i++)
	{
		for (int j = 0; j < GRIDSIZE; j++)
		{
			if (loadGrid[GRIDSIZE*i + j] == 'E' || loadGrid[GRIDSIZE*i + j] == 'B')
				enemyCount++;

			if (loadGrid[GRIDSIZE*i + j] == 'G')
				goalCount++;

			if (loadGrid[GRIDSIZE*i + j] == '#')
				wallCount++;
		}
	}

	//Save the read in map into a 2 dimensional array
	for (int i = 0; i < GRIDSIZE; i++)
	{
		for (int j = 0; j < GRIDSIZE; j++)
		{
			loadGrid2d[i][j] = loadGrid[GRIDSIZE*i + j];
		}
	}

	enemy = new entity[enemyCount]; //Set the size of the enemy array of entities
	goal = new entity[goalCount]; //Set the size of the goal array of entities
	walls = new entity[wallCount]; //Set the size of the walls array of entities

	//Save the counts to global variables for future use
	enemyCounter = enemyCount;
	goalCounter = goalCount;
	wallCounter = wallCount;

	enemyCount = 0, goalCount = 0, wallCount = 0; //Reset the counts

	//Save the read in amp into a 2 dimensional array
	for (int i = 0; i < GRIDSIZE; i++)
	{
		for (int j = 0; j < GRIDSIZE; j++)
		{
			//Load in the all the positions and other variables for the enemies
			if (loadGrid2d[i][j] == 'E')
			{
				enemy[enemyCount].x = i;
				enemy[enemyCount].y = j;
				enemy[enemyCount].influence = 0.5;
				enemy[enemyCount].range = 1.0;

				//Randomise the direction that the enemies are moving in
				int speedx = rand() % 2;
				int speedy = rand() % 2;

				int xDirection = rand() % 2;
				int yDirection = rand() % 2;

				if (speedx == 1)
				{
					if (xDirection == 1)
						enemy[enemyCount].speedX = 1;
					else
						enemy[enemyCount].speedX = -1;
				}
				else
				{
					enemy[enemyCount].speedX = 0;
				}
				if (speedy == 1)
				{
					if (yDirection == 1)
						enemy[enemyCount].speedY = 1;
					else
						enemy[enemyCount].speedY = -1;
				}
				else
				{
					enemy[enemyCount].speedY = 0;
				}
				enemyCount++;
			}

			//Load in all the details for the big enemy
			if (loadGrid2d[i][j] == 'B')
			{
				enemy[enemyCount].x = i;
				enemy[enemyCount].y = j;
				enemy[enemyCount].influence = 0.5;
				enemy[enemyCount].range = 3.0;
				enemy[enemyCount].speedX = 0.0;
				enemy[enemyCount].speedY = 0.0;
				enemyCount++;
			}

			//Load in all the details for the goals
			if (loadGrid2d[i][j] == 'G')
			{
				goal[goalCount].x = i;
				goal[goalCount].y = j;
				goal[goalCount].influence = 1.0;
				goal[goalCount].range = 1.0;
				goalCount++;
			}

			//Load in all the details for the walls
			if (loadGrid2d[i][j] == '#')
			{
				walls[wallCount].x = i;
				walls[wallCount].y = j;
				wallCount++;
			}

			if (loadGrid2d[i][j] == 'S') //look for the S in the maze and set the player's start position to it
			{
				player.x = i;
				player.y = j;
			}
		}
	}
}