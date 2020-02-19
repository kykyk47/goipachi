////小宅用

#include "pch.h"
#include <Windows.h>
#include <iostream>
#include <gl/glut.h>
#include <math.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <gdiplus.h>
#include <vector>


//松本用
/*
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <gdiplus.h>
#include <gl/glut.h>
#include <iostream>
#include <math.h>
#include <Windows.h>
#include <vector>
*/


using namespace Gdiplus;

#define WIDTH 1280
#define HEIGHT 720

#define TIME_LIMIT 100
#define dic_index_4 43226 //4文字辞書の単語数


GdiplusStartupInput gdiPSI;
ULONG_PTR gdiPT;
GLuint tex_player1;
GLuint tex_player2;
GLuint tex_player3;
GLuint tex_ground;
GLuint tex_hiragana_01;

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

GLuint tex_num_b0;
GLuint tex_num_b1;
GLuint tex_num_b2;
GLuint tex_num_b3;
GLuint tex_num_b4;
GLuint tex_num_b5;
GLuint tex_num_b6;
GLuint tex_num_b7;
GLuint tex_num_b8;
GLuint tex_num_b9;
GLuint tex_num_b_plus;


bool onMoveKeyPress_L = false;
bool onMoveKeyPress_R = false;
bool player_jump = false;
bool player_walk = false;
int jump_timer = 0;  //キャラクターがジャンプしてからの時間を計測
int walk_timer = 0;
int walk_timer100 = 0;
int lamp_timer_01 = 0; //★Ｋキー（決定ボタン）を押した後の赤ライトの点灯
int lamp_timer_02 = 0; //★プレイヤーのリアクションのエフェクト

int scene = 0; //◇シーンの追加．0:タイトル 1:ステージ選択 2:ポーズ画面 3:リザルト画面A 4リザルト画面B 5:プレイ画面 6:GAME OVER画面
int difficulty = 0; //◇難易度：松竹梅（+0~2）文字数（+00～20）はやさ（+000～200）
int difficulty_select = 0; //◇0：松竹梅選択 1：文字数選択 2：はやさ選択

int dic[dic_index_4][4] = { {} };
int score_get_hiragana = 0;
int score_leave_hiragana = 0;
int score_most_hiragana = 0;
int score_tango = 0;
int score_enemy = 0;
int score_cleared = 0;
int score = 0; //ゲーム中でのスコア


int high_score[5] = { 0,0,0,0,0 }; //レコードされているハイスコア
int slot[5] = { 0,0,0,0,0 }; //★スロット（のちのち5文字でもできるように）
int slot_select = 0; //★どの文字をさしているか
int score_word = 0; //１つ１つのワードのスコア
int time = TIME_LIMIT*60;

FILE *fp; //スコアファイル
FILE *fp_dic_4; //辞書ファイル
int dic_4_all = 0;
bool word_hit = false; //Ｋキー押下後，辞書に存在していたかどうか(trueなら得点＋エフェクト，falseなら時間減少＋エフェクト）

int i, j, k;

double camera_x = 0; //カメラの位置
double camera_y = 0;
double temp_camera_x = 0; //◇シーン移動によりカメラの位置をリセットするため，ゲーム中のカメラ位置をストックしておく
double temp_camera_y = 0;
//void LoadImagePNG(const wchar_t* filename, GLuint &texture);

bool flag_01 = true; //★次のタイマー関数が呼び出されるまでx方向の移動を与えないための制御
bool flag_02 = true; //★次のタイマー関数が呼び出されるまでy方向の移動を与えないための制御
int time_1flame; //★デバッグ用，1フレームでどれだけ進んだか，どれだけ時間がたっているか（ms）
double speed_1flame;
int time_temp;
double speed_temp;

struct Position
{
	double x, y;
	int direction;
};

Position player = {};



class Coordinate {
public:
	double x, y;

	Coordinate(double x, double y) : x(x), y(y) {
		//std::cout << "CORDINATE コンストラクタ" << std::endl;
	};

	Coordinate() { x = 0; y = 0; };

	void Print() {
		std::cout << "x = " << x << ", y = " << y << std::endl;
	};
	template <typename T>
	Coordinate operator+(T right) {
		return Coordinate{ x + right, y + right };
	};
	template <typename T>
	Coordinate operator-(T right) {
		return Coordinate{ x - right, y - right };
	};
	template <typename T>
	Coordinate operator*(T right) {
		return Coordinate{ x * right, y * right };
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
//class Object;
int grid = 64;
//void SetImage1(Object obj, GLuint &tex);

/*
class Object :Coordinate {
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
	void LoadImagePNG2(const wchar_t* filename, GLuint &texture) 
	{
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
*/

class GameObject {
public:
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
		tex = *filename; //★
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

	double Get_x() {
		return center_x;
	}
	double Get_y() {
		return center_y;
	}

	void LoadImagePNG2(const wchar_t* filename, GLuint &texture) //★追加
	{
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
	void SetImage(double center_x, double center_y) { 
		//std::cout << "SET IMAGE!!" << std::endl;
		if (center_x <= camera_x + 1800 && center_x >= camera_x - 1800) {
			//std::cout << "SETIMAGE" << std::endl;
			glBindTexture(GL_TEXTURE_2D, tex);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_ALPHA_TEST);
			glBegin(GL_POLYGON);
			glTexCoord2f(0.0f, 1.0f); glVertex2d(center_x + size_x / 2, center_y + size_y / 2);//右下
			glTexCoord2f(0.0f, 0.0f); glVertex2d(center_x + size_x / 2, center_y - size_y / 2);//右上
			glTexCoord2f(1.0f, 0.0f); glVertex2d(center_x - size_x / 2, center_y - size_y / 2);//←上
			glTexCoord2f(1.0f, 1.0f); glVertex2d(center_x - size_x / 2, center_y + size_y / 2);//←下
			glEnd();
			glDisable(GL_ALPHA_TEST);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);
		}
	};
	
};

class MoveObject :public GameObject {
public:
	MoveObject(double x, double y, double size_x, double size_y, const wchar_t *filename)
		:GameObject(x, y, size_x, size_y, filename) {
		std::cout << "MoveObjectのコンストラクタ" << std::endl;
	}
	void Move(double x, double y) {
		center_x += x;
		center_y += y;
		Update();
	}
};

class AnimationChara :public MoveObject {
public:
	std::vector<GLuint> texes;

	AnimationChara(double x, double y, double size_x, double size_y, const wchar_t *filename)
		:MoveObject(x, y, size_x, size_y, filename) {
		std::cout << "AnimationObjectのコンストラクタ" << std::endl;
		texes.push_back(tex);
		//texes.resize(num);
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
		//glTexCoord2f(0.0f, 1.0f); glVertex2d(64 + player2.center.x, 0 + player2.center.y);//左下
//glTexCoord2f(0.0f, 0.0f); glVertex2d(64 + player2.center.x, -64 + player2.center.y);//左上
//glTexCoord2f(1.0f, 0.0f); glVertex2d(0 + player2.center.x, -64 + player2.center.y);//右上
//glTexCoord2f(1.0f, 1.0f); glVertex2d(0 + player2.center.x, 0 + player2.center.y);//右下
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
			//glTexCoord2f(0.0f, 1.0f); glVertex2d(0 + player2.center.x, 0 + player2.center.y);//左下
//glTexCoord2f(0.0f, 0.0f); glVertex2d(0 + player2.center.x, -64 + player2.center.y);//左上
//glTexCoord2f(1.0f, 0.0f); glVertex2d(64 + player2.center.x, -64 + player2.center.y);//右上
//glTexCoord2f(1.0f, 1.0f); glVertex2d(64 + player2.center.x, 0 + player2.center.y);//右下
		}
	}



};

