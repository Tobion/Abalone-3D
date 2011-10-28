#include <stdlib.h>
#include <windows.h>
#include <string.h>

#include <gl\gl.h>
#include <gl\glu.h>	
#include <gl/glut.h>

#include <iostream>
#include <fstream>
#include <math.h>

#include "Abalone.h"
#include "Camera.h"
#include "AbaloneBoard.h"

extern "C"
{
	#include "gltx.h"
	#include "glm.h"
}


using namespace std;


#ifdef _DEBUG
	#define GameVariantsPath "../../GameVariants/"
	#define SavedGamesPath "../../SavedGames/"
	#define ModelsPath "../../Models/"
	#define TexturesPath "../../Textures/"
	#define SoundsPath "../../Sounds/"
#else
	#define GameVariantsPath "./GameVariants/"
	#define SavedGamesPath "./SavedGames/"
	#define ModelsPath "./Models/"
	#define TexturesPath "./Textures/"
	#define SoundsPath "./Sounds/"
#endif


#define BoardModelFile "board_bevel_subdivided.obj"

#define WindowSizeX 700
#define WindowSizeY 700

#define MaxBalls 64
#define FieldDistance 0.32
#define BallCenterY 0.15
#define BallRadius 0.15
#define BoardCenterX 0
#define BoardCenterZ 0

#define MaxTextLength 99
		

float diffx = 1, diffy = 1;								
int step, a;

GLfloat KameraPosX = 0.0f;
GLfloat KameraPosY = 3.0f; 
GLfloat KameraPosZ = 3.0f; 
int MausPosX= 0;
int MausPosY= 0;

int MouseButton;

float position_x, position_y, position_z;
bool Kamerafahrt = false;
bool FullScreen = false;
bool MenuInUse = false;
bool GameRunning = false;
bool InReplay = false;

string CurrentGameVariant;

GLMmodel* BoardModel;
GLuint BoardObj;
GLuint SkyboxObj;
AbaloneBoard Board(BoardCenterX, BoardCenterZ, FieldDistance, BallCenterY);

GLuint Textures[6];
GLuint TextureLists[6];


enum SkyboxPlanes { SkyboxFront, SkyboxRight, SkyboxBack, SkyboxLeft, SkyboxUp, SkyboxDown };


void showAbout()
{	
	cout << "Abalone 3D" << endl;	
	cout << "Copyright 2008 by Kerstin Hartig, Tobias Schultze, Janko Winkler" << endl;
	cout << endl << "Druecken Sie F1 fuer Hilfe und die rechte Maustaste zum Starten des Spiels." << endl;
}

void showHelp()
{	
	cout << "**************************************" << endl;
	cout << "Bedienung:" << endl;
	cout << "\t Rechte Maustaste:\t Menu (Spiel starten, laden, speichern)" << endl;
	cout << "\t Linke Maustaste:\t Kamera bewegen und Kugel auswaehlen" << endl;
	cout << "\t \t Die Pfeile zeigen die Richtungen an, in die man ziehen darf." << endl;
	cout << "\t \t Ein Klick auf diese fuehrt den Zug aus." << endl;
	cout << "\t I:\t Hereinzoomen" << endl;
	cout << "\t O:\t Herauszoomen" << endl;
	cout << "\t T:\t Ansicht von oben" << endl;
	cout << "\t F1:\t Hilfe" << endl;
	cout << "\t F12:\t Fullscreen" << endl;
	cout << "\t Escape:\t Programm beenden" << endl;
	cout << "**************************************" << endl;
	cout << "Spielregeln:" << endl;
	cout << " Ziel des Spiels ist es, 6 gegnerische Kugeln vom Spielfeld zu verdraengen." << endl;
	cout << " Jeder der zwei Spieler ist abwechselnd an der Reihe, seinen Zug durchzufuehren." << endl;
	cout << " Man kann 1, 2 oder maximal 3 Kugeln auf einmal bewegen und dadurch die Reihe " << endl;
	cout << " der gegnerischen Kugeln um jeweils ein Feld in jede Richtung verdraengen." << endl << endl;
	cout << " Um in der Lage zu sein, die Kugeln des Gegners verschieben zu koennen," << endl;
	cout << " muss die eigene Kugelreihe jener des Gegners ueberlegen sein:"<< endl;
	cout << " mit 3 Kugeln kann man eine oder zwei Kugeln und mit 2 Kugeln nur noch eine verdraengen." << endl << endl;
	cout << " Wenn die Anzahl an Kugeln auf beiden Seiten gleich ist" << endl;
	cout << " (eine Kugel gegen eine Kugel, oder zwei gegen zwei und drei gegen drei)," << endl;
	cout << " kann man keine gegnerische Kugel verschieben." << endl;
	cout << "**************************************" << endl;
}


