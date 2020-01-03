//小宅用

#include "pch.h"
#include <Windows.h>
#include <iostream>
#include <gl/glut.h>
#include <math.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <gdiplus.h>


//松本用
/*
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <gdiplus.h>
#include <gl/glut.h>
*/

using namespace Gdiplus;

#define WIDTH 800
#define HEIGHT 450

GdiplusStartupInput gdiPSI;
ULONG_PTR gdiPT;
GLuint tex_player1;
GLuint tex_player2;
GLuint tex_player3;
bool onMoveKeyPress_L = false;
bool onMoveKeyPress_R = false;
bool jump = false;
int jump_timer = 0;  //キャラクターがジャンプしてからの時間を計測

struct Position
{
	double x, y;
	int direction;
};

Position player = {};

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
}

void end()
{
	GdiplusShutdown(gdiPT);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);

	glBindTexture(GL_TEXTURE_2D, tex_player1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0f, 1.0f); glVertex2d(0 + player.x, 448 + player.y);//左下
	glTexCoord2f(0.0f, 0.0f); glVertex2d(0 + player.x, 416 + player.y);//左上
	glTexCoord2f(1.0f, 0.0f); glVertex2d(32 + player.x, 416 + player.y);//右上
	glTexCoord2f(1.0f, 1.0f); glVertex2d(32 + player.x, 448 + player.y);//右下
	glEnd();
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glutSwapBuffers();
}

void idle(void)
{
	if (onMoveKeyPress_L == true)
	{
		player.x -= 0.5;
	}

	if (onMoveKeyPress_R == true)
	{
		player.x += 0.5;
	}

	if (jump == true)
	{
		player.y = player.y - (12 * jump_timer - 9.8*jump_timer*jump_timer *0.5)*0.12;

		if (player.y > 0) //ブロックの上に着地する場合，その座標とする
		{
			player.y = 0;
			jump_timer = 0;
			jump = false;
		}

	}


	printf("%.2f,%.2f %d\n", player.x, player.y, jump_timer);
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'q':
	case '\033':  /* '\033' は ESC の ASCII コード */
		exit(0); break;
	case 'a': onMoveKeyPress_L = true; player.direction = 0; break;
	case 'd': onMoveKeyPress_R = true; player.direction = 1; break;
	case '\040': if (jump == false) { jump = true; } break;
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

void Init() {
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
	GdiplusStartup(&gdiPT, &gdiPSI, NULL);
	LoadImagePNG(L"walk1.png", tex_player1);
	LoadImagePNG(L"walk2.png", tex_player2);
	LoadImagePNG(L"walk3.png", tex_player3);
	player.x = 0;
	player.y = 0;
	player.direction = 1;

}

void timer(int value) {
	glutPostRedisplay();

	if (jump == true)
	{
		jump_timer++;
	}

	glutTimerFunc(100, timer, 0);
}

int main(int argc, char *argv[])
{
	atexit(end);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow("画像を読み込んで表示");
	glutDisplayFunc(display);
	glutTimerFunc(100, timer, 0);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutIdleFunc(idle);

	Init();
	glutMainLoop();
	return 0;
}