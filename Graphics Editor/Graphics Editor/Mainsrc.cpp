#include<stdio.h>
#include<math.h>
#include"..\Graphics Editor\include\GL\glut.h"
#include<iostream>
#include<stdlib.h>
#include <ctime>
#include <stack>
#include<vector>
#include <algorithm>
#define MAX_CHULL_POINTS 10
#define RAND_RANGE 5
#define BRUSH_DEFAULT_PIXEL 5
#define CCW_LEFT_TURN 1
#define CCW_RIGHT_TURN -1
#define CCW_COLINEAR 0

using std::stack;
#define Max_images 100;
bool Firstrun = true;
int selectedMenuItem = -1;
enum menuoptions{LINE , SQUARE , CIRCLE, TRIANGLE, BIEZER,ELLIPSE,MENU_END};
int xbegin = 0, xend = 1023, ybegin = 0, yend = 700;// Window size
enum colorplattel{ black = MENU_END, white, red, green, blue, yellow, cyan, magenta, COLOR_END };
enum tools{ PENCIL = COLOR_END, ERASER, BRUSH,COLORFILL, SPRAY,CHULL,TOOL_END };
enum sizeofpixel{ size1 = TOOL_END , size3, size5, size10,SIZE_END};
enum menuitems{CLEAR = SIZE_END , EXIT , ITEM_END};
int colorsel = black;
int toolsel = PENCIL;
int shapesel = MENU_END;
int pixelsize[] = { 1, 10, 15, 20 }; //default pixel size
int shapepoint[3][2]={-1,};
int controlpoints[4][2] ={-1,};
int numcontrolpoints = 0;
std::pair<int,int> p0 ; // first vertex for graham scan
std::vector< std::pair<int,int> > chullpoints(MAX_CHULL_POINTS);
int numCHselectedpoints = 0;
//std::vector< std::vector< bool > > visited(2*xend, std::vector<bool>(2*yend, 0));
struct Color {
	GLfloat r;
	GLfloat g;
	GLfloat b;
};

typedef struct{
	float x;
	float y;
	float xo;
	float yo;
}Points;
Points points;
int prevx;
int prevy;
int selpixelsz = 0;
bool mousepressed = false;
float colormat[20][3] = { { 0, 0, 0 }, { 1, 1, 1 }, { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 1, 1, 0 }, { 0, 1, 1 }, { 1, 0, 1 } };
void log(){}
void setselectedcolor(void);
void resetglobalparam(void);

template<typename First, typename ...Rest>
void log(First & first, Rest & ...rest)
{
   // std::cout << std::forward<First>(first);
	std::cout << first << " ";
    //log(std::forward<Rest>(rest)...);
    log(rest...);
	 std::cout << std::endl;
}
void clearcanvas(void)
{
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	glVertex2f(xbegin, ybegin);
	glVertex2f(xend, ybegin);
	glVertex2f(xend, yend-102);
	glVertex2f(xbegin, yend -102);
	glEnd();
	resetglobalparam();
	glFlush();

}
void HandleselectedMenu(int selected)
{
   switch(selected)
   	{
   	case CLEAR:
		clearcanvas();
		break;
	case EXIT:
	    exit(0);
		break;
	default:
		break;
   	}
}
void getcolor(float* r, float* g, float* b, int color){
	color = color - black;
	*r = colormat[color][0];
	*g = colormat[color][1];
	*b = colormat[color][2];
}
char * returnmenustring(int choice){
	switch (choice){
	//case GRAPEDIT:
		//return " ";// GRAPHICS EDITOR COP - 5705";
	case LINE:
		return "LINE";
	case SQUARE:
		return "SQUARE";
	case CIRCLE:
		return "CIRCLE";
	case TRIANGLE:
		return "TRIANGLE";
	case BIEZER:
		 return " S";
	case ELLIPSE:
		 return "EP";
	case CHULL:
		return"CH";
	}

}
void selectMenuColor(int y){
	switch (y){
	//case GRAPEDIT:
		//glColor3f(0.0, 0.2, 0.2);
		//break;
	default:
		glColor3f(0, 0, 0);
		break;
	}
}
void displaytext(int textposx, int textposy,void* font, int type){
	char* str = returnmenustring(type);
	glColor3f(0, 0, 0);
	glRasterPos2f(textposx, textposy); //Specifies the raster position for pixel operations

	while (*str != '\0'){
		glutBitmapCharacter(font, *str);
		str++;
	}
		
}
void renderpixel (GLfloat x, GLfloat y)
{
	glBegin(GL_POINTS);
	glVertex2f(x, y);
	glEnd();

}
void drawpixel(int x0, int y0, int x, int y){
	renderpixel(x + x0, y + y0);
	renderpixel(-x + x0, y + y0);
	renderpixel(x + x0, -y + y0);
	renderpixel(-x + x0, -y + y0);
	renderpixel(y + x0, x + y0);
	renderpixel(-y + x0, x + y0);
	renderpixel(y + x0, -x + y0);
	renderpixel(-y + x0, -x + y0);
}
void draw_circle(float x0, float y0, int radius , bool flagcanvas){
	int d = 1 - radius, x = 0, y = radius;
	if (flagcanvas)
		glLineWidth(pixelsize[selpixelsz]);
	while (y>x)
	{ 

	if(flagcanvas)// clip if overlapping menu icon
		{ 
		if((y0+radius)<yend-100) 
		drawpixel(x0, y0, x, y);
		}
	else
		drawpixel(x0, y0, x, y);
		if (d<0) d += 2 * x + 3;
		else
		{
			d += 2 * (x - y) + 5;
			--y;
		}
		++x;
	}
	if(flagcanvas)// clip if overlapping menu icon
		{ 
		if((y0+radius)<yend-100) 
		drawpixel(x0, y0, x, y);
		}
	else
		drawpixel(x0, y0, x, y);

}
void draw_circle1(float x0, float y0, int radius , float fragment){	
	log("draw_circle1" , "selpixelsz",selpixelsz,"");
	glLineWidth(pixelsize[selpixelsz]);
	glBegin(GL_LINE_LOOP);
	setselectedcolor();
	for(int i = 0; i<fragment ;++i){
		float angle = 2.0f * 3.1415926f* i/fragment;
		float xcod = radius * cosf(angle);
		float ycod = radius * sinf(angle);
		if (y0 + ycod < yend - 100)
			glVertex2f(x0 + xcod, y0 + ycod);
		else
			continue;
		}
	glEnd();
}
void plotellipsepoints (float xc, float yc, float x, float y)
{
    if(yc+y <yend -100){
    renderpixel (xc + x, yc + y);
    renderpixel (xc - x, yc + y);
    }
    renderpixel (xc + x, yc - y);
    renderpixel (xc - x, yc - y);
}