string getFilename(string directory, int number)
{
	HANDLE fHandle; 
	WIN32_FIND_DATA wfd; 
	int i = 0;
	string path;

	path = directory += "*";

	fHandle = FindFirstFile(path.c_str(), &wfd); 

	do 
	{ 
		if (!( (wfd.cFileName[0]=='.') && ( (wfd.cFileName[1]=='.' && wfd.cFileName[2]==0) || wfd.cFileName[1]==0 ) )) 
		{ 
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
			{ 
				if (number == i) 
				{
					FindClose(fHandle);
					return wfd.cFileName;
				}	
			i++;
			} 
		} 
	} 
	while (FindNextFile(fHandle,&wfd)); 
	FindClose(fHandle);
	return "";
}


void newGame(string gamevariantname, bool cameramovement)
{
	if (!(Board.loadSetup(GameVariantsPath + gamevariantname))) {
		cout << "Spielvariante nicht vorhanden!" << endl;
		exit(1);
	}
	else {
		//Board.printBoard();
		CurrentGameVariant = gamevariantname;
		GameRunning = true;
		if (cameramovement)
		{
			positionx();
			positiony();
			positionz();
			findangle();
			findangle_360();
			kamerafahrt_zuplayer(Board.getOppositeCurrentPlayer());
			Kamerafahrt = true;
		}
		else 
			Kamerafahrt = false;

		glutPostRedisplay();
	}
}

void saveGame()
{
	string savegame;

	cout << "Spielstellung speichern in Datei: ";
	//getline(cin, savegame);
	cin >> savegame; 
	Board.saveGame(SavedGamesPath + savegame, CurrentGameVariant);
	initMenu();
}


void replay(int value)			
{
	if (Board.moves.size() > value)
	{
		Board.move(Board.moves[value].fromX, Board.moves[value].fromY, Board.moves[value].direction, false);
		glutPostRedisplay();
		glutTimerFunc(1000, &replay, value + 1);
	}
	else
	{
		InReplay = false;
		checkGameOver();
	}
}

void loadGame(string filename)
{
	std::string line, file;

	file = SavedGamesPath + filename;
	std::ifstream myfile(file.c_str());

	Board.moves.clear();
	Board.initBoardFields();

	if (myfile.is_open())
	{
		int x = -1;
		while (!myfile.eof())
		{
			Move themove;

			std::getline(myfile,line);
			
			if (x == -1)
			{
				newGame(line, false);
			}
			else
			{
				themove.fromX = (int) line.at(0) - '0';
				themove.fromY = (int) line.at(1) - '0';
				themove.direction = MoveDirection((int) line.at(2) - '0');
				//themove.toX = (int) line.at(3) - '0';
				//themove.toY = (int) line.at(4) - '0';
				Board.moves.push_back(themove);
			}

			x++;
		}

		myfile.close();
	}
	glutPostRedisplay();
	InReplay = true;
	glutTimerFunc(1000, &replay, 0);
}


void loadSetup(string filename)
{
	if (!(Board.loadSetup(SavedGamesPath + filename))) {
		cout << "Savegame nicht vorhanden!" << endl;
		exit(1);
	}
	else {
		//Board.printBoard();
		GameRunning = true;
		glutPostRedisplay();
	}
}





void selectMainMenu(int value)
{
	switch(value)
	{
		case 0: 
			saveGame();
			break;
		case 1: 
			showHelp();
			break;
	}
}

void selectNewGameMenu(int value)
{
	newGame(getFilename(GameVariantsPath, value), true);
}

void selectLoadGameMenu(int value)
{
	loadGame(getFilename(SavedGamesPath, value));
}

void selectBackgroundMenu(int value)
{
	initSkybox(Skyboxes(value));
	glutPostRedisplay();
}

void MenuChange(int status, int x, int y)
{
	MenuInUse = (status == GLUT_MENU_IN_USE);
}

