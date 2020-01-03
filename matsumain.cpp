////小宅用
//
//#include "pch.h"
//#include <iostream>
//#include <gl/glut.h>
//#include <math.h>
//#include <objbase.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <gdiplus.h>


//松本用
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <gdiplus.h>
#include <gl/glut.h>
#include <iostream>



using namespace Gdiplus;
void LoadImagePNG(const wchar_t* filename, GLuint &texture);

#define WIDTH 1280	
#define HEIGHT 720

GdiplusStartupInput gdiPSI;
ULONG_PTR gdiPT;
GLuint tex;

bool onMoveKeyPress_L = false;
bool onMoveKeyPress_R = false;
bool changeflag = false;
bool setupFlag = true;

double center_x = 0;
double center_y = 0;
int jumpflag = 0;
float jumpspeedMax = 10;
float jumpspeed = jumpspeedMax;
float jump_a = -0.01f;
double camera_x = 0;
double camera_y = 0;

struct Pic {

	float lb[2];
	float lu[2];
	float ru[2];
	float rb[2];
	//wchar_t* filename;

	Pic(const wchar_t* filename, float x_max, float y_max, float x_min, float y_min) {
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
		GdiplusStartup(&gdiPT, &gdiPSI, NULL);
		LoadImagePNG(filename, tex);
		lb[0] = x_min;
		lb[1] = y_max;
		lu[0] = x_min;
		lu[1] = y_min;
		ru[0] = x_max;
		ru[1] = y_min;
		rb[0] = x_max;
		rb[1] = y_max;

		std::cout << lb[0] << " : " << lb[1] << std::endl;
		std::cout << lu[0] << " : " << lu[1] << std::endl;
		std::cout << ru[0] << " : " << ru[1] << std::endl;
		std::cout << rb[0] << " : " << rb[1] << std::endl;
	};
	Pic() {
		lb[0] = 10;
		lb[1] = 100;
		lu[0] = 10;
		lu[1] = 10;
		ru[0] = 100;
		ru[1] = 10;
		rb[0] = 100;
		rb[1] = 100;
		std::cout << "Pic宣言" << std::endl;
	}
};

Pic pic(L"block.png", 246, 66, 0, 0);

void LoadImagePNG(const wchar_t* filename, GLuint &texture)
{
	glEnable(GL_TEXTURE_2D);
	Bitmap bmp(filename);
	BitmapData data;
	bmp.LockBits(0, ImageLockModeRead, PixelFormat32bppARGB, &data);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, data.Width, data.Height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, data.Scan0);
	glBindTexture(GL_TEXTURE_2D, 0);
	bmp.UnlockBits(&data);
	std::cout << "Load image" << std::endl;
}

void end()
{
	GdiplusShutdown(gdiPT);
}

void Update() {
	//if (jumpflag >= 1) {
		//while (jumpspeed >= 0) {
		//	//float p_cen_y = pic.lu[1] - pic.lb[1] / 2;
			//std::cout << "aaaaa" << std::endl;
			//pic.lb[1] += jumpspeed;
			//pic.lu[1] += jumpspeed;
			//pic.ru[1] += jumpspeed;
			//pic.rb[1] += jumpspeed;
			//jumpspeed += jump_a;
		//}

		//while (jumpspeed <= jumpspeedMax) {
		//	std::cout << "bbbbb" << std::endl;
			
			//pic.lb[1] -= jumpspeed;
			//pic.lu[1] -= jumpspeed;
			//pic.ru[1] -= jumpspeed;
			//pic.rb[1] -= jumpspeed;
			//jumpspeed += 0.01f;
		//}
		//jumpflag = 0;
		//jumpspeed = jumpspeedMax;
	//}
}

void SetImage(double center_x, double center_y, GLuint &tex) {
	glBindTexture(GL_TEXTURE_2D, tex);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glBegin(GL_POLYGON);
	//glTexCoord2f(0.0f, 1.0f); glVertex2d(10, 230);//左下
	//glTexCoord2f(0.0f, 0.0f); glVertex2d(10, 10);//左上
	//glTexCoord2f(1.0f, 0.0f); glVertex2d(310, 10);//右上
	//glTexCoord2f(1.0f, 1.0f); glVertex2d(310, 230);//右下
	glTexCoord2f(0.0f, 1.0f); glVertex2d(pic.lb[0] + center_x, pic.lb[1] + center_y);//左下
	glTexCoord2f(0.0f, 0.0f); glVertex2d(pic.lu[0] + center_x, pic.lu[1] + center_y);//左上
	glTexCoord2f(1.0f, 0.0f); glVertex2d(pic.ru[0] + center_x, pic.ru[1] + center_y);//右上
	glTexCoord2f(1.0f, 1.0f); glVertex2d(pic.rb[0] + center_x, pic.rb[1] + center_y);//右下
	glEnd();
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

}