void draw_ellipse(int xc, int yc, int a, int b){ // using midpoint algorithm
  int asq = a*a;
  int bsq = b*b;
  int a2 = 2*asq;
  int b2 = 2*bsq;
  int temp = 0;
  int x= 0, y= b;
  int tempx = 0, tempy = a2*y;
  setselectedcolor();
  plotellipsepoints(xc,yc,x,y);
  log("draw_ellipse", xc, yc ,a ,b,"");
  temp = round(bsq - (asq*b)+(0.25 *asq));
  while(tempx <tempy){
  	x++;
	tempx+=b2;
	if(temp< 0)
		temp+= bsq+tempx;
  else{
  	y--;
	tempy -= a2;
	temp+=bsq+tempx -tempy;
  	}
  plotellipsepoints(xc,yc,x,y);
}
  temp = round(bsq*(x+0.5)*(x+0.5) + asq *(y-1)*(y-1)
 - asq*bsq);

   while(y>0){
  	y--;
	tempy-=a2;
	if(temp > 0)
		temp+= asq - tempy;
	else{
		x++;
		tempx+= b2;
		temp += asq - tempy +tempx;
		}
  plotellipsepoints(xc,yc,x,y);
}
}
void toolbarimage(int x1, int y1, int x2,int y2,int textposx, int textposy, void* font, int type){
	glColor3f(0, 0, 0);
	switch(type){
			case LINE:
			glBegin(GL_LINES);
			glVertex2f(x1 + 2, y2 - 2);
			glVertex2f(x2 - 2, y1 + 2);
			glEnd();
			break;
		case SQUARE:
			glBegin(GL_LINE_LOOP);
			glVertex2f(x1+3,y1+3);
			glVertex2f(x1 + 3, y2 - 3);
			glVertex2f(x2 - 3, y2 - 3);
			glVertex2f(x2 - 3, y1 + 3);
			glEnd();
			break;
		case CIRCLE:
			draw_circle(x1+8,(y1+y2)/2,5,false);// using circle midpoint algorithm
			break;
		case TRIANGLE:
			glBegin(GL_LINE_LOOP);
			glVertex2f(x1 + 3, y1 + 3);
			glVertex2f((x1 + x2)/2, y2-2);
			glVertex2f(x2 - 3, y1+3);
			glEnd();
			  break;
		case PENCIL:
			glBegin(GL_LINE_LOOP);
			glVertex2f(x1 + 2, (y1 + y2)/2);
			glVertex2f( x1+7, y2-7);
			glVertex2f(x2 - 3, y2- 7);
			glVertex2f(x2 - 3, y1 + 7);
			glVertex2f(x1 + 7, y1 + 7);
			glEnd();
			break;
		case ERASER:
			glColor3f(1, 1, 1);
			glBegin(GL_POLYGON);
			glVertex2f(x1 + 5, y1 + 5);
			glVertex2f(x1 + 5, y2 - 5);
			glVertex2f(x2 - 5, y2 - 5);
			glVertex2f(x2 - 5, y1 + 5);
			glEnd();
			break;
		case BRUSH:
			glColor3f(0,0,0);
			glBegin(GL_LINE_LOOP);
			glVertex2f(x1 + 3, y1 + 9);
			glVertex2f(x1 + 3, y2 - 9);
			glVertex2f(x2 - 8, y2 - 9);
			glVertex2f(x2 - 8, y1 + 9);
			glEnd();
			glBegin(GL_POLYGON); // draw Brush sticks
			glVertex2f(x2 - 8, y2 - 8);
			glVertex2f(x2 - 4, y2 - 6);
			glVertex2f(x2 - 3, y2 - 8);
			glVertex2f(x2 - 8, y2 - 10);
			glEnd();
			glBegin(GL_POLYGON);
			glVertex2f(x2 - 8, (y2+y1)/2 +1);
			glVertex2f(x2 - 2, (y2 + y1) / 2 +1);
			glVertex2f(x2 - 2, (y2 + y1) / 2 -1);
			glVertex2f(x2 - 8, (y2 + y1) / 2 - 1);
			glEnd();
			glBegin(GL_POLYGON);
			glVertex2f(x2 - 8, y1+9);
			glVertex2f(x2 - 4, y1+5);
			glVertex2f(x2 - 3, y1+7);
			glVertex2f(x2 - 8, y1+11);
			glEnd();
			break;
		case COLORFILL:
			glColor3f(0, 0, 0);
			glBegin(GL_LINE_LOOP);
			glVertex2f(x1 +8, y1 + 10);
			glVertex2f(x2 - 6, y1+ 6);
			glVertex2f(x2 - 3,y2 -10 );
			glVertex2f(x1+12, y2-6);
			glEnd();
			glColor3f(1, .75, 0);
			glBegin(GL_TRIANGLES);
			glVertex2f(x1 + 8, y1 + 12);
			glVertex2f(x1 + 8, y1 + 4);
			glVertex2f(x1 + 4, y1 + 5);
			glEnd();
			break;
		case SPRAY:
			draw_circle1((x1+x2)/2,(y1+y2)/2,2,200);
	  for(int r =0 ;r< 5;++r){
		for(int i = 0; i<10 ;++i){
		float angle = 2.0f * 3.1415926f* i/10;
		float xcod = r * cosf(angle);
		float ycod = r * sinf(angle);
		glBegin(GL_POINTS);
			glVertex2f((x1+x2)/2 + xcod, (y1+y2) /2+ ycod);
	    glEnd();
		}
	  	}
			break;
		 	
		case size1:
			glColor3f(0, 0, 0);
			glBegin(GL_POLYGON);
			glVertex2f(x1,y1);
			glVertex2f(x1, y2);
			glVertex2f(x2, y2);
			glVertex2f(x2, y1);
			glEnd();
			break;
		case size3:
			glColor3f(0, 0, 0);
			glBegin(GL_POLYGON);
			glVertex2f(x1, y1);
			glVertex2f(x1, y2);
			glVertex2f(x2, y2);
			glVertex2f(x2, y1);
			glEnd();
			break;
		case size5:
			glColor3f(0, 0, 0);
			glBegin(GL_POLYGON);
			glVertex2f(x1, y1);
			glVertex2f(x1, y2);
			glVertex2f(x2, y2);
			glVertex2f(x2, y1);
			glEnd();
			break;
		case size10:
			glColor3f(0, 0, 0);
			glBegin(GL_POLYGON);
			glVertex2f(x1, y1);
			glVertex2f(x1, y2);
			glVertex2f(x2, y2);
			glVertex2f(x2, y1);
			glEnd();
			break;
	default:
	displaytext(textposx , textposy,font, type);
		break;
		}
	
	
}

