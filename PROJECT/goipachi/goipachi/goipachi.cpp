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
#include <random>


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

#define WIDTH 1280 //デフォルトの画面のサイズ
#define HEIGHT 720

#define TIME_LIMIT 300 //タイマー．ほぼ1秒に１すすむ
#define dic_4_LIMIT 44000 //4文字辞書の単語数の余地（今後追加できるよう少し多めに設定）
#define OBJECT_LIMIT 10000 //ブロックの制限
#define PATTERN_LIMIT 45 //横に並べるブロック配置パターンの上限

GdiplusStartupInput gdiPSI;
ULONG_PTR gdiPT;

GLuint tex_num_a[10] = {}; //数字フォント（赤）
GLuint tex_num_b[11] = {}; //数字フォント（頭上のスコアの数字・黄緑） [10]は「＋」

int ranking; //ランキング表示のため

bool onMoveKeyPress_L = false;
bool onMoveKeyPress_R = false;
bool player_jump = false;
bool player_walk = false;
int jump_timer = 0;  //キャラクターがジャンプしてからの時間を計測
#define JUMP_HIGHEST 11 //キャラクターがジャンプしてから最高点に達した瞬間のjump_timer

int walk_timer = 0; //キャラクターのアニメーション
int lamp_timer_01 = 0; //Ｋキー（決定ボタン）を押した後の赤ライトの点灯
int lamp_timer_02 = 0; //プレイヤーのリアクションのエフェクト
int lamp_timer_block = 0; //ルーレットブロックのエフェクトのアニメーション
int hiragana_roulette_timer = 0; //ルーレットブロックの中身のタイマー
int hiragana_roulette[74] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75 }; //ルーレットの中身
int gun_timer = 0; //銃を構えているイラストの表示
int bullet_timer = 0; //弾丸が発射されてからの秒数 一定時間たつと消える（無限に飛び続けるのいやでしょ）

int scene = 0; //◇シーンの追加．0:タイトル 1:スタンバイ画面 2:ポーズ画面 3:リザルト画面A 4リザルト画面B 5:プレイ画面（メイン） 6:GAME OVER画面  7:ステージ・モード選択（今後実装予定）
int difficulty = 0; //◇難易度：松竹梅（+0~2）文字数（+00～20）はやさ（+000～200）
int difficulty_select = 0; //◇0：松竹梅選択 1：文字数選択 2：はやさ選択

int dic[dic_4_LIMIT][4] = { {} }; //辞書の情報を格納
int score_get_hiragana = 0; //入手したひらがなの数
int score_leave_hiragana = 0; //捨てたひらがなの数
int score_most_hiragana = 0; //最も単語に使ったひらがなの数
int max_most_hiragana = 0; //↑を求めるのに使う
int score_tango = 0; //作った単語の数
int score = 0; //ゲーム中でのスコア

int list_most_hiragana[80] = {}; //最も入手したひらがなを決めるのに必要

int hiragana_weight_4[80][5] = { {} }; //各何文字目かの登場回数（単語を構成するひらがなのほかにランダムなひらがなを配置するため，それの生成率を決める重み
float hiragana_score_4[80][5] = { {} }; //Kキーを押下した際スロットの各文字の点数を合計するための要素

int odai; //お題の番号を決める
int odai_hiragana[75][5] = {
{0,6,2,0},{0,0,2,43},{11,0,0,8},{0,46,0,46},{0,2,0,2},{0,3,0,3},{0,2,15,0},{14,2,0,0},{0,0,6,39},
{66,46,0,0},{0,47,0,3},{0,0,0,48},{6,0,6,0},{16,0,0,46},{0,47,0,2},{0,50,0,50},{0,50,41,0},{26,0,0,8},
{39,0,0,50},{0,0,50,65}
}; //お題のひらがなの配置

int choose_hiragana_weight[80] = { 0, //ひらがなブロックがステージに配置されるときの平仮名の種類の重み
12,20,18,10,16, //ア行
20,18,10,10,12,
12,15,10,10,9,
15,8,10,15,15,
15,9,5,10,13,
12,9,12,7,12,
16,13,11,11,12,
15,10,10,
8,12,10,12,12,
12,4,13,15,4,0,17, //わをんっヴ箱ー
10,10,10,9,14,
8,14,10,10,8,
15,6,6,14,14,
10,8,13,13,10,
9,9,9,9,7,
0,0,0,0
};

int choose_hiragana_weight_add[80] = {}; //最終的に乱数よりひらがなを決定するために 1~79だったら「あ」　80~144だったら「い」みたいにできるように

int high_score[5] = { 0,0,0,0,0 }; //レコードされているハイスコア
int slot[5] = { 0,0,0,0,0 }; //スロット（のちのち5文字でもできるように）
int slot_select = 0; //どの文字をさしているか
int slot_start[OBJECT_LIMIT] = {}; //オブジェクトがルーレットだった場合ルーレットはどっから始まるか
int object_on_stage = 0; //そのセッションで配置されたブロックの総数

int score_word = 0; //１つ１つのワードのスコア
int time = TIME_LIMIT * 60;

int stage_structure[PATTERN_LIMIT] = {};
int set_leftside = 0;

int object_block[OBJECT_LIMIT][3] = { {} }; //衝突判定に使う，プレイヤーとの距離の条件を満たすためにこの配列にオブジェクトの情報を記載（ブロックの種類,中心座標（ｘとｙ））
int height_c = 0; //衝突判定に使う，ジャンプした後着地できる位置（高さ）最も高い座標

FILE *fp; //スコアファイル
FILE *fp_dic_4; //辞書ファイル
int dic_4_all = 0;
bool word_hit = false; //Ｋキー押下後，辞書に存在していたかどうか(trueなら得点＋エフェクト，falseなら時間減少＋エフェクト）

int i, j, k, l;

double camera_x = 0; //カメラの位置
double camera_y = 0;
double temp_camera_x = 0; //◇シーン移動によりカメラの位置をリセットするため，ゲーム中のカメラ位置をストックしておく
double temp_camera_y = 0;
//void LoadImagePNG(const wchar_t* filename, GLuint &texture);

bool flag_move_x = true; //次のタイマー関数が呼び出されるまでx方向の移動を与えないための制御
bool flag_move_y = true; //次のタイマー関数が呼び出されるまでy方向の移動を与えないための制御
bool flag_collision_L = false; //衝突判定（プレイヤーの左方向）
bool flag_collision_R = false; //衝突判定（右方向）
bool flag_collision_U = false; //衝突判定（上方向）
bool flag_collision_D = false; //衝突判定（下方向）

bool flag_jump_slow = false; //ジャンプ→落下時ゆっくり降りるかんじにするトリガー
bool flag_bullet_exist = false; //弾丸が存在して動いている状態（当たり判定起動）


int time_1flame; //デバッグ用，1フレームでどれだけ進んだか，どれだけ時間がたっているか（ms）（下３つも同じ）
double speed_1flame;
int time_temp;
double speed_temp;

std::random_device rnd;
std::mt19937 mt(rnd()); //なんか乱数のタネを決定するやつ
std::uniform_int_distribution<> rand_hiragana(1, 75);
std::uniform_int_distribution<> rand_pattern(1, 15);
std::uniform_int_distribution<> rand_odai(1, 75);
std::uniform_int_distribution<> rand_dic_4(1, dic_4_LIMIT); //4文字辞書のステージで，ブロックを配置するときに使う

class Coordinate {
public:
	double x, y;