void initMenu()
{
	HANDLE fHandle; 
	WIN32_FIND_DATA wfd; 

	int gamevariantsmenu, loadgamesmenu, backgroundmenu, gameid = 0;
	gamevariantsmenu = glutCreateMenu(selectNewGameMenu);

	fHandle = FindFirstFile(GameVariantsPath "*", &wfd); 

	do 
	{ 
		if (!( (wfd.cFileName[0]=='.') && ( (wfd.cFileName[1]=='.' && wfd.cFileName[2]==0) || wfd.cFileName[1]==0 ) )) 
		{ 
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
			{ 
				glutAddMenuEntry(wfd.cFileName, gameid);	
				gameid++;
			} 
		} 
	} 
	while (FindNextFile(fHandle,&wfd)); 
	FindClose(fHandle);

	gameid = 0;

	loadgamesmenu = glutCreateMenu(selectLoadGameMenu);

	fHandle = FindFirstFile(SavedGamesPath "*", &wfd); 

	do 
	{ 
		if (!( (wfd.cFileName[0]=='.') && ( (wfd.cFileName[1]=='.' && wfd.cFileName[2]==0) || wfd.cFileName[1]==0 ) )) 
		{ 
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
			{ 
				glutAddMenuEntry(wfd.cFileName, gameid);	
				gameid++;
			} 
		} 
	} 
	while (FindNextFile(fHandle,&wfd)); 
	FindClose(fHandle);

	backgroundmenu = glutCreateMenu(selectBackgroundMenu);
	glutAddMenuEntry("Alps", Alps);
	glutAddMenuEntry("Islands", Islands);
	glutAddMenuEntry("Lagoon", Lagoon);
	glutAddMenuEntry("Morning", Morning);
	glutAddMenuEntry("Sahara", Sahara);
	
	glutCreateMenu(selectMainMenu);
	glutAddSubMenu("Neues Spiel", gamevariantsmenu);
	glutAddMenuEntry("Spiel speichern", 0);
	glutAddSubMenu("Spiel laden", loadgamesmenu);
	glutAddSubMenu("Hintergrund", backgroundmenu);
	glutAddMenuEntry("Hilfe", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	glutMenuStatusFunc(MenuChange);
}


void checkGameOver()
{
	GameRunning = (Board.getWinner() == None);
	if (!GameRunning)
	{
		PlaySound(SoundsPath "Win_Match.wav", NULL, SND_FILENAME|SND_SYNC|SND_NODEFAULT);
		kamerafahrt(0);
	}
}

void moveIt(MoveDirection d)
{
	for (int x = 0; x < BoardSize; x++) {

		for (int y = 0; y < BoardSize; y++) {

			if (Board.isPosInsideBoard(x, y) && Board.boardPos[x][y].selected && Board.isMoveAllowed(x, y, d)) 
			{
				PlaySound(SoundsPath "MoveBall.wav", NULL, SND_FILENAME|SND_ASYNC|SND_NODEFAULT);
				Board.move(x, y, d);
				checkGameOver();

				if (GameRunning) 
				{
					Kamerafahrt = true;
				
				if (Board.getCurrentPlayer()==Player1)
				{
				positionx();
				positiony();
				positionz();
				findangle();
				findangle_360();
			    kamerafahrt_zuplayer(2);
				}
				else if (Board.getCurrentPlayer()==Player2)
				{
				positionx();
				positiony();
				positionz();
				findangle();
				findangle_360();
			    kamerafahrt_zuplayer(1);
				}
				
				}
				
				glutPostRedisplay();
				return;
			}

		}
	}
}


void drawTable(float radius, float x, float y, float z)
{
	glPushMatrix();
	glTranslated(x,y,z);
	gluCylinder(gluNewQuadric(), radius, radius, 5, 100, 100);
	glPopMatrix();
}

void drawArrow()										
{
	
glPushMatrix();
	glDisable(GL_LIGHTING);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f (1.0, 0.0, 0.0);
	glTranslated(0,0.15,-0.1);
	glScalef(0.5,0.5,0.5);
glBegin(GL_QUADS);  
	glVertex3f(-0.04f, 0.0f, 0.0f);
	glVertex3f( 0.04f, 0.0f, 0.0f); 
	glVertex3f( 0.04f, 0.0f,-0.2f); 
	glVertex3f(-0.04f, 0.0f,-0.2f);
glEnd();
glBegin(GL_TRIANGLES);   
	glVertex3f( 0.1f, 0.0f,-0.2f); 
	glVertex3f( 0.0f, 0.0f,-0.4f);
	glVertex3f(-0.1f, 0.0f,-0.2f);
glEnd();
/*
glBegin(GL_QUADS);  
	glVertex3f(-0.04f, 0.1f, 0.0f);
	glVertex3f( 0.04f, 0.1f, 0.0f); 
	glVertex3f( 0.04f, 0.1f,-0.2f); 
	glVertex3f(-0.04f, 0.1f,-0.2f);
glEnd();
glBegin(GL_TRIANGLES);   
	glVertex3f( 0.1f, 0.1f,-0.2f); 
	glVertex3f( 0.0f, 0.1f,-0.4f);
	glVertex3f(-0.1f, 0.1f,-0.2f);
glEnd();
glBegin(GL_QUADS);  
	glVertex3f(-0.04f, 0.0f, 0.0f);
	glVertex3f(-0.04f, 0.1f, 0.0f);
	glVertex3f( 0.04f, 0.1f, 0.0f); 
	glVertex3f( 0.04f, 0.0f, 0.0f);
glEnd();
glBegin(GL_QUADS);  
	glVertex3f(-0.04f, 0.0f, 0.0f);
	glVertex3f(-0.04f, 0.1f, 0.0f);
	glVertex3f(-0.04f, 0.1f,-0.2f); 
	glVertex3f(-0.04f, 0.0f,-0.2f);
glEnd();
glBegin(GL_QUADS);  
	glVertex3f( 0.04f, 0.0f, 0.0f);
	glVertex3f( 0.04f, 0.1f, 0.0f);
	glVertex3f( 0.04f, 0.1f,-0.2f); 
	glVertex3f( 0.04f, 0.0f,-0.2f);
glEnd();
glBegin(GL_QUADS);  
	glVertex3f( 0.04f, 0.0f,-0.2f);
	glVertex3f( 0.04f, 0.1f,-0.2f);
	glVertex3f(  0.1f, 0.1f,-0.2f); 
	glVertex3f(  0.1f, 0.0f,-0.2f);
glEnd();
glBegin(GL_QUADS);  
	glVertex3f( 0.1f, 0.0f,-0.2f);
	glVertex3f( 0.1f, 0.1f,-0.2f);
	glVertex3f( 0.0f, 0.1f,-0.4f); 
	glVertex3f( 0.0f, 0.0f,-0.4f);
glEnd();
glBegin(GL_QUADS);  
	glVertex3f(-0.1f, 0.0f,-0.2f);
	glVertex3f(-0.1f, 0.1f,-0.2f);
	glVertex3f( 0.0f, 0.1f,-0.4f); 
	glVertex3f( 0.0f, 0.0f,-0.4f);
glEnd();
glBegin(GL_QUADS);  
	glVertex3f(-0.04f, 0.0f,-0.2f);
	glVertex3f(-0.04f, 0.1f,-0.2f);
	glVertex3f( -0.1f, 0.1f,-0.2f); 
	glVertex3f( -0.1f, 0.0f,-0.2f);
glEnd();
//glDepthFunc(GL_LEQUAL);


//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//glLineWidth (4);
glColor3f (0.0, 0.0, 0.0);
glBegin(GL_LINES);  
	glVertex3f(-0.04f, 0.0f, 0.0f);
	glVertex3f( 0.04f, 0.0f, 0.0f); 
	glVertex3f( 0.04f, 0.0f,-0.2f);   
	glVertex3f( 0.01f,  0.0f,-0.2f); 
	glVertex3f( 0.0f,  0.0f,-0.4f);
	glVertex3f(-0.1f,  0.0f,-0.2f);
	glVertex3f(-0.04f, 0.0f,-0.2f);
	glVertex3f(-0.04f, 0.0f, 0.0f);
	glVertex3f(-0.04f, 0.1f, 0.0f);
	glVertex3f( 0.04f, 0.1f, 0.0f);
	glVertex3f( 0.04f, 0.0f, 0.0f);
	glVertex3f( 0.04f, 0.1f, 0.0f);
	glVertex3f( 0.04f, 0.1f,-0.2f);
	glVertex3f( 0.04f, 0.0f,-0.2f);
	glVertex3f( 0.04f, 0.1f,-0.2f);
	glVertex3f( 0.1f,  0.1f,-0.2f); 
	glVertex3f( 0.1f,  0.0f,-0.2f);
	glVertex3f( 0.1f,  0.1f,-0.2f);
	glVertex3f( 0.0f,  0.1f,-0.4f);
	glVertex3f( 0.0f,  0.0f,-0.4f);
	glVertex3f( 0.0f,  0.1f,-0.4f);
	glVertex3f(-0.1f,  0.1f,-0.2f);
	glVertex3f(-0.1f,  0.0f,-0.2f);
	glVertex3f(-0.1f,  0.1f,-0.2f);
	glVertex3f(-0.04f, 0.1f,-0.2f);
	glVertex3f(-0.04f, 0.0f,-0.2f);
	glVertex3f(-0.04f, 0.1f,-0.2f);
	glVertex3f(-0.04f, 0.1f, 0.0f);
	glVertex3f(-0.04f, 0.0f, 0.0f);
glEnd();
*/
//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f (1.0, 1.0, 1.0);
	glEnable(GL_LIGHTING);
glPopMatrix();
}

void drawBall(float x, float y, float z, Player player, GLenum mode, int x1, int y1)				
{
	if (player == None) return;
	glPushMatrix();
	glTranslated(x,y,z);
	glRotatef  ( 30 , 0.0 , 1.0 , 0.0 );
	switch(player)
	{
		case Player1:
			glDisable(GL_LIGHT1);
			glDisable(GL_LIGHT0);
			glEnable(GL_LIGHT2);
			break;
		case Player2:
			glDisable(GL_LIGHT2);
			glEnable(GL_LIGHT0);
			glEnable(GL_LIGHT1);
			break;
	}
		
	if (Board.boardPos[x1][y1].selected && (Board.boardPos[x1][y1].owner == Board.getCurrentPlayer()))
	{
		for (int direction = 0; direction <= dWest; direction++) 
		{			
			if (Board.isMoveAllowed(x1, y1, MoveDirection(direction)))
			{
				glLoadName(direction + MaxBalls);
				drawArrow();
				glLoadName(0);
			}
				glRotatef  ( -60 , 0.0 , 1.0 , 0.0 );
		}
		glDisable(GL_LIGHT2);
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT0);
		glEnable(GL_LIGHT3);
	}
	if (mode == GL_SELECT && Board.boardPos[x1][y1].owner == Board.getCurrentPlayer())
		glLoadName(Board.boardPos[x1][y1].id);
	glutSolidSphere(BallRadius, 100.0, 100.0);
	glLoadName(0);
	glDisable(GL_LIGHT3);
	glPopMatrix();
}



