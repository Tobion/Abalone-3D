#include <stdlib.h>
#include <iostream>
#include <math.h>

#include <GL/glut.h>


//#include "camera.h"
#define PI 3.1415926535897932384626433832795
#define PIdiv180 (PI/180.0)

extern GLfloat KameraPosX; //= 0.0f;
extern GLfloat KameraPosY; //= 3.0f;
extern GLfloat KameraPosZ; //= 3.0f;
extern int step;		//schrittzähler für animation
float distanz_x, distanz_y, distanz_z ;
extern float position_x, position_y, position_z;
float zoomfaktor=1.01f;
float angle=0;
float angle_x=45;
extern bool GameRunning;
extern bool Kamerafahrt ;
extern bool MenuInUse;


void zoom_out ()
{
	float radius=sqrt(KameraPosY*KameraPosY+KameraPosZ*KameraPosZ+KameraPosX*KameraPosX);
	if (radius < 12.5f) 
	{
		KameraPosX=KameraPosX*zoomfaktor;
		KameraPosY=KameraPosY*zoomfaktor;
		KameraPosZ=KameraPosZ*zoomfaktor;
	}
}

void zoom_in ()
{
	float radius=sqrt(KameraPosY*KameraPosY+KameraPosZ*KameraPosZ+KameraPosX*KameraPosX);
	if (radius > 3.5f) 
	{
		KameraPosX=KameraPosX/zoomfaktor;
		KameraPosY=KameraPosY/zoomfaktor;
		KameraPosZ=KameraPosZ/zoomfaktor;
	}
}

void zoom_fahrt(int value)
{
	if(value==0)
	{
		if (KameraPosZ>1.0f) {
			KameraPosX=KameraPosX/zoomfaktor;
			KameraPosY=KameraPosY/zoomfaktor;
			KameraPosZ=KameraPosZ/zoomfaktor;
			glutTimerFunc(20, &zoom_fahrt,0);
			glutPostRedisplay();
		}
	}
	if(value==1)
	{
		if (KameraPosZ<10.0f) {
			KameraPosX=KameraPosX*zoomfaktor;
			KameraPosY=KameraPosY*zoomfaktor;
			KameraPosZ=KameraPosZ*zoomfaktor;
			glutTimerFunc(20, &zoom_fahrt,1);
			glutPostRedisplay();
		}
	}
}

float distanzx(float x)
{
	distanz_x = x - KameraPosX ;
	return distanz_x ;
}

float distanzy(float x)
{
	distanz_y = x - KameraPosY ;
	return distanz_y ;
}

float distanzz(float x)
{
	distanz_z = x - KameraPosZ ;
	return distanz_z ;
}

float positionx(void)
{
	position_x = KameraPosX ;
	return position_x ;
}

float positiony(void)
{
	position_y = KameraPosY ;
	return position_y ;
}

float positionz(void)
{
	position_z = KameraPosZ ;
	return position_z ;
}

int findangle()
{

	angle=atan(position_x/position_z)/PIdiv180;	
	return angle ;
}


int findangle_x()
{

	angle_x=asin(position_y/sqrt(position_z*position_z+position_x*position_x+position_y*position_y))/PIdiv180;
	return angle_x ;
}

float findangle_360()
{
	if(position_x>=0.0f && position_z>0.0f)	//1.quadrant
	{
	angle=atan(position_x/position_z)/PIdiv180;
	return angle ;}

	else if(position_x>=0.0f && position_z<0.0f)	//2.quadrant
	{
	angle=180+atan(position_x/position_z)/PIdiv180;
	return angle ;}

	else if(position_x<0.0f && position_z<0.0f)	//3.quadrant
	{
	angle=180+atan(position_x/position_z)/PIdiv180;
	return angle ;}

	else if(position_x<0.0f && position_z>0.0f)	//4.quadrant
	{
		if(angle==0) {return angle;}
	angle=360+atan(position_x/position_z)/PIdiv180;
	return angle ;}
}
void delay(int value)			
{
	glutPostRedisplay();
}

