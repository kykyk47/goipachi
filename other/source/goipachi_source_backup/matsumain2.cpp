////小宅用
//
//#include "pch.h"
//#include <Windows.h>
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
#include <math.h>
#include <Windows.h>
#include <vector>


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
bool onMoveKeyPress_L = false;
bool onMoveKeyPress_R = false;
bool player_jump = false;
bool player_walk = false;
int jump_timer = 0;  //キャラクターがジャンプしてからの時間を計測
int walk_timer = 0;
int walk_timer100 = 0;

double camera_x = 0; //カメラの位置
double camera_y = 0;
void LoadImagePNG(const wchar_t* filename, GLuint &texture);

struct Position
{
	double x, y;
	int direction;
};

Position player = {};



class Coordinate {
public :
	double x, y;

	Coordinate(double x, double y) : x(x), y(y) {
		std::cout << "CORDINATE コンストラクタ" << std::endl;
	};

	Coordinate() { x = 0; y = 0; };

	void Print() {
		std::cout << "x = " << x << ", y = " << y << std::endl;
	};
	template <typename T>
	Coordinate operator+(T right) {
		return Coordinate{ x + right, y + right};
	};
	template <typename T>
	Coordinate operator-(T right) {
		return Coordinate{ x - right, y - right};
	};
	template <typename T>
	Coordinate operator*(T right) {
		return Coordinate{ x * right, y * right};
	};
	template <typename T>
	Coordinate operator/(T right) {
		return Coordinate{ x / right, y / right };
	};
	Coordinate operator+(Coordinate right) {
		return Coordinate{ x + right.x, y + right.y };
	};
	Coordinate operator-(Coordinate right) {
		return Coordinate{ x - right.x, y - right.y };
	};
	Coordinate operator*(Coordinate right) {
		return Coordinate{ x * right.x, y * right.y };
	};
	Coordinate operator/(Coordinate right) {
		return Coordinate{ x / right.x, y / right.y };
	};
	Coordinate operator=(Coordinate right) {
		return Coordinate{ right.x, right.y };
	};



};