void createbutton(int x1, int y1, int x2, int y2, int textposx, int textposy, void* font, int type){
	
	selectMenuColor(type);
	float r = 0, g = 0, b = 0;
	glLineWidth(1);
	//if (type == GRAPEDIT)
	//	glBegin(GL_POLYGON);
	 if (type >= MENU_END && type < COLOR_END){
		getcolor(&r, &g, &b,type);
		glColor3f(r,g,b);
		glBegin(GL_QUADS); // for color platell

	}
	else
		glBegin(GL_LINE_LOOP);
	glVertex2f(x1, y2);
	glVertex2f(x1, y1);
	glVertex2f(x2, y1);
	glVertex2f(x2, y2);
	glEnd();
	if (type < MENU_END || type >= COLOR_END) 
	 toolbarimage(x1, y1,x2,y2,textposx ,textposy,font, type);
}
void createmenubackground(){
	glColor3f(0.85, 0.85, 0.85); 
	//glDisable(GL_DEPTH_TEST);
	glBegin(GL_POLYGON);
	glVertex2f(0, yend-102);
	glVertex2f(0,yend);
	glVertex2f(xend, yend);
	glVertex2f(xend, yend-102);
	glEnd();
	//glEnable(GL_DEPTH_TEST);
}
void createshapebox(int* choice){
	int startToolboxY = yend - 100;
	int endToolboxY = yend - 5;
	int startToolboxXpos = xend / 2;
	int endToolboxXpos = startToolboxXpos + 80;
	glColor3f(1, 1, 1);
	glLineWidth(1);
	glBegin(GL_LINE_LOOP); // create bounding box for toolbox
	glVertex2f(startToolboxXpos, startToolboxY);
	glVertex2f(startToolboxXpos, endToolboxY);
	glVertex2f(endToolboxXpos, endToolboxY);
	glVertex2f(endToolboxXpos, startToolboxY);
	glEnd();
	// cretebuttons inside toolbox
	int yoffset = 15;
	int begin = startToolboxXpos+2;
	int end = endToolboxXpos;
	int yend = endToolboxY;
	int ybegin = endToolboxY - yoffset;
	int xoffset = 15;
	int i = 0;
	int count = 0;
	while (i < MENU_END)
	{ 
		if ((i) % 4 == 0 && count != 0){			// 4 items in each row
			yend = ybegin;
			ybegin = yend - yoffset-2; //gap of 2 between row boxes 
			begin = startToolboxXpos+2;
			end = endToolboxXpos;
		}
		count++;
		createbutton(begin, ybegin, begin + xoffset, yend, begin + 2, ybegin + 5, GLUT_BITMAP_HELVETICA_10, (*choice)++); // choice mapped to enum menuoptions
		begin = begin + xoffset + 5;// 10 unit gap between consecutive menu items
		i++;
	}
}
void createcolorpalette(){
	int startcolorboxY = yend - 100;
	int endcolorboxY = yend -5;
	int Xposstartcolorbox = xend/2 + 100;
	int Xposendcolorbox = Xposstartcolorbox + 240;
	glColor3f(1, 1, 1);
	glLineWidth(1);
	glBegin(GL_LINE_LOOP); // create bounding box for Color palette	
	glVertex2f(Xposstartcolorbox, startcolorboxY);
	glVertex2f(Xposstartcolorbox, endcolorboxY);
	glVertex2f(Xposendcolorbox, endcolorboxY);
	glVertex2f(Xposendcolorbox, startcolorboxY);
	glEnd();
	int yoffset = 20;
	int begin = Xposstartcolorbox + 2;
	int end = Xposendcolorbox;
	int yend = endcolorboxY;
	int ybegin = endcolorboxY - yoffset;
	int xoffset = 20;
	int i = MENU_END;
	int choice = MENU_END;
	int count = 0;
	while (i < COLOR_END){
		if ((i- MENU_END) % 10 == 0 && count != 0){			// 4 items in each row
			yend = ybegin;
			ybegin = yend - yoffset - 2; //gap of 2 between row boxes 
			begin = Xposstartcolorbox + 2;
			end = Xposendcolorbox;
		}
		count++;
		createbutton(begin, ybegin, begin + xoffset, yend, begin + 2, ybegin + 5, GLUT_BITMAP_HELVETICA_10, choice); // choice mapped to enum menuoptions
		choice += 1;
		std::cout << choice << std::endl;
		begin = begin + xoffset + 5;// 10 unit gap between consecutive menu items
		i++;
	}

}
void createtoolBox(){
	int startToolboxY = yend - 100;
	int endToolboxY = yend - 5;
	int startToolboxXpos = xend / 2 -100;
	int endToolboxXpos = startToolboxXpos + 80;
	glColor3f(1, 1, 1);
	glLineWidth(1);
	glBegin(GL_LINE_LOOP); // create bounding box for toolbox
	glVertex2f(startToolboxXpos, startToolboxY);
	glVertex2f(startToolboxXpos, endToolboxY);
	glVertex2f(endToolboxXpos, endToolboxY);
	glVertex2f(endToolboxXpos, startToolboxY);
	glEnd();
	int yoffset = 20;
	int begin = startToolboxXpos + 2;
	int end = endToolboxXpos;
	int yend = endToolboxY;
	int ybegin = endToolboxY - yoffset;
	int xoffset = 20;
	int i = COLOR_END;
	int choice = COLOR_END;
	int count = 0;
	while (i < TOOL_END){
		if ((i - COLOR_END) % 2 == 0 && count != 0){			// 4 items in each row
			yend = ybegin;
			ybegin = yend - yoffset - 2; //gap of 2 between row boxes 
			begin = startToolboxXpos + 2;
			end = endToolboxXpos;
		}
		count++;
		createbutton(begin, ybegin, begin + xoffset, yend, begin + 2, ybegin + 5, GLUT_BITMAP_HELVETICA_10, choice); // choice mapped to enum menuoptions
		choice += 1;
		std::cout << choice << std::endl;
		begin = begin + xoffset + 5;// 10 unit gap between consecutive menu items
		i++;
	}


}
void createsizeBox(){
	int startsizeY = yend - 100;
	int endsizeY = yend - 5;
	int startsizeXpos = xend / 2 - 200;
	int endsizeXpos = startsizeXpos + 80;
	glColor3f(1, 1, 1);
	glLineWidth(1);
	glBegin(GL_LINE_LOOP); // create bounding box for toolbox
	glVertex2f(startsizeXpos, startsizeY);
	glVertex2f(startsizeXpos, endsizeY);
	glVertex2f(endsizeXpos, endsizeY);
	glVertex2f(endsizeXpos, startsizeY);
	glEnd();
	int yoffset = 15;
	int begin = startsizeXpos + 2;
	int end = endsizeXpos - 2;
	int ybegin = endsizeY - yoffset;
	int i = TOOL_END;
	int yend = 0;
	int choice = TOOL_END;
		while (i < SIZE_END){
		int size = pixelsize[i - size1]-4;
		yend = ybegin - size;
		createbutton(begin, ybegin, end, yend, begin + 2, ybegin , GLUT_BITMAP_HELVETICA_10, choice); // choice mapped to enum menuoptions
		choice += 1;
		std::cout << choice << std::endl;
		ybegin = yend - yoffset;
		i++;
	}


}
void createselectedcolor(){
	setselectedcolor();
	glBegin(GL_POLYGON);
	glVertex2f((xend/2)+350 , yend -60);
	glVertex2f((xend/2)+350, yend -40);
	glVertex2f((xend/2)+370 , yend -40);
	glVertex2f((xend/2)+370 , yend -60);
	glEnd();
}
void createmenu(){
	int startHeaderY = yend - 40;
	int endHeaderY = yend - 10;
	glClearColor(1.0, 1.0, 1.0, 1.0);
	int menu;
	menu = glutCreateMenu(HandleselectedMenu); // Notify glut that HandleselectedMenu is handler for menu pressed event
	if (Firstrun == true)
		glClear(GL_COLOR_BUFFER_BIT); //clear bufferbit for first time so that menu items are rendered at first instance.
	Firstrun = false;
	glutAddMenuEntry("CLEAR", CLEAR);
	glutAddMenuEntry("EXIT", EXIT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	createmenubackground();	
	int choice = 0;
	//createbutton(xbegin, startHeaderY, xend, endHeaderY, (xbegin + xend) / 4, startHeaderY + 5, GLUT_BITMAP_HELVETICA_18, choice++);
	createtoolBox();
	createshapebox(&choice);
	createcolorpalette();
	createsizeBox();
	// display selected color 
    createselectedcolor();
}

void display(void){
	if (selectedMenuItem == -1)
		createmenu();
	glFlush();
}

void resetshapepoints(){
	for (int i = 0; i < 3; ++i){
		for (int j = 0; j < 2; ++j){
			shapepoint[i][j] = -1;
		}
	}
	//numcontrolpoints = 0;// reset control points on any other shape
}
/* Draw shape handling starts here*/
void setselectedcolor(){
	int r, g, b;
	r = colormat[colorsel - black][0];
	g = colormat[colorsel - black][1];
	b = colormat[colorsel - black][2];
	glColor3f(r, g, b);
}
void paintpencil(){
	glLineWidth(1); //pencil default size one
	glBegin(GL_LINES);
	glVertex2f(shapepoint[0][0], shapepoint[0][1]);
	glVertex2f(shapepoint[1][0], shapepoint[1][1]);
	glEnd();	
}

void paintBrush(bool pixelflag){
	glLineWidth(pixelsize[selpixelsz]+3);
	glBegin(GL_LINES);
	glVertex2f(shapepoint[0][0], shapepoint[0][1]);
	glVertex2f(shapepoint[1][0], shapepoint[1][1]);
	glEnd();	
}

void eraselastline(){
		glLineWidth(pixelsize[selpixelsz]);
		glColor3f(1,1,1);
		glBegin(GL_LINES);
		glVertex2f(shapepoint[1][0], shapepoint[1][1]);
		glVertex2f(shapepoint[2][0], shapepoint[2][1]);
		glEnd();
}
void drawline(){
	glLineWidth(pixelsize[selpixelsz]);
	//eraselastline();
	setselectedcolor();
	glBegin(GL_LINE_STRIP);
	glVertex2f(shapepoint[0][0], shapepoint[0][1]);
	glVertex2f(shapepoint[1][0], shapepoint[1][1]);
	glEnd();
}

void drawrect(){
	glLineWidth(pixelsize[selpixelsz]);
	setselectedcolor();
	//int offset = abs(shapepoint[1][0] - shapepoint[0][0]);
	if ((shapepoint[0][1] ) < yend - 100)
	{
		glBegin(GL_LINE_LOOP);
		glVertex2f(shapepoint[1][0], shapepoint[1][1]);
		glVertex2f(shapepoint[0][0], shapepoint[1][1]); //square aligned with x axis hence shapepoint[1][1]
		glVertex2f(shapepoint[0][0], shapepoint[0][1] );
		glVertex2f(shapepoint[1][0], shapepoint[0][1] );
		glEnd();
	}
}

void drawtriangle(){
	glLineWidth(pixelsize[selpixelsz]);
	setselectedcolor();
	int len = abs(shapepoint[1][0] - shapepoint[0][0]);
	float height = sqrtf(3) / 2 * len;
	if ((shapepoint[0][1] + height) < yend - 100) // Draw only when does not cross canvas boundary
	{
		glBegin(GL_LINE_LOOP);
		glVertex2f(shapepoint[1][0], shapepoint[0][1]);
		glVertex2f(shapepoint[0][0], shapepoint[0][1]); //square aligned with x axis hence shapepoint[1][1]
		glVertex2f((shapepoint[1][0] + shapepoint[0][0]) / 2, shapepoint[0][1] + height);
		glEnd();
	}
}
Color getPixelColor(int x, int y) {
	Color  color;
	glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, &color);
	return color;
}