	Coordinate(double x, double y) : x(x), y(y) {

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

		file = filename;
		tex = *filename; 
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
			glTexCoord2f(0.0f, 1.0f); glVertex2d(center_x + 64.0 / 2, center_y + 64.0 / 2);//左下
			glTexCoord2f(0.0f, 0.0f); glVertex2d(center_x + 64.0 / 2, center_y - 64.0 / 2);//左上
			glTexCoord2f(1.0f, 0.0f); glVertex2d(center_x - 64.0 / 2, center_y - 64.0 / 2);//右上
			glTexCoord2f(1.0f, 1.0f); glVertex2d(center_x - 64.0 / 2, center_y + 64.0 / 2);//右下
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

	void LoadImagePNG2(const wchar_t* filename, GLuint &texture) //追加
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

	}
	void SetImage(double center_x, double center_y) {

		if (center_x <= camera_x + 1800 && center_x >= camera_x - 1800) {

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
	int direction;

	AnimationChara(double x, double y, double size_x, double size_y, const wchar_t *filename)
		:MoveObject(x, y, size_x, size_y, filename) {

		texes.push_back(tex);
		direction = 1;

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
		//std::cout << texes.size() << std::endl;
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
			glTexCoord2f(0.0f, 1.0f); glVertex2d(center_x + 64.0 / 2, center_y + 64.0 / 2);//左下
			glTexCoord2f(0.0f, 0.0f); glVertex2d(center_x + 64.0 / 2, center_y - 64.0 / 2);//左上
			glTexCoord2f(1.0f, 0.0f); glVertex2d(center_x - 64.0 / 2, center_y - 64.0 / 2);//右上
			glTexCoord2f(1.0f, 1.0f); glVertex2d(center_x - 64.0 / 2, center_y + 64.0 / 2);//右下
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
			glTexCoord2f(1.0f, 1.0f); glVertex2d(center_x + 64.0 / 2, center_y + 64.0 / 2);//左下
			glTexCoord2f(1.0f, 0.0f); glVertex2d(center_x + 64.0 / 2, center_y - 64.0 / 2);//左上
			glTexCoord2f(0.0f, 0.0f); glVertex2d(center_x - 64.0 / 2, center_y - 64.0 / 2);//右上
			glTexCoord2f(0.0f, 1.0f); glVertex2d(center_x - 64.0 / 2, center_y + 64.0 / 2);//右下
			glEnd();
			glDisable(GL_ALPHA_TEST);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);
			
		}
	}

};

GameObject floor1 = GameObject(0, 0, 64, 64, L"./pic/ground.png");
GameObject player1 = GameObject(0, 0, 64, 64, L"./pic/player_walk1.png");
GameObject player2 = GameObject(0, 0, 64, 64, L"./pic/player_walk2.png");
GameObject player3 = GameObject(0, 0, 64, 64, L"./pic/player_walk3.png");
GameObject UI_title = GameObject(0, 0, 768, 256, L"./pic/title_a.png");

GameObject UI_gamestart = GameObject(0, 0, 512, 64, L"./pic/game_start.png");
GameObject UI_pressstart = GameObject(0, 0, 128, 64, L"./pic/press.png");

GameObject block_alphabet_p = GameObject(0, 0, 128, 128, L"./pic/block_alphabet_p.png");
GameObject block_alphabet_a = GameObject(0, 0, 128, 128, L"./pic/block_alphabet_a.png");
GameObject block_alphabet_u = GameObject(0, 0, 128, 128, L"./pic/block_alphabet_u.png");
GameObject block_alphabet_s = GameObject(0, 0, 128, 128, L"./pic/block_alphabet_s.png");
GameObject block_alphabet_e = GameObject(0, 0, 128, 128, L"./pic/block_alphabet_e.png");
GameObject UI_return = GameObject(0, 0, 768, 96, L"./pic/menu_return.png");
GameObject UI_highscore = GameObject(0, 0, 256, 32, L"./pic/highscore.png");
GameObject UI_gameover = GameObject(0, 0, 1024, 128, L"./pic/gameover.png");
GameObject UI_tonext = GameObject(0, 0, 1024, 128, L"./pic/menu_tonext01.png");
GameObject UI_difficulty01 = GameObject(0, 0, 256, 64, L"./pic/difficulty_menu01.png");
GameObject UI_difficulty02 = GameObject(0, 0, 256, 64, L"./pic/difficulty_menu02.png");
GameObject UI_difficulty03 = GameObject(0, 0, 256, 64, L"./pic/difficulty_menu03.png");
GameObject UI_07 = GameObject(0, 0, 384, 192, L"./pic/menu_UI_07.png");
GameObject UI_08 = GameObject(0, 0, 512, 256, L"./pic/menu_UI_08.png");
GameObject UI_09 = GameObject(0, 0, 1024, 64, L"./pic/menu_UI_09.png");
GameObject UI_13 = GameObject(0, 0, 1024, 128, L"./pic/menu_tonext02.png");
GameObject UI_14 = GameObject(0, 0, 1024, 128, L"./pic/are_you_ready.png");
GameObject UI_result01 = GameObject(0, 0, 384, 48, L"./pic/result_01.png");
GameObject UI_result02 = GameObject(0, 0, 384, 48, L"./pic/result_02.png");
GameObject UI_result03 = GameObject(0, 0, 384, 48, L"./pic/result_03.png");
GameObject UI_result04 = GameObject(0, 0, 384, 48, L"./pic/result_04.png");
GameObject UI_score = GameObject(0, 0, 128, 32, L"./pic/score.png");
GameObject UI_newrecord = GameObject(0, 0, 512, 256, L"./pic/result_newrecord.png");
GameObject UI_ranking = GameObject(0, 0, 384, 192, L"./pic/result_ranking.png");
GameObject UI_rank01 = GameObject(0, 0, 80, 40, L"./pic/ranking_01.png");
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
GameObject UI_11 = GameObject(0, 0, 256, 64, L"./pic/menu_UI_11.png");
GameObject UI_12 = GameObject(0, 0, 384, 192, L"./pic/menu_UI_12.png");
GameObject UI_slot_base = GameObject(0, 0, 768, 192, L"./pic/slot_base.png");
GameObject UI_slot_highlight = GameObject(0, 0, 192, 192, L"./pic/slot_highlight.png");
GameObject UI_slot_decision = GameObject(0, 0, 192, 192, L"./pic/slot_decision.png");
GameObject UI_slot_decision_green = GameObject(0, 0, 192, 192, L"./pic/slot_decision_green.png");

GameObject arrow = GameObject(0, 0, 128, 128, L"./pic/arrow.png");
GameObject select_highlight = GameObject(0, 0, 136, 136, L"./pic/Y.png");

GameObject player_penalty = GameObject(0, 0, 96, 24, L"./pic/player_penalty.png");
GameObject player_maru = GameObject(0, 0, 64, 64, L"./pic/maru.png");
GameObject player_batsu = GameObject(0, 0, 64, 64, L"./pic/batsu.png");
GameObject player_fukidashi01 = GameObject(0, 0, 64, 64, L"./pic/fukidashi1.png");
GameObject player_fukidashi02 = GameObject(0, 0, 64, 64, L"./pic/fukidashi2.png");


GameObject block_hiragana_UI[80] = {
GameObject(0, 0, 96, 96, L"./pic/block_blank.png"), //空白 ID:0
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
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_45.png"), //を ID:45
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_46.png"), //ん ID:46
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_73.png"), //っ ID:47
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_74.png"), //ヴ
GameObject(0, 0, 96, 96, L"./pic/block_wood.png"), //木材（足場ブロック）ID:49
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_47.png"), //ー ID:50
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
GameObject(0, 0, 96, 96, L"./pic/block_hiragana_72.png"),//ぽ
GameObject(0, 0, 96, 96, L"./pic/ground.png"), //地面 ID:76
GameObject(0, 0, 96, 96, L"./pic/block_undifined.png"), //未使用（お題箱とかに使う？
GameObject(0, 0, 96, 96, L"./pic/block_undifined.png"), //未使用（お題箱とかに使う？
GameObject(0, 0, 96, 96, L"./pic/block_undifined.png") //未使用（お題箱とかに使う？
};

GameObject block_hiragana[80] = {
GameObject(0, 0, 64, 64, L"./pic/block_blank.png"), //空白 ID:0
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_01.png"), //あ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_02.png"), //い
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_03.png"), //う
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_04.png"), //え
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_05.png"), //お
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_06.png"), //か
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_07.png"), //き
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_08.png"), //く
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_09.png"), //け
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_10.png"), //こ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_11.png"), //さ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_12.png"), //し
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_13.png"), //す
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_14.png"), //せ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_15.png"), //そ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_16.png"), //た
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_17.png"), //ち
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_18.png"), //つ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_19.png"), //て
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_20.png"), //と
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_21.png"), //な
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_22.png"), //に
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_23.png"), //ぬ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_24.png"), //ね
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_25.png"), //の
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_26.png"), //は
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_27.png"), //ひ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_28.png"), //ふ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_29.png"), //へ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_30.png"), //ほ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_31.png"), //ま
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_32.png"), //み
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_33.png"), //む
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_34.png"), //め
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_35.png"), //も
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_36.png"), //や
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_37.png"), //ゆ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_38.png"), //よ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_39.png"), //ら
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_40.png"), //り
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_41.png"), //る
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_42.png"), //れ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_43.png"), //ろ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_44.png"), //わ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_45.png"), //を ID:45
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_46.png"), //ん ID:46
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_73.png"), //っ ID:47
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_74.png"), //ヴ
GameObject(0, 0, 64, 64, L"./pic/block_wood.png"), //木材（足場ブロック）ID:49
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_47.png"), //ー ID:50
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_48.png"), //が
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_49.png"), //ぎ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_50.png"), //ぐ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_51.png"), //げ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_52.png"), //ご
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_53.png"), //ざ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_54.png"), //じ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_55.png"), //ず
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_56.png"), //ｚｗ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_57.png"), //ぞ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_58.png"), //だ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_59.png"), //ぢ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_60.png"), //づ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_61.png"), //で
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_62.png"), //ど
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_63.png"), //ば
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_64.png"), //び
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_65.png"), //ぶ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_66.png"), //べ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_67.png"), //ぼ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_68.png"), //ぱ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_69.png"), //ぴ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_70.png"), //ぷ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_71.png"), //ぺ
GameObject(0, 0, 64, 64, L"./pic/block_hiragana_72.png"),//ぽ
GameObject(0, 0, 64, 64, L"./pic/ground.png"), //地面 ID:76
GameObject(0, 0, 64, 64, L"./pic/block_odai.png"), //おだいばこ
GameObject(0, 0, 64, 64, L"./pic/block_odai_off.png"), //お題箱オフ
GameObject(0, 0, 64, 64, L"./pic/block_undifined.png") //未使用（お題箱とかに使う？
};