//GameObject floor2 = GameObject(0, 0, 64,64, L"./pic/ground.png");
GameObject floor1 = GameObject(0, 0, 64,64, L"./pic/ground.png");
GameObject player1 = GameObject(0, 0, 64, 64, L"./pic/player_walk1.png");
GameObject player2 = GameObject(0, 0, 64, 64, L"./pic/player_walk2.png");
GameObject player3 = GameObject(0, 0, 64, 64, L"./pic/player_walk3.png");
GameObject UI_title = GameObject(0,0,768,256, L"./pic/title_a.png");
//GameObject title_start = GameObject(0, 0, , , L"./pic/.png");
GameObject UI_gamestart= GameObject(0, 0, 512, 64, L"./pic/game_start.png");
GameObject UI_pressstart = GameObject(0, 0, 128, 64, L"./pic/press.png");
GameObject select_matsu= GameObject(0, 0, 128, 128, L"./pic/button_matsu.png");
GameObject select_take = GameObject(0, 0, 128, 128, L"./pic/button_take.png");
GameObject select_ume = GameObject(0, 0, 128, 128, L"./pic/button_ume.png");
GameObject select_3 = GameObject(0, 0, 128, 128, L"./pic/button_3.png");
GameObject select_4 = GameObject(0, 0, 128, 128, L"./pic/button_4.png");
GameObject select_5 = GameObject(0, 0, 128, 128, L"./pic/button_5.png");
GameObject select_slow = GameObject(0, 0, 128, 128, L"./pic/button_slow.png");
GameObject select_normal = GameObject(0, 0, 128, 128, L"./pic/button_normal.png");
GameObject select_fast = GameObject(0, 0, 128, 128, L"./pic/button_fast.png");
GameObject block_alphabet_p= GameObject(0, 0, 128, 128, L"./pic/block_alphabet_p.png");
GameObject block_alphabet_a = GameObject(0, 0, 128, 128, L"./pic/block_alphabet_a.png");
GameObject block_alphabet_u = GameObject(0, 0, 128, 128, L"./pic/block_alphabet_u.png");
GameObject block_alphabet_s = GameObject(0, 0, 128, 128, L"./pic/block_alphabet_s.png");
GameObject block_alphabet_e = GameObject(0, 0, 128, 128, L"./pic/block_alphabet_e.png");
GameObject UI_return = GameObject(0, 0, 768, 96, L"./pic/menu_return.png");
GameObject UI_highscore= GameObject(0, 0, 256, 32, L"./pic/highscore.png");
GameObject UI_gameover= GameObject(0, 0, 1024, 128, L"./pic/gameover.png");
GameObject UI_tonext = GameObject(0, 0, 1024, 128, L"./pic/menu_tonext01.png");
GameObject UI_difficulty01= GameObject(0, 0, 256, 64, L"./pic/difficulty_menu01.png");
GameObject UI_difficulty02 = GameObject(0, 0, 256, 64, L"./pic/difficulty_menu02.png");
GameObject UI_difficulty03 = GameObject(0, 0, 256, 64, L"./pic/difficulty_menu03.png");
GameObject UI_07= GameObject(0, 0, 384, 192, L"./pic/menu_UI_07.png");
GameObject UI_08 = GameObject(0, 0, 512, 256, L"./pic/menu_UI_08.png");
GameObject UI_09 = GameObject(0, 0, 1024, 64, L"./pic/menu_UI_09.png");
GameObject UI_result01= GameObject(0, 0, 384, 48, L"./pic/result_01.png");
GameObject UI_result02 = GameObject(0, 0, 384, 48, L"./pic/result_02.png");
GameObject UI_result03 = GameObject(0, 0, 384, 48, L"./pic/result_03.png");
GameObject UI_result04 = GameObject(0, 0, 384, 48, L"./pic/result_04.png");
GameObject UI_score= GameObject(0, 0, 128, 32, L"./pic/score.png");
GameObject UI_newrecord= GameObject(0, 0, 512, 256, L"./pic/result_newrecord.png");
GameObject UI_ranking= GameObject(0, 0, 384, 192, L"./pic/result_ranking.png");
GameObject UI_rank01= GameObject(0, 0, 80, 40, L"./pic/ranking_01.png");
GameObject UI_rank02 = GameObject(0, 0, 80, 40, L"./pic/ranking_02.png");
GameObject UI_rank03 = GameObject(0, 0, 80, 40, L"./pic/ranking_03.png");
GameObject UI_rank04 = GameObject(0, 0, 80, 40, L"./pic/ranking_04.png");
GameObject UI_rank05 = GameObject(0, 0, 80, 40, L"./pic/ranking_05.png");
GameObject BG_01 = GameObject(0, 0, 1024, 512, L"./pic/bg01.png");
GameObject BG_02 = GameObject(0, 0, 1024, 512, L"./pic/bg02.png");
GameObject BG_03 = GameObject(0, 0, 1024, 512, L"./pic/bg03.png");
GameObject BG_04 = GameObject(0, 0, 1024, 512, L"./pic/bg04.png");
GameObject BG_05 = GameObject(0, 0, 1024, 512, L"./pic/bg05.png");
GameObject UI_10 = GameObject(0, 0, 384, 192, L"./pic/menu_UI_10.png");
GameObject UI_11= GameObject(0, 0, 256, 64, L"./pic/menu_UI_11.png");
GameObject UI_12 = GameObject(0, 0, 384, 192, L"./pic/menu_UI_12.png");
GameObject UI_slot_base = GameObject(0, 0, 768, 192, L"./pic/slot_base.png");
GameObject UI_slot_highlight = GameObject(0, 0, 192, 192, L"./pic/slot_highlight.png");
GameObject UI_slot_decision = GameObject(0, 0, 192, 192, L"./pic/slot_decision.png");

//GameObject block_hiragana_01 = GameObject(0, 0, 64, 64, L"./pic/.png");

GameObject arrow = GameObject(0, 0, 128, 128, L"./pic/arrow.png");
GameObject select_highlight = GameObject(0, 0, 136, 136, L"./pic/Y.png");

GameObject player_penalty = GameObject(0, 0, 96, 24, L"./pic/player_penalty.png");
GameObject player_maru = GameObject(0, 0, 64, 64, L"./pic/maru.png");
GameObject player_batsu = GameObject(0, 0, 64, 64, L"./pic/batsu.png");
GameObject player_fukidashi01 = GameObject(0, 0, 64, 64, L"./pic/fukidashi1.png");
GameObject player_fukidashi02 = GameObject(0, 0, 64, 64, L"./pic/fukidashi2.png");