int grid = 64;
class Object :Coordinate{
public:
	Coordinate center; //配置したい場所
	GLuint tex;
	Coordinate right; 
	Coordinate left;//center-grid/2
	Coordinate up;
	Coordinate bottom;
	Coordinate left_u;
	Coordinate right_u;
	Coordinate right_b;
	Coordinate left_b;
	const wchar_t* file;
	Object(double x, double y, const wchar_t* filename) {
		//std::cout << "コンストラクタ" << std::endl;
		center.x = x;
		center.y = y;

		right.x = center.x + (double)grid / 2;
		right.y = center.y;
		left.x = center.x - (double)grid / 2;
		left.y = center.y;
		up.x = center.x;
		up.y = center.y - (double)grid / 2;
		bottom.x = center.x;
		bottom.y = center.y + (double)grid / 2;
	

		left_b.x = center.x + (double)grid / 2;
		left_b.y = center.y + (double)grid / 2;
		left_u.x = center.x + (double)grid / 2;
		left_u.y = center.y - (double)grid / 2;
		right_u.x = center.x - (double)grid / 2;
		right_u.y = center.y - (double)grid / 2;
		right_b.x = center.x - (double)grid / 2;
		right_b.y = center.y + (double)grid / 2;
		

		
		file = filename;
		//LoadImagePNG2(filename, tex);
		//SetImage2();
		//std::cout << "クラスのやつ" << std::endl;
	};
	Object() {
		std::cout << "コンストラクタ(変数ナシ)" << std::endl;
		center.x = 0;
		center.y = 0;
	};
	void LoadImagePNG2(const wchar_t* filename, GLuint &texture) {
		GdiplusStartup(&gdiPT, &gdiPSI, NULL);
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
		//printf("%ls\n", filename);
		//std::cout << "afsafdsafsdafdsafa" << std::endl;
	}
	void SetImage(GLuint tex) {
		//std::cout << "SET IMAGE!!" << std::endl;
		if (center.x <= camera_x + 500 && center.x >= camera_x - 1500) {
			//std::cout << "SETIMAGE" << std::endl;
			glBindTexture(GL_TEXTURE_2D, tex);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_ALPHA_TEST);
			glBegin(GL_POLYGON);
			glTexCoord2f(0.0f, 1.0f); glVertex2d(left_b.x, left_b.y);//左下
			glTexCoord2f(0.0f, 0.0f); glVertex2d(left_u.x, left_u.y);//左上
			glTexCoord2f(1.0f, 0.0f); glVertex2d(right_u.x, right_u.y);//右上
			glTexCoord2f(1.0f, 1.0f); glVertex2d(right_b.x, right_b.y);//右下
			glEnd();
			glDisable(GL_ALPHA_TEST);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);
		}
	};
	void SetImage(double x, double y, GLuint tex) {
		//std::cout << "SET IMAGE!!" << std::endl;
		center.x = x;
		center.y = y;
		right.x = center.x + (double)grid / 2;
		right.y = center.y;
		left.x = center.x - (double)grid / 2;
		left.y = center.y;
		up.x = center.x;
		up.y = center.y - (double)grid / 2;
		bottom.x = center.x;
		bottom.y = center.y + (double)grid / 2;
		left_u.x = center.x + (double)grid / 2;
		left_u.y = center.y - (double)grid / 2;
		right_u.x = center.x - (double)grid / 2;
		right_u.y = center.y - (double)grid / 2;
		right_b.x = center.x - (double)grid / 2;
		right_b.y = center.y + (double)grid / 2;
		left_b.x = center.x + (double)grid / 2;
		left_b.y = center.y + (double)grid / 2;
		if (center.x <= camera_x + 500 && center.x >= camera_x - 1500) {
			//std::cout << "SETIMAGE" << std::endl;
			glBindTexture(GL_TEXTURE_2D, tex);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_ALPHA_TEST);
			glBegin(GL_POLYGON);
			glTexCoord2f(0.0f, 1.0f); glVertex2d(left_b.x, left_b.y);//左下
			glTexCoord2f(0.0f, 0.0f); glVertex2d(left_u.x, left_u.y);//左上
			glTexCoord2f(1.0f, 0.0f); glVertex2d(right_u.x, right_u.y);//右上
			glTexCoord2f(1.0f, 1.0f); glVertex2d(right_b.x, right_b.y);//右下
			glEnd();
			glDisable(GL_ALPHA_TEST);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);
		}
	};

	void Move(double x, double y) {
		Coordinate diff(x, y);
		diff.Print();
		center.x += x;
		center.y += y;
		right.x += x;
		right.y += y;
		left.x += x;
		left.y += y;
		up.x += x;
		up.y += y;
		bottom.x += x;
		bottom.y += y;
		left_u.x += x;
		left_u.y += y;
		right_u.x += x;
		right_u.y += y;
		right_b.x += x;
		right_b.x += y;
		left_b.x += x;
		left_b.y += y;
		Update();
	};
	void Update() {
		std::cout << "画像更新" << std::endl;
		if (center.x <= camera_x + 500 && center.x >= camera_x - 1500) {
			//std::cout << "SETIMAGE" << std::endl;
			glBindTexture(GL_TEXTURE_2D, tex);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_ALPHA_TEST);
			glBegin(GL_POLYGON);
			glTexCoord2f(0.0f, 1.0f); glVertex2d(left_b.x, left_b.y);//左下
			glTexCoord2f(0.0f, 0.0f); glVertex2d(left_u.x, left_u.y);//左上
			glTexCoord2f(1.0f, 0.0f); glVertex2d(right_u.x, right_u.y);//右上
			glTexCoord2f(1.0f, 1.0f); glVertex2d(right_b.x, right_b.y);//右下
			glEnd();
			glDisable(GL_ALPHA_TEST);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);
		}
	}
	void SizeChange(double x, double y) {

	}
	void ImageChange(GLuint tex_change) {
		//glBindTexture(GL_TEXTURE_2D, tex_change);
		tex = tex_change;
		glBindTexture(GL_TEXTURE_2D, tex);
	}

};