void setPixelColor(int x, int y, Color color) {
	glColor3f(color.r, color.g, color.b);
	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
}
bool checkvalidboundary(int x, int y){
	return (x >= 0 && x <= xend - 1 && y >= 0 && y < yend - 1);
}
void floodFill(GLint x, GLint y, float r, float g, float b) {
	if (r == 1.0f && g == 1.0f && b == 1.0f) // new color same as background
		return;
	std::vector< std::vector< bool > > visited(2*xend, std::vector<bool>(2*yend, 0));
	unsigned char pick_col[3];
	float R, G, B;
	Color pixeloldcolor;
	std::stack<std::pair<int,int> > s1;
	s1.push(std::make_pair(x, y));
	
	while (!s1.empty())
	{
		std::pair<int, int> temp = s1.top();
		s1.pop();
		if (visited[temp.first][temp.second] == 0){
			visited[temp.first][temp.second] = 1;
			pixeloldcolor = getPixelColor(temp.first, temp.second);
			if (checkvalidboundary(temp.first, temp.second) && pixeloldcolor.r == 1.0f && pixeloldcolor.g == 1.0f && pixeloldcolor.b == 1.0f && (pixeloldcolor.r != r || pixeloldcolor.g != g || pixeloldcolor.b != b))
			{
				Color newpixelcolor;
				newpixelcolor.r = r;
				newpixelcolor.g = g;
				newpixelcolor.b = b;
				setPixelColor(temp.first, temp.second, newpixelcolor);
	     		visited[x][y] = 1;
				if (checkvalidboundary(temp.first + 1, temp.second) && !visited[temp.first + 1][temp.second])s1.push(std::make_pair(temp.first + 1, temp.second));
				if (checkvalidboundary(temp.first, temp.second + 1) && !visited[temp.first][temp.second + 1])s1.push(std::make_pair(temp.first, temp.second + 1));
				if (checkvalidboundary(temp.first, temp.second - 1) && !visited[temp.first][temp.second - 1])s1.push(std::make_pair(temp.first, temp.second - 1));
				if (checkvalidboundary(temp.first - 1, temp.second) && !visited[temp.first - 1][temp.second])s1.push(std::make_pair(temp.first - 1, temp.second));
			}
		}		
	}
}
# if 0 //working recursive
void floodFill(GLint x, GLint y, float r, float g, float b) {
	unsigned char pick_col[3];
	float R, G, B;
	Color color;
		color = getPixelColor(x, y);
		//log("x", x,"Y", y, " ");
		if (visited[x][y] == 1)
			return;
	if (x >= 0 && x <= xend - 1 && y >= 0 && y < yend - 1 && color.r == 1.0 && color.g == 1.0f && color.b == 1.0f &&( color.r != r || color.g != g || color.b != b)){
		glColor3f(r, g, b);
		glBegin(GL_POINTS);
		glVertex2i(x, y);
		visited[x][y] = 1;
		glEnd();
		if(!visited[x+1][y])floodFill(x + 1, y, r, g, b);
		if (!visited[x ][y+1])floodFill(x, y + 1, r, g, b);
		if (!visited[x][y-1])floodFill(x, y - 1, r, g, b);
		if (!visited[x  - 1][y])floodFill(x - 1, y, r, g, b);
	}
}
#endif