GameObject block_light_1 = GameObject(0, 0, 64, 64, L"./pic/block_highlight_1.png"); //ルーレットブロックのライトエフェクト（明
GameObject block_light_2 = GameObject(0, 0, 64, 64, L"./pic/block_highlight_2.png"); //ルーレットブロックのライトエフェクト（暗


AnimationChara *player; //プレイヤーのあらゆる情報を扱う
AnimationChara *bullet; //弾丸のあらゆる情報を扱う


void end() {
	GdiplusShutdown(gdiPT);
}

void check_goi(int moji[])
{
	for (i = 0; i < dic_4_all; i++)
	{
		if (moji[0] == dic[i][0] && moji[1] == dic[i][1] && moji[2] == dic[i][2] && moji[3] == dic[i][3])
		{	
			word_hit = true;
			
			score_word = (int)(hiragana_score_4[moji[0]][0] + hiragana_score_4[moji[1]][1] + hiragana_score_4[moji[2]][2] + hiragana_score_4[moji[3]][3]);

			score += score_word;
			score_tango++;
			list_most_hiragana[moji[0]] ++;
			list_most_hiragana[moji[1]] ++;
			list_most_hiragana[moji[2]] ++;
			list_most_hiragana[moji[3]] ++;

			break;
		}

	}
	if (word_hit == false)
	{
		time -= 30*60;
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

		glBindTexture(GL_TEXTURE_2D, tex_num_a[num / 100000]);

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
		glBindTexture(GL_TEXTURE_2D, tex_num_a[(num / 10000) % 10]);

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
		glBindTexture(GL_TEXTURE_2D, tex_num_a[(num / 1000) % 10]);

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
		glBindTexture(GL_TEXTURE_2D, tex_num_a[(num / 100) % 10]);

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
		glBindTexture(GL_TEXTURE_2D, tex_num_a[(num / 10) % 10]);

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

	//1の位
	glBindTexture(GL_TEXTURE_2D, tex_num_a[num % 10]);

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
		glBindTexture(GL_TEXTURE_2D, tex_num_b[num / 100000]);

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
		glBindTexture(GL_TEXTURE_2D, tex_num_b[(num / 10000) % 10]);
		
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
		glBindTexture(GL_TEXTURE_2D, tex_num_b[(num / 1000) % 10]);
		
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
		glBindTexture(GL_TEXTURE_2D, tex_num_b[(num / 100) % 10]);
		
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
		glBindTexture(GL_TEXTURE_2D, tex_num_b[(num / 10) % 10]);
		
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

	glBindTexture(GL_TEXTURE_2D, tex_num_b[num % 10]);
	
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

	//ここからプラス記号の描画
	if (num >= 100000)
	{
		glBindTexture(GL_TEXTURE_2D, tex_num_b[10]);
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
		glBindTexture(GL_TEXTURE_2D, tex_num_b[10]);
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
		glBindTexture(GL_TEXTURE_2D, tex_num_b[10]);
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
		glBindTexture(GL_TEXTURE_2D, tex_num_b[10]);
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
		glBindTexture(GL_TEXTURE_2D, tex_num_b[10]);
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
		glBindTexture(GL_TEXTURE_2D, tex_num_b[10]);
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

int choose_hiragana(void)
{
	int a;

	do {
		a = rand_hiragana(mt);
	} while (a == 49);

	return a;
}

int choose_pattern(void)
{
	int a;

	do {
		a = rand_pattern(mt);
	} while (a >= 15);

	return a;
}

int choose_odai(void)
{
	int a;

	do {
		a = rand_odai(mt);
	} while (a >= 15);

	return a;
}

void set_block_info(int type, int x_grid, int y_grid, int leftside) //leftsideはそのまま使う 
{
	object_block[j][0] = type; object_block[j][1] = (-64)*(leftside + x_grid); object_block[j][2] = (-64)*(y_grid);
}

void block_standby(void)
{

	j = 0; //オブジェクトブロックを配置するための添え字，1個設置したら1増える．
	object_on_stage = 0;

	int Q[12] = {}; //地形パターンを生成する際，同じ辞書の単語に含まれるひらがなを所定の位置に配置するため，その単語の番号を乱数で取得する



	//ここから100行程度，導入部分のブロックを配置
	set_block_info(76, 0, -1, 0); j++;
	set_block_info(76, -1, -1, 0); j++;
	set_block_info(76, -2, -1, 0); j++;
	set_block_info(76, -3, -1, 0); j++;
	set_block_info(76, -4, -1, 0); j++;
	set_block_info(76, -5, -1, 0); j++;
	set_block_info(76, -6, -1, 0); j++;
	set_block_info(76, -7, -1, 0); j++;
	set_block_info(76, -8, -1, 0); j++;
	set_block_info(76, -9, -1, 0); j++;
	set_block_info(76, -10, -1, 0); j++;
	set_block_info(76, -11, -1, 0); j++;
	set_block_info(76, -12, -1, 0); j++;
	set_block_info(49, -1, 0, 0); j++;
	set_block_info(49, -1, 1, 0); j++;
	set_block_info(49, -1, 2, 0); j++;
	set_block_info(49, -1, 3, 0); j++;
	set_block_info(49, -1, 4, 0); j++;
	set_block_info(49, -1, 5, 0); j++;
	set_block_info(49, -1, 6, 0); j++;
	set_block_info(49, -1, 7, 0); j++;
	set_block_info(49, -1, 8, 0); j++;


	for (k = 0; k <= 39; k++) //床の配置
	{
		set_block_info(76, k, -1, 0); j++;
	}

	set_block_info(11, 5, 0, 0); j++;
	set_block_info(2, 6, 0, 0); j++;
	set_block_info(14, 7, 0, 0); j++;
	set_block_info(2, 8, 0, 0); j++;
	set_block_info(49, 11, 0, 0); j++;
	set_block_info(49, 12, 0, 0); j++;
	set_block_info(49, 13, 0, 0); j++;
	set_block_info(49, 14, 0, 0); j++;
	set_block_info(49, 15, 0, 0); j++;
	set_block_info(49, 16, 0, 0); j++;
	set_block_info(49, 17, 0, 0); j++;
	set_block_info(49, 18, 0, 0); j++;
	set_block_info(5, 13, 1, 0); j++;
	set_block_info(10, 14, 1, 0); j++;
	set_block_info(21, 15, 1, 0); j++;
	set_block_info(2, 16, 1, 0); j++;
	set_block_info(14, 23, 0, 0); j++;
	set_block_info(7, 24, 0, 0); j++;
	set_block_info(44, 25, 0, 0); j++;
	set_block_info(9, 26, 0, 0); j++;
	set_block_info(5, 23, 3, 0); j++;
	set_block_info(5, 24, 3, 0); j++;
	set_block_info(59, 25, 3, 0); j++;
	set_block_info(7, 26, 3, 0); j++;
	set_block_info(38, 23, 6, 0); j++;
	set_block_info(10, 24, 6, 0); j++;
	set_block_info(63, 25, 6, 0); j++;
	set_block_info(21, 26, 6, 0); j++;
	set_block_info(49, 20, 2, 0); j++;
	set_block_info(49, 21, 2, 0); j++;
	set_block_info(49, 22, 2, 0); j++;
	set_block_info(49, 23, 2, 0); j++;
	set_block_info(49, 24, 2, 0); j++;
	set_block_info(49, 25, 2, 0); j++;
	set_block_info(49, 26, 2, 0); j++;
	set_block_info(49, 27, 2, 0); j++;
	set_block_info(49, 22, 5, 0); j++;
	set_block_info(49, 23, 5, 0); j++;
	set_block_info(49, 24, 5, 0); j++;
	set_block_info(49, 25, 5, 0); j++;
	set_block_info(49, 26, 5, 0); j++;
	set_block_info(49, 27, 5, 0); j++;
	set_block_info(49, 28, 5, 0); j++;
	set_block_info(49, 29, 5, 0); j++;
	set_block_info(49, 20, 3, 0); j++;
	set_block_info(49, 20, 4, 0); j++;
	set_block_info(49, 20, 5, 0); j++;
	set_block_info(49, 20, 6, 0); j++;
	set_block_info(49, 29, 0, 0); j++;
	set_block_info(49, 29, 1, 0); j++;
	set_block_info(49, 29, 2, 0); j++;
	set_block_info(49, 29, 3, 0); j++;
	set_block_info(49, 29, 4, 0); j++;
	set_block_info(49, 29, 5, 0); j++;
	set_block_info(12, 32, 0, 0); j++;
	set_block_info(12, 32, 1, 0); j++;
	set_block_info(25, 32, 2, 0); j++;
	set_block_info(2, 32, 3, 0); j++;
	set_block_info(12, 34, 0, 0); j++;
	set_block_info(12, 34, 1, 0); j++;
	set_block_info(31, 34, 2, 0); j++;
	set_block_info(2, 34, 3, 0); j++;
	set_block_info(49, 37, 0, 0); j++;
	set_block_info(49, 38, 0, 0); j++;
	set_block_info(49, 38, 1, 0); j++;
	set_block_info(49, 38, 2, 0); j++;
	set_block_info(49, 39, 0, 0); j++;
	set_block_info(49, 39, 1, 0); j++;
	set_block_info(49, 39, 2, 0); j++;
	set_block_info(49, 39, 3, 0); j++;
	set_block_info(49, 39, 4, 0); j++;


	set_leftside = 40;

	for (i = 0; i <= PATTERN_LIMIT; i++) //structureの配列をもとに左から配置していく
	{
		switch (stage_structure[i])
		{
		case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12: case 13:
		{
			for (k = 0; k <= 16; k++) //床の配置
			{
				set_block_info(76, k, -1, set_leftside); j++;
			}

			for (k = 0; k <= 5; k++) //配置するブロックがどの単語のひらがなか，の単語の番号を決める
			{
				Q[k] = rand_dic_4(mt);
			}

			set_block_info(dic[Q[0]][0], 3, 0, set_leftside); j++;
			set_block_info(dic[Q[0]][2], 3, 2, set_leftside); j++;
			set_block_info(dic[Q[0]][3], 3, 4, set_leftside); j++;
			set_block_info(dic[Q[0]][1], 4, 0, set_leftside); j++;
			set_block_info(dic[Q[1]][2], 4, 2, set_leftside); j++;
			set_block_info(dic[Q[1]][0], 4, 4, set_leftside); j++;
			set_block_info(dic[Q[1]][3], 6, 0, set_leftside); j++;
			set_block_info(dic[Q[2]][1], 5, 2, set_leftside); j++;
			set_block_info(dic[Q[1]][1], 6, 4, set_leftside); j++;
			set_block_info(dic[Q[2]][0], 7, 0, set_leftside); j++;
			set_block_info(dic[Q[2]][2], 7, 2, set_leftside); j++;
			set_block_info(choose_hiragana() , 7, 4, set_leftside); j++;
			set_block_info(dic[Q[2]][3], 8, 0, set_leftside); j++;
			set_block_info(choose_hiragana(), 8, 2, set_leftside); j++;
			set_block_info(choose_hiragana(), 8, 4, set_leftside); j++;
			set_block_info(dic[Q[4]][1], 10, 0, set_leftside); j++;
			set_block_info(dic[Q[4]][0], 9, 2, set_leftside); j++;
			set_block_info(choose_hiragana(), 10, 4, set_leftside); j++;
			set_block_info(dic[Q[5]][1], 11, 0, set_leftside); j++;
			set_block_info(dic[Q[5]][3], 11, 2, set_leftside); j++;
			set_block_info(dic[Q[4]][2], 11, 4, set_leftside); j++;
			set_block_info(dic[Q[5]][0], 12, 0, set_leftside); j++;
			set_block_info(dic[Q[5]][2], 12, 2, set_leftside); j++;
			set_block_info(dic[Q[4]][3], 12, 4, set_leftside); j++;

			set_block_info(49, 0, 1, set_leftside); j++;
			set_block_info(49, 1, 3, set_leftside); j++;
			set_block_info(49, 15, 1, set_leftside); j++;
			set_block_info(49, 14, 3, set_leftside); j++;

			set_leftside += 17;
		}break;

		case 254: //ボーナスエリア
		{
			for (k = 0; k <= 16; k++) //床の配置
			{
				set_block_info(76, k, -1, set_leftside); j++;
			}

			set_block_info(79, 4, 0, set_leftside); j++;
			set_block_info(79, 5, 0, set_leftside); j++;
			set_block_info(79, 4, 2, set_leftside); j++;
			set_block_info(79, 5, 2, set_leftside); j++;
			set_block_info(79, 4, 4, set_leftside); j++;
			set_block_info(79, 5, 4, set_leftside); j++;
			set_block_info(79, 11, 0, set_leftside); j++;
			set_block_info(79, 12, 0, set_leftside); j++;
			set_block_info(79, 11, 2, set_leftside); j++;
			set_block_info(79, 12, 2, set_leftside); j++;
			set_block_info(79, 11, 4, set_leftside); j++;
			set_block_info(79, 12, 4, set_leftside); j++;

			set_block_info(77, 5, 6, set_leftside); j++;
			set_block_info(77, 11, 6, set_leftside); j++;

			set_block_info(49, 1, 3, set_leftside); j++;
			set_block_info(49, 2, 1, set_leftside); j++;
			set_block_info(49, 7, 1, set_leftside); j++;
			set_block_info(49, 8, 3, set_leftside); j++;
			set_block_info(49, 9, 1, set_leftside); j++;
			set_block_info(49, 14, 1, set_leftside); j++;
			set_block_info(49, 15, 3, set_leftside); j++;


			set_leftside += 17;
		}break;

		case 255: //終端
		{
			for (k = 0; k <= 10; k++) //床の配置
			{
				set_block_info(76, k, -1, set_leftside); j++;
			}

			for (k = 0; k <= 10; k++) //床の配置
			{
				for (l = 0; l <= 10; l++) //床の配置
				{
					set_block_info(49, l, k, set_leftside); j++;
				}
			}

			set_leftside += 11;
		}break;

		default:
		{
			set_block_info(76, 0, -1, set_leftside); j++;
			set_leftside += 1;
		}break;
		}
	}

	object_on_stage = j;
	std::cout << object_on_stage <<"個のオブジェクトが配置されました" << std::endl;
}

void game_reset(void) //ステージ構造などゲームを開始する直前にゲームを準備する
{
	score = 0;
	score_get_hiragana = 0;
	score_leave_hiragana = 0;
	max_most_hiragana = 0;
	score_most_hiragana = 0;
	score_tango = 0;
	time = TIME_LIMIT * 60;
	slot[0] = 0;
	slot[1] = 0;
	slot[2] = 0;
	slot[3] = 0;

	//ステージの構造を再構成

	for (i = 0; i < object_on_stage; i++) //ブロックの情報をリセット
	{
		object_block[i][0] = 0;
		object_block[i][1] = 0;
		object_block[i][2] = 0;
	}

	for (i = 0; i < object_on_stage; i++)
	{
		slot_start[i] = choose_hiragana();
	}

	for (i = 0; i < PATTERN_LIMIT; i++)
	{
		stage_structure[i] = choose_pattern();

		if (i % 6 == 5) //6パターンおきにボーナスステージ配置
		{
			stage_structure[i] = 254;
		}
	}

	stage_structure[18] = 255; //最後の壁（暫定）なんか無限だとクソヌルゲーになるらしいので

	block_standby(); //ブロックの配置（再構成）
}

void game_shutdown(void) //Escキーを押下されたとき諸々を終了させる
{
	fclose(fp);
	fclose(fp_dic_4); 
	exit(0);
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

		UI_title.SetImage(0, -352);
		UI_gamestart.SetImage(-80, 48);
		UI_pressstart.SetImage(140, -32);

		glutPostRedisplay();

	}break;


	case 1:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		UI_08.SetImage(0, -172);
		UI_13.SetImage(0, 64); //次へ進む

		UI_14.SetImage(0, -444); //are you ready?

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
		UI_08.SetImage(0, -172);

	}break;

	case 3:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		UI_score.SetImage(164, -396);

		if (ranking == 0)
		{
			UI_newrecord.SetImage(0, -426);
		}

		UI_ranking.SetImage(0, -344);
		SetNumImage(-224, -420, 320, 40, score);

		UI_09.SetImage(0, 96);
		UI_rank01.SetImage(204, -228);
		UI_rank02.SetImage(204, -180);
		UI_rank03.SetImage(204, -132);
		UI_rank04.SetImage(204, -84);
		UI_rank05.SetImage(204, -36);
		SetNumImage(-244, -244, 320, 40, high_score[0]);
		SetNumImage(-244, -200, 320, 40, high_score[1]);
		SetNumImage(-244, -152, 320, 40, high_score[2]);
		SetNumImage(-244, -104, 320, 40, high_score[3]);
		SetNumImage(-244, -56, 320, 40, high_score[4]);

		arrow.SetImage(334, -224 + 48 * ranking);

	}break;

	case 4:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		UI_score.SetImage(164, -396);

		if (ranking == 0)
		{
			UI_newrecord.SetImage(0, -426);
		}

		SetNumImage(-224, -420, 320, 40, score);

		UI_09.SetImage(0, 96);
		UI_result01.SetImage(128, -232);
		UI_result02.SetImage(128, -184);
		UI_result03.SetImage(128, -136);
		UI_result04.SetImage(128, -88);

		block_hiragana[score_most_hiragana].SetImage(-224, -132);
		SetNumImage(-244, -244, 320, 40, score_get_hiragana);
		SetNumImage(-244, -200, 320, 40, score_leave_hiragana);

		SetNumImage(-244, -104, 320, 40, score_tango);

	}break;

	case 5:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y + 128, 0, camera_x, camera_y + 128, 1, 0, 1, 0);

		UI_10.SetImage(440 + player->center_x, 200);
		UI_11.SetImage(0 + player->center_x, 272);
		UI_12.SetImage(-440 + player->center_x, 200);
		UI_slot_base.SetImage(0 + player->center_x, 200); //スロットの基盤
		SetNumImage(360 + player->center_x, 132, 160, 20, time / 60); //タイマー
		SetNumImage(360 + player->center_x, 200, 160, 20, score); //スコア
		SetNumImage(360 + player->center_x, 268, 160, 20, high_score[0]); //ハイスコア

		block_hiragana_UI[slot[0]].SetImage(216 + player->center_x, 176); //ひらがなスロット
		block_hiragana_UI[slot[1]].SetImage(72 + player->center_x, 176);
		block_hiragana_UI[slot[2]].SetImage(-72 + player->center_x, 176);
		block_hiragana_UI[slot[3]].SetImage(-216 + player->center_x, 176);


		if (lamp_timer_01 == 0)
		{
			UI_slot_highlight.SetImage(216 - slot_select * 144 + player->center_x, 176); //スロットの選択箇所
		}

		for (int i = -6400; i < 6400; i++) { //ここから５つ背景を描画
			BG_05.SetImage(i * 1024 + (player->center_x *1.0), -224);
		}

		for (int i = -6400; i < 6400; i++) {
			BG_04.SetImage(i * 1024 + (player->center_x *0.75), -224);
		}

		for (int i = -6400; i < 6400; i++) {
			BG_03.SetImage(i * 1024 + (player->center_x *0.5), -224);
		}

		for (int i = -6400; i < 6400; i++) {
			BG_02.SetImage(i * 1024 + (player->center_x *0.25), -224);
		}

		for (int i = -6400; i < 6400; i++) {
			BG_01.SetImage(i * 1024 + (player->center_x *0.0), -224);
		}

		if (gun_timer > 0) //銃を発射しているときの
		{
			if (player->direction == 1) { player->ChangeImage(4); }
			else { player->ChangeImage(9); }
		}

		else if (player_jump == true)
		{
			if (player->direction == 1) { player->ChangeImage(3); }
			else { player->ChangeImage(8); }
		}

		else if (walk_timer == 0)
		{
			if (player->direction == 1) { player->ChangeImage(1); }
			else { player->ChangeImage(6); }
		}
		else
		{
			if (walk_timer % 12 >= 0 && walk_timer % 12 <= 2) { if (player->direction == 1) { player->ChangeImage(1); } else { player->ChangeImage(6); } }
			else if (walk_timer % 12 >= 3 && walk_timer % 12 <= 5) { if (player->direction == 1) { player->ChangeImage(2); } else { player->ChangeImage(7); } }
			else if (walk_timer % 12 >= 6 && walk_timer % 12 <= 8) { if (player->direction == 1) { player->ChangeImage(1); } else { player->ChangeImage(6); } }
			else if (walk_timer % 12 >= 9 && walk_timer % 12 <= 11) { if (player->direction == 1) { player->ChangeImage(0); } else { player->ChangeImage(5); } }

		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);

		switch (player->direction)
		{
		case 0:
		{
			player->UpdateDirR();
		}	break;

		case 1:
		{
			player->UpdateDirR();
		}	break;
		}
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);

		for (i = 0; i < object_on_stage; i++) //オブジェクトとなるブロックここで全部描画
		{
			if (object_block[i][0] != 0 && object_block[i][0] != 77 && object_block[i][0] != 79)
			{
				block_hiragana[object_block[i][0]].SetImage(double(object_block[i][1]), double(object_block[i][2]));
			}

			else if (object_block[i][0] == 77) //お題箱描画
			{
				if (slot[0] == 0 && slot[1] == 0 && slot[2] == 0 && slot[3] == 0)
				{
					block_hiragana[77].SetImage(double(object_block[i][1]), double(object_block[i][2]));
				}

				else //スロットが空っぽじゃなかったらお題箱は起動しない
				{
					block_hiragana[78].SetImage(double(object_block[i][1]), double(object_block[i][2]));
				}

			}

			else if (object_block[i][0] == 79) //お題箱描画
			{
				block_hiragana[hiragana_roulette[((hiragana_roulette_timer + (slot_start[i])*60))%(74*60) / 60]].SetImage(double(object_block[i][1]), double(object_block[i][2]));
				if (lamp_timer_block % 20 >= 0 && lamp_timer_block % 20 <= 4) { block_light_2.SetImage(double(object_block[i][1]), double(object_block[i][2])); }
				if (lamp_timer_block % 20 >= 5 && lamp_timer_block % 20 <= 9) { block_light_1.SetImage(double(object_block[i][1]), double(object_block[i][2])); }
				if (lamp_timer_block % 20 >= 10 && lamp_timer_block % 20 <= 14) { block_light_2.SetImage(double(object_block[i][1]), double(object_block[i][2])); }
			}
		}

		if (lamp_timer_01 % 10 >= 6 && lamp_timer_01 > 0) //Ｋキーを押した後スロットを点滅させる
		{
			switch (word_hit)
			{
			case false:
			{
				UI_slot_decision.SetImage(216 + player->center_x, 176);
				UI_slot_decision.SetImage(72 + player->center_x, 176);
				UI_slot_decision.SetImage(-72 + player->center_x, 176);
				UI_slot_decision.SetImage(-216 + player->center_x, 176);
			}break;

			case true:
			{
				UI_slot_decision_green.SetImage(216 + player->center_x, 176);
				UI_slot_decision_green.SetImage(72 + player->center_x, 176);
				UI_slot_decision_green.SetImage(-72 + player->center_x, 176);
				UI_slot_decision_green.SetImage(-216 + player->center_x, 176);
			}break;
			}
		}


		if (lamp_timer_02 > 0) //Ｋキーを押した後ふきだしとエフェクト点灯
		{
			switch (word_hit)
			{
			case true: //プレイヤーの吹き出しと加点スコアを描画
			{
				player_fukidashi01.SetImage(player->center_x, player->center_y - 80);
				if (score >= 100000) { SetNumImage_2(player->center_x - 12 * 9, player->center_y - 128, 192, 24, score_word); }
				else if (score >= 10000 && score <= 99999) { SetNumImage_2(player->center_x - 12 * 8, player->center_y - 128, 192, 24, score_word); }
				else if (score >= 1000 && score <= 9999) { SetNumImage_2(player->center_x - 12 * 7, player->center_y - 128, 192, 24, score_word); }
				else if (score >= 100 && score <= 999) { SetNumImage_2(player->center_x - 12 * 6, player->center_y - 128, 192, 24, score_word); }
				else if (score >= 10 && score <= 99) { SetNumImage_2(player->center_x - 12 * 5, player->center_y - 128, 192, 24, score_word); }
				else if (score >= 0 && score <= 9) { SetNumImage_2(player->center_x - 12 * 4, player->center_y - 128, 192, 24, score_word); }

			}break;

			case false: //プレイヤーの吹き出しとペナルティを描画
			{
				player_fukidashi02.SetImage(player->center_x, player->center_y - 80);
				player_penalty.SetImage(player->center_x, player->center_y - 120);

			}break;
			}
		}

		if (lamp_timer_02 % 7 >= 3 && lamp_timer_02 > 0) //Ｋキーを押した後プレイヤーエフェクト点滅
		{
			switch (word_hit)
			{
			case true:
			{
				player_maru.SetImage(player->center_x, player->center_y);
			}break;

			case false:
			{
				player_batsu.SetImage(player->center_x, player->center_y);
			}break;
			}
		}

		if (flag_bullet_exist == true)
		{
			bullet->ChangeImage(0); //弾丸の描画
		}


	}break;

	case 6:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		UI_gameover.SetImage(0, -416);
		UI_tonext.SetImage(0, 64);

	}break;

	}
	glutSwapBuffers();
}