GameObject block_hiragana[76] = { 
GameObject(0, 0, 96, 96, L"./pic/block_blank.png"), //空白
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_01.png"), //あ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_02.png"), //い
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_03.png"), //う
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_04.png"), //え
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_05.png"), //お
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_06.png"), //か
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_07.png"), //き
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_08.png"), //く
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_09.png"), //け
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_10.png"), //こ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_11.png"), //さ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_12.png"), //し
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_13.png"), //す
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_14.png"), //せ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_15.png"), //そ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_16.png"), //た
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_17.png"), //ち
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_18.png"), //つ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_19.png"), //て
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_20.png"), //と
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_21.png"), //な
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_22.png"), //に
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_23.png"), //ぬ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_24.png"), //ね
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_25.png"), //の
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_26.png"), //は
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_27.png"), //ひ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_28.png"), //ふ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_29.png"), //へ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_30.png"), //ほ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_31.png"), //ま
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_32.png"), //み
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_33.png"), //む
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_34.png"), //め
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_35.png"), //も
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_36.png"), //や
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_37.png"), //ゆ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_38.png"), //よ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_39.png"), //ら
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_40.png"), //り
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_41.png"), //る
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_42.png"), //れ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_43.png"), //ろ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_44.png"), //わ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_45.png"), //を
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_46.png"), //ん
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_73.png"), //っ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_74.png"), //ヴ
GameObject(0, 0, 96, 96, L"./pic/block_blank.png"), //未使用（お題箱？）４９は未使用
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_47.png"), //ー
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_48.png"), //が
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_49.png"), //ぎ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_50.png"), //ぐ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_51.png"), //げ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_52.png"), //ご
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_53.png"), //ざ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_54.png"), //じ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_55.png"), //ず
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_56.png"), //ｚｗ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_57.png"), //ぞ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_58.png"), //だ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_59.png"), //ぢ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_60.png"), //づ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_61.png"), //で
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_62.png"), //ど
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_63.png"), //ば
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_64.png"), //び
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_65.png"), //ぶ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_66.png"), //べ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_67.png"), //ぼ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_68.png"), //ぱ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_69.png"), //ぴ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_70.png"), //ぷ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_71.png"), //ぺ
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_72.png") //ぽ
};



//floor2.LoadImagePNG2(L"./pic/block.png");
//GameObject *gameObject;
//GameObject *gameObject2;
//AnimationChara *player10;
AnimationChara *sample;



//void SetImage1(Object obj, GLuint &tex) {
//	if (obj.center.x <= camera_x + 500 && obj.center.x >= camera_x - 1500){
//		glBindTexture(GL_TEXTURE_2D, tex);
//		glEnable(GL_BLEND);
//		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//		glEnable(GL_TEXTURE_2D);
//		glEnable(GL_ALPHA_TEST);
//		glBegin(GL_POLYGON);
//		//glTexCoord2f(0.0f, 1.0f); glVertex2d(obj.left_b.x, obj.left_b.y);//左下
//		//glTexCoord2f(0.0f, 0.0f); glVertex2d(obj.left_u.x, obj.left_u.y);//左上
//		//glTexCoord2f(1.0f, 0.0f); glVertex2d(obj.right_u.x, obj.right_u.y);//右上
//		//glTexCoord2f(1.0f, 1.0f); glVertex2d(obj.right_b.x, obj.right_b.y);//右下
//		glTexCoord2f(1.0f, 1.0f); glVertex2d(obj.right_b.x, obj.right_b.y);//右下
//		glTexCoord2f(1.0f, 0.0f); glVertex2d(obj.right_u.x, obj.right_u.y);//右上
//		glTexCoord2f(0.0f, 0.0f); glVertex2d(obj.left_u.x, obj.left_u.y);//左上
//		glTexCoord2f(0.0f, 1.0f); glVertex2d(obj.left_b.x, obj.left_b.y);//左下
//		glEnd();
//		glDisable(GL_ALPHA_TEST);
//		glDisable(GL_TEXTURE_2D);
//		glDisable(GL_BLEND);
//	}
//}


void end() {
	GdiplusShutdown(gdiPT);
}



void check_goi(int moji[])
{
	printf("slot=[%d,%d,%d,%d]\n\n", moji[0], moji[1], moji[2], moji[3]);

	printf("dic_4_all = %d\n", dic_4_all);

	for (i = 0; i < dic_4_all; i++)
	{
		if (moji[0] == dic[i][0] && moji[1] == dic[i][1] && moji[2] == dic[i][2] && moji[3] == dic[i][3])
		{
			printf("辞書番号%dと一致：%d点獲得\n", i, 0);
			word_hit = true;
			score_word = 45;

			score += score_word;

			break;
		}
		
	}
	if (word_hit == false)
	{
		time -= 3000;
		printf("辞書と一致せず\n");
	}
}


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