void drawAllBalls(GLenum mode)
{
	for (int x = 0; x < BoardSize; x++) {

		for (int y = 0; y < BoardSize; y++) {

			if (Board.isPosInsideBoard(x, y)) 
			{
				drawBall(Board.boardPos[x][y].x, Board.boardPos[x][y].y, Board.boardPos[x][y].z, Board.boardPos[x][y].owner, mode ,x ,y);
			}

		}
	}
}


void drawLostBalls()
{
	for (int b = 0; b < MaxBallsOut; b++) 
	{	
		if (Board.lostBalls[b].owner != None)
		{
		glPushMatrix();
		if (Board.lostBalls[b].owner == Player1) 
		{
			glDisable(GL_LIGHT1);
			glDisable(GL_LIGHT0);
			glEnable(GL_LIGHT2);
		}
		else if (Board.lostBalls[b].owner == Player2)
		{
			glDisable(GL_LIGHT2);
			glEnable(GL_LIGHT0);
			glEnable(GL_LIGHT1);
		}
		
	
	glTranslated(Board.lostBalls[b].x,Board.lostBalls[b].y,Board.lostBalls[b].z);
	glutSolidSphere(BallRadius, 100.0, 100.0);
	
	glPopMatrix();
		}
	}

}


void initBoard(void) 
{
	BoardModel = glmReadOBJ(ModelsPath BoardModelFile);
	glmUnitize(BoardModel);
	BoardObj = glmList(BoardModel, GLM_SMOOTH);
   //obj = glmList(model, GLM_SMOOTH | GLM_MATERIAL);

}