void idle(void)
{
	double speed = 8;
	double player_y_before = 0;


	if (player_jump == true && flag_move_y == true) //ジャンプ
	{
		player_y_before = player->center_y;

		switch (flag_jump_slow)
		{
		case true://ふわふわ落下モード
		{
			player->Move(0, -(15.8 * jump_timer - 9.8* jump_timer * sqrt(jump_timer) *0.5)*0.0015 * 36);
		}break;

		case false:
		{
			player->Move(0, -(15.8 * jump_timer - 9.8* jump_timer * sqrt(jump_timer) *0.5)*0.01 * 128);
		}break;
		}

		if (player->center_y - player_y_before > 0) //前のフレームのｙ座標よりも落ちていたらふわふわ落下モードに
		{
			flag_jump_slow = true;
		}

		flag_collision_U = false;
		flag_collision_D = false;

		for (i = 0; i < object_on_stage; i++)
		{
			if (player->center_y < object_block[i][2] && object_block[i][0] != 0) //プレイやーがブロックより下側にいる時，かつ比較するオブジェクトブロックが空白でないとき
			{
				if (abs(player->center_x - double(object_block[i][1])) < 48 && abs(player->center_y - double(object_block[i][2])) <= 64) //ブロックとの距離がx<48 y<64であるとき
				{
					flag_collision_D = true;

					if (height_c > object_block[i][2])
					{
						height_c = object_block[i][2];
					}
				}
			}
		}

		for (i = 0; i < object_on_stage; i++)
		{
			if (player->center_y > object_block[i][2] && object_block[i][0] != 0) //プレイやーがブロックより上側にいる時，かつ比較するオブジェクトブロックが空白でないとき
			{
				if (abs(player->center_x - double(object_block[i][1])) < 48 && abs(player->center_y - double(object_block[i][2])) <= 64) //ブロックとの距離がx<48 y<64であるとき
				{
					flag_collision_U = true;
				}
			}
		}

		if (flag_collision_U == true) //天井との衝突を感知したフレームでの処理
		{

			jump_timer = JUMP_HIGHEST;
			player->center_y -= ((int)(player->center_y)
				) % 64;
		}

		if (flag_collision_D == true) //地面との衝突を感知したフレームでの処理
		{
			player->center_y = height_c - 64;

			player_jump = false;
			jump_timer = 0;
			flag_jump_slow = false;
		}

		else if (flag_collision_D == false) //地面との衝突を感知したフレームでの処理
		{
			height_c = 8000; //自分の位置よりも十分に低い場所に衝突判定を戻す
		}

		flag_move_y = false;

	}

	else if (player_jump == false && flag_move_y == true) //自由落下の制御
	{
		flag_collision_D = false;

		for (i = 0; i < object_on_stage; i++)
		{
			if (player->center_y < object_block[i][2] && object_block[i][0] != 0) //プレイやーがブロックより下側にいる時，かつ比較するオブジェクトブロックが空白でないとき
			{
				if (abs(player->center_x - double(object_block[i][1])) < 48 && abs(player->center_y - double(object_block[i][2])) <= 64) //ブロックとの距離がx<48 y<64であるとき
				{
					flag_collision_D = true;

					if (height_c > object_block[i][2])
					{
						height_c = object_block[i][2];
					}
				}
			}
		}

		if (flag_collision_D == false) //地面との衝突を感知したフレームでの処理
		{
			height_c = 8000;
		}

		if (flag_collision_D == false) { jump_timer = JUMP_HIGHEST; player_jump = true; } //足場がなくなると自由落下（タイマー = JUMP_HIGHEST で鉛直投げ上げ最高点）

		flag_move_y = false;
	}

	if (onMoveKeyPress_L == true && flag_move_x == true) { //左に移動


		camera_x += speed;

		player->Move(speed, 0);

		for (i = 0; i < object_on_stage; i++)
		{
			if (player->center_x < object_block[i][1] && object_block[i][0] != 0) //プレイやーがブロックより←側にいる時，かつ比較するオブジェクトブロックが空白でないとき
			{
				if (abs(player->center_x - double(object_block[i][1])) < 48 && abs(player->center_y - double(object_block[i][2])) < 60
					) //ブロックとの距離がx<48 y<64であるとき
				{
					flag_collision_L = true;
				}
			}
		}

		if (flag_collision_L == true)
		{
			camera_x -= speed;
			player->Move(-speed, 0);
		}

		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);
		flag_move_x = false;
		flag_collision_L = false;
	}

	if (onMoveKeyPress_R == true && flag_move_x == true) { //右に移動

		camera_x -= speed;

		player->Move(-speed, 0);

		for (i = 0; i < object_on_stage; i++)
		{
			if (player->center_x > object_block[i][1] && object_block[i][0] != 0) //プレイやーがブロックより→側にいる時，かつ比較するオブジェクトブロックが空白でないとき
			{
				if (abs(player->center_x - double(object_block[i][1])) < 48 && abs(player->center_y - double(object_block[i][2])) < 60) //ブロックとの距離がx<48 y<64であるとき
				{
					flag_collision_R = true;
				}
			}
		}

		if (flag_collision_R == true)
		{
			camera_x += speed;
			player->Move(speed, 0);
		}
		
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);
		flag_move_x = false;
		flag_collision_R = false;
	}

	if (flag_bullet_exist == true) //弾丸と語彙ブロックの衝突判定
	{
		switch (bullet->direction) //弾をそのときプレイヤーが向いていた方向に動かす
		{
		case 0:
		{
			bullet->center_x += 16;
		}break;

		case 1:
		{
			bullet->center_x -= 16;
		}break;
		}

		for (i = 0; i < object_on_stage; i++)
		{
			if (flag_bullet_exist == true && object_block[i][0] != 0 && abs(bullet->center_x - double(object_block[i][1])) < 32 && abs(bullet->center_y - double(object_block[i][2])) < 32) //ブロックとの距離がx<32 y<64で32あるとき（ブロックＩＤ＝０すなわち空気の時はスルー）
			{
				flag_bullet_exist = false;
				if (slot[slot_select] == 0 && object_block[i][0] != 49 && object_block[i][0] != 76 && object_block[i][0]!= 77 && object_block[i][0] != 78 && object_block[i][0] != 79)//すでにスロットにひらがなが入っている場合は衝突してもブロック消えないしひらがなも保持されない,あと木はスロットには入れられない（当然
				{
					slot[slot_select] = object_block[i][0]; //弾丸が衝突したブロックをスロットに格納
					object_block[i][0] = 0; //弾丸とブロックが衝突したらお互いの情報を０にする
					score_get_hiragana++;
				}

				else if (object_block[i][0] == 77 && slot[0]==0 && slot[1] == 0 && slot[2] == 0 && slot[3] == 0) //お題箱にヒットしたとき（ごいスロットに何かあるときはＯＦＦ状態になる）
				{
					odai = choose_odai();
					slot[0] = odai_hiragana[odai][0];
					slot[1] = odai_hiragana[odai][1];
					slot[2] = odai_hiragana[odai][2];
					slot[3] = odai_hiragana[odai][3];
					object_block[i][0] = 0; //弾丸とブロックが衝突したらお互いの情報を０にする
				}

				else if (object_block[i][0] == 79) //ひらがなルーレットにヒットした場合
				{
					slot[slot_select] = hiragana_roulette[((hiragana_roulette_timer + (slot_start[i]) * 60)) % (74 * 60) / 60]; //弾丸が衝突したブロックをスロットに格納
					object_block[i][0] = 0; //弾丸とブロックが衝突したらお互いの情報を０にする
					score_get_hiragana++;
				}
			}

		}
	}


	if (time < 0)
	{
		scene = 6;

		fclose(fp);

		if ((fopen_s(&fp, "./dat/score.dat", "w")) != 0) //スコアファイルを読み込む
		{
			std::cout << "スコアファイルを開けませんでした\n" << std::endl;
			exit(4);
		}

		time = TIME_LIMIT;

		if (high_score[0] < score)
		{
			ranking = 0;
			high_score[4] = high_score[3];
			high_score[3] = high_score[2];
			high_score[2] = high_score[1];
			high_score[1] = high_score[0];
			high_score[0] = score;
		}

		else if (high_score[1] < score)
		{
			ranking = 1;
			high_score[4] = high_score[3];
			high_score[3] = high_score[2];
			high_score[2] = high_score[1];
			high_score[1] = score;
		}
		else if (high_score[2] < score)
		{
			ranking = 2;
			high_score[4] = high_score[3];
			high_score[3] = high_score[2];
			high_score[2] = score;
		}

		else if (high_score[3] < score)
		{
			ranking = 3;
			high_score[4] = high_score[3];
			high_score[3] = score;
		}

		else if (high_score[4] < score)
		{
			ranking = 4;
			high_score[4] = score;
		}

		else
		{
			ranking = 5; //ランク外
		}

		for (i = 0; i <= 4; i++)
		{
			fprintf(fp, "%d\n", high_score[i]); //スコアファイルを更新
		}

		for (i = 0; i <= 79; i++)
		{
			if (list_most_hiragana[i] > max_most_hiragana)
			{
				max_most_hiragana = list_most_hiragana[i];
				score_most_hiragana = i;
			}
		}


	}

	if (lamp_timer_02 <= 0)
	{
		word_hit = false; //正解or間違いの演出のエフェクト終了後，語彙スロット未完成状態に
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
		case '\033': game_shutdown(); break;
		case 'g': scene = 1; break; //PRESS GAME START 次の画面へ

		default:
			break;
		}
	}break;

	case 1:
	{
		switch (key) {
		case '\033': game_shutdown(); break;

		case 'l': scene = 5; game_reset(); break;

		default:
			break;
		}
	}break;

	case 2:
	{
		switch (key) {
		case 'p': camera_x = temp_camera_x; camera_y = temp_camera_y;  scene = 5; break;//再開 カメラの位置をゲーム中のに戻す
		case '\033': game_shutdown(); break;
		}
	}break;

	case 3:
	{
		switch (key) {
		case 'l': scene = 4; break; //リザルト画面切り替え
		case 'o': camera_x = 640; camera_y = -544; player->center_x = 0; player->center_y = 0; scene = 5; game_reset(); break;//リトライ
		case 'p': camera_x = 640; camera_y = -544; player->center_x = 0; player->center_y = 0; scene = 1; game_reset(); break;//メニューに戻る
		case '\033':game_shutdown(); break;
		}
	}break;

	case 4:
	{
		switch (key) {
		case 'l': scene = 3; break; //リザルト画面切り替え
		case 'o': camera_x = 640; camera_y = -544; player->center_x = 0; player->center_y = 0; scene = 5; game_reset();  break;//リトライ
		case 'p': camera_x = 640; camera_y = -544; player->center_x = 0; player->center_y = 0; scene = 1; game_reset(); break;//メニューに戻る

		case '\033': game_shutdown(); break;
		}
	}break;

	case 5: //ゲーム画面
	{
		switch (key) {

		case '\033': game_shutdown(); break;
		case 'a': onMoveKeyPress_L = true; /*MoveLock_R = true;*/ player->direction = 0; break;
		case 'd': onMoveKeyPress_R = true; /*MoveLock_L = true;*/ player->direction = 1; break;

		case 'j': if (slot_select > 0 && lamp_timer_02 == 0) { slot_select--; } break; //スロット移動
		case 'l': if (slot_select < 3 && lamp_timer_02 == 0) { slot_select++; } break;
		case 'i': if (lamp_timer_02 == 0) { if (slot[slot_select] != 0) { score_leave_hiragana++; } slot[slot_select] = 0; }  break; //選択中のスロットの場所をからっぽにする

		case 'k': if (lamp_timer_02 == 0 && slot[0] != 0 && slot[1] != 0 && slot[2] != 0 && slot[3] != 0) 
		{ check_goi(slot); lamp_timer_02 = 100;  lamp_timer_01 = 50; slot[0] = 0; slot[1] = 0; slot[2] = 0; slot[3] = 0; }  break;//単語チェック
		case 'v': if (flag_bullet_exist == false) 
		{ 
			bullet->direction = player->direction; bullet_timer = 0; gun_timer = 60; flag_bullet_exist = true; 
			if (player->direction == 0) { bullet->center_x = player->center_x+40; bullet->center_y = player->center_y; }
			else if (player->direction == 1) { bullet->center_x = player->center_x-40; bullet->center_y = player->center_y; }
		} break;

		case 'p': scene = 2; temp_camera_x = camera_x; temp_camera_y = camera_y; camera_x = 640; camera_y = -544; break; //ポーズ カメラの位置をＧＵＩ用の位置にリセット
		//case 't': scene = 6; temp_camera_x = camera_x; temp_camera_y = camera_y; camera_x = 640; camera_y = -544; break; //デバッグ用トリガー1 強制ゲームオーバー
		//case 'b': slot[3] = 19; break; //デバッグ用トリガー2
		//case 'n': slot[0] = 5; slot[1] = 12; slot[2] = 47;  slot[3] = 10; break; //デバッグ用トリガー3（成功時シミュレーション）
		//case 'm': slot[0] = 5; slot[1] = 12; slot[2] = 47;  slot[3] = 6; break; //デバッグ用トリガー4（失敗時シミュレーション）

		case '\040': if (player_jump == false) { player_jump = true; flag_collision_D = false; } break;
		default:
			break;
		}
	}break;

	case 6: //ゲームオーバー画面
	{
		switch (key) {
		case 'l': scene = 3; 
			lamp_timer_01 = 0; lamp_timer_02 = 0;  break; //リザルト画面へ
		case '\033': game_shutdown(); break;
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

	camera_x = 640;
	camera_y = -544;

	if ((double)h/(double)w <= (double)HEIGHT / (double)WIDTH) //規定のアスペクト比より横長の場合
	{
		glViewport((w-(double)h/0.5625)/2, 0, (double)h / 0.5625, h);
	}

	else //規定のあすひより縦長の場合
	{
		glViewport(0, (h-(double)w * 0.5625)/2 , w, (double)w * 0.5625);
	}
	
	glLoadIdentity();
	glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);

	gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

}