class GameObject{
public :
	double center_x = 0;
	double center_y = 0;
	double size_x = 0;
	double size_y = 0;
	const wchar_t *file;
	GLuint tex;
	GameObject(double x, double y, double size_x, double size_y, const wchar_t *filename)
		:center_x(x), center_y(y), size_x(size_x), size_y(size_y) {
		std::cout << "GameObjectコンストラクタ" << std::endl;
		file = filename;
		tex = -1;
		GdiplusStartup(&gdiPT, &gdiPSI, NULL);
		glEnable(GL_TEXTURE_2D);
		Bitmap bmp(filename);
		BitmapData data;
		bmp.LockBits(0, ImageLockModeRead, PixelFormat32bppARGB, &data);
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, data.Width, data.Height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, data.Scan0);
		glBindTexture(GL_TEXTURE_2D, 0);
		bmp.UnlockBits(&data);	
		Update();
	}
	void Update() {
		if (center_x <= camera_x + 500 && center_x >= camera_x - 1500) {
			glBindTexture(GL_TEXTURE_2D, tex);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_ALPHA_TEST);
			glBegin(GL_POLYGON);
			glTexCoord2f(0.0f, 1.0f); glVertex2d(center_x + (double)grid / 2, center_y + (double)grid / 2);//左下
			glTexCoord2f(0.0f, 0.0f); glVertex2d(center_x + (double)grid / 2, center_y - (double)grid / 2);//左上
			glTexCoord2f(1.0f, 0.0f); glVertex2d(center_x - (double)grid / 2, center_y - (double)grid / 2);//右上
			glTexCoord2f(1.0f, 1.0f); glVertex2d(center_x - (double)grid / 2, center_y + (double)grid / 2);//右下
			glEnd();
			glDisable(GL_ALPHA_TEST);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);
		}
	}

	void Set(double x, double y) {
		center_x = x;
		center_y = y;
		Update();
	}
};

class MoveObject:public GameObject {
public:
	MoveObject(double x, double y, double size_x, double size_y, const wchar_t *filename) 
		:GameObject(x, y, size_x, size_y, filename){
		std::cout << "MoveObjectのコンストラクタ" << std::endl;
	}
	void Move(double x, double y) {
		center_x += x;
		center_y += y;
		Update();
	}
};

class AnimationChara :public MoveObject {
public :
	std::vector<GLuint> texes;
	
	AnimationChara(double x, double y, double size_x, double size_y, const wchar_t *filename)
		:MoveObject(x, y, size_x, size_y, filename){
		std::cout << "AnimationObjectのコンストラクタ" << std::endl;
		texes.push_back(tex);
	}
	
	void LoadPNGImage(const wchar_t *filename) {
		GLuint texture = 0;
		GdiplusStartup(&gdiPT, &gdiPSI, NULL);
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
		Update();
		texes.push_back(texture);
		std::cout << texes.size() << std::endl;
	}

	void ChangeImage(const int num) {
		tex = texes.at(num);
		Update();
	}

	void UpdateDirR() {
		if (center_x <= camera_x + 500 && center_x >= camera_x - 1500) {
			glBindTexture(GL_TEXTURE_2D, tex);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_ALPHA_TEST);
			glBegin(GL_POLYGON);
			glTexCoord2f(0.0f, 1.0f); glVertex2d(center_x + (double)grid / 2, center_y + (double)grid / 2);//左下
			glTexCoord2f(0.0f, 0.0f); glVertex2d(center_x + (double)grid / 2, center_y - (double)grid / 2);//左上
			glTexCoord2f(1.0f, 0.0f); glVertex2d(center_x - (double)grid / 2, center_y - (double)grid / 2);//右上
			glTexCoord2f(1.0f, 1.0f); glVertex2d(center_x - (double)grid / 2, center_y + (double)grid / 2);//右下
			glEnd();
			glDisable(GL_ALPHA_TEST);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);
		}
	}
	void UpdateDirL() {
		if (center_x <= camera_x + 500 && center_x >= camera_x - 1500) {
			glBindTexture(GL_TEXTURE_2D, tex);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_ALPHA_TEST);
			glBegin(GL_POLYGON);
			glTexCoord2f(0.0f, 1.0f); glVertex2d(center_x - (double)grid / 2, center_y + (double)grid / 2);//左下
			glTexCoord2f(0.0f, 0.0f); glVertex2d(center_x - (double)grid / 2, center_y - (double)grid / 2);//左上
			glTexCoord2f(1.0f, 0.0f); glVertex2d(center_x + (double)grid / 2, center_y - (double)grid / 2);//右上
			glTexCoord2f(1.0f, 1.0f); glVertex2d(center_x + (double)grid / 2, center_y + (double)grid / 2);//右下
			glEnd();
			glDisable(GL_ALPHA_TEST);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);
		}
	}
};