void anim(int value)			
{
	if(step<=20){
	KameraPosX = position_x + distanz_x * step/20;
	KameraPosY = position_y + distanz_y * step/20;
	KameraPosZ = position_z + distanz_z * step/20;
	glutTimerFunc(10, &anim,0);
	glutPostRedisplay();
	step++;	
	}
}

void animation(float zu_x,float zu_y,float zu_z)	// kameraflug von der momentanen position
{													// zur übergebenen position
	distanzx(zu_x);									// taste y
	distanzy(zu_y);
	distanzz(zu_z);
	positionx();
	positiony();
	positionz();
	glutTimerFunc(10, &anim,0);
}

void animation_twopoints(float von_x,float von_y,float von_z, float zu_x,float zu_y,float zu_z)
{													
	distanz_x= zu_x - von_x	;							
	distanz_y= zu_y - von_y ;
	distanz_z= zu_z - von_z ;
	position_x= von_x ;
	position_y= von_y ;
	position_z= von_z ;
	angle=atan(position_x/position_z);
	glutTimerFunc(100, &anim,0);
}

void rotate_y_li(float diffx)
{
	if(position_z<0){angle=angle+180;}
	angle=angle-(1*diffx/2);
	if(angle>360 || angle<-360){angle=fmod( angle, 360 );}
	KameraPosX=sqrt(position_z*position_z+position_x*position_x)*sin(angle*PIdiv180);
	KameraPosY=position_y;
	KameraPosZ=sqrt(position_z*position_z+position_x*position_x)*cos(angle*PIdiv180);
}
void rotate_y_re(float diffx)
{
	if(position_z<0){angle=angle+180;}
	angle=angle+(1*diffx/2);
	if(angle>360 || angle<-360){angle=fmod( angle, 360 );}
	KameraPosX=sqrt(position_z*position_z+position_x*position_x)*sin(angle*PIdiv180);
	KameraPosY=position_y;
	KameraPosZ=sqrt(position_z*position_z+position_x*position_x)*cos(angle*PIdiv180);
}

void rotate_x_up(float diffy)			//drehung um x-achse nach oben bis zur spitze
{
	float radius=sqrt(position_y*position_y+position_z*position_z+position_x*position_x);
	if(position_z<=0){angle=angle+180;}
	angle_x=angle_x+(1*diffy/2);
	if(angle_x<89){

	KameraPosY=radius*sin(angle_x*PIdiv180);
	float dist_xz=KameraPosY/(tan(angle_x*PIdiv180));
	KameraPosX=dist_xz*sin(angle*PIdiv180);
	KameraPosZ=dist_xz*cos(angle*PIdiv180);
	}
	
}
void rotate_x_down(float diffy)			//drehung um x-achse nach unten
{

	float radius=sqrt(position_y*position_y+position_z*position_z+position_x*position_x);
	if(position_z<=0){angle=angle+180;}
	angle_x=angle_x-(1*diffy/2);
	if(angle_x>0){
	
	KameraPosY=radius*sin(angle_x*PIdiv180);
	float dist_xz=KameraPosY/(tan(angle_x*PIdiv180));
	KameraPosX=dist_xz*sin(angle*PIdiv180);
	KameraPosZ=dist_xz*cos(angle*PIdiv180);
	}
}

void kamerafahrt(int value)
{
	if (value==0){
		if (GameRunning){return;}
		if(MenuInUse==false)
		{
			angle=angle+1;
			if(angle>360 || angle<-360){angle=fmod( angle, 360 );}
			KameraPosX=3.0f*sin(angle*PIdiv180);
			KameraPosY=1.7f;
			KameraPosZ=3.0f*cos(angle*PIdiv180);
		}
		glutTimerFunc(35, &kamerafahrt,0);
		glutPostRedisplay();

	}
	if (value==1){
		if (angle<180) 
		{
			angle=angle+1;
			KameraPosX=3.0f*sin(angle*PIdiv180);
			KameraPosY=3.0f;
			KameraPosZ=3.0f*cos(angle*PIdiv180);
			glutTimerFunc(35, &kamerafahrt,1);
			glutPostRedisplay();
		}
	}
	if (value==2){
		if (angle>0) 
		{
			angle=angle-1;
			KameraPosX=3.0f*sin(angle*PIdiv180);
			KameraPosY=3.0f;
			KameraPosZ=3.0f*cos(angle*PIdiv180);	
			glutTimerFunc(35, &kamerafahrt,2);
			glutPostRedisplay();
		}
	}
}