void Init() {


	if ((fopen_s(&fp, "./dat/score.dat", "r")) != 0) //スコアファイルを読み込む
	{
		std::cout << "スコアファイルを開けませんでした" << std::endl;
		exit(1);
	}

	if ((fopen_s(&fp_dic_4, "./dat/4moji_dic.dat", "r")) != 0) //辞書ファイルを読み込む
	{
		std::cout << "辞書ファイルを開けませんでした" << std::endl;
		exit(10);
	}

	i = 0;
	while (fscanf_s(fp_dic_4, "%d,%d,%d,%d,", &dic[i][0], &dic[i][1], &dic[i][2], &dic[i][3]) != EOF)
	{
		/*
		if (dic[i][0] == 0 || dic[i][1] == 0 || dic[i][2] == 0 || dic[i][3] == 0)
		{
			std::cout << i << "番目の単語が読み取れませんでした" << std::endl;
			exit(3);
		}
		*/
		i++;
	}

	dic_4_all = i; 
	std::cout << "4文字辞書" << dic_4_all << "単語を読み込みました" << std::endl;


	glClearColor(0.0, 0.0, 0.0, 1.0);
	glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
	GdiplusStartup(&gdiPT, &gdiPSI, NULL);


	LoadImagePNG(L"./pic/num_a0.png", tex_num_a[0]);
	LoadImagePNG(L"./pic/num_a1.png", tex_num_a[1]);
	LoadImagePNG(L"./pic/num_a2.png", tex_num_a[2]);
	LoadImagePNG(L"./pic/num_a3.png", tex_num_a[3]);
	LoadImagePNG(L"./pic/num_a4.png", tex_num_a[4]);
	LoadImagePNG(L"./pic/num_a5.png", tex_num_a[5]);
	LoadImagePNG(L"./pic/num_a6.png", tex_num_a[6]);
	LoadImagePNG(L"./pic/num_a7.png", tex_num_a[7]);
	LoadImagePNG(L"./pic/num_a8.png", tex_num_a[8]);
	LoadImagePNG(L"./pic/num_a9.png", tex_num_a[9]);
	LoadImagePNG(L"./pic/num_b0.png", tex_num_b[0]);
	LoadImagePNG(L"./pic/num_b1.png", tex_num_b[1]);
	LoadImagePNG(L"./pic/num_b2.png", tex_num_b[2]);
	LoadImagePNG(L"./pic/num_b3.png", tex_num_b[3]);
	LoadImagePNG(L"./pic/num_b4.png", tex_num_b[4]);
	LoadImagePNG(L"./pic/num_b5.png", tex_num_b[5]);
	LoadImagePNG(L"./pic/num_b6.png", tex_num_b[6]);
	LoadImagePNG(L"./pic/num_b7.png", tex_num_b[7]);
	LoadImagePNG(L"./pic/num_b8.png", tex_num_b[8]);
	LoadImagePNG(L"./pic/num_b9.png", tex_num_b[9]);
	LoadImagePNG(L"./pic/num_plus.png", tex_num_b[10]);
	player1.LoadImagePNG2(player1.file, player1.tex);
	player2.LoadImagePNG2(player2.file, player2.tex);
	player3.LoadImagePNG2(player3.file, player3.tex);

	floor1.LoadImagePNG2(floor1.file, floor1.tex);


	UI_title.LoadImagePNG2(UI_title.file, UI_title.tex);
	UI_gamestart.LoadImagePNG2(UI_gamestart.file, UI_gamestart.tex);
	UI_pressstart.LoadImagePNG2(UI_pressstart.file, UI_pressstart.tex);
	
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
	UI_13.LoadImagePNG2(UI_13.file, UI_13.tex);
	UI_14.LoadImagePNG2(UI_14.file, UI_14.tex);
	UI_slot_base.LoadImagePNG2(UI_slot_base.file, UI_slot_base.tex);
	UI_slot_highlight.LoadImagePNG2(UI_slot_highlight.file, UI_slot_highlight.tex);
	UI_slot_decision.LoadImagePNG2(UI_slot_decision.file, UI_slot_decision.tex);
	UI_slot_decision_green.LoadImagePNG2(UI_slot_decision_green.file, UI_slot_decision_green.tex);
	player_penalty.LoadImagePNG2(player_penalty.file, player_penalty.tex);
	player_maru.LoadImagePNG2(player_maru.file, player_maru.tex);
	player_batsu.LoadImagePNG2(player_batsu.file, player_batsu.tex);
	player_fukidashi01.LoadImagePNG2(player_fukidashi01.file, player_fukidashi01.tex);
	player_fukidashi02.LoadImagePNG2(player_fukidashi02.file, player_fukidashi02.tex);
	//bullet.LoadImagePNG2(bullet.file, bullet.tex);
	block_light_1.LoadImagePNG2(block_light_1.file, block_light_1.tex);
	block_light_2.LoadImagePNG2(block_light_2.file, block_light_2.tex);


	for (i = 0; i <= 79; i++)
	{
		block_hiragana_UI[i].LoadImagePNG2(block_hiragana_UI[i].file, block_hiragana_UI[i].tex);
		block_hiragana[i].LoadImagePNG2(block_hiragana[i].file, block_hiragana[i].tex);
	}
	for (i = 1; i <= 79; i++)
	{
		choose_hiragana_weight_add[i] = choose_hiragana_weight_add[i - 1] + choose_hiragana_weight[i];
	}


	//コピー用
	//.LoadImagePNG2(.file, .tex);

	player = new AnimationChara(0.0, 0.0, 64.0, 64.0, L"./pic/player_walk1.png");
	player->LoadPNGImage(L"./pic/player_walk2.png");
	player->LoadPNGImage(L"./pic/player_walk3.png");
	player->LoadPNGImage(L"./pic/player_jump.png");
	player->LoadPNGImage(L"./pic/player_kamae.png");
	player->LoadPNGImage(L"./pic/player_walk1_r.png");
	player->LoadPNGImage(L"./pic/player_walk2_r.png");
	player->LoadPNGImage(L"./pic/player_walk3_r.png");
	player->LoadPNGImage(L"./pic/player_jump_r.png");
	player->LoadPNGImage(L"./pic/player_kamae_r.png");

	bullet = new AnimationChara(0.0, 0.0, 64.0, 64.0, L"./pic/bullet.png");


	scene = 0;


	i = 0;
	while (fscanf_s(fp, "%d", &high_score[i]) != EOF)
	{
		i++;
	}

	i = 0;

	for (k = 0; k < dic_4_all; k++)
	{
		//if (k >= 35000 && k< 38000)
		//{
		//	printf("%d:%d %d %d %d\n", k, dic[k][0], dic[k][1], dic[k][2], dic[k][3]);
		//}

		for (j = 0; j <= 3; j++)
		{
			hiragana_weight_4[dic[k][j]][j]++; //ひらがなの登場回数を計算して格納する
		}
	}

	for (k = 0; k < 80; k++) //何文字目の何のひらがなであれば何点，という点数を付与する
	{
		for (j = 0; j <= 3; j++)
		{
			if (hiragana_weight_4[k][j] >= 2000) { hiragana_score_4[k][j] = 2.5; }
			else if (hiragana_weight_4[k][j] >= 1600) { hiragana_score_4[k][j] = 3; }
			else if (hiragana_weight_4[k][j] >= 1300) { hiragana_score_4[k][j] = 5; }
			else if (hiragana_weight_4[k][j] >= 1000) { hiragana_score_4[k][j] = 7.5; }
			else if (hiragana_weight_4[k][j] >= 750) { hiragana_score_4[k][j] = 10; }
			else if (hiragana_weight_4[k][j] >= 500) { hiragana_score_4[k][j] = 15; }
			else if (hiragana_weight_4[k][j] >= 300) { hiragana_score_4[k][j] = 20; }
			else if (hiragana_weight_4[k][j] >= 150) { hiragana_score_4[k][j] = 30; }
			else if (hiragana_weight_4[k][j] >= 75) { hiragana_score_4[k][j] = 50; }
			else if (hiragana_weight_4[k][j] >= 0) { hiragana_score_4[k][j] = 75; }
		}

	}

}


