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
GLuint tex_ground;
GLuint tex_hiragana_01;
GLuint tex_highlight;

GLuint tex_arrow;
GLuint tex_title;
GLuint tex_btn_start;
GLuint tex_btn_matsu;
GLuint tex_btn_take;
GLuint tex_btn_ume;
GLuint tex_btn_3;
GLuint tex_btn_4;
GLuint tex_btn_5;
GLuint tex_btn_slow;
GLuint tex_btn_normal;
GLuint tex_btn_fast;
GLuint tex_char_hs;

GLuint tex_num_a0;
GLuint tex_num_a1;
GLuint tex_num_a2;
GLuint tex_num_a3;
GLuint tex_num_a4;
GLuint tex_num_a5;
GLuint tex_num_a6;
GLuint tex_num_a7;
GLuint tex_num_a8;
GLuint tex_num_a9;



bool onMoveKeyPress_L = false;
bool onMoveKeyPress_R = false;
bool player_jump = false;
bool player_walk = false;
int jump_timer = 0;  //キャラクターがジャンプしてからの時間を計測
int walk_timer = 0;
int walk_timer100 = 0;

int scene = 0; //◇シーンの追加．0:タイトル 1:ステージ選択 2:ポーズ画面 3:リザルト画面A 4リザルト画面B 5:プレイ画面 
int difficulty = 0; //◇難易度：松竹梅（+0~2）文字数（+00～20）はやさ（+000～200）
int difficulty_select = 0; //◇0：松竹梅選択 1：文字数選択 2：はやさ選択

double camera_x = 0; //カメラの位置
double camera_y = 0;

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

void SetImage(double x, double y, GLuint &tex) {

	if (x <=camera_x+500 && x >=camera_x-1500)
	{
		glBindTexture(GL_TEXTURE_2D, tex);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(0 + x, 64 + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(0 + x, 0 + y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(64 + x, 0 + y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(64 + x, 64 + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}
}

void SetButtonImage(double x, double y, int size_x, int size_y, GLuint &tex) {

	glBindTexture(GL_TEXTURE_2D, tex);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x + x, size_y + y);//左下
	glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x + x, y);//左上
	glTexCoord2f(1.0f, 0.0f); glVertex2d(x, y);//右上
	glTexCoord2f(1.0f, 1.0f); glVertex2d(x, size_y + y);//右下
	glEnd();
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	switch (scene)
	{
	case 0:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		SetButtonImage(-384, -480, 768, 256, tex_title);
		SetButtonImage(-128, 16, 256, 80, tex_btn_start);

	}break;

	case 1:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		SetButtonImage(124 - 160 * (difficulty % 10), -484, 136, 136, tex_highlight);
		SetButtonImage(124 - 160 * ((difficulty/10) % 10), -244, 136, 136, tex_highlight);
		SetButtonImage(124 - 160 * ((difficulty/100) % 10), -4, 136, 136, tex_highlight);

		SetButtonImage(128, -480, 128, 128, tex_btn_matsu);
		SetButtonImage(-32, -480, 128, 128, tex_btn_take);
		SetButtonImage(-192, -480, 128, 128, tex_btn_ume);

		SetButtonImage(128, -240, 128, 128, tex_btn_3);
		SetButtonImage(-32, -240, 128, 128, tex_btn_4);
		SetButtonImage(-192, -240, 128, 128, tex_btn_5);

		SetButtonImage(128, 0, 128, 128, tex_btn_slow);
		SetButtonImage(-32, 0, 128, 128, tex_btn_normal);
		SetButtonImage(-192, 0, 128, 128, tex_btn_fast);

		SetButtonImage(-512, -480, 256, 32, tex_char_hs);


	

		SetButtonImage(480, -480 + 240 * difficulty_select, 128, 128, tex_arrow);


	}break;

	case 5:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

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
			glTexCoord2f(0.0f, 1.0f); glVertex2d(0 + player.x, 0 + player.y);//左下
			glTexCoord2f(0.0f, 0.0f); glVertex2d(0 + player.x, -64 + player.y);//左上
			glTexCoord2f(1.0f, 0.0f); glVertex2d(64 + player.x, -64 + player.y);//右上
			glTexCoord2f(1.0f, 1.0f); glVertex2d(64 + player.x, 0 + player.y);//右下
			break;
		}

		case 1:
		{
			glTexCoord2f(0.0f, 1.0f); glVertex2d(64 + player.x, 0 + player.y);//左下
			glTexCoord2f(0.0f, 0.0f); glVertex2d(64 + player.x, -64 + player.y);//左上
			glTexCoord2f(1.0f, 0.0f); glVertex2d(0 + player.x, -64 + player.y);//右上
			glTexCoord2f(1.0f, 1.0f); glVertex2d(0 + player.x, 0 + player.y);//右下
			break;
		}
		}
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);


		for (int i = -1000; i < 4; i++) {
			SetImage(i * 64, 0, tex_ground);
		}
	}break;
	}

	glutSwapBuffers();
}