void SetNumImage(double x, double y, int size_x, int size_y, int num) {

	if (num >= 100000) //10万の位
	{
		switch (num / 100000)
		{
		case 1: {glBindTexture(GL_TEXTURE_2D, tex_num_a1); }break;
		case 2: {glBindTexture(GL_TEXTURE_2D, tex_num_a2); }break;
		case 3: {glBindTexture(GL_TEXTURE_2D, tex_num_a3); }break;
		case 4: {glBindTexture(GL_TEXTURE_2D, tex_num_a4); }break;
		case 5: {glBindTexture(GL_TEXTURE_2D, tex_num_a5); }break;
		case 6: {glBindTexture(GL_TEXTURE_2D, tex_num_a6); }break;
		case 7: {glBindTexture(GL_TEXTURE_2D, tex_num_a7); }break;
		case 8: {glBindTexture(GL_TEXTURE_2D, tex_num_a8); }break;
		case 9: {glBindTexture(GL_TEXTURE_2D, tex_num_a9); }break;
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 6 + x + 20, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 6 + x + 20, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 5 + x + 20, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 5 + x + 20, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	if (num >= 10000) //1万の位
	{
		switch ((num / 10000) % 10)
		{
		case 0: {glBindTexture(GL_TEXTURE_2D, tex_num_a0); }break;
		case 1: {glBindTexture(GL_TEXTURE_2D, tex_num_a1); }break;
		case 2: {glBindTexture(GL_TEXTURE_2D, tex_num_a2); }break;
		case 3: {glBindTexture(GL_TEXTURE_2D, tex_num_a3); }break;
		case 4: {glBindTexture(GL_TEXTURE_2D, tex_num_a4); }break;
		case 5: {glBindTexture(GL_TEXTURE_2D, tex_num_a5); }break;
		case 6: {glBindTexture(GL_TEXTURE_2D, tex_num_a6); }break;
		case 7: {glBindTexture(GL_TEXTURE_2D, tex_num_a7); }break;
		case 8: {glBindTexture(GL_TEXTURE_2D, tex_num_a8); }break;
		case 9: {glBindTexture(GL_TEXTURE_2D, tex_num_a9); }break;
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 5 + x + 16, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 5 + x + 16, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 4 + x + 16, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 4 + x + 16, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	if (num >= 1000) //1000の位
	{
		switch ((num / 1000) % 10)
		{
		case 0: {glBindTexture(GL_TEXTURE_2D, tex_num_a0); }break;
		case 1: {glBindTexture(GL_TEXTURE_2D, tex_num_a1); }break;
		case 2: {glBindTexture(GL_TEXTURE_2D, tex_num_a2); }break;
		case 3: {glBindTexture(GL_TEXTURE_2D, tex_num_a3); }break;
		case 4: {glBindTexture(GL_TEXTURE_2D, tex_num_a4); }break;
		case 5: {glBindTexture(GL_TEXTURE_2D, tex_num_a5); }break;
		case 6: {glBindTexture(GL_TEXTURE_2D, tex_num_a6); }break;
		case 7: {glBindTexture(GL_TEXTURE_2D, tex_num_a7); }break;
		case 8: {glBindTexture(GL_TEXTURE_2D, tex_num_a8); }break;
		case 9: {glBindTexture(GL_TEXTURE_2D, tex_num_a9); }break;
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 4 + x + 12, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 4 + x + 12, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 3 + x + 12, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 3 + x + 12, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	if (num >= 100) //100の位
	{
		switch ((num / 100) % 10)
		{
		case 0: {glBindTexture(GL_TEXTURE_2D, tex_num_a0); }break;
		case 1: {glBindTexture(GL_TEXTURE_2D, tex_num_a1); }break;
		case 2: {glBindTexture(GL_TEXTURE_2D, tex_num_a2); }break;
		case 3: {glBindTexture(GL_TEXTURE_2D, tex_num_a3); }break;
		case 4: {glBindTexture(GL_TEXTURE_2D, tex_num_a4); }break;
		case 5: {glBindTexture(GL_TEXTURE_2D, tex_num_a5); }break;
		case 6: {glBindTexture(GL_TEXTURE_2D, tex_num_a6); }break;
		case 7: {glBindTexture(GL_TEXTURE_2D, tex_num_a7); }break;
		case 8: {glBindTexture(GL_TEXTURE_2D, tex_num_a8); }break;
		case 9: {glBindTexture(GL_TEXTURE_2D, tex_num_a9); }break;
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 3 + x + 8, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 3 + x + 8, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 2 + x + 8, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 2 + x + 8, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	if (num >= 10) //10の位
	{
		switch ((num / 10) % 10)
		{
		case 0: {glBindTexture(GL_TEXTURE_2D, tex_num_a0); }break;
		case 1: {glBindTexture(GL_TEXTURE_2D, tex_num_a1); }break;
		case 2: {glBindTexture(GL_TEXTURE_2D, tex_num_a2); }break;
		case 3: {glBindTexture(GL_TEXTURE_2D, tex_num_a3); }break;
		case 4: {glBindTexture(GL_TEXTURE_2D, tex_num_a4); }break;
		case 5: {glBindTexture(GL_TEXTURE_2D, tex_num_a5); }break;
		case 6: {glBindTexture(GL_TEXTURE_2D, tex_num_a6); }break;
		case 7: {glBindTexture(GL_TEXTURE_2D, tex_num_a7); }break;
		case 8: {glBindTexture(GL_TEXTURE_2D, tex_num_a8); }break;
		case 9: {glBindTexture(GL_TEXTURE_2D, tex_num_a9); }break;
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 2 + x + 4, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 2 + x + 4, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 1 + x + 4, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 1 + x + 4, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}


	switch (num % 10) // 1の位
	{
	case 0: {glBindTexture(GL_TEXTURE_2D, tex_num_a0); }break;
	case 1: {glBindTexture(GL_TEXTURE_2D, tex_num_a1); }break;
	case 2: {glBindTexture(GL_TEXTURE_2D, tex_num_a2); }break;
	case 3: {glBindTexture(GL_TEXTURE_2D, tex_num_a3); }break;
	case 4: {glBindTexture(GL_TEXTURE_2D, tex_num_a4); }break;
	case 5: {glBindTexture(GL_TEXTURE_2D, tex_num_a5); }break;
	case 6: {glBindTexture(GL_TEXTURE_2D, tex_num_a6); }break;
	case 7: {glBindTexture(GL_TEXTURE_2D, tex_num_a7); }break;
	case 8: {glBindTexture(GL_TEXTURE_2D, tex_num_a8); }break;
	case 9: {glBindTexture(GL_TEXTURE_2D, tex_num_a9); }break;
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 1 + x, size_y + y);//左下
	glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 1 + x, y);//左上
	glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 0 + x, y);//右上
	glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 0 + x, size_y + y);//右下
	glEnd();
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);


}

void SetNumImage_2(double x, double y, int size_x, int size_y, int num) { //プレイヤーエフェクトとしての数字

	if (num >= 100000) //10万の位
	{
		switch (num / 100000)
		{
		case 1: {glBindTexture(GL_TEXTURE_2D, tex_num_b1); }break;
		case 2: {glBindTexture(GL_TEXTURE_2D, tex_num_b2); }break;
		case 3: {glBindTexture(GL_TEXTURE_2D, tex_num_b3); }break;
		case 4: {glBindTexture(GL_TEXTURE_2D, tex_num_b4); }break;
		case 5: {glBindTexture(GL_TEXTURE_2D, tex_num_b5); }break;
		case 6: {glBindTexture(GL_TEXTURE_2D, tex_num_b6); }break;
		case 7: {glBindTexture(GL_TEXTURE_2D, tex_num_b7); }break;
		case 8: {glBindTexture(GL_TEXTURE_2D, tex_num_b8); }break;
		case 9: {glBindTexture(GL_TEXTURE_2D, tex_num_b9); }break;
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 6 + x + 20, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 6 + x + 20, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 5 + x + 20, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 5 + x + 20, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	if (num >= 10000) //1万の位
	{
		switch ((num / 10000) % 10)
		{
		case 0: {glBindTexture(GL_TEXTURE_2D, tex_num_b0); }break;
		case 1: {glBindTexture(GL_TEXTURE_2D, tex_num_b1); }break;
		case 2: {glBindTexture(GL_TEXTURE_2D, tex_num_b2); }break;
		case 3: {glBindTexture(GL_TEXTURE_2D, tex_num_b3); }break;
		case 4: {glBindTexture(GL_TEXTURE_2D, tex_num_b4); }break;
		case 5: {glBindTexture(GL_TEXTURE_2D, tex_num_b5); }break;
		case 6: {glBindTexture(GL_TEXTURE_2D, tex_num_b6); }break;
		case 7: {glBindTexture(GL_TEXTURE_2D, tex_num_b7); }break;
		case 8: {glBindTexture(GL_TEXTURE_2D, tex_num_b8); }break;
		case 9: {glBindTexture(GL_TEXTURE_2D, tex_num_b9); }break;
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 5 + x + 16, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 5 + x + 16, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 4 + x + 16, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 4 + x + 16, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	if (num >= 1000) //1000の位
	{
		switch ((num / 1000) % 10)
		{
		case 0: {glBindTexture(GL_TEXTURE_2D, tex_num_b0); }break;
		case 1: {glBindTexture(GL_TEXTURE_2D, tex_num_b1); }break;
		case 2: {glBindTexture(GL_TEXTURE_2D, tex_num_b2); }break;
		case 3: {glBindTexture(GL_TEXTURE_2D, tex_num_b3); }break;
		case 4: {glBindTexture(GL_TEXTURE_2D, tex_num_b4); }break;
		case 5: {glBindTexture(GL_TEXTURE_2D, tex_num_b5); }break;
		case 6: {glBindTexture(GL_TEXTURE_2D, tex_num_b6); }break;
		case 7: {glBindTexture(GL_TEXTURE_2D, tex_num_b7); }break;
		case 8: {glBindTexture(GL_TEXTURE_2D, tex_num_b8); }break;
		case 9: {glBindTexture(GL_TEXTURE_2D, tex_num_b9); }break;
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 4 + x + 12, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 4 + x + 12, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 3 + x + 12, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 3 + x + 12, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	if (num >= 100) //100の位
	{
		switch ((num / 100) % 10)
		{
		case 0: {glBindTexture(GL_TEXTURE_2D, tex_num_b0); }break;
		case 1: {glBindTexture(GL_TEXTURE_2D, tex_num_b1); }break;
		case 2: {glBindTexture(GL_TEXTURE_2D, tex_num_b2); }break;
		case 3: {glBindTexture(GL_TEXTURE_2D, tex_num_b3); }break;
		case 4: {glBindTexture(GL_TEXTURE_2D, tex_num_b4); }break;
		case 5: {glBindTexture(GL_TEXTURE_2D, tex_num_b5); }break;
		case 6: {glBindTexture(GL_TEXTURE_2D, tex_num_b6); }break;
		case 7: {glBindTexture(GL_TEXTURE_2D, tex_num_b7); }break;
		case 8: {glBindTexture(GL_TEXTURE_2D, tex_num_b8); }break;
		case 9: {glBindTexture(GL_TEXTURE_2D, tex_num_b9); }break;
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 3 + x + 8, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 3 + x + 8, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 2 + x + 8, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 2 + x + 8, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	if (num >= 10) //10の位
	{
		switch ((num / 10) % 10)
		{
		case 0: {glBindTexture(GL_TEXTURE_2D, tex_num_b0); }break;
		case 1: {glBindTexture(GL_TEXTURE_2D, tex_num_b1); }break;
		case 2: {glBindTexture(GL_TEXTURE_2D, tex_num_b2); }break;
		case 3: {glBindTexture(GL_TEXTURE_2D, tex_num_b3); }break;
		case 4: {glBindTexture(GL_TEXTURE_2D, tex_num_b4); }break;
		case 5: {glBindTexture(GL_TEXTURE_2D, tex_num_b5); }break;
		case 6: {glBindTexture(GL_TEXTURE_2D, tex_num_b6); }break;
		case 7: {glBindTexture(GL_TEXTURE_2D, tex_num_b7); }break;
		case 8: {glBindTexture(GL_TEXTURE_2D, tex_num_b8); }break;
		case 9: {glBindTexture(GL_TEXTURE_2D, tex_num_b9); }break;
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 2 + x + 4, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 2 + x + 4, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 1 + x + 4, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 1 + x + 4, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}


	switch (num % 10) // 1の位
	{
	case 0: {glBindTexture(GL_TEXTURE_2D, tex_num_b0); }break;
	case 1: {glBindTexture(GL_TEXTURE_2D, tex_num_b1); }break;
	case 2: {glBindTexture(GL_TEXTURE_2D, tex_num_b2); }break;
	case 3: {glBindTexture(GL_TEXTURE_2D, tex_num_b3); }break;
	case 4: {glBindTexture(GL_TEXTURE_2D, tex_num_b4); }break;
	case 5: {glBindTexture(GL_TEXTURE_2D, tex_num_b5); }break;
	case 6: {glBindTexture(GL_TEXTURE_2D, tex_num_b6); }break;
	case 7: {glBindTexture(GL_TEXTURE_2D, tex_num_b7); }break;
	case 8: {glBindTexture(GL_TEXTURE_2D, tex_num_b8); }break;
	case 9: {glBindTexture(GL_TEXTURE_2D, tex_num_b9); }break;
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 1 + x, size_y + y);//左下
	glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 1 + x, y);//左上
	glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 0 + x, y);//右上
	glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 0 + x, size_y + y);//右下
	glEnd();
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	//★ここからプラス記号の描画
	if (num >= 100000)
	{
		glBindTexture(GL_TEXTURE_2D, tex_num_b_plus);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 7 + x + 20, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 7 + x + 20, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 6 + x + 20, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 6 + x + 20, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	else if (num >= 10000 && num <= 99999)
	{
		glBindTexture(GL_TEXTURE_2D, tex_num_b_plus);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 6 + x + 20, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 6 + x + 20, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 5 + x + 20, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 5 + x + 20, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	else if (num >= 1000 && num <= 9999)
	{
		glBindTexture(GL_TEXTURE_2D, tex_num_b_plus);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 5 + x + 20, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 5 + x + 20, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 4 + x + 20, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 4 + x + 20, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	else if (num >= 100 && num <= 999)
	{
		glBindTexture(GL_TEXTURE_2D, tex_num_b_plus);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 4 + x + 20, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 4 + x + 20, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 3 + x + 20, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 3 + x + 20, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	else if (num >= 10 && num <= 99)
	{
		glBindTexture(GL_TEXTURE_2D, tex_num_b_plus);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 3 + x + 20, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 3 + x + 20, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 2 + x + 20, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 2 + x + 20, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	else if (num >= 1 && num <= 9)
	{
		glBindTexture(GL_TEXTURE_2D, tex_num_b_plus);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 2 + x + 20, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 2 + x + 20, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 1 + x + 20, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 1 + x + 20, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
	//gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

	switch (scene)
	{
	case 0:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		UI_title.SetImage(0, -352);
		UI_gamestart.SetImage(-80,48);
		UI_pressstart.SetImage(140,-32);

		glutPostRedisplay();

	}break;


	case 1:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		select_highlight.SetImage(192 - 160 * (difficulty % 10), -416);
		select_highlight.SetImage(192 - 160 * ((difficulty / 10) % 10), -176);
		select_highlight.SetImage(192 - 160 * ((difficulty / 100) % 10), 64);

		select_matsu.SetImage(192, -416);
		select_take.SetImage(32, -416);
		select_ume.SetImage(-128, -416);

		select_3.SetImage(192, -176);
		select_4.SetImage(32, -176);
		select_5.SetImage(-128, -176);

		select_slow.SetImage(192, 64);
		select_normal.SetImage(32, 64);
		select_fast.SetImage(-128, 64);

		UI_highscore.SetImage(-384,-464);

		SetNumImage(-544, -432, 256, 32, high_score[0]);

		UI_difficulty01.SetImage(432,-208);
		UI_difficulty02.SetImage(432,-464);
		UI_difficulty03.SetImage(432,32);
		UI_07.SetImage(-424,32);

		arrow.SetImage(544, -416 + 240 * difficulty_select);

	}break;

	case 2:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		block_alphabet_p.SetImage(256, -416);
		block_alphabet_a.SetImage(128, -416);
		block_alphabet_u.SetImage(0, -416);
		block_alphabet_s.SetImage(-128, -416);
		block_alphabet_e.SetImage(-256, -416);

		UI_return.SetImage(0, 72);
		UI_08.SetImage(0,-172);

	}break;

	case 3:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		UI_score.SetImage(164,-396);
		UI_newrecord.SetImage(0,-426);
		UI_ranking.SetImage(0,-344);
		SetNumImage(-224, -420, 320, 40, score);

		UI_09.SetImage(0,96);
		UI_rank01.SetImage(204,-224);
		UI_rank02.SetImage(204, -180);
		UI_rank03.SetImage(204, -132);
		UI_rank04.SetImage(204, -84);
		UI_rank05.SetImage(204, -36);
		SetNumImage(-244, -244, 320, 40, high_score[0]);
		SetNumImage(-244, -200, 320, 40, high_score[1]);
		SetNumImage(-244, -152, 320, 40, high_score[2]);
		SetNumImage(-244, -104, 320, 40, high_score[3]);
		SetNumImage(-244, -56, 320, 40, high_score[4]);

	}break;

	case 4:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		UI_score.SetImage(164, -396);
		UI_newrecord.SetImage(0, -426);
		SetNumImage(-224, -420, 320, 40, score);

		UI_09.SetImage(0, 96);
		UI_result01.SetImage(128,-232);
		UI_result02.SetImage(128, -184);
		UI_result03.SetImage(128, -136);
		UI_result04.SetImage(128, -88);
		
		SetNumImage(-244, -244, 320, 40, score_get_hiragana);
		SetNumImage(-244, -200, 320, 40, score_leave_hiragana);
		//SetButtonImage(-244, -152, 48, 48, tex_hiragana_01);
		SetNumImage(-244, -104, 320, 40, score_cleared);

	}break;

	case 5:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y+128, 0, camera_x, camera_y+128, 1, 0, 1, 0);

		UI_10.SetImage(440 +sample->center_x,200);
		UI_11.SetImage(0 + sample->center_x, 272);
		UI_12.SetImage(-440 + sample->center_x, 200);
		UI_slot_base.SetImage(0 + sample->center_x,200); //スロットの基盤
		SetNumImage(360 + sample->center_x,132,160,20,time/60); //タイマー
		SetNumImage(360 + sample->center_x, 200, 160, 20, score); //スコア
		SetNumImage(360 + sample->center_x, 268, 160, 20, high_score[0]); //ハイスコア

		block_hiragana[slot[0]].SetImage(216 + sample->center_x, 176); //ひらがなスロット
		block_hiragana[slot[1]].SetImage(72 + sample->center_x, 176);
		block_hiragana[slot[2]].SetImage(-72 + sample->center_x, 176);
		block_hiragana[slot[3]].SetImage(-216 + sample->center_x, 176);


		if (lamp_timer_01 == 0)
		{
			UI_slot_highlight.SetImage(216 - slot_select * 144 + sample->center_x, 176); //★スロットの選択箇所
		}

	
		

		for (int i = -6400; i < 6400; i++) {
			BG_05.SetImage(i * 1024 + (sample->center_x *1.0), -224);
		}

		for (int i = -6400; i < 6400; i++) {
			BG_04.SetImage(i * 1024 + (sample->center_x *0.75), -224);
		}

		for (int i = -6400; i < 6400; i++) {
			BG_03.SetImage(i * 1024 + (sample->center_x *0.5), -224);
		}

		for (int i = -6400; i < 6400; i++) {
			BG_02.SetImage(i * 1024 + (sample->center_x *0.25), -224);
		}

		for (int i = -6400; i < 6400; i++) {
			BG_01.SetImage(i * 1024 + (sample->center_x *0.0), -224);
		}
		

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
		switch (player.direction)
		{
		case 0:
		{
			//player2.SetImage(player2.center.x, player2.center.y, player2.tex);
			sample->UpdateDirL();
			//glTexCoord2f(0.0f, 1.0f); glVertex2d(0 + player2.center.x, 0 + player2.center.y);//左下
			//glTexCoord2f(0.0f, 0.0f); glVertex2d(0 + player2.center.x, -64 + player2.center.y);//左上
			//glTexCoord2f(1.0f, 0.0f); glVertex2d(64 + player2.center.x, -64 + player2.center.y);//右上
			//glTexCoord2f(1.0f, 1.0f); glVertex2d(64 + player2.center.x, 0 + player2.center.y);//右下
			break;
		}

		case 1:
		{
			//player2.SetImage(player2.center.x, player2.center.y, player2.tex);
			sample->UpdateDirR();
			//glTexCoord2f(0.0f, 1.0f); glVertex2d(64 + player2.center.x, 0 + player2.center.y);//左下
			//glTexCoord2f(0.0f, 0.0f); glVertex2d(64 + player2.center.x, -64 + player2.center.y);//左上
			//glTexCoord2f(1.0f, 0.0f); glVertex2d(0 + player2.center.x, -64 + player2.center.y);//右上
			//glTexCoord2f(1.0f, 1.0f); glVertex2d(0 + player2.center.x, 0 + player2.center.y);//右下
			break;
		}
		}
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		//gameObject->Update();
		//gameObject2->Update();
		//player10->Update();
		//sample->Update();

		for (int i = -6400; i < 100; i++) {
			//SetImage(i * 64, 0, tex_ground);
			floor1.SetImage(i * 64, 64);
			//floor2.SetImage(i * 64, 200, floor2.tex);
		}

		
		if (lamp_timer_01 % 10 >= 6 && lamp_timer_01 > 0) //★Ｋキーを押した後スロットを点滅させる
		{
			UI_slot_decision.SetImage(216 + sample->center_x, 176);
			UI_slot_decision.SetImage(72 + sample->center_x, 176);
			UI_slot_decision.SetImage(-72 + sample->center_x, 176);
			UI_slot_decision.SetImage(-216 + sample->center_x, 176);
		}

		if (lamp_timer_02 > 0) //★Ｋキーを押した後ふきだしとエフェクト点灯
		{
			switch (word_hit)
			{
			case true: //プレイヤーの吹き出しと加点スコアを描画
			{
				player_fukidashi01.SetImage(sample->center_x,  sample->center_y-80);
				if (score >= 100000) { SetNumImage_2(sample->center_x -12*9, sample->center_y - 128, 192, 24, score_word); }
				else if (score >= 10000 && score <= 99999) { SetNumImage_2(  sample->center_x - 12 * 8, sample->center_y - 128, 192, 24, score_word); }
				else if (score >= 1000 && score <= 9999) { SetNumImage_2( sample->center_x - 12 * 7, sample->center_y - 128, 192, 24, score_word); }
				else if (score >= 100 && score <= 999) { SetNumImage_2( sample->center_x - 12 * 6, sample->center_y - 128, 192, 24, score_word); }
				else if (score >= 10 && score <= 99) { SetNumImage_2( sample->center_x - 12 *5, sample->center_y - 128, 192, 24, score_word); }
				else if (score >= 0 && score <= 9) { SetNumImage_2( sample->center_x - 12 * 4, sample->center_y - 128, 192, 24, score_word); }

			}break;

			case false: //プレイヤーの吹き出しとペナルティを描画
			{
				player_fukidashi02.SetImage(sample->center_x,  sample->center_y-80);
				player_penalty.SetImage(sample->center_x, sample->center_y - 120);

			}break;
			}
		}

		if (lamp_timer_02 % 7 >= 3 && lamp_timer_02 > 0) //★Ｋキーを押した後プレイヤーエフェクト点滅
		{
			switch (word_hit)
			{
			case true:
			{
				player_maru.SetImage(sample->center_x, sample->center_y);
			}break;

			case false:
			{
				player_batsu.SetImage(sample->center_x, sample->center_y);
			}break;
			}
		}
	

	}break;

	case 6:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		UI_gameover.SetImage(0,-416);
		UI_tonext.SetImage(0, 64);

	}break;

	}
	glutSwapBuffers();
}

void idle(void)
{
	double speed = 8;

	if (onMoveKeyPress_L == true && flag_01 == true) {
		//player.x += speed;
		camera_x += speed;
		//floor2.Move(speed, 0);
		//glLoadIdentity();
		//player2.Move(speed, 0);
		sample->Move(speed, 0);

		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);
		flag_01 = false;
	}

	if (onMoveKeyPress_R == true && flag_01 == true) {
		//player.x -= speed;
		camera_x -= speed;
		//floor2.Move(-speed, 0);
		//glLoadIdentity();
		//player2.Move(-speed, 0);
		sample->Move(-speed, 0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);
		flag_01 = false;
	}

	if (player_jump == true && flag_02 == true)
	{
		//std::cout << "ジャンプ" << std::endl;
		if (jump_timer < 10)
		{
			//player2.center.y = player2.center.y - (15.0 * jump_timer - 9.68* jump_timer * sqrt(jump_timer) *0.5)*0.01 * 50;//ジャンプのときのプレイヤーの動き
			sample->Move(0, -(15.0 * jump_timer - 9.68* jump_timer * sqrt(jump_timer) *0.5)*0.01 * 128);
		}

		else if (jump_timer >= 10)
		{
			//player2.center.y = player2.center.y - (15.0 * jump_timer - 9.68* jump_timer * sqrt(jump_timer) *0.5)*0.0015 * 50;//ジャンプのときのプレイヤーの動き
			sample->Move(0, -(15.0 * jump_timer - 9.68* jump_timer * sqrt(jump_timer) *0.5)*0.0015 * 36);
		}

		if (sample->center_y > 0) //ブロックの上に着地する場合，その座標とする
		{
			sample->Set(sample->center_x, 0);
			//player2.center.y = 0;
			jump_timer = 0;
			player_jump = false;
		}
		flag_02 = false;
	}
	//printf("%.2f,%.2f %d %d \n", player.x, player.y, jump_timer, walk_timer100);
	if (time < 0)
	{
		scene = 6;
		time = TIME_LIMIT;
	}

	if (lamp_timer_02 <= 0)
	{
		word_hit = false;
		
	}

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
		case '\033': fclose(fp); fclose(fp_dic_4); /* '\033' は ESC の ASCII コード */
			exit(0); break;
		case 'g': scene = 1; break; //PRESS GAME START 次の画面へ

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
			if (difficulty_select != 0)
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
				if ((difficulty / 10) % 10 != 0)
				{
					difficulty -= 10;
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


		case 'l': scene = 5; break;

		default:
			break;
		}
	}break;

	case 2:
	{
		switch (key) {
		case 'p': camera_x = temp_camera_x; camera_y = temp_camera_y;  scene = 5; break;//再開 カメラの位置をゲーム中のに戻す
		case '\033': fclose(fp); fclose(fp_dic_4); /* '\033' は ESC の ASCII コード */
			exit(0); break;
		}
	}break;

	case 3:
	{
		switch (key) {
		case 'l': scene = 4; break; //リザルト画面切り替え
		case 'o': camera_x = 640; camera_y = -544; sample->center_x = 0; sample->center_y = 0; scene = 5; break;//リトライ
		case 'p': camera_x = 640; camera_y = -544; sample->center_x = 0; sample->center_y = 0; scene = 1; break;//メニューに戻る
		case '\033': fclose(fp); fclose(fp_dic_4); /* '\033' は ESC の ASCII コード */
			exit(0); break;
		}
	}break;

	case 4:
	{
		switch (key) {
		case 'l': scene = 3; break; //リザルト画面切り替え
		case 'o': camera_x = 640; camera_y = -544; sample->center_x = 0; sample->center_y = 0; scene = 5; break;//リトライ
		case 'p': camera_x = 640; camera_y = -544; sample->center_x = 0; sample->center_y = 0; scene = 1; break;//メニューに戻る

		case '\033': fclose(fp); fclose(fp_dic_4); /* '\033' は ESC の ASCII コード */
			exit(0); break;
		}
	}break;

	case 5: //ゲーム画面
	{
		switch (key) {
		case 'q':
		case '\033':  /* '\033' は ESC の ASCII コード */
			fclose(fp); fclose(fp_dic_4);  exit(0); break;
		case 'a': onMoveKeyPress_L = true; /*MoveLock_R = true;*/ player.direction = 0; break;
		case 'd': onMoveKeyPress_R = true; /*MoveLock_L = true;*/ player.direction = 1; break;

		case 'j': if (slot_select >0 && lamp_timer_02 == 0) { slot_select--; } break; //スロット移動
		case 'l': if (slot_select  <3 && lamp_timer_02 == 0) { slot_select++; } break;
		case 'i': if (lamp_timer_02 == 0) { slot[slot_select] = 0;  } break; //選択中のスロットの場所をからっぽにする

		case 'k': if (lamp_timer_02 == 0) { check_goi(slot); lamp_timer_02 = 100;  lamp_timer_01 = 50; slot[0] = 0; slot[1] = 0; slot[2] = 0; slot[3] = 0; }  break;//単語チェック


		case 'p': scene = 2; temp_camera_x = camera_x; temp_camera_y = camera_y; camera_x = 640; camera_y = -544; break; //ポーズ カメラの位置をＧＵＩ用にリセット
		case 't': scene = 6; temp_camera_x = camera_x; temp_camera_y = camera_y; camera_x = 640; camera_y = -544; break; //デバッグ用トリガー1 強制ゲームオーバー
		case 'b': slot[3] = 19; break; //★デバッグ用トリガー2
		case 'n': slot[0] = 5; slot[1] = 12; slot[2] = 47;  slot[3] = 10; break; //★デバッグ用トリガー3（成功時シミュレーション）
		case 'm': slot[0] = 5; slot[1] = 12; slot[2] = 47;  slot[3] = 6; break; //★デバッグ用トリガー4（失敗時シミュレーション）

		case '\040': if (player_jump == false) { player_jump = true; } break;
		default:
			break;
		}
	}break;

	case 6: //ゲームオーバー画面
	{
		switch (key) {
		case 'l': scene = 3; time = TIME_LIMIT * 60; lamp_timer_01 = 0; lamp_timer_02 = 0;  break; //リザルト画面へ
		case '\033':  /* '\033' は ESC の ASCII コード */
			fclose(fp); fclose(fp_dic_4);
			exit(0); break;
		}
	}break;
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
	//LoadImagePNG(L"./pic/walk1.png", tex_player1);
	//LoadImagePNG(L"./pic/walk2.png", tex_player2);
	//LoadImagePNG(L"./pic/walk3.png", tex_player3);
	//LoadImagePNG(L"./pic/block_hiragana_01.png", tex_ground);
	//LoadImagePNG(L"./pic/block_hiragana_01.png", tex_hiragana_01);
	LoadImagePNG(L"./pic/num_a0.png", tex_num_a0);
	LoadImagePNG(L"./pic/num_a1.png", tex_num_a1);
	LoadImagePNG(L"./pic/num_a2.png", tex_num_a2);
	LoadImagePNG(L"./pic/num_a3.png", tex_num_a3);
	LoadImagePNG(L"./pic/num_a4.png", tex_num_a4);
	LoadImagePNG(L"./pic/num_a5.png", tex_num_a5);
	LoadImagePNG(L"./pic/num_a6.png", tex_num_a6);
	LoadImagePNG(L"./pic/num_a7.png", tex_num_a7);
	LoadImagePNG(L"./pic/num_a8.png", tex_num_a8);
	LoadImagePNG(L"./pic/num_a9.png", tex_num_a9);
	LoadImagePNG(L"./pic/num_b0.png", tex_num_b0);
	LoadImagePNG(L"./pic/num_b1.png", tex_num_b1);
	LoadImagePNG(L"./pic/num_b2.png", tex_num_b2);
	LoadImagePNG(L"./pic/num_b3.png", tex_num_b3);
	LoadImagePNG(L"./pic/num_b4.png", tex_num_b4);
	LoadImagePNG(L"./pic/num_b5.png", tex_num_b5);
	LoadImagePNG(L"./pic/num_b6.png", tex_num_b6);
	LoadImagePNG(L"./pic/num_b7.png", tex_num_b7);
	LoadImagePNG(L"./pic/num_b8.png", tex_num_b8);
	LoadImagePNG(L"./pic/num_b9.png", tex_num_b9);
	LoadImagePNG(L"./pic/num_plus.png", tex_num_b_plus);
	player1.LoadImagePNG2(player1.file, player1.tex);
	player2.LoadImagePNG2(player2.file, player2.tex);
	player3.LoadImagePNG2(player3.file, player3.tex);
	//floor2.LoadImagePNG2(floor2.file, floor2.tex);
	floor1.LoadImagePNG2(floor1.file, floor1.tex);


	UI_title.LoadImagePNG2(UI_title.file, UI_title.tex);
	UI_gamestart.LoadImagePNG2(UI_gamestart.file, UI_gamestart.tex);
	UI_pressstart.LoadImagePNG2(UI_pressstart.file, UI_pressstart.tex);
	select_matsu.LoadImagePNG2(select_matsu.file, select_matsu.tex);
	select_take.LoadImagePNG2(select_take.file, select_take.tex);
	select_ume.LoadImagePNG2(select_ume.file, select_ume.tex);
	select_3.LoadImagePNG2(select_3.file, select_3.tex);
	select_4.LoadImagePNG2(select_4.file, select_4.tex);
	select_5.LoadImagePNG2(select_5.file, select_5.tex);
	select_slow.LoadImagePNG2(select_slow.file, select_slow.tex);
	select_normal.LoadImagePNG2(select_normal.file, select_normal.tex);
	select_fast.LoadImagePNG2(select_fast.file, select_fast.tex);
	select_highlight.LoadImagePNG2(select_highlight.file, select_highlight.tex);
	block_alphabet_p.LoadImagePNG2(block_alphabet_p.file, block_alphabet_p.tex);
	block_alphabet_a.LoadImagePNG2(block_alphabet_a.file, block_alphabet_a.tex);
	block_alphabet_u.LoadImagePNG2(block_alphabet_u.file, block_alphabet_u.tex);
	block_alphabet_s.LoadImagePNG2(block_alphabet_s.file, block_alphabet_s.tex);
	block_alphabet_e.LoadImagePNG2(block_alphabet_e.file, block_alphabet_e.tex);
	UI_return.LoadImagePNG2(UI_return.file, UI_return.tex);
	UI_highscore.LoadImagePNG2(UI_highscore.file, UI_highscore.tex);
	UI_gameover.LoadImagePNG2(UI_gameover.file, UI_gameover.tex);
	UI_tonext.LoadImagePNG2(UI_tonext.file, UI_tonext.tex);
	UI_07.LoadImagePNG2(UI_07.file, UI_07.tex);
	UI_08.LoadImagePNG2(UI_08.file, UI_08.tex);
	UI_09.LoadImagePNG2(UI_09.file, UI_09.tex);
	UI_result01.LoadImagePNG2(UI_result01.file, UI_result01.tex);
	UI_result02.LoadImagePNG2(UI_result02.file, UI_result02.tex);
	UI_result03.LoadImagePNG2(UI_result03.file, UI_result03.tex);
	UI_result04.LoadImagePNG2(UI_result04.file, UI_result04.tex);
	UI_score.LoadImagePNG2(UI_score.file, UI_score.tex);
	UI_newrecord.LoadImagePNG2(UI_newrecord.file, UI_newrecord.tex);
	UI_ranking.LoadImagePNG2(UI_ranking.file, UI_ranking.tex);
	UI_rank01.LoadImagePNG2(UI_rank01.file, UI_rank01.tex);
	UI_rank02.LoadImagePNG2(UI_rank02.file, UI_rank02.tex);
	UI_rank03.LoadImagePNG2(UI_rank03.file, UI_rank03.tex);
	UI_rank04.LoadImagePNG2(UI_rank04.file, UI_rank04.tex);
	UI_rank05.LoadImagePNG2(UI_rank05.file, UI_rank05.tex);
	UI_difficulty01.LoadImagePNG2(UI_difficulty01.file, UI_difficulty01.tex);
	UI_difficulty02.LoadImagePNG2(UI_difficulty02.file, UI_difficulty02.tex);
	UI_difficulty03.LoadImagePNG2(UI_difficulty03.file, UI_difficulty03.tex);
	arrow.LoadImagePNG2(arrow.file, arrow.tex);
	BG_01.LoadImagePNG2(BG_01.file, BG_01.tex);
	BG_02.LoadImagePNG2(BG_02.file, BG_02.tex);
	BG_03.LoadImagePNG2(BG_03.file, BG_03.tex);
	BG_04.LoadImagePNG2(BG_04.file, BG_04.tex);
	BG_05.LoadImagePNG2(BG_05.file, BG_05.tex);
	
	UI_10.LoadImagePNG2(UI_10.file, UI_10.tex);
	UI_11.LoadImagePNG2(UI_11.file, UI_11.tex);
	UI_12.LoadImagePNG2(UI_12.file, UI_12.tex);
	UI_slot_base.LoadImagePNG2(UI_slot_base.file, UI_slot_base.tex);
	UI_slot_highlight.LoadImagePNG2(UI_slot_highlight.file, UI_slot_highlight.tex);
	UI_slot_decision.LoadImagePNG2(UI_slot_decision.file, UI_slot_decision.tex);
	player_penalty.LoadImagePNG2(player_penalty.file, player_penalty.tex);
	player_maru.LoadImagePNG2(player_maru.file, player_maru.tex);
	player_batsu.LoadImagePNG2(player_batsu.file, player_batsu.tex);
	player_fukidashi01.LoadImagePNG2(player_fukidashi01.file, player_fukidashi01.tex);
	player_fukidashi02.LoadImagePNG2(player_fukidashi02.file, player_fukidashi02.tex);

	
	for (i = 0; i <= 76; i++)
	{
		block_hiragana[i].LoadImagePNG2(block_hiragana[i].file, block_hiragana[i].tex);
	}

	//コピー用
	//.LoadImagePNG2(.file, .tex);

	//gameObject = new GameObject(10.0, -100.0, 64.0, 64.0, L"./pic/block1.png");
	//gameObject2 = new GameObject(10.0, -200.0, 64.0, 64.0, L"./pic/arrow.png");
	//gameObject.LoadPNGImage(gameObject.file, gameObject.tex);
	//player10 = new AnimationChara(10.0, -300.0, 64.0, 64.0, L"./pic/block1.png");
	//player10->LoadPNGImage(L"./pic/arrow.png");
	//player10->ChangeImage(1);
	//player10->LoadPNGImage(L"./pic/player_walk1.png");
	//player10->ChangeImage(0);
	sample = new AnimationChara(0.0, 0, 64.0, 64.0, L"./pic/player_walk1.png");
	sample->LoadPNGImage(L"./pic/player_walk2.png");
	sample->LoadPNGImage(L"./pic/player_walk3.png");

	//player10->ChangeImage(1);
	player.x = 0;
	player.y = 0;
	player.direction = 1;

	scene = 0;

	if ((fopen_s(&fp, "score.dat", "r")) != 0) //スコアファイルを読み込む
	{
		printf("スコアファイルを開けませんでした\n");
		exit(1);
	}

	if ((fopen_s(&fp_dic_4, "4moji_dic.dat", "r")) != 0) //★辞書ファイルを読み込む
	{
		printf("辞書ファイルを開けませんでした\n");
		exit(2);
	}

	i = 0;
	while (fscanf_s(fp_dic_4, "%d,%d,%d,%d,",&dic[i][0], &dic[i][1], &dic[i][2], &dic[i][3]) != EOF)
	{
		/*
		if (dic[i][0] == 0 || dic[i][1] == 0 || dic[i][2] == 0 || dic[i][3] == 0)
		{
			printf("%d番目の単語が読み取れませんでした\n",i);
			exit(3);
		}
		*/
		i++;
	}

	dic_4_all = i; printf("4文字辞書%d単語を読み込みました\n",dic_4_all);

	i = 0;
	while (fscanf_s(fp, "%d", &high_score[i]) != EOF)
	{
		i++;
	}

}

void timer(int value) {

	glutPostRedisplay();
	glutTimerFunc(16, timer, 0);

	flag_01 = true;
	flag_02 = true;

	if (scene == 5) //ゲーム中，タイマー起動
	{
		time--; //ゲーム時間の残りタイムをへらす

		if (lamp_timer_01>0) //スロットの点滅時間
		{
			lamp_timer_01--;
		}

		if (lamp_timer_02 > 0) //キャラクターのリアクションの時間
		{
			lamp_timer_02--;
		}
	}

	if (scene == 6)
	{
		temp_camera_x = camera_x; temp_camera_y = camera_y; camera_x = 640; camera_y = -544;
	}

	if (player_jump == true)
	{
		jump_timer++;
		//printf("jump_timer = %d   ",jump_timer);
		//printf("player_y = %.4f\n",sample->center_y);
	}


	
	

	if (onMoveKeyPress_L == true || onMoveKeyPress_R == true) //歩きアニメーションのため （画像１→２→３→２というふうに歩き中には４枚の画像を連続で表示する）
	{
		
		//speed_1flame = sample->center_x - speed_temp;
		//printf("speed_temp = %.4f\n",speed_1flame);

		if (walk_timer100 == 300)
		{
			walk_timer100 = 0;
		}

		else
		{
			walk_timer100 += 100;
		}

		//speed_temp = sample->center_x;
	}

	else
	{
		//speed_temp = 0;
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
	glutTimerFunc(16, timer, 0);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutIdleFunc(idle);

	Init();
	glutMainLoop();

	return 0;
}