void spray(float x , float y)
{
		setselectedcolor();
		float points[2][50];
			int i, j;
			for( i = 0 ; i < 50 ; i++){
				float val = rand() % RAND_RANGE;
				if(i%2==0)
				points[0][i] = val + 1 + x;
				else
				 points[0][i] = x -val + 1;
				val = rand() % RAND_RANGE;
				if(i%2==0)
				points[1][i] = val  + 1 + y;
				else
				points[1][i] = y - val  + 1 ;	
			}
				glBegin(GL_POINTS);				
				for( j = 0 ; j < 50 ; j++)					    
						glVertex2f(points[0][j],points[1][j]);
						glEnd();
					}
void Mouse_Motion_CB(int x, int y){
	setselectedcolor();
	points.xo = points.x; 
	points.yo = points.y;   
	points.x = x;
	points.y = yend - y; // since screen coord start from 0,0 topmost
	log("Tool", toolsel, "shape", shapesel, "");
	glPushMatrix();
	float rf, gf, bf;
	rf = colormat[colorsel - black][0];
	gf = colormat[colorsel - black][1];
	bf = colormat[colorsel - black][2];
	if (points.y < yend - 100) // i.e inside canvas
	{
		switch (toolsel){	
		case PENCIL:
			shapepoint[1][0] = shapepoint[0][0];
			shapepoint[1][1] = shapepoint[0][1];
			shapepoint[0][0] = points.x;
			shapepoint[0][1] = points.y;
			log("current PENCIL point", shapepoint[0][0], shapepoint[0][1], shapepoint[1][0], shapepoint[1][1], " ");
			if(shapepoint[1][0]!=-1 && shapepoint[1][1]!=-1)
			paintpencil();
			break;
		case ERASER:
			glColor3f(1, 1, 1);
			shapepoint[1][0] = shapepoint[0][0];
			shapepoint[1][1] = shapepoint[0][1];
			shapepoint[0][0] = points.x;
			shapepoint[0][1] = points.y;
			if(shapepoint[1][0]!=-1 && shapepoint[1][1]!=-1)
			paintBrush(true);
			break;
		case BRUSH:
			shapepoint[1][0] = shapepoint[0][0];
			shapepoint[1][1] = shapepoint[0][1];
			shapepoint[0][0] = points.x;
			shapepoint[0][1] = points.y;
			if(shapepoint[1][0]!=-1 && shapepoint[1][1]!=-1)
			paintBrush(true);
			break;
		default:
			break;
			}
		switch(shapesel){		// select initial points here			
			case LINE:
				//renderpixel(x, y); // to show current selected point
				if (shapepoint[1][0] == -1 && shapepoint[1][1] == -1)
				{

					shapepoint[1][0] = points.x;
					shapepoint[1][1] = points.y;
				}
				shapepoint[2][0] = shapepoint[0][0]; // to erase last drawn line
				shapepoint[2][1] = shapepoint[0][1];
			    shapepoint[0][0] = points.x;
				shapepoint[0][1] = points.y;
				break;
			case SQUARE:
				if (shapepoint[1][0] == -1 && shapepoint[1][1] == -1)
				{

					shapepoint[1][0] = points.x;
					shapepoint[1][1] = points.y;
				}
				shapepoint[0][0] = points.x;
				shapepoint[0][1] = points.y;
				break;
			case CIRCLE:
			case TRIANGLE:
		    case ELLIPSE:
				if (shapepoint[1][0] == -1 && shapepoint[1][1] == -1)
				{

					shapepoint[1][0] = points.x;
					shapepoint[1][1] = points.y;
				}
				shapepoint[0][0] = points.x;
				shapepoint[0][1] = points.y;
				break;
			default:
				break;
			}
		
		}
		glPopMatrix();
	}