void timer(int value) {

	glutPostRedisplay();
	glutTimerFunc(16, timer, 0); //だいたい60fpsを目指す

	flag_move_x = true;
	flag_move_y = true;

	if (scene == 5) //ゲーム中，タイマー起動
	{
		time--; //ゲーム時間の残りタイムをへらす

		if (lamp_timer_01 > 0) //スロットの点滅時間
		{
			lamp_timer_01--;
		}

		if (lamp_timer_02 > 0) //キャラクターのリアクションの時間
		{
			lamp_timer_02--;
		}

		if (gun_timer > 0) //キャラクターのリアクションの時間
		{
			gun_timer--;
		}

		if (flag_bullet_exist == true) //キャラクターのリアクションの時間
		{
			bullet_timer++;
		}

		if (bullet_timer > 30) //弾が飛んでいる間，弾が発射できなかったりする
		{
			flag_bullet_exist = false;
		}

		lamp_timer_block++;
		if (lamp_timer_block > 400) //ルーレットブロックのランプの点灯エフェクト
		{
			lamp_timer_block=0;
		}

		hiragana_roulette_timer++;
		if (hiragana_roulette_timer > 74*60) //ルーレットブロックの中身の遷移のタイマー
		{
			hiragana_roulette_timer = 0;
		}

		if (player_jump == true)
		{
			jump_timer++;
		}
	}

	if (scene == 6)
	{
		temp_camera_x = camera_x; temp_camera_y = camera_y; camera_x = 640; camera_y = -544;
	}


	if (onMoveKeyPress_L == true || onMoveKeyPress_R == true) //歩きアニメーションのため （画像１→２→３→２というふうに歩き中には４枚の画像を連続で表示する）
	{
		walk_timer++;
	}
}


int main(int argc, char *argv[])
{
	atexit(end);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow("goipachi ver.1.0.9");
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