void kamerafahrt_zuplayer(int value)		
{

	if (value==1){	//taste o
		if(angle>360 || angle<0){angle=fmod( angle,360 );}
		if(position_x>=0.0f && position_z>=0.0f)	//1.quadrant
			{
				angle=angle+3;
				if(angle<=180) 
				{
				KameraPosX=sqrt(position_z*position_z+position_x*position_x)*sin(angle*PIdiv180);
				KameraPosY=position_y;
				KameraPosZ=sqrt(position_z*position_z+position_x*position_x)*cos(angle*PIdiv180);
				glutTimerFunc(10, &kamerafahrt_zuplayer,1);	
				glutPostRedisplay();
				}
				else 
				{positionx();
				positiony();
				positionz();
				findangle();
				animation(0.0f,3.0f,-3.0f);	// kameraflug von der momentanen position
				step=0;						// zur ausgangsposition
				glutPostRedisplay();
				}
				return;
			}
		
		if(position_x>0.0f && position_z<0.0f)	//2.quadrant
			{
				angle=angle+3;
				if(angle<=180) 
				{
				KameraPosX=sqrt(position_z*position_z+position_x*position_x)*sin(angle*PIdiv180);
				KameraPosY=position_y;
				KameraPosZ=sqrt(position_z*position_z+position_x*position_x)*cos(angle*PIdiv180);
				glutTimerFunc(10, &kamerafahrt_zuplayer,1);	
				glutPostRedisplay();
				}
				else 
				{positionx();
				positiony();
				positionz();
				findangle();
				animation(0.0f,3.0f,-3.0f);	// kameraflug von der momentanen position
				step=0;						// zur ausgangsposition
				glutPostRedisplay();
				}
				return;
			}	
		if(position_x<0.0f && position_z<0.0f)	//3.quadrant
			{
				angle=angle-3;
				if(angle>=180) 
				{
				KameraPosX=sqrt(position_z*position_z+position_x*position_x)*sin(angle*PIdiv180);
				KameraPosY=position_y;
				KameraPosZ=sqrt(position_z*position_z+position_x*position_x)*cos(angle*PIdiv180);
				glutTimerFunc(10, &kamerafahrt_zuplayer,1);	
				glutPostRedisplay();
				}
				else 
				{positionx();
				positiony();
				positionz();
				findangle();
				animation(0.0f,3.0f,-3.0f);	// kameraflug von der momentanen position
				step=0;						// zur ausgangsposition
				glutPostRedisplay();
				}
				return;
			}	
		if(position_x<0.0f && position_z>0.0f)	//4.quadrant
			{
				angle=angle-3;
				if(angle>=180) 
				{
				KameraPosX=sqrt(position_z*position_z+position_x*position_x)*sin(angle*PIdiv180);
				KameraPosY=position_y;
				KameraPosZ=sqrt(position_z*position_z+position_x*position_x)*cos(angle*PIdiv180);
				glutTimerFunc(10, &kamerafahrt_zuplayer,1);	
				glutPostRedisplay();
				}
				else 
				{positionx();
				positiony();
				positionz();
				findangle();
				animation(0.0f,3.0f,-3.0f);	// kameraflug von der momentanen position
				step=0;						// zur ausgangsposition
				glutPostRedisplay();
				}
				return;
			}
	}
			
	if (value==2){	
		if(angle>360 || angle<0){angle=fmod( angle,360 );}
		if(position_x>=0.0f && position_z>=0.0f)	//1.quadrant
			{
				angle=angle-3;
				if(angle>=0) 
				{
				KameraPosX=sqrt(position_z*position_z+position_x*position_x)*sin(angle*PIdiv180);
				KameraPosY=position_y;
				KameraPosZ=sqrt(position_z*position_z+position_x*position_x)*cos(angle*PIdiv180);
				glutTimerFunc(10, &kamerafahrt_zuplayer,2);	
				glutPostRedisplay();
				}
				else 
				{positionx();
				positiony();
				positionz();
				findangle();
				animation(0.0f,3.0f,3.0f);	// kameraflug von der momentanen position
				step=0;						// zur ausgangsposition
				glutPostRedisplay();
				}
				return;
			}
		
		if(position_x>=0.0f && position_z<0.0f)	//2.quadrant
			{
				angle=angle-3;
				if(angle>=0) 
				{
				KameraPosX=sqrt(position_z*position_z+position_x*position_x)*sin(angle*PIdiv180);
				KameraPosY=position_y;
				KameraPosZ=sqrt(position_z*position_z+position_x*position_x)*cos(angle*PIdiv180);
				glutTimerFunc(10, &kamerafahrt_zuplayer,2);	
				glutPostRedisplay();
				}
				else 
				{positionx();
				positiony();
				positionz();
				findangle();
				animation(0.0f,3.0f,3.0f);	// kameraflug von der momentanen position
				step=0;						// zur ausgangsposition
				glutPostRedisplay();
				}
				return;
			}	
		if(position_x<0.0f && position_z<0.0f)	//3.quadrant
			{
				angle=angle+3;
				if(angle<=360) 
				{
				KameraPosX=sqrt(position_z*position_z+position_x*position_x)*sin(angle*PIdiv180);
				KameraPosY=position_y;
				KameraPosZ=sqrt(position_z*position_z+position_x*position_x)*cos(angle*PIdiv180);
				glutTimerFunc(10, &kamerafahrt_zuplayer,2);	
				glutPostRedisplay();
				}
				else 
				{positionx();
				positiony();
				positionz();
				findangle();
				animation(0.0f,3.0f,3.0f);	// kameraflug von der momentanen position
				step=0;						// zur ausgangsposition
				glutPostRedisplay();
				}
				return;
			}	
		if(position_x<0.0f && position_z>0.0f)	//4.quadrant
			{
				angle=angle+3;
				if(angle<=0) 
				{
				KameraPosX=sqrt(position_z*position_z+position_x*position_x)*sin(angle*PIdiv180);
				KameraPosY=position_y;
				KameraPosZ=sqrt(position_z*position_z+position_x*position_x)*cos(angle*PIdiv180);
				glutTimerFunc(10, &kamerafahrt_zuplayer,1);	
				glutPostRedisplay();
				}
				else 
				{positionx();
				positiony();
				positionz();
				findangle();
				animation(0.0f,3.0f,3.0f);	// kameraflug von der momentanen position
				step=0;						// zur ausgangsposition
				glutPostRedisplay();
				}
				return;
			}
	}
}