void Key_pressed_CB(unsigned char key, int x, int y){
	switch(key){
		case 'r':
			resetglobalparam();
			break;
		case 'p':
			toolsel = PENCIL;
			break;
		case 'b':
			toolsel = BRUSH;
		case 'c':
        clearcanvas();
			break;
		case 'q':
			exit(0);
			break;
		default:
			break;
			
		}

}
void setcolor(float x, float y){
	int yoffset = 20;
	int xoffset = 20;
	int colorXposstart = xend / 2 + 100;
	int colorXposend = colorXposstart + 240;
	int gap = 5;
	int comp = colorXposstart;
	if (x >= comp && x <= comp + xoffset){
		colorsel = black;
		return;
	}
	comp = comp + xoffset + gap;
	if (x >= comp && x <= comp + xoffset){
		colorsel = white;
		return;
	}
	comp = comp + xoffset + gap;
	if (x >= comp && x <= comp + xoffset){
		colorsel = red;
		return;
	}
	comp = comp + xoffset + gap;
	if (x >= comp && x <= comp + xoffset){
		colorsel = green;
		return;
	}
	comp = comp + xoffset + gap;
	if (x >= comp && x <= comp + xoffset){
		colorsel = blue;
		return;
	}
	comp = comp + xoffset + gap;
	if (x >= comp && x <= comp + xoffset){
		colorsel = yellow;
		return;
	}
	comp = comp + xoffset + gap;
	if (x >= comp && x <= comp + xoffset){
		colorsel = cyan;
		return;
	}
	comp = comp + xoffset + gap;
	if (x >= comp && x <= comp + xoffset){
		colorsel = magenta;
		return;
	}

}
void settool(int x, int y){
	int yoffset = 20;
	int xoffset = 20;
	int startToolboxXpos = xend / 2 - 100;
	int endToolboxXpos = startToolboxXpos + 80;
	int gap = 5;
	int comp = startToolboxXpos;
	shapesel	= MENU_END;// reset shape when tool selected Mutual exclusive events
	if (x >= comp && x <= comp + xoffset){
		if (y >= 0 && y <= yoffset)
			toolsel = PENCIL;
		else if(y>=yoffset && y<=2*yoffset)
			toolsel = BRUSH;
		else
			toolsel = SPRAY;
		std::cout << "Selected Tool " << toolsel<<std::endl;
		return;
	}
	comp = comp + xoffset + gap;
	if (x >= comp && x <= comp + xoffset){
		if (y >= 0 && y <= yoffset)
			toolsel = ERASER;
		else if(y>=yoffset && y<=2*yoffset)
			toolsel = COLORFILL;
		else 
			toolsel = CHULL;
		return;
	}
}