void drawBitmapString (char *str, void *font, GLclampf r, GLclampf g, GLclampf b, GLfloat x, GLfloat y)  

{
  unsigned int i;		/* Laufvariable */

  GLint matrixMode;		/* Zwischenspeicher akt. Matrixmode */

  glDisable(GL_LIGHTING);

  /* aktuellen Matrixmode speichern */

  glGetIntegerv (GL_MATRIX_MODE, &matrixMode);

  glMatrixMode (GL_PROJECTION);

  /* aktuelle Projektionsmatrix sichern */

  glPushMatrix ();

  /* neue orthogonale 2D-Projektionsmatrix erzeugen */

  glLoadIdentity ();

  gluOrtho2D (0.0, 1.0, 0.0, 1.0);

  glMatrixMode (GL_MODELVIEW);

  /* aktuelle ModelView-Matrix sichern */

  glPushMatrix ();

  /* neue ModelView-Matrix zuruecksetzen */

  glLoadIdentity ();

  /* aktuelle Zeichenfarbe (u.a. Werte) sichern */

  glPushAttrib (GL_CURRENT_BIT);

  /* neue Zeichenfarbe einstellen */

  glColor3f (r, g, b);

  /* an uebergebenene Stelle springen */

  glRasterPos3f (x, y, 0.0);

  /* Zeichenfolge zeichenweise zeichnen */

  for (i = 0; i < strlen (str); ++i)
  {
      glutBitmapCharacter (font, str[i]);
  }

  /* alte Zeichenfarbe laden */

  glPopAttrib ();

  /* alte ModelView-Matrix laden */

  glPopMatrix ();

  glMatrixMode (GL_PROJECTION);

  /* alte Projektionsmatrix laden */

  glPopMatrix ();

  /* alten Matrixmode laden */

  glMatrixMode (matrixMode);

  glEnable(GL_LIGHTING);
}