void kamerafahrt_endl(int value)
{
	if (value==0){
		angle=angle+1;
		if(angle>360 || angle<-360){angle=fmod( angle, 360 );}
		KameraPosX=sqrt(position_z*position_z+position_x*position_x)*sin(angle*PIdiv180);
		KameraPosY=position_y;
		KameraPosZ=sqrt(position_z*position_z+position_x*position_x)*cos(angle*PIdiv180);
		glutTimerFunc(35, &kamerafahrt_endl,0);
		glutPostRedisplay();
	}
	if (value==1){
		if (angle<180) 
		{
			angle=angle+1;
			if(angle>360 || angle<-360){angle=fmod( angle, 360 );}
			KameraPosX=3.0f*sin(angle*PIdiv180);
			KameraPosY=3.0f;
			KameraPosZ=3.0f*cos(angle*PIdiv180);
			glutTimerFunc(15, &kamerafahrt,1);
			glutPostRedisplay();
		}
	}
	if (value==2){
		if (angle>0) 
		{
			angle=angle-1;
			if(angle>360 || angle<-360){angle=fmod( angle, 360 );}
			KameraPosX=3.0f*sin(angle*PIdiv180);
			KameraPosY=3.0f;
			KameraPosZ=3.0f*cos(angle*PIdiv180);	
			glutTimerFunc(15, &kamerafahrt,2);
			glutPostRedisplay();
		}
	}
}