void setsize(float x, float y){
	int yoffset = 15;
	int comp = 15;
	if (y >= comp && y <= comp+yoffset){
		selpixelsz = 0;
		return;
		}
	comp = comp + yoffset;
	if (y >= comp && y <= comp + yoffset){
		selpixelsz = 1;
		return;
		}
	comp = comp + yoffset;
	if (y >= comp && y <= comp + yoffset){
		selpixelsz = 2;
		return;
		}
	comp = comp + yoffset;
	if (y >= comp && y <= comp + yoffset){
		selpixelsz = 3;
		return;
		}
}
void setshape(int x, int y)
{
	int yoffset = 15;
	int xoffset = 15;
	int startshapeboxXpos = xend / 2;
	int endshapeboxXpos = startshapeboxXpos + 80;
	int gap = 5;
	int comp = startshapeboxXpos;
	toolsel = TOOL_END;// reset tool sel if shape is selected
	resetshapepoints();
	if (x >= comp && x <= comp + xoffset){
		if (y >= 0 && y <= yoffset)
			shapesel = LINE;
		else
			shapesel = BIEZER;
		log("selected shape",shapesel," ");
		return;
	}
	comp = comp + xoffset + gap;
	if (x >= comp && x <= comp + xoffset){
	if (y >= 0 && y <= yoffset)
		shapesel = SQUARE;
	else
		shapesel = ELLIPSE;
		return;
	}
	comp = comp + xoffset + gap;
	if (x >= comp && x <= comp + xoffset){
		shapesel = CIRCLE;
		return;
	}
	comp = comp + xoffset + gap;
	if (x >= comp && x <= comp + xoffset){
		shapesel = TRIANGLE;
		return;
	}
}
void settoolcolorshape(int x, int y){
	int colorXposstart= xend / 2 + 100;
	int colorXposend = colorXposstart + 240;
	int startToolboxXpos = xend / 2 - 100;
	int endToolboxXpos = startToolboxXpos + 80;
	int startsizeXpos = xend / 2 - 200;
	int endsizeXpos = startsizeXpos + 80;
	int startshapeboxXpos = xend / 2;
	int endshapeboxXpos = startshapeboxXpos + 80;
//	std::cout << " X " << x << " " << "Y" << y << " " << " start y " << starty << " " << " endy " << endy << "  yend- y " << (yend - y) << " yend " << yend << " Xposstart " << Xposstart << " Xposend " << Xposend<< std::endl;
	if (y >= 0 && y <= 100){
		if (x >= colorXposstart  && x <= colorXposend)
		setcolor(x, y);
		if (x >= startToolboxXpos && x <= endToolboxXpos)
		settool(x, y);
	 	if (x >= startsizeXpos && x <= endsizeXpos)
		setsize(x, y);
		if (x >= startshapeboxXpos && x <= endshapeboxXpos)
			setshape(x, y);
	}
	createselectedcolor();
}
bool checkvaliddraw(){
	return(shapepoint[1][0] != -1 && shapepoint[1][1] != -1);
}
void findbiezerpoint(float* xnew, float*ynew, double t){ 	
  *xnew = pow((1-t),3) * controlpoints[0][0] +3*t*pow((1-t),2)*controlpoints[1][0]+3*(1-t)*pow(t,2)*controlpoints[2][0] + pow(t,3)*controlpoints[3][0];	
  *ynew = pow((1-t),3) * controlpoints[0][1] +3*t*pow((1-t),2)*controlpoints[1][1]+3*(1-t)*pow(t,2)*controlpoints[2][1] + pow(t,3)*controlpoints[3][1];	
}

int counterclocktest(const std::pair<int, int > &p, const std::pair<int, int >  &q, const std::pair<int, int >  &r)
{
	int val = (q.second - p.second) * (r.first - q.first) -
		(q.first - p.first) * (r.second - q.second);

	if (val == 0) return CCW_COLINEAR;  // colinear
	return (val > 0) ? CCW_RIGHT_TURN: CCW_LEFT_TURN; // clock or counterclock wise
}
double distSq(const std::pair<int, int > p1, const std::pair<int, int > p2)
{
	return (p1.first - p2.first)*(p1.first - p2.first) +
		(p1.second - p2.second)*(p1.second - p2.second);
}
  bool compare(const std::pair<int,int > &p1 , const std::pair<int,int> &p2)
  	{
   // Find orientation
   int res = counterclocktest(p0, p1, p2);
   if (res == CCW_LEFT_TURN)
   	   return true;
   else if(res = CCW_COLINEAR)
   	{
   	double d1 = distSq(p0,p1);
	double d2 = distSq(p0,p2);

	if(d1 < d2)
		return true;
   	}
  return false;
  	}
  std::pair<int, int> nexttotop(std::stack<std::pair<int, int> > s){
	  std::pair<int, int> topofStack = s.top();
	  s.pop();
	  if (!s.empty())
	  {
	  std::pair<int, int> toret = s.top();
	  s.push(topofStack);
	  return toret;
  }
}
void drawGrahamscan(std::vector<std::pair<int,int> > CHpoint){
	int minindex = 0;
	int ymin = CHpoint[0].second ;
	//find min y point
	for(int i = 1; i<numCHselectedpoints ;++i)
		{
		  int cury = CHpoint[i].second;
		  if((cury <ymin)|| (ymin == cury && CHpoint[i].first < CHpoint[minindex].first))
		  	{
		  	ymin = CHpoint[i].second;
		    minindex = i;
		  }
		}
	int tempx = 0, tempy= 0;
	tempx = CHpoint[0].first;
	tempy = CHpoint[0].second;
	CHpoint[0].first = CHpoint[minindex].first;
	CHpoint[0].second = CHpoint[minindex].second;
	CHpoint[minindex].first = tempx;
	CHpoint[minindex].second = tempy;
	p0 = CHpoint[0];
	std::sort(CHpoint.begin()+1, CHpoint.end(), compare);
	int newindex = 1;
	for(int i =1; i<numCHselectedpoints-1;++i)
		{
			while (i<numCHselectedpoints - 1 && counterclocktest(p0, CHpoint[i], CHpoint[i + 1]) == 0)
				i++;
			CHpoint[newindex] = CHpoint[i];
			newindex++;
		}
	if (newindex<3)
		return; // Not enough points

	std::stack<std::pair<int,int> > s;
	s.push(CHpoint[0]);
	s.push(CHpoint[1]);
	s.push(CHpoint[2]);
	for(int i =3; i<numCHselectedpoints ;++i){
		while (!s.empty() && counterclocktest(nexttotop(s), s.top(), CHpoint[i]) != CCW_LEFT_TURN)
			s.pop();

		s.push(CHpoint[i]);
		}
	setselectedcolor();
	std::pair<int,int> p1 = p0;

	while(!s.empty()){
          std::pair<int,int> p2 = s.top();	
		  s.pop();
		  
		  glBegin(GL_LINES);
		  glVertex2f(p1.first,p1.second);
		  glVertex2f(p2.first, p2.second);
		  glEnd();
		  p1 = p2;
		  glFlush();
	}

}