void drawSkybox(float x, float y, float z, float width, float height, float length)
{
	// Center the Skybox around the given x,y,z position
	x = x - width  / 2;
	y = y - height / 2;
	z = z - length / 2;

	glEnable(GL_TEXTURE_2D);

	// Draw Front side
	//glBindTexture(GL_TEXTURE_2D, Textures[SkyboxFront]);
	glCallList(TextureLists[SkyboxFront]);
	glBegin(GL_QUADS);		
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x,		  y+height,	z);	
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x,		  y+height,	z+length); 
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x,		  y,		z+length);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x,		  y,		z);		
	glEnd();


	// Draw Right side
	//glBindTexture(GL_TEXTURE_2D, Textures[SkyboxRight]);
	glCallList(TextureLists[SkyboxRight]);
	glBegin(GL_QUADS);		
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x+width, y,		z);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x+width, y+height, z); 
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x,		  y+height,	z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x,		  y,		z);
	glEnd();


	// Draw Back side
	//glBindTexture(GL_TEXTURE_2D, Textures[SkyboxBack]);
	glCallList(TextureLists[SkyboxBack]);
	glBegin(GL_QUADS);		
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y,		z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x+width, y,		z+length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x+width, y+height,	z+length); 
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y+height,	z);
	glEnd();

	// Draw Left side
	//glBindTexture(GL_TEXTURE_2D, Textures[SkyboxLeft]);
	glCallList(TextureLists[SkyboxLeft]);
	glBegin(GL_QUADS);	
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x,		  y,		z+length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x,		  y+height, z+length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y+height, z+length); 
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y,		z+length);
	glEnd();

	// Draw Up side
	//glBindTexture(GL_TEXTURE_2D, Textures[SkyboxUp]);
	glCallList(TextureLists[SkyboxUp]);
	glBegin(GL_QUADS);		
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y+height, z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x+width, y+height, z+length); 
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x,		  y+height,	z+length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x,		  y+height,	z);
	glEnd();

	// Draw Down side
	//glBindTexture(GL_TEXTURE_2D, Textures[SkyboxDown]);
	glCallList(TextureLists[SkyboxDown]);
	glBegin(GL_QUADS);		
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x,		  y,		z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x,		  y,		z+length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x+width, y,		z+length); 
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y,		z);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}



void loadTexture(string filename, GLuint textureIndex, GLuint listIndex) 
{
	GLTXimage *texImg;

	texImg = gltxReadRGB(filename.c_str());

	if (!texImg) exit(1);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glBindTexture(GL_TEXTURE_2D, textureIndex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, texImg->components, texImg->width, texImg->height, 
		0, GL_RGB, GL_UNSIGNED_BYTE, texImg->data);

	glNewList (listIndex, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, textureIndex);
	glEndList();
	
	gltxDelete(texImg);
}


void initSkybox(Skyboxes skybox) 
{
	string skyboxname;

	// delete existing the 6 textures and the corresponding lists for binding them + skybox list when changing the skybox
	if (Textures[SkyboxFront] > 0) {
		glDeleteTextures(6, Textures);
		glDeleteLists(TextureLists[SkyboxFront], 7);
	}

	glGenTextures(6, Textures);

	TextureLists[SkyboxFront] = glGenLists(7);
	TextureLists[SkyboxRight] = TextureLists[SkyboxFront] + 1;
	TextureLists[SkyboxBack] = TextureLists[SkyboxFront] + 2;
	TextureLists[SkyboxLeft] = TextureLists[SkyboxFront] + 3;
	TextureLists[SkyboxUp] = TextureLists[SkyboxFront] + 4;
	TextureLists[SkyboxDown] = TextureLists[SkyboxFront] + 5;

	switch(skybox)
	{
		case Morning:
			skyboxname = "morning";
			break;
		case Lagoon:
			skyboxname = "lagoon";
			break;
		case Islands:
			skyboxname = "islands";
			break;
		case Alps:
			skyboxname = "alps";
			break;
		default:
			skyboxname = "sahara";
			break;
	}

	loadTexture(TexturesPath + skyboxname + "_north.rgb", Textures[SkyboxFront], TextureLists[SkyboxFront]);
	loadTexture(TexturesPath + skyboxname + "_east.rgb", Textures[SkyboxRight], TextureLists[SkyboxRight]);
	loadTexture(TexturesPath + skyboxname + "_south.rgb", Textures[SkyboxBack], TextureLists[SkyboxBack]);
	loadTexture(TexturesPath + skyboxname + "_west.rgb", Textures[SkyboxLeft], TextureLists[SkyboxLeft]);
	loadTexture(TexturesPath + skyboxname + "_up.rgb", Textures[SkyboxUp], TextureLists[SkyboxUp]);
	loadTexture(TexturesPath + skyboxname + "_down.rgb", Textures[SkyboxDown], TextureLists[SkyboxDown]);

	SkyboxObj = TextureLists[SkyboxFront] + 6;
	glNewList(SkyboxObj, GL_COMPILE);
		glDisable(GL_LIGHTING);
		drawSkybox(0, 0, 0, 256, 256, 256);
		glEnable(GL_LIGHTING);
	glEndList();
}



