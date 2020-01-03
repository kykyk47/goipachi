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

#define WIDTH 1280
#define HEIGHT 720

GdiplusStartupInput gdiPSI;
ULONG_PTR gdiPT;
GLuint tex_player1;
GLuint tex_player2;
GLuint tex_player3;
bool onMoveKeyPress_L = false;
bool onMoveKeyPress_R = false;
bool player_jump = false;
bool player_walk = false;
int jump_timer = 0;  //キャラクターがジャンプしてからの時間を計測
int walk_timer = 0;
int walk_timer100 = 0;


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

	if (walk_timer == 0)
	{
		glBindTexture(GL_TEXTURE_2D, tex_player2);
	}
	else
	{
		switch (walk_timer)
		{
		case 1:  glBindTexture(GL_TEXTURE_2D, tex_player3); break;
		case 2:  glBindTexture(GL_TEXTURE_2D, tex_player2); break;
		case 3:  glBindTexture(GL_TEXTURE_2D, tex_player1); break;
		case 0:  glBindTexture(GL_TEXTURE_2D, tex_player2); break;
		}
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glBegin(GL_POLYGON);
	switch (player.direction)
	{
	case 0:
	{
		glTexCoord2f(0.0f, 1.0f); glVertex2d(32 + player.x, 448 + player.y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(32 + player.x, 416 + player.y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(0 + player.x, 416 + player.y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(0 + player.x, 448 + player.y);//右下
		break;
	}

	case 1:
	{
		glTexCoord2f(0.0f, 1.0f); glVertex2d(0 + player.x, 448 + player.y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(0 + player.x, 416 + player.y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(32 + player.x, 416 + player.y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(32 + player.x, 448 + player.y);//右下
		break;
	}
	}


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
		player.x -= 0.125;
	}

	if (onMoveKeyPress_R == true)
	{
		player.x += 0.125;
	}

	if (player_jump == true)
	{
		if (jump_timer < 10)
		{
			player.y = player.y - (15.0 * jump_timer - 9.68* jump_timer * sqrt(jump_timer) *0.5)*0.02;//ジャンプのときのプレイヤーの動き
		}

		else if (jump_timer >= 10)
		{
			player.y = player.y - (15.0 * jump_timer - 9.68* jump_timer * sqrt(jump_timer) *0.5)*0.0015;//ジャンプのときのプレイヤーの動き
		}

		if (player.y > 0) //ブロックの上に着地する場合，その座標とする
		{
			player.y = 0;
			jump_timer = 0;
			player_jump = false;
		}

	}


	printf("%.2f,%.2f %d %d \n", player.x, player.y, jump_timer, walk_timer100);
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'q':
	case '\033':  /* '\033' は ESC の ASCII コード */
		exit(0); break;
	case 'a': onMoveKeyPress_L = true; /*MoveLock_R = true;*/ player.direction = 0; break;
	case 'd': onMoveKeyPress_R = true; /*MoveLock_L = true;*/ player.direction = 1; break;
	case '\040': if (player_jump == false) { player_jump = true; } break;
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
	glutTimerFunc(15, timer, 0);

	if (player_jump == true)
	{
		jump_timer++;
	}

	if (onMoveKeyPress_L == true || onMoveKeyPress_R == true) //歩きアニメーションのため （画像１→２→３→２というふうに歩き中には４枚の画像を連続で表示する）
	{
		if (walk_timer100 == 300)
		{
			walk_timer100 = 0;
		}

		else
		{
			walk_timer100 += 100;
		}
	}

	else
	{
		walk_timer100 = 0;
	}

	walk_timer = walk_timer100 / 100;
}


int main(int argc, char *argv[])
{
	atexit(end);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow("goipachi ver.0.0.0");
	glutDisplayFunc(display);
	glutTimerFunc(15, timer, 0);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutIdleFunc(idle);

	Init();
	glutMainLoop();
	return 0;
}