void Mouse_pressed_CB(int button, int state, int x, int y){
	int radius = 0 ;
	if (button == GLUT_LEFT_BUTTON && state ==GLUT_DOWN)
	{
	if (yend-y < yend - 100)
		mousepressed = true;
		points.x = x;
		points.y = y;
		points.xo =x;
		points.yo = y;
		settoolcolorshape( x,  y);
	    float rf, gf, bf;
	rf = colormat[colorsel - black][0];
	gf = colormat[colorsel - black][1];
	bf = colormat[colorsel - black][2];
		if(toolsel == COLORFILL && mousepressed == true)
			floodFill(points.x, yend - points.y, rf, gf, bf);
		if(toolsel == SPRAY && mousepressed == true)
			spray(points.x , yend -points.y);
	}
	else {

		if (toolsel >= PENCIL && toolsel < TOOL_END)
		{
			if (toolsel == CHULL)
			{
              if(yend - y < yend -100){
				chullpoints[numCHselectedpoints] = std::make_pair(x, yend - y);
				numCHselectedpoints++;
              	}
                glBegin(GL_POINTS);
				glVertex2i(x, yend - y);
				glEnd();
				draw_circle1(x, yend - y, 3, 200);
				if (numCHselectedpoints == MAX_CHULL_POINTS)
				{
					drawGrahamscan(chullpoints);
					numCHselectedpoints = 0;
				}
				glFlush();
			}
			resetshapepoints();
		}
			if (yend - y < yend - 100) // i.e inside canvas
			{
				switch (shapesel){
				case LINE:
					//renderpixel(x, y); // to show current selected point
					shapepoint[0][0] = points.x;
					shapepoint[0][1] = points.y;
					log("current line point", shapepoint[0][0], shapepoint[0][1], shapepoint[1][0], shapepoint[1][1], " ");
					if (checkvaliddraw()){
						drawline();
					}
					glFlush();
					resetshapepoints();
					break;
				case SQUARE:
					shapepoint[0][0] = points.x;
					shapepoint[0][1] = points.y;
					if (checkvaliddraw()){
						drawrect();
						glFlush();
						resetshapepoints();
					}
					break;
				case CIRCLE:
					shapepoint[0][0] = points.x;
					shapepoint[0][1] = points.y;
					radius = (int)sqrtf(pow((shapepoint[0][0] - shapepoint[1][0]), 2) - pow((shapepoint[0][1] - shapepoint[1][1]), 2));
					if (checkvaliddraw())
					{
						draw_circle1(shapepoint[1][0], shapepoint[1][1], radius, 200);
						log("current circle point", shapepoint[1][0], shapepoint[1][1], " radius", radius, " ");
						glFlush();
						resetshapepoints();
					}
					break;
				case ELLIPSE:
					shapepoint[0][0] = points.x;
					shapepoint[0][1] = points.y;
					{
						int rh = abs(shapepoint[0][0] - shapepoint[1][0]);
						int rv = abs(shapepoint[0][1] - shapepoint[1][1]);
						if (checkvaliddraw())
						{
							draw_ellipse(shapepoint[1][0], shapepoint[1][1], rh, rv);
							log("current Ellipse point", shapepoint[1][0], shapepoint[1][1], " rh rv", rh, rv, " ");
							//glFlush();
							glFlush();
							resetshapepoints();
						}
					}
					break;
				case TRIANGLE:
					shapepoint[0][0] = points.x;
					shapepoint[0][1] = points.y;
					if (checkvaliddraw()){
						drawtriangle();
						//glFlush();
						glFlush();
						resetshapepoints();
					}
					break;
				case BIEZER:
					if(yend - y < yend -100)
					{
					controlpoints[numcontrolpoints][0] = x;
					controlpoints[numcontrolpoints][1] = yend - y;
					numcontrolpoints++;
					}
					glBegin(GL_POINTS);
					glVertex2i(x, yend - y);
					glEnd();
					draw_circle1(x, yend - y, 3, 200);// circle around control points
					
					{
						setselectedcolor();
						float xold = controlpoints[0][0];
						float yold = controlpoints[0][1];

						log("BIEZER", "numcontrolpoints", numcontrolpoints, "");
						if (numcontrolpoints == 4)
						{
							log("inside draw", "");
							float xbeiz = 0.0f;
							float ybeiz = 0.0f;
							for (double t = 0.0f; t < 1.0f; t += 0.05f){

								findbiezerpoint(&xbeiz, &ybeiz, t);
								log("BIEZER", "for loop", " xbeiz , ybeiz , xold, yold", xbeiz, ybeiz, xold, yold, "");
								glBegin(GL_LINES);
								glVertex2f(xold, yold);
								glVertex2f(xbeiz, ybeiz);
								glEnd();
								xold = xbeiz;
								yold = ybeiz;
								//glFlush();
								glFlush();
							}
							glBegin(GL_LINES);
							glVertex2f(xold, yold);
							glVertex2f(controlpoints[3][0], controlpoints[3][1]);
							glEnd();
							numcontrolpoints = 0;
						}
					}
					//glFlush();
					glFlush();
					break;
				default:
					break;
				}
				mousepressed = false;
			}
		}

	}
void resetglobalparam(void){
 colorsel = black;
 toolsel = PENCIL;
 shapesel = MENU_END;
 selpixelsz = 0;
}
void Reshape_Window(int x  , int y){
	glClearColor(1.0f, 1.0f, 1.0f, 1);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, (GLdouble)x, 0.0, (GLdouble)y, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, x, y);
	glFlush();
	//glFlush();
	xend = x;
	yend = y;
	resetglobalparam();
	glutPostRedisplay();
}
void setprojection(void){
	glClearColor(1.0f, 1.0f, 1.0f, 1);// white background
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0f, 1028.0f, 0.0f, 700.0f);//set projection as screen coord
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, 1028.0f, 700.0f);
	glFlush();
	points.x = 0;
	points.y = 0;
	points.xo = 0;
	points.yo = 0;
	xend = 1028;
	yend = 700;
	resetglobalparam();
	}

// OPENGL function reference https://www.opengl.org/resources/libraries/glut/spec3/node51.html
int main(int argc, char** argv){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGBA); // GL_SINGLE since only one framebuffer needed use glFlush()
	glEnable(GLUT_MULTISAMPLE);
	glutInitWindowSize(1028, 700);
	glutCreateWindow("Graphics editor");
	setprojection();
	glutDisplayFunc(display);
	glutMotionFunc(Mouse_Motion_CB); // get what button has been pressed with the glutMouseFunc and then track the mouse with glutMotionFunc()
	glutKeyboardFunc(Key_pressed_CB);
	glutReshapeFunc(Reshape_Window);
	glutMouseFunc(Mouse_pressed_CB);
	glutMainLoop();
	return 0;
}