void initScene(void) 
{
	cout << ".";

	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 30.0 };
	GLfloat pos0[] = { 1.0, 1.0, 1.0, 0.0 };
	GLfloat col0[] = { 0.5, 0.5, 0.5, 0.0 };
	GLfloat pos1[] = { 1.0, 1.0, 1.0, 0.0 };
	GLfloat col1[] = { 1.0, 1.0, 1.0, 0.0 };
	GLfloat pos2[] = { 1.0, 1.0, 1.0, 0.0 };
	GLfloat col2[] = { 0.0, 0.0, 0.0, 0.0 };
	GLfloat pos3[] = { 1.0, 1.0, 1.0, 0.0 };
	GLfloat col3[] = { 1.0, 0.0, 0.0, 0.0 };

	glClearColor (0.0, 0.0, 1.0, 0.0);
	glShadeModel (GL_SMOOTH);
  
	initBoard();

	cout << ".";

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, col1);
	glLightfv(GL_LIGHT0, GL_POSITION, pos0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, col0);
	glLightfv(GL_LIGHT1, GL_POSITION, pos1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, mat_specular);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, col1);
	glLightfv(GL_LIGHT2, GL_POSITION, pos2);
	glLightfv(GL_LIGHT2, GL_SPECULAR, mat_specular);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, col2);
	glLightfv(GL_LIGHT3, GL_POSITION, pos3);
	glLightfv(GL_LIGHT3, GL_SPECULAR, mat_specular);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, col3);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glEnable(GL_DEPTH_TEST);

	glClearColor (0.0, 0.0, 1.0, 0.0);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);

   	initSkybox(Sahara);

	cout << ".";
}

void draw(GLenum mode)
{
	//glEnable(GL_MULTISAMPLE_ARB);
	//glEnable(GL_MULTISAMPLE); 

	int w = glutGet(GLUT_WINDOW_WIDTH), h = glutGet(GLUT_WINDOW_HEIGHT);

	glFrustum(-1.0 * ((float) w / h), 1.0 * ((float) w / h), -1.0, 1.0, 1.5, 300.0);
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(KameraPosX, KameraPosY,  KameraPosZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	glPushMatrix();

	if (mode == GL_RENDER)
	{
		glCallList(SkyboxObj);

		glDisable(GL_LIGHT1);
   		glPushMatrix();
		glScalef(2,2,2);
		glTranslated(0,0,0);
		glCallList(BoardObj);
		glPopMatrix();
		drawLostBalls();
		glEnable(GL_LIGHT1);
					
		if (GameRunning)
		{
			if (Board.getCurrentPlayer() == Player1)
				drawBitmapString ("Schwarz ist am Zug", GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f, 0.05f, 0.95f);		
			else 
				drawBitmapString ("Weiss ist am Zug", GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f, 0.05f, 0.95f);
		}
		else
		{	
			if (Board.getWinner() == Player1)
				drawBitmapString ("Schwarz hat gewonnen!", GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f, 0.05f, 0.95f);
			else if (Board.getWinner() == Player2)
				drawBitmapString ("Weiss hat gewonnen!", GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f, 0.05f, 0.95f);
		}

	}	

	drawAllBalls(mode);	
	glDisable(GL_LIGHT2);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glPopMatrix();

	//glDisable(GL_MULTISAMPLE); 
	//glDisable(GL_MULTISAMPLE_ARB);
}


void display(void)
{
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();								
   draw(GL_RENDER);

   glutSwapBuffers();
}


void mouseMotion(int cmx, int cmy)
{
	if (KameraPosX == 0 && KameraPosY == 3 && (KameraPosZ == 3 || KameraPosZ == -3))
		Kamerafahrt = false;
	if ((MouseButton != GLUT_LEFT_BUTTON) || (Kamerafahrt == true) || GameRunning == false) return;
	float diffx;
	float diffy;
	
	if(cmx < MausPosX)
	{
		diffx = MausPosX - cmx;
		positionx(); 
		positiony();
		positionz();
		findangle();
		rotate_y_li(diffx);		
	}
	else if(cmx > MausPosX)
	{
		diffx = cmx - MausPosX;
		positionx(); 
		positiony();
		positionz();
		findangle();
		rotate_y_re(diffx);
	}
	
	if(cmy < MausPosY)							
	{
		diffy = MausPosY - cmy;
		positionx();
		positiony();
		positionz();
		findangle();
		findangle_x();
		rotate_x_up(diffy);
	}
	else if(cmy > MausPosY)						
	{
		diffy = cmy - MausPosY;
		positionx();
		positiony();
		positionz();
		findangle();
		findangle_x();
		rotate_x_down(diffy);

	}

	MausPosX = cmx;
	MausPosY = cmy;
	
    glutPostRedisplay();
}


void processHits(GLint hits, GLuint buffer[])									
{
  int i,id=0;

  if (InReplay) return;

  GLuint *ptr=0, *pick=0;
  ptr = (GLuint *) buffer;
  pick=ptr;
  pick++;
  for (i = 0; i < hits; i++) 
  { 
    ptr++;
	if (*pick >=  *ptr)
	{
		pick=ptr;
		ptr++;
		ptr++;
		id=*ptr;
	}
	else
	{
		ptr++;	
		ptr++;

	}

	if (*ptr >= MaxBalls && *ptr <= MaxBalls+6 && Kamerafahrt == false)
	{
		moveIt(MoveDirection(*ptr-MaxBalls));
		return;
    }
	ptr++;
  }
	if (id > 0)
	{
		for (int x = 0; x < BoardSize; x++) 
		{
			for (int y = 0; y < BoardSize; y++) 
			{
				if (Board.isPosInsideBoard(x, y)) 
				{
					Board.boardPos[x][y].selected = (Board.boardPos[x][y].id == id);
				}
			}
		}
		glutPostRedisplay();
	}
}


#define BUFSIZE 512

void mouseClick(int button, int state, int mx, int my)
{
  GLuint selectBuf[BUFSIZE];
  GLint hits;
  GLint viewport[4];
  MouseButton = button;

  if (KameraPosX == 0 && KameraPosY == 3 && (KameraPosZ == 3 || KameraPosZ == -3))
		Kamerafahrt = false;
  
  if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN ||Kamerafahrt || !GameRunning)
  return;
  
	MausPosX=mx;

    MausPosY=my;

  glGetIntegerv(GL_VIEWPORT, viewport);

  glSelectBuffer(BUFSIZE, selectBuf);
  glRenderMode(GL_SELECT);

  glInitNames();
  glPushName((GLuint) ~0);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
/*  create 0.1x0.1 pixel picking region near cursor location */
  gluPickMatrix((GLdouble) mx, (GLdouble) (viewport[3] - my),
    0.1, 0.1, viewport);
  
  //glOrtho(0.0, 8.0, 0.0, 8.0, -0.1, 2.5);
  draw(GL_SELECT);
  glPopMatrix();
  glFlush();
  hits = glRenderMode(GL_RENDER);
  processHits(hits, selectBuf);
}