void display(void)
{
	//std::cout << "Display" << std::endl;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
	gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);
	//glBindTexture(GL_TEXTURE_2D, tex);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_TEXTURE_2D);
	//glEnable(GL_ALPHA_TEST);
	//glBegin(GL_POLYGON);
	//glTexCoord2f(0.0f, 1.0f); glVertex2d(pic.lb[0] + center_x, pic.lb[1] + center_y);//左下
	//glTexCoord2f(0.0f, 0.0f); glVertex2d(pic.lu[0] + center_x, pic.lu[1] + center_y);//左上
	//glTexCoord2f(1.0f, 0.0f); glVertex2d(pic.ru[0] + center_x, pic.ru[1] + center_y);//右上
	//glTexCoord2f(1.0f, 1.0f); glVertex2d(pic.rb[0] + center_x, pic.rb[1] + center_y);//右下
	//glTexCoord2f(0.0f, 1.0f); glVertex2d(10, 400);//左下
	//glTexCoord2f(0.0f, 0.0f); glVertex2d(10, 10);//左上
	//glTexCoord2f(1.0f, 0.0f); glVertex2d(400, 10);//右上
	//glTexCoord2f(1.0f, 1.0f); glVertex2d(400, 400);//右下
	//glEnd();
	//glDisable(GL_ALPHA_TEST);
	//glDisable(GL_TEXTURE_2D);
	//glDisable(GL_BLEND);
	
	//SetImage(center_x, center_y, tex);
	//SetImage(center_x + 246, center_y, tex);
	//SetImage(100, 100, tex);
	for (int i = -2; i < 40; i++) {
		SetImage(center_x + i * 246, center_y + 150, tex);
	}
	//SetImage(center_x-246, center_y, tex);
	
	Update();
	glutSwapBuffers();
}

double counter = 1;

void idle(void)
{
	
	if (onMoveKeyPress_R) {
		std::cout << "d押しました" << std::endl;
		//pic.lb[0] += 5;
		//pic.lu[0] += 5;
		//pic.ru[0] += 5;
		//pic.rb[0] += 5;
		//camera_x += 0.00001;
		//glTranslated(-1, 0, 0);
		camera_x += counter;
		//glLoadIdentity();
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);
		std::cout << camera_x << ":" << camera_y << std::endl;
	}

	if (onMoveKeyPress_L) {
		std::cout << "a押しました" << std::endl;
		//pic.lb[0] -= 5;
		//pic.lu[0] -= 5;
		//pic.ru[0] -= 5;
		//pic.rb[0] -= 5;
		//camera_x -= 0.00001;
		
		//glTranslated(1, 0, 0);
		//gluLookAt(camera_x, center_y, 0, camera_x, center_y, 1, 0, 1, 0);
		camera_x -= counter;
		//glLoadIdentity();
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);
		std::cout << camera_x << ":" << camera_y << std::endl;
	}

	glutPostRedisplay();
}

void resize(int w, int h){
	center_x = (double)w / (double)2.0;
	center_y = (double)h / (double)2.0;
	camera_x = center_x;
	camera_y = center_y;
	std::cout << camera_x << std::endl;
	std::cout << camera_y << std::endl;
	/* ウィンドウ全体をビューポートにする */
	glViewport(0, 0, w, h);
	/* 変換行列の初期化 */
	//glLoadIdentity();
	/* スクリーン上の表示領域をビューポートの大きさに比例させる */
	//glOrtho(-w / 200.0, w / 200.0, -h / 200.0, h / 200.0, -1.0, 1.0);
	glLoadIdentity();
	glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
	//gluPerspective(30.0, (double)w / (double)h, 0.1, 100.0);
	
	gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);
	//gluLookAt(100.0f, 200.0f, 200.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	std::cout << "resize" << std::endl;
}

void Init() {
	glClearColor(0.0, 0.0, 0.0, 1.0);
	//glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
	//if (setupFlag) {
	//	GdiplusStartup(&gdiPT, &gdiPSI, NULL);
	//	LoadImagePNG(L"walk2.png", tex);
	//	setupFlag = false;
	//	std::cout << "start完了" << std::endl;
	//}
}



void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'q':
		break;
	case 'Q':
		break;
	case '\033':  /* '\033' は ESC の ASCII コード */
		exit(0);
		break;
	case 'd': 
		onMoveKeyPress_R = true;
		//std::cout << camera_x << " : " << camera_y << std::endl;
		break;
	case 'a':
		onMoveKeyPress_L = true;
		break;
	case '\040':
		jumpflag = 1;
		break;
	default:
		break;
	}
}

void keyboardUp(unsigned char key, int x, int y)
{
	switch (key) {
	case 'a': onMoveKeyPress_L = false; break;
	case 'd': onMoveKeyPress_R = false; break;

	default:
		break;
	}
}

int main(int argc, char *argv[]){
	//struct Pic pic(L"walk2.png", 400, 400, 10, 10);
	atexit(end);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow("sample");
	if (setupFlag) {
		struct Pic pic(L"block.png", 246, 66, 0, 0);
		setupFlag = false;
		std::cout << "2" << std::endl;
	}
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutIdleFunc(idle);
	
	Init();

	//struct Pic pic(L"walk2.png", 400, 400, 10, 10);
	glutMainLoop();
	return 0;
}