Object floor2 = Object(50, 50, L"./pic/block1.png");
Object floor1 = Object(114, 50, L"./pic/block1.png");
Object player1 = Object(0, 0, L"./pic/walk1.png");
Object player2 = Object(0, 0, L"./pic/walk2.png");
Object player3 = Object(0, 0, L"./pic/walk3.png");
AnimationChara *player10;
AnimationChara *sample;

void end(){
	GdiplusShutdown(gdiPT);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
	gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

	if (walk_timer == 0)
	{
		glBindTexture(GL_TEXTURE_2D, player2.tex);
	}
	else
	{
		switch (walk_timer)
		{
		case 1:  sample->ChangeImage(2); break;//glBindTexture(GL_TEXTURE_2D, player3.tex); break;
		case 2:  sample->ChangeImage(1); break;//glBindTexture(GL_TEXTURE_2D, player2.tex); break;
		case 3:  sample->ChangeImage(0); break;//glBindTexture(GL_TEXTURE_2D, player1.tex); break;
		case 0:  sample->ChangeImage(1); break;// glBindTexture(GL_TEXTURE_2D, player2.tex); break;
		}
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glBegin(GL_POLYGON);
	switch (player.direction){
		case 0:{
			sample->UpdateDirL();break;
		}
		case 1:{
			sample->UpdateDirR();break;
		}
	}
	glEnd();
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	player10->Update();
	
	for (int i = -1000; i < 4; i++) {
		floor1.SetImage(i * 64, 50, floor1.tex);
	}
	glutSwapBuffers();
}

void idle(void)
{
	double speed = 2;

	if (onMoveKeyPress_L == true){
		camera_x += speed;
		sample->Move(speed, 0);	
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);
	}

	if (onMoveKeyPress_R == true){
		camera_x -= speed;
		sample->Move(-speed, 0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);
	}

	if (player_jump == true)
	{
		std::cout << "ジャンプ" << std::endl;
		if (jump_timer < 10){//ジャンプのときのプレイヤーの動き
			sample->Move(0, -(15.0 * jump_timer - 9.68* jump_timer * sqrt(jump_timer) *0.5)*0.01 * 7);
		}

		else if (jump_timer >= 10){ //10フレーム後落下
			sample->Move(0, -(15.0 * jump_timer - 9.68* jump_timer * sqrt(jump_timer) *0.5)*0.0015 * 7);
		}

		if (sample->center_y > 0){ //ブロックの上に着地する場合，その座標とする
			sample->Set(sample->center_x, 0);
			jump_timer = 0;
			player_jump = false;
		}
	}
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

void resize(int w, int h) {
	camera_x = (double)w / (double)2.0;
	camera_y = -544;
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

}

void Init() {
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
	GdiplusStartup(&gdiPT, &gdiPSI, NULL);
	player1.LoadImagePNG2(player1.file, player1.tex);
	player2.LoadImagePNG2(player2.file, player2.tex);
	player3.LoadImagePNG2(player3.file, player3.tex);
	floor2.LoadImagePNG2(floor2.file, floor2.tex);
	floor1.LoadImagePNG2(floor1.file, floor1.tex);
	player10 = new AnimationChara(10.0, -300.0, 64.0, 64.0, L"./pic/block1.png");
	player10->LoadPNGImage(L"./pic/arrow.png");
	player10->ChangeImage(1);
	player10->LoadPNGImage(L"./pic/walk1.png");
	player10->ChangeImage(0);
	sample = new AnimationChara(10.0, -20.0, 64.0, 64.0, L"./pic/walk1.png");
	sample->LoadPNGImage(L"./pic/walk2.png");
	sample->LoadPNGImage(L"./pic/walk3.png");
	player.x = 0;
	player.y = 0;
	player.direction = 1;

}

void timer(int value) {

	glutPostRedisplay();
	glutTimerFunc(50, timer, 0);

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
	glutInitWindowPosition(0, 0);
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