void reshape (int w, int h)
{
	glViewport(0, 0, w, h); 
}

void keyboard(unsigned char key, int u, int i)
{
	switch (key) {
		case 27:
			exit(0);
			break;

		case 'w':
			moveIt(dNorthWest);
			break;
		case 'e': 
			moveIt(dNorthEast);
			break;
		case 'd': 
			moveIt(dEast);
			break;
		case 'x': 
			moveIt(dSouthEast);
			break;
		case 'y': 
			moveIt(dSouthWest);
			break;
		case 'a': 
			moveIt(dWest);
			break;

		case 't': 
			if (GameRunning == true)
			{
				Kamerafahrt = false;
				if (Board.getCurrentPlayer() == Player1)
					animation(0.0f,5.0f,0.1f);
				else
					animation(0.0f,5.0f,-0.1f);
				step = 0;
				glutPostRedisplay();
			}	
			break;

		case 'i': 
			if (GameRunning == true)
			{
				Kamerafahrt = false;
				zoom_in();
				glutPostRedisplay();
			}
			break;
		case 'o':
			if (GameRunning == true)
			{
				Kamerafahrt = false;
				zoom_out();
			}
			glutPostRedisplay();
			break;

   }

}


void specialKey(int key, int x, int y)
{
	switch (key) {
		case GLUT_KEY_F1:
			showHelp();
			break;
		case GLUT_KEY_F12:
			FullScreen = !FullScreen;
			if (FullScreen)
				glutFullScreen();
			else
				glutReshapeWindow(WindowSizeX, WindowSizeY);
			glutPostRedisplay();
			break;
	}
}





int main(int argc, char** argv)
{
	showAbout();

	cout << "Spiel laedt .";

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH  ); // GLUT_MULTISAMPLE
	glutInitWindowSize(WindowSizeX, WindowSizeY); 
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Abalone 3D");

	initScene();

	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseMotion);
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);	
	glutSpecialFunc(specialKey);

	initMenu();

	cout << " fertig" << endl;

	kamerafahrt(0);

	glutMainLoop();

	return 0;
}