void idle(void)
{
	switch (scene)
	{
	case 5:
	{
		double speed = 0.125;

		if (onMoveKeyPress_L == true)
		{
			player.x += speed;
			camera_x += speed;
			//glLoadIdentity();
			gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);
		}

		if (onMoveKeyPress_R == true)
		{
			player.x -= speed;
			camera_x -= speed;
			//glLoadIdentity();
			gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);
		}

		if (player_jump == true)
		{
			if (jump_timer < 10)
			{
				player.y = player.y - (15.0 * jump_timer - 9.68* jump_timer * sqrt(jump_timer) *0.5)*0.01;//ジャンプのときのプレイヤーの動き
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
	}break;
	}


	//printf("%.2f,%.2f %d %d \n", player.x, player.y, jump_timer, walk_timer100);
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (scene)
	{
	case 0:
	{
		switch (key) {
		case 'q':
		case '\033':  /* '\033' は ESC の ASCII コード */
			exit(0); break;
		case 's': scene = 1; break; //PRESS START 次の画面へ

		default:
			break;
		}
	}break;

	case 1:
	{
		switch (key) {
		case '\033':  /* '\033' は ESC の ASCII コード */
			exit(0); break;

		case 'w':
		{
			if (difficulty_select!=0)
			{
				difficulty_select--;
			}

		}break;

		case 's':
		{
			if (difficulty_select != 2)
			{
				difficulty_select++;
			}

		}break;

		case 'a':
		{
			switch (difficulty_select)
			{
			case 0:
			{
				if (difficulty % 10 != 0)
				{
					difficulty--;
				}
			}break;
			case 1:
			{
				if ((difficulty/10) % 10 != 0)
				{
					difficulty-=10;
				}
			}break;
			case 2:
			{
				if ((difficulty / 100) % 10 != 0)
				{
					difficulty -= 100;
				}
			}break;
			
			}
		}break;

		case 'd':
		{
			switch (difficulty_select)
			{
			case 0:
			{
				if (difficulty % 10 != 2)
				{
					difficulty++;
				}
			}break;
			case 1:
			{
				if ((difficulty / 10) % 10 != 2)
				{
					difficulty += 10;
				}
			}break;
			case 2:
			{
				if ((difficulty / 100) % 10 != 2)
				{
					difficulty += 100;
				}
			}break;

			}
		}break;


		case 'l': scene = 5; break; //PRESS START 次の画面へ

		default:
			break;
		}
	}break;

	case 5:
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
	}break;
	}
}

void keyboardUp(unsigned char key, int x, int y)
{
	switch (scene)
	{
	case 5:
	{
		switch (key) {
		case 'a': onMoveKeyPress_L = false; break;
		case 'd': onMoveKeyPress_R = false; break;
		default:
			break;
		}
	}break;
	}
}

void resize(int w, int h) {

	switch(scene)
	{
	case 0: case 1: case 2: 
	{
		camera_x = (double)w / (double)2.0;
		camera_y = -544;
	}break;

	case 5:
	{
	camera_x = (double)w / (double)2.0;
	camera_y = -544;
	std::cout << camera_x << std::endl;
	std::cout << camera_y << std::endl;
	}break;
	}

	glViewport(0, 0, w, h);

	glLoadIdentity();
	glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);

	gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

}

void Init() {
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
	GdiplusStartup(&gdiPT, &gdiPSI, NULL); 
	LoadImagePNG(L"./pic/title_a.png", tex_title);
	LoadImagePNG(L"./pic/title_start.png", tex_btn_start);
	LoadImagePNG(L"./pic/button_matsu.png", tex_btn_matsu);
	LoadImagePNG(L"./pic/button_take.png", tex_btn_take);
	LoadImagePNG(L"./pic/button_ume.png", tex_btn_ume);
	LoadImagePNG(L"./pic/button_3.png", tex_btn_3);
	LoadImagePNG(L"./pic/button_4.png", tex_btn_4);
	LoadImagePNG(L"./pic/button_5.png", tex_btn_5);
	LoadImagePNG(L"./pic/button_slow.png", tex_btn_slow);
	LoadImagePNG(L"./pic/button_normal.png", tex_btn_normal);
	LoadImagePNG(L"./pic/button_fast.png", tex_btn_fast);
	LoadImagePNG(L"./pic/walk1.png", tex_player1);
	LoadImagePNG(L"./pic/walk2.png", tex_player2);
	LoadImagePNG(L"./pic/walk3.png", tex_player3);
	LoadImagePNG(L"./pic/ground.png", tex_ground);
	LoadImagePNG(L"./pic/block_hiragana_01.png", tex_hiragana_01);
	LoadImagePNG(L"./pic/arrow.png", tex_arrow);
	LoadImagePNG(L"./pic/Y.png", tex_highlight);
	LoadImagePNG(L"./pic/0.png", tex_num_a0);
	LoadImagePNG(L"./pic/1.png", tex_num_a1);
	LoadImagePNG(L"./pic/2.png", tex_num_a2);
	LoadImagePNG(L"./pic/3.png", tex_num_a3);
	LoadImagePNG(L"./pic/4.png", tex_num_a4);
	LoadImagePNG(L"./pic/5.png", tex_num_a5);
	LoadImagePNG(L"./pic/6.png", tex_num_a6);
	LoadImagePNG(L"./pic/7.png", tex_num_a7);
	LoadImagePNG(L"./pic/8.png", tex_num_a8);
	LoadImagePNG(L"./pic/9.png", tex_num_a9);
	LoadImagePNG(L"./pic/highscore.png", tex_char_hs);

	player.x = 0;
	player.y = 0;
	player.direction = 1;
	scene = 0;

}

void timer(int value) {

	glutPostRedisplay();
	glutTimerFunc(50, timer, 0);
	printf("[%03d]\n",difficulty);

	switch (scene)
	{
	case 5:
	{

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
	}break;
	}
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
	glutReshapeFunc(resize);
	glutTimerFunc(50, timer, 0);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutIdleFunc(idle);
	

	Init();
	glutMainLoop();
	return 0;
}