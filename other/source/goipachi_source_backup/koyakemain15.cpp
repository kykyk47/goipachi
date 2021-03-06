// goipachi.cpp

//使用ライブラリ：GLUT(OpenGL 3.7.6), GdiPlus, SDL2_mixer(SDL2 2.0.10, SDL2_mixer 2.0.4)

//★ステージクリアモードにおいて，地形パターンを増やす手順
//1.choose_pattern()の各文字のdo-whileの条件を変える
//2.block_standbyでcase 0,100,200なり新しく追加する

//★ループ処理の規模が大きくなりそうなところはポインタを使用して安定化を図る
//ポインタの名前は変数(または配列)名の略称っぽいので統一しようということに

//kykyk47用
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
#include <SDL.h>
#include <SDL_mixer.h>


//matsuodora用
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

#define TIME_LIMIT 180 //タイマー．ほぼ1秒に１すすむ
#define dic_3_LIMIT 44000 //3文字辞書の単語数の余地（今後追加できるよう少し多めに設定）
#define dic_4_LIMIT 44000 //4文字辞書の単語数の余地（今後追加できるよう少し多めに設定）
#define dic_5_LIMIT 44000 //5文字辞書の単語数の余地（今後追加できるよう少し多めに設定）
#define OBJECT_LIMIT 10000 //ブロックの制限
#define PATTERN_LIMIT 45 //横に並べるブロック配置パターンの上限
#define MADE_LIMIT 100 //ステージモードにおいて，作れる単語の限界数（すてーじもーどで単語重複不可のステージの時に使う）
#define STAGE_LIMIT 518 //ステージモードの上限
#define STAGE_AVAILABLE 518 //★提出段階で見せられるステージの数

GdiplusStartupInput gdiPSI;
ULONG_PTR gdiPT;

GLuint tex_num[7][11] = { {} }; //数字フォント，[0][]：赤, [1][]：頭上のスコアの数字・緑，[2][0]：ステージセレクト番号・黄色，[3][0]：ステージセレクト番号・未定義茶色，[6][]：記号類，*][10]は「＋」

void play_SE(Mix_Chunk *filename);
void end();
void check_goi(int* moji); //Kキーが押されたあとの語彙スロットの正誤判定
void LoadImagePNG(const wchar_t* filename, GLuint &texture); //画像ロード
void SetNumImage(double x, double y, int size_x, int size_y, int num, int font, int d); //数字を画像にして表示
int choose_hiragana(void); //乱数によりひらがなを1文字選ぶ
int choose_pattern(void); //乱数によりスコアアタックモードのステージ配置パターンを選ぶ
int choose_odai(void); //乱数により「お題ブロック」のお題番号を選ぶ
void set_block_info(int type, int x_grid, int y_grid, int leftside, int blocknum); //ステージファイルやblock_standbyで定義された数値群より実際に配置するブロックの情報へ置き換える
void block_standby(void); //スコアアタックモードで，ステージの生成（準備）をする
void standby_stage(int stage_num); //ステージクリアモードで，ステージの生成（準備）をする
void game_reset(void); //スコアアタックモードでステージを再構成する
void game_shutdown(void); //ゲームを終了し，もろもろを開放する


int ranking; //ランキング表示のため

bool onMoveKeyPress_L = false;
bool onMoveKeyPress_R = false;
bool player_jump = false;
bool player_walk = false;
int jump_timer = 0;  //キャラクターがジャンプしてからの時間を計測
#define JUMP_HIGHEST 11 //キャラクターがジャンプしてから最高点に達した瞬間のjump_timer

bool move_lock = false; //ポーズ時trueになる なんのオブジェクトの移動も許さないマン
int walk_timer = 0; //キャラクターのアニメーション
int lamp_timer_01 = 0; //Ｋキー（決定ボタン）を押した後の赤ライトの点灯
int lamp_timer_02 = 0; //プレイヤーのリアクションのエフェクト
int lamp_timer_block = 0; //ルーレットブロックのエフェクトのアニメーション
int lamp_timer_clear = 0; //クリアランプ全点灯でゴージャスになる
int hiragana_roulette_timer = 0; //ルーレットブロックの中身のタイマー
int hiragana_roulette[74] = { 1,2,3,48,4,5,6,51,7,52,8,53,9,54,10,55,11,56,12,57,13,58,14,59,15,60,16,61,17,62,18,63,47,19,64,20,65,21,22,23,24,25,26,66,71,27,67,72,28,68,73,29,69,74,30,70,75,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,50 }; //ルーレットの中身の順番
int gun_timer = 0; //銃を構えているイラストの表示
int bullet_timer = 0; //弾丸が発射されてからの秒数 一定時間たつと消える（無限に飛び続けるのいやでしょ）

int scene = 0; //◇シーンの追加．0:タイトル 1:スタンバイ画面 2:ポーズ画面 3:リザルト画面A 4リザルト画面B 5:プレイ画面（メイン） 6:GAME OVER画面  7:ステージ・モード選択（ 8:文字数選択メニュー 9:ステージ選択画面 10:ほんとうによろしいですか
int next_scene = 0; //ゲームをやめる→ほんとうによろしいですか？→の分岐などに使う
int mode = 0; //0:スコアアタック 1:ステージモード スコアの計算にも影響する
int mode_mojisu = 4; //スコアアタックの現在選択しているモードの文字数
int stage_medal[3] = {}; //全ステージ中の獲得したメダルの総数を記録

int made_tango[MADE_LIMIT + 1][5] = { {} }; //ステージクリアモードのとき，重複した単語は作れない
int stage_select = 1; //ステージクリアモードで選んでいるステージの番号
int stage_clear[STAGE_LIMIT + 1][5] = { {} }; //ステージクリアモードの進捗状況  0~2 メダル 3:クリア時間ハイスコア 4:ミス回数ハイスコア
int stage_info[STAGE_LIMIT + 1] = {}; //0:未定義（カミングスーン) 3-5:スロット固定あり言葉作成ミッション（固定情報がなければ通常単語作成ミッション），13-15：しりとりミッション， 23-25：ABには同じ言葉が入りますミッション
int stage_nolma[STAGE_LIMIT + 1] = {}; //例：ステージ１で10単語作れミッション→ [1]=10
int stage_time_limit[STAGE_LIMIT + 1] = {}; //例：ステージ1は200秒以内 [1]=200
int stage_time_limit_gold[STAGE_LIMIT + 1] = {}; //例：ステージ1のメダル獲得タイム50秒以内→[1]=50
int stage_slot_constraint[STAGE_LIMIT + 1][5] = { {} }; //語彙スロット固定（例：ステージ1で[][][い][ろ]のときは [1][0]～[1][3]：＿＿いろになる
int stage_block_info[OBJECT_LIMIT][3] = { {} }; //ステージモードの時に配置されるブロックの情報
int score_miss = 0; //スコア（ミス数）（ステージモードのみ情報を使う

int dic_3moji[dic_3_LIMIT][3] = { {} }; //辞書の情報を格納
int dic_4moji[dic_4_LIMIT][4] = { {} }; //辞書の情報を格納
int dic_5moji[dic_5_LIMIT][5] = { {} }; //辞書の情報を格納
int dic_sample[dic_5_LIMIT][5] = { {} }; //辞書の情報を格納

int score_get_hiragana = 0; //入手したひらがなの数
int score_leave_hiragana = 0; //捨てたひらがなの数
int score_most_hiragana = 0; //最も単語に使ったひらがなの数
int max_most_hiragana = 0; //↑を求めるのに使う
int score_tango = 0; //作った単語の数
int score = 0; //ゲーム中でのスコア

int list_most_hiragana[80] = {}; //最も入手したひらがなを決めるのに必要

int hiragana_weight_3[80][5] = { {} }; //各何文字目かの登場回数（単語を構成するひらがなのほかにランダムなひらがなを配置するため，それの生成率を決める重み
int hiragana_weight_4[80][5] = { {} }; //各何文字目かの登場回数（単語を構成するひらがなのほかにランダムなひらがなを配置するため，それの生成率を決める重み
int hiragana_weight_5[80][5] = { {} }; //各何文字目かの登場回数（単語を構成するひらがなのほかにランダムなひらがなを配置するため，それの生成率を決める重み
float hiragana_score_3[80][3] = { {} }; //Kキーを押下した際スロットの各文字の点数を合計するための要素
float hiragana_score_4[80][4] = { {} }; //Kキーを押下した際スロットの各文字の点数を合計するための要素
float hiragana_score_5[80][5] = { {} }; //Kキーを押下した際スロットの各文字の点数を合計するための要素


int odai; //お題の番号を決める

int odai_hiragana_3[75][5] = {
{0,46,0},{0,47,0},{0,50,0},{0,3,0},{0,2,0},{0,0,41},{0,0,33},{0,0,13}
}; //お題のひらがなの配置
#define odai_hiragana_3_all 8

int odai_hiragana_4[75][5] = {
{0,6,2,0},{0,0,2,43},{11,0,0,8},{0,46,0,46},{0,2,0,2},{0,3,0,3},{0,2,15,0},{14,2,0,0},{0,0,6,39},{66,46,0,0},
{0,47,0,3},{0,0,0,48},{6,0,6,0},{16,0,0,46},{0,47,0,2},{0,50,0,50},{0,50,41,0},{26,0,0,8},{39,0,0,50},{0,0,50,65}
}; //お題のひらがなの配置
#define odai_hiragana_4_all 20

int odai_hiragana_5[75][5] = {
{0,0,0,32,41},{0,46,0,46,0},{0,50,0,50,0},{0,3,0,2,0},{0,46,0,0,7},{0,0,0,2,43},{14,2,0,0,0},{0,0,0,4,41},{0,0,0,42,41},{0,0,45,0,0},
{0,45,0,0,0},{0,47,0,0,0}
}; //お題のひらがなの配置
#define odai_hiragana_5_all 12

int high_score_3[5] = { 0,0,0,0,0 }; //レコードされているハイスコア
int high_score_4[5] = { 0,0,0,0,0 }; //レコードされているハイスコア
int high_score_5[5] = { 0,0,0,0,0 }; //レコードされているハイスコア
int slot[5] = { 0,0,0,0,0 }; //語彙スロットの中身
int slot_select = 0; //現在語彙スロットの何文字目を選択しているか
int slot_start[OBJECT_LIMIT] = {}; //オブジェクトがルーレットだった場合ルーレットはどっから始まるか

int object_on_stage = 0; //そのセッションで配置されたブロックの総数

int score_word = 0; //１つ１つのワードのスコア
int time = TIME_LIMIT * 60; //制限時間（単位：フレーム）

int stage_structure[PATTERN_LIMIT] = {}; //スコアアタックモードでの地形生成パターンの生成順
int set_leftside = 0; //スコアアタックモードでのブロック配置時，地形パターンの生成の左端を指定し，そこを基準に配置していく（グローバル座標的なやつ）

int before_stageclear_miss; //ステージクリアモードにて，new record表示には更新前のクリアタイムなどが必要なのでそれをとっておく
int before_stageclear_time; //〃

int object_block[OBJECT_LIMIT][3] = { {} }; //衝突判定に使う，プレイヤーとの距離の条件を満たすためにこの配列にオブジェクトの情報を記載（ブロックの種類,中心座標（ｘとｙ））
int height_c = 0; //衝突判定に使う，ジャンプした後着地できる位置（高さ）最も高い座標


FILE *fp_score_3; //スコアファイル
FILE *fp_score_4; //スコアファイル
FILE *fp_score_5; //スコアファイル
FILE *fp_dic_3; //３文字辞書ファイル（未完成）
FILE *fp_dic_4; //４文字辞書ファイル
FILE *fp_dic_5; //５文字辞書ファイル（未完成）
FILE *fp_stageclear; //ステージクリア進捗
FILE *fp_stage_structure_info; //ステージの情報
FILE *fp_stage_nolma_info; //ステージのノルマ情報

FILE *fp_dic_sample_i; //★辞書ファイル（ひらがな格納）を数値化できるように工夫するテスト（まだ実用化には至っておらず・・・がんばります）
FILE *fp_dic_sample_o;

int dic_3_all = 0; //実際に辞書に登録されていた単語数
int dic_4_all = 0;
int dic_5_all = 0;
int dic_sample_all = 0;
bool word_hit = false; //Ｋキー押下後，辞書に存在していたかどうか(trueなら得点＋エフェクト，falseなら時間減少＋エフェクト）

int i, j, k, l;

double camera_x = 0; //カメラの位置
double camera_y = 0;
double temp_camera_x = 0; //◇シーン移動によりカメラの位置をリセットするため，ゲーム中のカメラ位置をストックしておく
double temp_camera_y = 0;
//void LoadImagePNG(const wchar_t* filename, GLuint &texture);

int speed = 0; //プレイヤーが進むスピード

bool flag_move_x = true; //次のタイマー関数が呼び出されるまでx方向の移動を与えないための制御
bool flag_move_y = true; //次のタイマー関数が呼び出されるまでy方向の移動を与えないための制御
bool flag_collision_L = false; //衝突判定（プレイヤーの左方向）
bool flag_collision_R = false; //衝突判定（右方向）
bool flag_collision_U = false; //衝突判定（上方向）
bool flag_collision_D = false; //衝突判定（下方向）

bool flag_move_bullet = true; //次のタイマー関数が呼び出されるまで弾丸の移動を与えないための制御
bool flag_jump_slow = false; //ジャンプ→落下時ゆっくり降りるかんじにするトリガー
bool flag_bullet_exist = false; //弾丸が存在して動いている状態（当たり判定起動）

bool flag_SE = true; //SDL_mixerでセットアップできなかったとき以降音を鳴らさないようにする（デフォルトはtrue，ラボのマイクがないＰＣでもできるように）

int time_1flame; //デバッグ用，1フレームでどれだけ進んだか，どれだけ時間がたっているか（ms）（下３つも同じ）
double speed_1flame;
int time_temp;
double speed_temp;


std::random_device rnd;
std::mt19937 mt(rnd()); //なんか乱数のタネを決定するやつ
std::uniform_int_distribution<> rand_hiragana(1, 75); //スコアアタックモードのパターン中に空いている部分やルーレットブロックの開始点をばらばらにするために使う
std::uniform_int_distribution<> rand_pattern3(1, 15); //スコアアタックモードで3文字用に適したステージの構造（パターンID1~100）をランダムに生成するために選ぶ
std::uniform_int_distribution<> rand_pattern4(101, 115); //スコアアタックモードで4文字用に適したステージの構造（パターンID101~200）をランダムに生成するために選ぶ
std::uniform_int_distribution<> rand_pattern5(201, 215); //スコアアタックモードで5文字用に適したステージの構造（パターンID201~300）をランダムに生成するために選ぶ
std::uniform_int_distribution<> rand_odai(1, 75); //?ブロックを選んだときにスロットの中身をランダムに選ぶ
std::uniform_int_distribution<> rand_dic_3(1, dic_3_LIMIT); //3文字辞書のステージで，ブロックを配置するときに使う
std::uniform_int_distribution<> rand_dic_4(1, dic_4_LIMIT); //4文字辞書のステージで，ブロックを配置するときに使う
std::uniform_int_distribution<> rand_dic_5(1, dic_5_LIMIT); //5文字辞書のステージで，ブロックを配置するときに使う


class GameObject  //大元となるクラス．
{
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
		if (center_x <= camera_x + 1800 && center_x >= camera_x - 1800) {
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

class MoveObject :public GameObject
{
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

class AnimationChara :public MoveObject
{
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

GameObject UI_title = GameObject(0, 0, 768, 256, L"./pic/title_a.png"); //タイトル画像
GameObject UI_quit = GameObject(0, 0, 256, 32, L"./pic/title_quit.png"); //タイトル画像での「Q:ゲームをやめる」

GameObject UI_gamestart = GameObject(0, 0, 512, 64, L"./pic/game_start.png"); //タイトル画面での「Game Start」
GameObject UI_pressstart = GameObject(0, 0, 128, 64, L"./pic/press.png"); //タイトル画面での「PRESS START↓」

GameObject UI_pause = GameObject(0, 0, 1024, 128, L"./pic/menu_pause.png"); //ポーズ画面の「PAUSE」
GameObject UI_return_0 = GameObject(0, 0, 768, 96, L"./pic/pause_menu_0.png"); //ポーズ画面の「ゲームに戻る，ステージ選択に戻る，初めからやり直す」
GameObject UI_return_1 = GameObject(0, 0, 768, 96, L"./pic/pause_menu_1.png"); //ポーズ画面の「ゲームに戻る，ステージ選択に戻る，初めからやり直す」
GameObject UI_gameover = GameObject(0, 0, 1024, 128, L"./pic/gameover.png"); //ゲームオーバー画面の「TIME UP!」
GameObject UI_tonext = GameObject(0, 0, 1024, 128, L"./pic/menu_tonext01.png"); // 「K: つぎへすすむ」
GameObject UI_difficulty01 = GameObject(0, 0, 256, 64, L"./pic/difficulty_menu01.png"); //未使用
GameObject UI_difficulty02 = GameObject(0, 0, 256, 64, L"./pic/difficulty_menu02.png"); //未使用
GameObject UI_difficulty03 = GameObject(0, 0, 256, 64, L"./pic/difficulty_menu03.png"); //未使用
GameObject UI_07 = GameObject(0, 0, 384, 192, L"./pic/menu_UI_07.png"); //難易度選択操作方法，Let's GO
GameObject UI_08 = GameObject(0, 0, 512, 256, L"./pic/menu_UI_08.png"); //ゲーム中での操作方法をあらわした簡単な画面
GameObject UI_09 = GameObject(0, 0, 1024, 64, L"./pic/menu_UI_09.png");  //リザルト画面の「表示切替，リトライ．メニューに戻る」
GameObject UI_13 = GameObject(0, 0, 1024, 128, L"./pic/menu_tonext02.png"); //「K: Let's go!」
GameObject UI_14 = GameObject(0, 0, 1024, 128, L"./pic/are_you_ready.png"); //Are you ready?画面のAre you ready?
GameObject UI_result01 = GameObject(0, 0, 384, 48, L"./pic/result_01.png"); //リザルト画面の「入手したひらがなの数：」
GameObject UI_result02 = GameObject(0, 0, 384, 48, L"./pic/result_02.png"); //リザルト画面の「捨てたひらがなの数：」
GameObject UI_result03 = GameObject(0, 0, 384, 48, L"./pic/result_03.png"); //リザルト画面の「もっとも入手したひらがな：」
GameObject UI_result04 = GameObject(0, 0, 384, 48, L"./pic/result_04.png"); //リザルト画面の「作った単語数」
GameObject UI_score = GameObject(0, 0, 128, 32, L"./pic/score.png"); //リザルト画面（スコアアタックモード）の「score」
GameObject UI_newrecord = GameObject(0, 0, 512, 256, L"./pic/result_newrecord.png"); //リザルト画面（スコアアタックモード）の「new record」
GameObject UI_newrecord_stage = GameObject(0, 0, 96, 96, L"./pic/result_newrecord_stage.png"); //リザルト画面（ステージクリアモード）の「new record」
GameObject UI_ranking = GameObject(0, 0, 384, 192, L"./pic/result_ranking.png"); //リザルト画面（スコアアタックモード）の「ranking」
GameObject UI_rank01 = GameObject(0, 0, 80, 40, L"./pic/ranking_01.png"); //リザルト画面（スコアアタックモード）の「ランキング1位」
GameObject UI_rank02 = GameObject(0, 0, 80, 40, L"./pic/ranking_02.png"); //リザルト画面（スコアアタックモード）の「ランキング2位」
GameObject UI_rank03 = GameObject(0, 0, 80, 40, L"./pic/ranking_03.png"); //リザルト画面（スコアアタックモード）の「ランキング3位」
GameObject UI_rank04 = GameObject(0, 0, 80, 40, L"./pic/ranking_04.png"); //リザルト画面（スコアアタックモード）の「ランキング4位」
GameObject UI_rank05 = GameObject(0, 0, 80, 40, L"./pic/ranking_05.png"); //リザルト画面（スコアアタックモード）の「ランキング5位」
GameObject BG_01 = GameObject(0, 0, 1024, 512, L"./pic/bg01.png"); //プレイ画面の背景
GameObject BG_02 = GameObject(0, 0, 1024, 512, L"./pic/bg02.png"); //プレイ画面の背景
GameObject BG_03 = GameObject(0, 0, 1024, 512, L"./pic/bg03.png"); //プレイ画面の背景
GameObject BG_04 = GameObject(0, 0, 1024, 512, L"./pic/bg04.png"); //プレイ画面の背景
GameObject BG_05 = GameObject(0, 0, 1024, 512, L"./pic/bg05.png"); //プレイ画面の背景
GameObject UI_10 = GameObject(0, 0, 384, 192, L"./pic/menu_UI_10.png"); //プレイ画面（スコアアタックモード）の下左「timer, score, high score」
GameObject UI_10_2 = GameObject(0, 0, 384, 192, L"./pic/menu_UI_10_2.png"); //プレイ画面（ステージクリアモード）の左下「stage no, timer, score , miss」
GameObject UI_11 = GameObject(0, 0, 256, 64, L"./pic/menu_UI_11.png"); //プレイ画面の画面下スロット操作方法簡易
GameObject UI_12 = GameObject(0, 0, 384, 192, L"./pic/menu_UI_12.png"); //プレイ画面右下「P:ポーズ，操作方法」
GameObject UI_15 = GameObject(0, 0, 1024, 64, L"./pic/menu_UI_15.png"); //リザルト画面（ステージクリアモード）の「リトライ，ステージ選択に戻る」
GameObject UI_16 = GameObject(0, 0, 768, 96, L"./pic/menu_UI_16.png"); //ステージ選択画面（定義済）の「もどる，ステージ選択，決定」
GameObject UI_17 = GameObject(0, 0, 768, 96, L"./pic/menu_UI_17.png"); //ステージ選択画面（未定義）の「もどる，ステージ選択」
GameObject UI_slot_base_3 = GameObject(0, 0, 768, 192, L"./pic/slot_base_3.png"); //プレイ画面の3文字スロットの下地
GameObject UI_slot_base_4 = GameObject(0, 0, 768, 192, L"./pic/slot_base_4.png"); //プレイ画面の4文字スロットの下地
GameObject UI_slot_base_5 = GameObject(0, 0, 768, 192, L"./pic/slot_base_5.png"); //プレイ画面の5文字スロットの下地
GameObject UI_slot_highlight = GameObject(0, 0, 192, 192, L"./pic/slot_highlight.png"); //プレイ画面のスロットの今選択している位置のハイライト
GameObject UI_slot_decision = GameObject(0, 0, 192, 192, L"./pic/slot_decision.png"); //プレイ画面の単語決定時→スロット赤く点滅部分
GameObject UI_slot_decision_green = GameObject(0, 0, 192, 192, L"./pic/slot_decision_green.png");  //プレイ画面の単語決定時→スロット緑に点滅部分


GameObject UI_slot_same[3] = { //スロット用
GameObject(0, 0, 192, 192, L"./pic/slot_null.png"), //空白 ID:0
GameObject(0, 0, 192, 192, L"./pic/slot_A.png"), //ＡＢには同じ単語が入りますミッションでのＡ
GameObject(0, 0, 192, 192, L"./pic/slot_B.png") }; //ＡＢには同じ単語が入りますミッションでのＢ

GameObject slot_same_block[3] = { //ステージ概要用
GameObject(0, 0, 92, 92, L"./pic/slot_null.png"), //空白 ID:0
GameObject(0, 0, 92, 92, L"./pic/slot_A.png"),
GameObject(0, 0, 92, 92, L"./pic/slot_B.png") };

GameObject slot_same_block_mini[3] = { //エディタ用（ここでは使ってない
GameObject(0, 0, 48, 48, L"./pic/slot_null.png"), //空白 ID:0
GameObject(0, 0, 48, 48, L"./pic/slot_A.png"),
GameObject(0, 0, 48, 48, L"./pic/slot_B.png") };

GameObject arrow = GameObject(0, 0, 128, 128, L"./pic/arrow.png"); //リザルト画面のハイスコア「今何位だったよ」
GameObject select_highlight = GameObject(0, 0, 136, 136, L"./pic/Y.png"); //難易度選択などのハイライト

GameObject player_penalty = GameObject(0, 0, 96, 24, L"./pic/player_penalty.png"); //プレイ画面でのプレイヤーの上にバツ印
GameObject player_maru = GameObject(0, 0, 64, 64, L"./pic/maru.png"); //プレイ画面のプレイヤーの上に丸印
GameObject player_batsu = GameObject(0, 0, 64, 64, L"./pic/batsu.png");//プレイ画面でのプレイヤーの上にバツ印
GameObject player_fukidashi01 = GameObject(0, 0, 64, 64, L"./pic/fukidashi1.png");//プレイ画面でのプレイヤーの上に吹き出し（〇
GameObject player_fukidashi02 = GameObject(0, 0, 64, 64, L"./pic/fukidashi2.png");//プレイ画面でのプレイヤーの上に吹き出し（×

GameObject UI_confirm = GameObject(0, 0, 1024, 128, L"./pic/confirm.png"); //本当によろしいですか？
GameObject UI_confirm_2 = GameObject(0, 0, 1024, 128, L"./pic/confirm_2.png"); //「Y:はい　N:いいえ」
GameObject UI_mode_score_attack = GameObject(0, 0, 512, 256, L"./pic/mode_score_attack.png"); //モード選択画面の「SCORE ATTACK」
GameObject UI_mode_stage_clear = GameObject(0, 0, 512, 256, L"./pic/mode_stage_clear.png"); //モード選択画面の「STAGE CLEAR」
GameObject UI_mode_highlight = GameObject(0, 0, 512, 256, L"./pic/mode_highlight.png"); // モード選択画面での選択中のハイライト
GameObject UI_mode_description_1 = GameObject(0, 0, 512, 512, L"./pic/mode_description_1.png"); //モード選択画面の「SCORE ATTACK」の説明
GameObject UI_mode_description_2 = GameObject(0, 0, 512, 512, L"./pic/mode_description_2.png"); //モード選択画面の「STAGE CLEAR」の説明
GameObject UI_slot_AB_description = GameObject(0, 0, 512, 32, L"./pic/slot_constraint_description2.png"); //ステージ選択画面の「ＡＢには同じひらがなが入ります」


GameObject UI_mode_select_UI = GameObject(0, 0, 768, 96, L"./pic/mode_select_UI.png"); //モード選択画面の「もどる，モード選択，決定」

GameObject UI_mode_mojisu = GameObject(0, 0, 1024, 128, L"./pic/select_mojisu.png"); //文字数選択画面の「文字数選択」
GameObject UI_mode_stage = GameObject(0, 0, 1024, 128, L"./pic/select_stage.png"); //ステージ選択画面の「ステージ選択」
GameObject UI_mode_3 = GameObject(0, 0, 192, 192, L"./pic/3.png"); //文字数選択画面の３
GameObject UI_mode_4 = GameObject(0, 0, 192, 192, L"./pic/4.png"); //文字数選択画面の４
GameObject UI_mode_5 = GameObject(0, 0, 192, 192, L"./pic/5.png"); //文字数選択画面の５
GameObject UI_mode_mojisu_highlight = GameObject(0, 0, 192, 192, L"./pic/mojisu_highlight.png"); //文字数選択画面のハイライト
GameObject UI_mode_select_mojisu_UI = GameObject(0, 0, 1024, 128, L"./pic/mode_select_mojisu_UI.png"); //文字数選択画面の「もどる，文字数選択，決定」
GameObject UI_mode_waku = GameObject(0, 0, 512, 512, L"./pic/waku_1.png"); //モード選択画面の右下のハイスコアやクリア状況を囲う枠
GameObject UI_mode_highscore_1 = GameObject(0, 0, 512, 512, L"./pic/mode_highscore_1.png"); //モード選択画面右下のスコアアタックのハイスコア状況の数字部分以外
GameObject UI_mode_highscore_2 = GameObject(0, 0, 512, 512, L"./pic/mode_highscore_2.png"); //モード選択画面右下のステージクリアモードのステージクリア状況の数字部分以外
GameObject UI_slot_locked = GameObject(0, 0, 192, 192, L"./pic/slot_locked.png"); //プレイ画面でのスロットの固定枠
GameObject UI_block_stage_num = GameObject(0, 0, 96, 96, L"./pic/block_stage_num.png"); //ステージ選択画面でのステージ番号の背景のブロック
GameObject UI_block_stage_num_select = GameObject(0, 0, 192, 192, L"./pic/block_stage_num_selected.png"); //ステージ選択画面での選択しているステージのブロックの背景ハイライト
GameObject UI_clear_lamp_on_1 = GameObject(0, 0, 160, 40, L"./pic/clear_lamp_on_1.png"); //ステージ選択画面でのクリアメダル
GameObject UI_clear_lamp_on_2 = GameObject(0, 0, 160, 40, L"./pic/clear_lamp_on_2.png"); //ステージ選択画面でのクリアメダル
GameObject UI_clear_lamp_on_3 = GameObject(0, 0, 160, 40, L"./pic/clear_lamp_on_3.png"); //ステージ選択画面でのクリアメダル
GameObject UI_clear_lamp_on_1_lux = GameObject(0, 0, 160, 40, L"./pic/clear_lamp_on_1_lux.png"); //ステージ選択画面でのクリアメダル（光
GameObject UI_clear_lamp_on_2_lux = GameObject(0, 0, 160, 40, L"./pic/clear_lamp_on_2_lux.png"); //ステージ選択画面でのクリアメダル（光
GameObject UI_clear_lamp_on_3_lux = GameObject(0, 0, 160, 40, L"./pic/clear_lamp_on_3_lux.png"); //ステージ選択画面でのクリアメダル（光
GameObject UI_clear_lamp_off = GameObject(0, 0, 160, 40, L"./pic/clear_lamp_off.png"); //ステージ選択画面でのクリアメダル背景（デフォルトはオフ
GameObject UI_time_limit_description = GameObject(0, 0, 1024, 128, L"./pic/time_limit_description.png"); //ステージ選択画面での制限時間や自己ベストタイム情報など
GameObject UI_coming_soon64 = GameObject(0, 0, 64, 64, L"./pic/coming_soon.png"); //ステージ選択画面未定義ステージ
GameObject UI_coming_soon192 = GameObject(0, 0, 192, 192, L"./pic/coming_soon.png"); 
GameObject UI_slot_constraint_description = GameObject(0, 0, 1024, 64, L"./pic/slot_constraint_description.png"); //ステージ選択画面の「語彙スロット制限：」
GameObject UI_mission_description_0 = GameObject(0, 0, 1024, 64, L"./pic/mission_description_0.png"); //ステージ選択画面の「＿＿単語つくれ」
GameObject UI_mission_description_1 = GameObject(0, 0, 1024, 64, L"./pic/mission_description_1.png"); //ステージ選択画面の「＿で＿＿単語作れ」
GameObject UI_mission_description_2 = GameObject(0, 0, 1024, 64, L"./pic/mission_description_2.png"); //ステージ選択画面の「しりとりで＿＿単語つなげろ」
GameObject UI_num_aslash = GameObject(0, 0, 20, 20, L"./pic/num_aslash.png"); //プレイ画面（ステージクリアモード）の左下ノルマ「何分の何」のスラッシュ
GameObject UI_num_aslash_big = GameObject(0, 0, 40, 40, L"./pic/num_aslash.png"); 
GameObject UI_result_stage = GameObject(0, 0, 768, 384, L"./pic/result_stage.png"); //リザルト画面（ステージクリアモード）の「ノルマクリア，ミス＿＿回，所要時間＿＿秒」
GameObject UI_result_stage_title = GameObject(0, 0, 384, 96, L"./pic/result_stage_title.png"); //リザルト画面上（ステージクリアモード）の「STAGE ___」
GameObject UI_result_stage_medal_1 = GameObject(0, 0, 768, 384, L"./pic/result_stage_medal_1.png"); //リザルト画面上（ステージクリアモード）のクリアメダル（光
GameObject UI_result_stage_medal_2 = GameObject(0, 0, 768, 384, L"./pic/result_stage_medal_2.png"); //リザルト画面上（ステージクリアモード）のクリアメダル（光
GameObject UI_result_stage_medal_3 = GameObject(0, 0, 768, 384, L"./pic/result_stage_medal_3.png"); //リザルト画面上（ステージクリアモード）のクリアメダル（光
GameObject UI_result_stage_medal_1_lux = GameObject(0, 0, 768, 384, L"./pic/result_stage_medal_1_lux.png"); //リザルト画面上（ステージクリアモード）のクリアメダル（光
GameObject UI_result_stage_medal_2_lux = GameObject(0, 0, 768, 384, L"./pic/result_stage_medal_2_lux.png"); //リザルト画面上（ステージクリアモード）のクリアメダル（光
GameObject UI_result_stage_medal_3_lux = GameObject(0, 0, 768, 384, L"./pic/result_stage_medal_3_lux.png"); //リザルト画面上（ステージクリアモード）のクリアメダル（光
GameObject UI_arrow_2L = GameObject(0, 0, 64, 64, L"./pic/arrow_2L.png"); //ステージ選択画面のステージ選択矢印←
GameObject UI_arrow_2R = GameObject(0, 0, 64, 64, L"./pic/arrow_2R.png"); //ステージ選択画面のステージ選択矢印→
GameObject UI_arrow_2D = GameObject(0, 0, 64, 64, L"./pic/arrow_2D.png"); //ステージ選択画面のステージ選択矢印↓

GameObject block_hiragana_UI[80] = { //スロット用
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

GameObject block_hiragana[80] = { //ブロック用
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

GameObject block_hiragana_mini[80] = { //エディタ用
GameObject(0, 0, 32, 32, L"./pic/block_blank.png"), //空白 ID:0
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_01.png"), //あ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_02.png"), //い
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_03.png"), //う
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_04.png"), //え
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_05.png"), //お
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_06.png"), //か
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_07.png"), //き
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_08.png"), //く
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_09.png"), //け
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_10.png"), //こ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_11.png"), //さ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_12.png"), //し
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_13.png"), //す
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_14.png"), //せ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_15.png"), //そ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_16.png"), //た
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_17.png"), //ち
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_18.png"), //つ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_19.png"), //て
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_20.png"), //と
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_21.png"), //な
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_22.png"), //に
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_23.png"), //ぬ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_24.png"), //ね
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_25.png"), //の
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_26.png"), //は
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_27.png"), //ひ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_28.png"), //ふ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_29.png"), //へ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_30.png"), //ほ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_31.png"), //ま
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_32.png"), //み
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_33.png"), //む
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_34.png"), //め
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_35.png"), //も
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_36.png"), //や
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_37.png"), //ゆ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_38.png"), //よ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_39.png"), //ら
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_40.png"), //り
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_41.png"), //る
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_42.png"), //れ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_43.png"), //ろ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_44.png"), //わ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_45.png"), //を ID:45
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_46.png"), //ん ID:46
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_73.png"), //っ ID:47
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_74.png"), //ヴ
GameObject(0, 0, 32, 32, L"./pic/block_wood.png"), //木材（足場ブロック）ID:49
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_47.png"), //ー ID:50
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_48.png"), //が
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_49.png"), //ぎ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_50.png"), //ぐ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_51.png"), //げ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_52.png"), //ご
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_53.png"), //ざ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_54.png"), //じ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_55.png"), //ず
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_56.png"), //ｚｗ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_57.png"), //ぞ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_58.png"), //だ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_59.png"), //ぢ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_60.png"), //づ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_61.png"), //で
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_62.png"), //ど
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_63.png"), //ば
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_64.png"), //び
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_65.png"), //ぶ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_66.png"), //べ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_67.png"), //ぼ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_68.png"), //ぱ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_69.png"), //ぴ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_70.png"), //ぷ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_71.png"), //ぺ
GameObject(0, 0, 32, 32, L"./pic/block_hiragana_72.png"),//ぽ
GameObject(0, 0, 32, 32, L"./pic/ground.png"), //地面 ID:76
GameObject(0, 0, 32, 32, L"./pic/block_odai.png"), //おだいばこ
GameObject(0, 0, 32, 32, L"./pic/block_odai_off.png"), //お題箱オフ
GameObject(0, 0, 32, 32, L"./pic/block_undifined.png") //未使用（お題箱とかに使う？
};

GameObject block_light_1 = GameObject(0, 0, 64, 64, L"./pic/block_highlight_1.png"); //ルーレットブロックのライトエフェクト（明
GameObject block_light_2 = GameObject(0, 0, 64, 64, L"./pic/block_highlight_2.png"); //ルーレットブロックのライトエフェクト（暗


AnimationChara *player; //プレイヤーのあらゆる情報を扱う
AnimationChara *bullet; //弾丸のあらゆる情報を扱う

Mix_Chunk *SE_select = NULL; //効果音たち
Mix_Chunk *SE_back = NULL;
Mix_Chunk *SE_batsu = NULL;
Mix_Chunk *SE_enter = NULL;
Mix_Chunk *SE_hit = NULL;
Mix_Chunk *SE_jump = NULL;
Mix_Chunk *SE_maru = NULL;
Mix_Chunk *SE_shoot = NULL;
Mix_Chunk *SE_throw = NULL;

void play_SE(Mix_Chunk *filename) //効果音を鳴らす
{
	if (flag_SE == true){ Mix_PlayChannel(-1, filename, 0); } //SDL_mixerでセットアップが正しく行われたときのみ音をならす
}

void end() {
	GdiplusShutdown(gdiPT);
}

void check_goi(int* moji)
{
	float *hs3 = &hiragana_score_3[0][0];
	float *hs4 = &hiragana_score_4[0][0];
	float *hs5 = &hiragana_score_5[0][0];
	int *dic3 = &dic_3moji[0][0];
	int *dic4 = &dic_4moji[0][0];
	int *dic5 = &dic_5moji[0][0];
	int *lmhr = &list_most_hiragana[0];
	int *si = &stage_info[0];
	int *mdtn = &made_tango[0][0];

	switch (mode)
	{
	case 0: //スコアアタックモードのとき
	{
		switch (mode_mojisu)
		{
		case 3:
		{
			for (i = 0; i < dic_3_all; i++)
			{
				if (moji[0] == *(dic3 + i * 3) && moji[1] == *(dic3 + i * 3 + 1) && moji[2] == *(dic3 + i * 3 + 2))
				{
					word_hit = true;
					play_SE(SE_maru);

					score_word = (int)(*(hs3 + 3 * moji[0]) + *(hs3 + 3 * moji[1] + 1) + *(hs3 + 3 * moji[2] + 2));

					score += score_word;
					score_tango++;
					(*(lmhr + moji[0]))++;
					(*(lmhr + moji[1]))++;
					(*(lmhr + moji[2]))++;
					(*(lmhr + moji[2]))++;

					break;
				}

			}
			if (word_hit == false) //ペナルティ
			{
				play_SE(SE_batsu);
				time -= 30 * 60;
			}
		}break;

		case 4:
		{
			for (i = 0; i < dic_4_all; i++)
			{
				if (moji[0] == *(dic4 + i * 4) && moji[1] == *(dic4 + i * 4 + 1) && moji[2] == *(dic4 + i * 4 + 2) && moji[3] == *(dic4 + i * 4 + 3))
				{
					word_hit = true;
					play_SE(SE_maru);

					score_word = (int)(*(hs4 + 4 * moji[0]) + *(hs4 + 4 * moji[1] + 1) + *(hs4 + 4 * moji[2] + 2) + *(hs4 + 4 * moji[3] + 3));

					score += score_word;
					score_tango++;
					(*(lmhr + moji[0]))++;
					(*(lmhr + moji[1]))++;
					(*(lmhr + moji[2]))++;
					(*(lmhr + moji[3]))++;

					break;
				}

			}
			if (word_hit == false) //ペナルティ
			{
				play_SE(SE_batsu);
				time -= 30 * 60;
			}
		}break;

		case 5:
		{
			for (i = 0; i < dic_5_all; i++)
			{
				if (moji[0] == *(dic5 + i * 5) && moji[1] == *(dic5 + i * 5 + 1) && moji[2] == *(dic5 + i * 5 + 2) && moji[3] == *(dic5 + i * 5 + 3) && moji[4] == *(dic5 + i * 5 + 4))
				{
					word_hit = true;
					play_SE(SE_maru);

					score_word = (int)(*(hs5 + 5 * moji[0]) + *(hs5 + 5 * moji[1] + 1) + *(hs5 + 5 * moji[2] + 2) + *(hs5 + 5 * moji[3] + 3) + *(hs5 + 5 * moji[4] + 4));

					score += score_word;
					score_tango++;
					(*(lmhr + moji[0]))++;
					(*(lmhr + moji[1]))++;
					(*(lmhr + moji[2]))++;
					(*(lmhr + moji[3]))++;
					(*(lmhr + moji[4]))++;

					break;
				}

			}
			if (word_hit == false) //ペナルティ
			{
				play_SE(SE_batsu);
				time -= 30 * 60;
			}
		}break;
		}
	}break;

	case 1: //ステージクリアモードのとき
	{
		switch (*(si + stage_select) % 10)
		{
		case 3:
		{
			for (i = 0; i < dic_3_all; i++)
			{
				if (moji[0] == *(dic3 + i * 3) && moji[1] == *(dic3 + i * 3 + 1) && moji[2] == *(dic3 + i * 3 + 2))
				{
					for (k = 0; k <= score; k++)
					{
						if (*(mdtn + k * 5 + 0) == moji[0] && *(mdtn + k * 5 + 1) == moji[1] && *(mdtn + k * 5 + 2) == moji[2])
						{
							word_hit = false;
							break;

						}

						word_hit = true;
						play_SE(SE_maru);
						break;
					}

				}

			}

			if (word_hit == true)
			{
				*(mdtn + score * 5 + 0) = moji[0];
				*(mdtn + score * 5 + 1) = moji[1];
				*(mdtn + score * 5 + 2) = moji[2];

				score_word = 1;
				score += score_word;
			}

			if (word_hit == false) //ペナルティ
			{
				play_SE(SE_batsu);
				time -= 30 * 60;
				score_miss++;
			}
		}break;

		case 4:
		{
			for (i = 0; i < dic_4_all; i++)
			{
				if (moji[0] == *(dic4 + i * 4) && moji[1] == *(dic4 + i * 4 + 1) && moji[2] == *(dic4 + i * 4 + 2) && moji[3] == *(dic4 + i * 4 + 3))
				{
					for (k = 0; k <= score; k++)
					{
						if (*(mdtn + k * 5 + 0) == moji[0] && *(mdtn + k * 5 + 1) == moji[1] && *(mdtn + k * 5 + 2) == moji[2] && *(mdtn + k * 5 + 3) == moji[3])
						{
							word_hit = false;
							break;

						}

						word_hit = true;
						play_SE(SE_maru);
						break;
					}

				}

			}

			if (word_hit == true)
			{
				*(mdtn + score * 5 + 0) = moji[0];
				*(mdtn + score * 5 + 1) = moji[1];
				*(mdtn + score * 5 + 2) = moji[2];
				*(mdtn + score * 5 + 3) = moji[3];

				score_word = 1;
				score += score_word;
			}

			if (word_hit == false) //ペナルティ
			{
				play_SE(SE_batsu);
				time -= 30 * 60;
				score_miss++;
			}
		}break;

		case 5:
		{
			for (i = 0; i < dic_5_all; i++)
			{
				if (moji[0] == *(dic5 + i * 5) && moji[1] == *(dic5 + i * 5 + 1) && moji[2] == *(dic5 + i * 5 + 2) && moji[3] == *(dic5 + i * 5 + 3) && moji[4] == *(dic5 + i * 5 + 4))
				{
					for (k = 0; k <= score; k++)
					{
						if (*(mdtn + k * 5 + 0) == moji[0] && *(mdtn + k * 5 + 1) == moji[1] && *(mdtn + k * 5 + 2) == moji[2] && *(mdtn + k * 5 + 3) == moji[3] && *(mdtn + k * 5 + 4) == moji[4])
						{
							word_hit = false;
							break;

						}

						word_hit = true;
						play_SE(SE_maru);
						break;
					}

				}

			}

			if (word_hit == true)
			{
				*(mdtn + score * 5 + 0) = moji[0];
				*(mdtn + score * 5 + 1) = moji[1];
				*(mdtn + score * 5 + 2) = moji[2];
				*(mdtn + score * 5 + 3) = moji[3];
				*(mdtn + score * 5 + 4) = moji[4];

				score_word = 1;
				score += score_word;
			}

			if (word_hit == false) //ペナルティ
			{
				play_SE(SE_batsu);
				time -= 30 * 60;
				score_miss++;
			}
		}break;
		}
	}break;
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

void SetNumImage(double x, double y, int size_x, int size_y, int num, int font, int d) { //プレイヤーエフェクトとしての数字 //dは文字同士の間隔

	GLuint *tn = &tex_num[0][0];

	if (num >= 100000) //10万の位 *(tn+font*11+(num / 10000) % 10)
	{
		glBindTexture(GL_TEXTURE_2D, *(tn + font * 11 + num / 100000));

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 6 + x + d * 5, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 6 + x + d * 5, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 5 + x + d * 5, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 5 + x + d * 5, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	if (num >= 10000) //1万の位
	{
		glBindTexture(GL_TEXTURE_2D, *(tn + font * 11 + (num / 10000) % 10));

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 5 + x + d * 4, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 5 + x + d * 4, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 4 + x + d * 4, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 4 + x + d * 4, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	if (num >= 1000) //1000の位
	{
		glBindTexture(GL_TEXTURE_2D, *(tn + font * 11 + (num / 1000) % 10));

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 4 + x + d * 3, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 4 + x + d * 3, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 3 + x + d * 3, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 3 + x + d * 3, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	if (num >= 100) //100の位
	{
		glBindTexture(GL_TEXTURE_2D, *(tn + font * 11 + (num / 100) % 10));

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 3 + x + d * 2, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 3 + x + d * 2, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 2 + x + d * 2, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 2 + x + d * 2, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	if (num >= 10) //10の位
	{
		glBindTexture(GL_TEXTURE_2D, *(tn + font * 11 + (num / 10) % 10));

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f); glVertex2d(size_x / 8 * 2 + x + d * 1, size_y + y);//左下
		glTexCoord2f(0.0f, 0.0f); glVertex2d(size_x / 8 * 2 + x + d * 1, y);//左上
		glTexCoord2f(1.0f, 0.0f); glVertex2d(size_x / 8 * 1 + x + d * 1, y);//右上
		glTexCoord2f(1.0f, 1.0f); glVertex2d(size_x / 8 * 1 + x + d * 1, size_y + y);//右下
		glEnd();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	glBindTexture(GL_TEXTURE_2D, *(tn + font * 11 + num % 10));

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
		glBindTexture(GL_TEXTURE_2D, *(tn + font * 11 + 10));
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
		glBindTexture(GL_TEXTURE_2D, *(tn + font * 11 + 10));
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
		glBindTexture(GL_TEXTURE_2D, *(tn + font * 11 + 10));
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
		glBindTexture(GL_TEXTURE_2D, *(tn + font * 11 + 10));
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
		glBindTexture(GL_TEXTURE_2D, *(tn + font * 11 + 10));
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
		glBindTexture(GL_TEXTURE_2D, *(tn + font * 11 + 10));
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

int choose_hiragana(void) //ひらがなブロックをランダムに選ぶ関数
{
	int a;

	do {
		a = rand_hiragana(mt);
	} while (a == 49);

	return a;
}

int choose_pattern(int MOJISU) //ステージのブロック配置パターンをランダムに選ぶ関数
{
	int a;

	switch (MOJISU)
	{
	case 3:
	{
		do { a = rand_pattern3(mt); } while (a <= 0 || a >= 4); //1~3から選ぶ 余裕があるときにパターンを増やす
	}break;

	case 4:
	{
		do { a = rand_pattern4(mt); } while (a <= 100 || a >= 104); //101~103から選ぶ 余裕があるときにパターンを増やす
	}break;

	case 5:
	{
		do { a = rand_pattern5(mt); } while (a <= 200 || a >= 203); //201~202から選ぶ 余裕があるときにパターンを増やす
	}break;
	}

	return a;
}

int choose_odai(int MOJISU) //ヒントブロックの中身をランダムに選ぶ関数
{
	int a;

	switch (MOJISU)
	{
	case 3:
	{
		do {a = rand_odai(mt);} while (a >= odai_hiragana_3_all);
	}break;

	case 4:
	{
		do { a = rand_odai(mt); } while (a >= odai_hiragana_4_all);
	}break;

	case 5:
	{
		do { a = rand_odai(mt); } while (a >= odai_hiragana_5_all);
	}break;
	}

	return a;
}

void set_block_info(int type, int x_grid, int y_grid, int leftside, int blocknum) //leftsideはそのまま使う 
{
	int *obbl = &object_block[blocknum][0];
	*(obbl) = type; *(obbl + 1) = (-64)*(leftside + x_grid); *(obbl + 2) = (-64)*(y_grid);
}

void block_standby(void) //スコアアタックモード
{
	j = 0; //オブジェクトブロックを配置するための添え字，1個設置したら1増える．
	object_on_stage = 0;

	int Q[12] = {}; //地形パターンを生成する際，同じ辞書の単語に含まれるひらがなを所定の位置(かたまって)に配置するため，その単語の番号を乱数で取得する

	switch (mode_mojisu) 	//ここから各文字数モードのステージチュートリアル部分
	{
	case 3:
	{
		for (k = -1; k <= 13; k++) { set_block_info(49, -1, k, 0, j); j++; } //スタート地点の壁の配置
		for (k = -10; k <= 28; k++) { set_block_info(76, k, -1, 0, j); j++; } //床の配置
		for (k = 10; k <= 16; k++) { for (l = 0; l <= 1; l++) { set_block_info(49, k, l, 0, j); j++; } }
		for (k = 2; k <= 7; k++) { set_block_info(49, 18, k, 0, j); j++; }
		for (k = 0; k <= 5; k++) { set_block_info(49, 26, k, 0, j); j++; }
		for (k = 19; k <= 24; k++) { set_block_info(49, k, 2, 0, j); j++; }
		for (k = 20; k <= 25; k++) { set_block_info(49, k, 5, 0, j); j++; }

		set_block_info(26, 5, 0, 0, j); j++;
		set_block_info(43, 6, 0, 0, j); j++;
		set_block_info(50, 7, 0, 0, j); j++;
		set_block_info(16, 12, 2, 0, j); j++;
		set_block_info(22, 13, 2, 0, j); j++;
		set_block_info(12, 14, 2, 0, j); j++;
		set_block_info(38, 21, 0, 0, j); j++;
		set_block_info(44, 22, 0, 0, j); j++;
		set_block_info(2, 23, 0, 0, j); j++;
		set_block_info(28, 21, 3, 0, j); j++;
		set_block_info(18, 22, 3, 0, j); j++;
		set_block_info(3, 23, 3, 0, j); j++;
		set_block_info(18, 21, 6, 0, j); j++;
		set_block_info(38, 22, 6, 0, j); j++;
		set_block_info(2, 23, 6, 0, j); j++;

		set_leftside = 29;
	}break;

	case 4:
	{
		for (k = -1; k <= 13; k++) { set_block_info(49, -1, k, 0, j); j++; } //スタート地点の壁の配置
		for (k = -10; k <= 30; k++) {set_block_info(76, k, -1, 0, j); j++;} //床の配置
		for (k = 10; k <= 17; k++) { for (l = 0; l <= 2; l++) { set_block_info(49, k, l, 0, j); j++; }} 
		for (k = 2; k <= 7; k++) { set_block_info(49, 19, k, 0, j); j++; } 
		for (k = 0; k <= 5; k++) { set_block_info(49, 28, k, 0, j); j++; }
		for (k = 20; k <= 26; k++) { set_block_info(49, k, 2, 0, j); j++; }
		for (k = 21; k <= 27; k++) { set_block_info(49, k, 5, 0, j); j++; }

		set_block_info(1, 4, 0, 0, j); j++;
		set_block_info(2, 5, 0, 0, j); j++;
		set_block_info(11, 6, 0, 0, j); j++;
		set_block_info(18, 7, 0, 0, j); j++;
		set_block_info(5, 12, 3, 0, j); j++;
		set_block_info(26, 13, 3, 0, j); j++;
		set_block_info(38, 14, 3, 0, j); j++;
		set_block_info(3, 15, 3, 0, j); j++;
		set_block_info(5, 22, 0, 0, j); j++;
		set_block_info(5, 23, 0, 0, j); j++;
		set_block_info(59, 24, 0, 0, j); j++;
		set_block_info(7, 25, 0, 0, j); j++;
		set_block_info(14, 22, 3, 0, j); j++;
		set_block_info(7, 23, 3, 0, j); j++;
		set_block_info(44, 24, 3, 0, j); j++;
		set_block_info(9, 25, 3, 0, j); j++;
		set_block_info(38, 22, 6, 0, j); j++;
		set_block_info(10, 23, 6, 0, j); j++;
		set_block_info(63, 24, 6, 0, j); j++;
		set_block_info(21, 25, 6, 0, j); j++;

		set_leftside = 31;
	}break;

	case 5:
	{
		for (k = -1; k <= 13; k++) { set_block_info(49, -1, k, 0, j); j++; } //スタート地点の壁の配置
		for (k = -10; k <= 33; k++) { set_block_info(76, k, -1, 0, j); j++; } //床の配置
		for (k = 11; k <= 19; k++) { for (l = 0; l <= 2; l++) { set_block_info(49, k, l, 0, j); j++; } }
		for (k = 2; k <= 7; k++) { set_block_info(49, 21, k, 0, j); j++; }
		for (k = 0; k <= 5; k++) { set_block_info(49, 31, k, 0, j); j++; }
		for (k = 22; k <= 29; k++) { set_block_info(49, k, 2, 0, j); j++; }
		for (k = 23; k <= 30; k++) { set_block_info(49, k, 5, 0, j); j++; }

		set_block_info(10, 4, 0, 0, j); j++;
		set_block_info(46, 5, 0, 0, j); j++;
		set_block_info(22, 6, 0, 0, j); j++;
		set_block_info(17, 7, 0, 0, j); j++;
		set_block_info(26, 8, 0, 0, j); j++;
		set_block_info(10, 13, 3, 0, j); j++;
		set_block_info(46, 14, 3, 0, j); j++;
		set_block_info(66, 15, 3, 0, j); j++;
		set_block_info(46, 16, 3, 0, j); j++;
		set_block_info(26, 17, 3, 0, j); j++;
		set_block_info(12, 24, 0, 0, j); j++;
		set_block_info(21, 25, 0, 0, j); j++;
		set_block_info(25, 26, 0, 0, j); j++;
		set_block_info(51, 27, 0, 0, j); j++;
		set_block_info(44, 28, 0, 0, j); j++;
		set_block_info(6, 24, 3, 0, j); j++;
		set_block_info(68, 25, 3, 0, j); j++;
		set_block_info(20, 26, 3, 0, j); j++;
		set_block_info(33, 27, 3, 0, j); j++;
		set_block_info(12, 28, 3, 0, j); j++;
		set_block_info(42, 24, 6, 0, j); j++;
		set_block_info(2, 25, 6, 0, j); j++;
		set_block_info(60, 26, 6, 0, j); j++;
		set_block_info(3, 27, 6, 0, j); j++;
		set_block_info(10, 28, 6, 0, j); j++;

		set_leftside = 34;
	}break;

	}

	for (i = 0; i <= PATTERN_LIMIT; i++) //structureの配列をもとに左から配置していく
	{
		switch (stage_structure[i])
		{

		case -3: //4文字サンプル
		{
			for (k = 0; k <= 16; k++) //床の配置
			{
				set_block_info(76, k, -1, set_leftside, j); j++;
			}

			set_block_info(10, 3, 0, set_leftside, j); j++;
			set_block_info(20, 3, 2, set_leftside, j); j++;
			set_block_info(68, 3, 4, set_leftside, j); j++;
			set_block_info(7, 4, 0, set_leftside, j); j++;
			set_block_info(54, 4, 2, set_leftside, j); j++;
			set_block_info(46, 4, 4, set_leftside, j); j++;
			set_block_info(6, 6, 0, set_leftside, j); j++;
			set_block_info(50, 5, 2, set_leftside, j); j++;
			set_block_info(46, 6, 4, set_leftside, j); j++;
			set_block_info(13, 7, 0, set_leftside, j); j++;
			set_block_info(75, 7, 2, set_leftside, j); j++;
			set_block_info(22, 7, 4, set_leftside, j); j++;
			set_block_info(18, 8, 0, set_leftside, j); j++;
			set_block_info(2, 8, 2, set_leftside, j); j++;
			set_block_info(19, 8, 4, set_leftside, j); j++;
			set_block_info(17, 10, 0, set_leftside, j); j++;
			set_block_info(40, 9, 2, set_leftside, j); j++;
			set_block_info(47, 10, 4, set_leftside, j); j++;
			set_block_info(45, 11, 0, set_leftside, j); j++;
			set_block_info(18, 11, 2, set_leftside, j); j++;
			set_block_info(3, 11, 4, set_leftside, j); j++;
			set_block_info(19, 12, 0, set_leftside, j); j++;
			set_block_info(3, 12, 2, set_leftside, j); j++;
			set_block_info(33, 12, 4, set_leftside, j); j++;

			set_block_info(49, 0, 1, set_leftside, j); j++;
			set_block_info(49, 1, 3, set_leftside, j); j++;
			set_block_info(49, 15, 1, set_leftside, j); j++;
			set_block_info(49, 14, 3, set_leftside, j); j++;

			set_leftside += 17;
		}break;

		case 1: //3文字モードパターン１
		{
			for (k = 0; k <= 15; k++) //床の配置
			{
				set_block_info(76, k, -1, set_leftside, j); j++;
			}

			for (k = 0; k <= 4; k++) //配置するブロックがどの単語のひらがなか，の単語の番号を決める
			{
				do
				{
					Q[k] = rand_dic_3(mt);
				} while (Q[k] >= dic_3_all);
			}

			set_block_info(dic_3moji[Q[0]][0], 4, 0, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[1]][0], 5, 0, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[1]][2], 7, 0, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[1]][1], 8, 0, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 10, 0, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[3]][1], 11, 0, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[0]][2], 4, 2, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[2]][2], 5, 2, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 6, 2, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[4]][2], 9, 2, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[3]][0], 10, 2, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[3]][2], 11, 2, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[0]][1], 4, 4, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[2]][0], 5, 4, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[2]][1], 7, 4, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[4]][1], 8, 4, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[4]][0], 10, 4, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 11, 4, set_leftside, j); j++;
			
			set_block_info(49, 0, 1, set_leftside, j); j++;
			set_block_info(49, 1, 3, set_leftside, j); j++;
			set_block_info(49, 14, 1, set_leftside, j); j++;
			set_block_info(49, 13, 3, set_leftside, j); j++;

			set_leftside += 16;
		}break;

		case 2: //3文字モードパターン2
		{
			for (k = 0; k <= 15; k++) //床の配置
			{
				set_block_info(76, k, -1, set_leftside, j); j++;
			}

			for (k = 0; k <= 4; k++) //配置するブロックがどの単語のひらがなか，の単語の番号を決める
			{
				do
				{
					Q[k] = rand_dic_3(mt);
				} while (Q[k] >= dic_3_all);
			}

			set_block_info(dic_3moji[Q[1]][2], 4, 0, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[1]][1], 5, 0, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 7, 0, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[3]][2], 8, 0, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[4]][0], 10, 0, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[4]][1], 11, 0, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[0]][0], 4, 2, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[1]][0], 5, 2, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 6, 2, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[2]][1], 9, 2, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[3]][1], 10, 2, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[4]][2], 11, 2, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[0]][1], 4, 4, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[0]][2], 5, 4, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[2]][0], 7, 4, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[2]][2], 8, 4, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[3]][0], 10, 4, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 11, 4, set_leftside, j); j++;

			set_block_info(49, 0, 1, set_leftside, j); j++;
			set_block_info(49, 1, 3, set_leftside, j); j++;
			set_block_info(49, 14, 1, set_leftside, j); j++;
			set_block_info(49, 13, 3, set_leftside, j); j++;

			set_leftside += 16;
		}break;

		case 3: //3文字モードパターン3
		{
			for (k = 0; k <= 15; k++) //床の配置
			{
				set_block_info(76, k, -1, set_leftside, j); j++;
			}

			for (k = 0; k <= 4; k++) //配置するブロックがどの単語のひらがなか，の単語の番号を決める
			{
				do
				{
					Q[k] = rand_dic_3(mt);
				} while (Q[k] >= dic_3_all);
			}

			set_block_info(dic_3moji[Q[0]][0], 4, 0, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[0]][2], 5, 0, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[0]][1], 7, 0, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[4]][2], 8, 0, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[2]][0], 10, 0, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[3]][2], 11, 0, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[3]][1], 4, 2, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[4]][1], 5, 2, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[3]][0], 6, 2, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[2]][2], 9, 2, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[2]][1], 10, 2, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 11, 2, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 4, 4, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[1]][0], 5, 4, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[1]][1], 7, 4, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[1]][2], 8, 4, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 10, 4, set_leftside, j); j++;
			set_block_info(dic_3moji[Q[4]][0], 11, 4, set_leftside, j); j++;

			set_block_info(49, 0, 1, set_leftside, j); j++;
			set_block_info(49, 1, 3, set_leftside, j); j++;
			set_block_info(49, 14, 1, set_leftside, j); j++;
			set_block_info(49, 13, 3, set_leftside, j); j++;

			set_leftside += 16;
		}break;

		case 101: //４文字パターン１
		{
			for (k = 0; k <= 17; k++) //床の配置
			{
				set_block_info(76, k, -1, set_leftside, j); j++;
			}

			for (k = 0; k <= 4; k++) //配置するブロックがどの単語のひらがなか，の単語の番号を決める
			{
				do
				{
					Q[k] = rand_dic_4(mt);
				} while (Q[k] >= dic_4_all);
			}

			set_block_info(dic_4moji[Q[0]][0], 4, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[0]][1], 5, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[4]][3], 7, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[4]][2], 8, 0, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 9, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[3]][2], 11, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[3]][1], 12, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[1]][2], 13, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[0]][2], 4, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[0]][3], 5, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[2]][1], 6, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[2]][2], 8, 2, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 9, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[3]][0], 10, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[3]][3], 12, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[1]][3], 13, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[2]][0], 4, 4, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[2]][3], 5, 4, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[4]][1], 7, 4, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[4]][0], 8, 4, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 9, 4, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 11, 4, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[1]][0], 12, 4, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[1]][1], 13, 4, set_leftside, j); j++;

			set_block_info(49, 0, 1, set_leftside, j); j++;
			set_block_info(49, 1, 3, set_leftside, j); j++;
			set_block_info(49, 16, 1, set_leftside, j); j++;
			set_block_info(49, 15, 3, set_leftside, j); j++;

			set_leftside += 18;
		}break;

		case 102: //４文字パターン2
		{
			for (k = 0; k <= 17; k++) //床の配置
			{
				set_block_info(76, k, -1, set_leftside, j); j++;
			}

			for (k = 0; k <= 4; k++) //配置するブロックがどの単語のひらがなか，の単語の番号を決める
			{
				do
				{
					Q[k] = rand_dic_4(mt);
				} while (Q[k] >= dic_4_all);
			}

			set_block_info(dic_4moji[Q[0]][1], 4, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[1]][1], 5, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[1]][2], 7, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[4]][2], 8, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[4]][0], 9, 0, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 11, 0, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 12, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[3]][3], 13, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[0]][2], 4, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[1]][0], 5, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[1]][3], 6, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[2]][2], 8, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[4]][3], 9, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[4]][1], 10, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[2]][3], 12, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[3]][2], 13, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[0]][3], 4, 4, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[0]][0], 5, 4, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 7, 4, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[2]][0], 8, 4, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[2]][1], 9, 4, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 11, 4, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[3]][0], 12, 4, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[3]][1], 13, 4, set_leftside, j); j++;

			set_block_info(49, 0, 1, set_leftside, j); j++;
			set_block_info(49, 1, 3, set_leftside, j); j++;
			set_block_info(49, 16, 1, set_leftside, j); j++;
			set_block_info(49, 15, 3, set_leftside, j); j++;

			set_leftside += 18;
		}break;

		case 103: //４文字パターン3
		{
			for (k = 0; k <= 17; k++) //床の配置
			{
				set_block_info(76, k, -1, set_leftside, j); j++;
			}

			for (k = 0; k <= 4; k++) //配置するブロックがどの単語のひらがなか，の単語の番号を決める
			{
				do
				{
					Q[k] = rand_dic_4(mt);
				} while (Q[k] >= dic_4_all);
			}

			set_block_info(dic_4moji[Q[0]][3], 4, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[0]][2], 5, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[1]][0], 7, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[1]][3], 8, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[1]][1], 9, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[4]][0], 11, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[4]][3], 12, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[4]][2], 13, 0, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[0]][1], 4, 2, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 5, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[2]][2], 6, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[1]][2], 8, 2, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 9, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[3]][2], 10, 2, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 12, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[4]][1], 13, 2, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[0]][0], 4, 4, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[2]][0], 5, 4, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[2]][1], 7, 4, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[2]][3], 8, 4, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[3]][3], 9, 4, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[3]][0], 11, 4, set_leftside, j); j++;
			set_block_info(dic_4moji[Q[3]][1], 12, 4, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 13, 4, set_leftside, j); j++;

			set_block_info(49, 0, 1, set_leftside, j); j++;
			set_block_info(49, 1, 3, set_leftside, j); j++;
			set_block_info(49, 16, 1, set_leftside, j); j++;
			set_block_info(49, 15, 3, set_leftside, j); j++;

			set_leftside += 18;
		}break;

		case 201: //5文字パターン1
		{
			for (k = 0; k <= 20; k++) //床の配置
			{
				set_block_info(76, k, -1, set_leftside, j); j++;
			}

			for (k = 0; k <= 4; k++) //配置するブロックがどの単語のひらがなか，の単語の番号を決める
			{
				do
				{
					Q[k] = rand_dic_5(mt);
				} while (Q[k] >= dic_5_all);
			}

			set_block_info(dic_5moji[Q[0]][0], 4, 0, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[0]][2], 5, 0, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[0]][3], 6, 0, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[1]][2], 8, 0, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 9, 0, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[3]][1], 10, 0, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[3]][4], 12, 0, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[2]][2], 13, 0, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[2]][4], 15, 0, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[4]][3], 16, 0, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[0]][1], 4, 2, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[0]][4], 5, 2, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[1]][0], 7, 2, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[1]][1], 8, 2, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 10, 2, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[3]][2], 11, 2, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[3]][3], 12, 2, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[4]][2], 14, 2, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 15, 2, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[4]][4], 16, 2, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[1]][3], 4, 4, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[1]][4], 5, 4, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 6, 4, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[2]][0], 8, 4, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[2]][1], 9, 4, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[3]][0], 10, 4, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[4]][0], 12, 4, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[4]][1], 13, 4, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 15, 4, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[2]][3], 16, 4, set_leftside, j); j++;
			
			set_block_info(49, 0, 1, set_leftside, j); j++;
			set_block_info(49, 1, 3, set_leftside, j); j++;
			set_block_info(49, 19, 1, set_leftside, j); j++;
			set_block_info(49, 18, 3, set_leftside, j); j++;

			set_leftside += 21;
		}break;

		case 202: //5文字パターン2
		{
			for (k = 0; k <= 20; k++) //床の配置
			{
				set_block_info(76, k, -1, set_leftside, j); j++;
			}

			for (k = 0; k <= 4; k++) //配置するブロックがどの単語のひらがなか，の単語の番号を決める
			{
				do
				{
					Q[k] = rand_dic_5(mt);
				} while (Q[k] >= dic_5_all);
			}

			set_block_info(dic_5moji[Q[0]][3], 4, 0, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 5, 0, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[2]][0], 6, 0, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[2]][3], 8, 0, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[2]][4], 9, 0, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[4]][4], 10, 0, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[3]][4], 12, 0, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 13, 0, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[4]][0], 15, 0, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[4]][1], 16, 0, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[0]][2], 4, 2, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[0]][4], 5, 2, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[1]][2], 7, 2, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[1]][3], 8, 2, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[2]][1], 10, 2, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[2]][2], 11, 2, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[3]][3], 12, 2, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[3]][2], 14, 2, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 15, 2, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[4]][3], 16, 2, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[1]][0], 4, 4, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[0]][0], 5, 4, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[0]][1], 6, 4, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[1]][1], 8, 4, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 9, 4, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[1]][4], 10, 4, set_leftside, j); j++;
			set_block_info(choose_hiragana(), 12, 4, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[3]][0], 13, 4, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[3]][1], 15, 4, set_leftside, j); j++;
			set_block_info(dic_5moji[Q[4]][2], 16, 4, set_leftside, j); j++;

			set_block_info(49, 0, 1, set_leftside, j); j++;
			set_block_info(49, 1, 3, set_leftside, j); j++;
			set_block_info(49, 19, 1, set_leftside, j); j++;
			set_block_info(49, 18, 3, set_leftside, j); j++;

			set_leftside += 21;
		}break;

		case -1: //ボーナスエリア
		{
			for (k = 0; k <= 16; k++) //床の配置
			{
				set_block_info(76, k, -1, set_leftside, j); j++;
			}

			set_block_info(79, 4, 0, set_leftside, j); j++;
			set_block_info(79, 5, 0, set_leftside, j); j++;
			set_block_info(79, 4, 2, set_leftside, j); j++;
			set_block_info(79, 5, 2, set_leftside, j); j++;
			set_block_info(79, 4, 4, set_leftside, j); j++;
			set_block_info(79, 5, 4, set_leftside, j); j++;
			set_block_info(79, 11, 0, set_leftside, j); j++;
			set_block_info(79, 12, 0, set_leftside, j); j++;
			set_block_info(79, 11, 2, set_leftside, j); j++;
			set_block_info(79, 12, 2, set_leftside, j); j++;
			set_block_info(79, 11, 4, set_leftside, j); j++;
			set_block_info(79, 12, 4, set_leftside, j); j++;

			set_block_info(77, 5, 6, set_leftside, j); j++;
			set_block_info(77, 11, 6, set_leftside, j); j++;

			set_block_info(49, 1, 3, set_leftside, j); j++;
			set_block_info(49, 2, 1, set_leftside, j); j++;
			set_block_info(49, 7, 1, set_leftside, j); j++;
			set_block_info(49, 8, 3, set_leftside, j); j++;
			set_block_info(49, 9, 1, set_leftside, j); j++;
			set_block_info(49, 14, 1, set_leftside, j); j++;
			set_block_info(49, 15, 3, set_leftside, j); j++;


			set_leftside += 17;
		}break;

		case -2: //終端
		{
			for (k = 0; k <= 10; k++) //床の配置
			{
				set_block_info(76, k, -1, set_leftside, j); j++;
			}

			for (k = 0; k <= 10; k++) //壁の配置
			{
				for (l = 0; l <= 10; l++) 
				{
					set_block_info(49, l, k, set_leftside, j); j++;
				}
			}

			set_leftside += 11;
		}break;

		default:
		{
			set_block_info(76, 0, -1, set_leftside, j); j++;
			set_leftside += 1;
		}break;
		}
	}

	object_on_stage = j;
	std::cout << "<info 024: " << object_on_stage << "個のオブジェクトがステージに配置されました" << std::endl;
}

void standby_stage(int stage_num) //ステージモード
{
	time = stage_time_limit[stage_num] * 60; //制限時間をリセット

	player->center_x = 0; //プレイヤーの初期位置をリセット
	player->center_y = -8;
	score = 0; //スコアをリセット
	score_miss = 0; //ミスの回数をリセット
	lamp_timer_01 = 0; //各種アニメーションエフェクトをリセット
	lamp_timer_02 = 0;
	slot_select = 0; //スロットの選択位置を最左に固定
	slot[0] = 0; //スロットの情報をリセット
	slot[1] = 0;
	slot[2] = 0;
	slot[3] = 0;
	slot[4] = 0;

	int *mdtn = &made_tango[0][0];
	int *obbl = &object_block[0][0];
	int *sbi = &stage_block_info[0][0];
	int *ssc = &stage_slot_constraint[0][0];
	int *si = &stage_info[0];

	char file_path[32]; //ステージファイルを読み込むときのステージ名

	for (i = 0; i < MADE_LIMIT; i++) //作った単語のやつをリセットする
	{
		*(mdtn + i * 5 + 0) = 0;
		*(mdtn + i * 5 + 1) = 0;
		*(mdtn + i * 5 + 2) = 0;
		*(mdtn + i * 5 + 3) = 0;
		*(mdtn + i * 5 + 4) = 0;
	}

	for (i = 0; i <= OBJECT_LIMIT; i++) //ゲーム内ブロックをリセットする
	{
		*(obbl + i * 3 + 0) = 0;
		*(obbl + i * 3 + 1) = 0;
		*(obbl + i * 3 + 2) = 0;
	}

	object_on_stage = 0;

	sprintf_s(file_path, "./dat/stage_%03d.dat", stage_num); //ステージファイルを読み込む
	if ((fopen_s(&fp_stage_structure_info, file_path, "r")) != 0)
	{
		printf("%s\n", file_path);
		std::cout << "<info 034: ステージファイルを開けませんでした" << std::endl;	exit(34);
	}

	i = 0;
	while (fscanf_s(fp_stage_structure_info, "%d,%d,%d", &stage_block_info[i][0], &stage_block_info[i][1], &stage_block_info[i][2]) != EOF)
	{
		i++;
	}

	object_on_stage = i;

	std::cout << "<info 035: ステージファイルを読み込みました>" << std::endl;

	for (i = 0; i <= object_on_stage; i++) //ファイルの情報をobject_blockに書き出していく
	{
		*(obbl + i * 3 + 0) = *(sbi + i * 3 + 0);
		*(obbl + i * 3 + 1) = *(sbi + i * 3+ 1) * (-64);
		*(obbl + i * 3 + 2) = *(sbi + i *3+ 2) * (-64);
	}

	int *slst = &slot_start[0];

	for (i = 0; i < object_on_stage; i++)
	{
		*(slst + i) = choose_hiragana(); //ヒントブロックの表示ひらがなをばらけさせるためにそれぞれのオブジェクトに割り振る（ルーレットブロックが登場するステージもあるので）
	}

	if (*(si+stage_select) >= 3 && *(si+stage_select) <= 5) //語彙スロット固定の場合は固定平名を設定．
	{
		slot[0] = *(ssc + stage_num * 5 + 0);
		slot[1] = *(ssc + stage_num * 5 + 1);
		slot[2] = *(ssc + stage_num * 5 + 2);
		slot[3] = *(ssc + stage_num * 5 + 3);
		slot[4] = *(ssc + stage_num * 5 + 4);
	}

	fclose(fp_stage_structure_info);
	std::cout << "<info 036: ステージファイルを閉じました>" << std::endl;
}

void game_reset(void) //ステージ構造などゲームを開始する直前にゲームを準備する（スコアアタックモード
{
	player->center_x = 0;
	player->center_y = -8;
	lamp_timer_01 = 0;
	lamp_timer_02 = 0;

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
	slot[4] = 0;
	slot_select = 0;

	int *obbl = &object_block[0][0];
	int *stst = &stage_structure[0];
	int *slst = &slot_start[0];

	//ステージの構造を再構成

	for (i = 0; i < object_on_stage; i++) //ブロックの情報をリセット
	{
		*(obbl + i * 3) = 0;
		*(obbl + i * 3 + 1) = 0;
		*(obbl + i * 3 + 2) = 0;
	}

	for (i = 0; i < PATTERN_LIMIT; i++)
	{
		*(stst + i) = choose_pattern(mode_mojisu);

		if (i % 6 == 5) //6パターンおきにボーナスステージ配置
		{
			*(stst + i) = -1;
		}
	}

	*(stst + 18) = -2; //最後の壁（暫定）なんか無限だとクソヌルゲーになるらしいので

	block_standby(); //ブロックの配置（再構成）



	for (i = 0; i < object_on_stage; i++)
	{
		*(slst + i) = choose_hiragana(); //ヒントブロックの表示ひらがなをばらけさせるためにそれぞれのオブジェクトに割り振る
	}
}

void game_shutdown(void) //Escキーやタイトル→やめるを押下されたとき諸々を終了させる
{
	fclose(fp_score_3);
	fclose(fp_score_4);
	fclose(fp_score_5); std::cout << "<info 017: スコアファイルを閉じました>" << std::endl;
	fclose(fp_dic_3);
	fclose(fp_dic_4);
	fclose(fp_dic_5); std::cout << "<info 018: 5文字辞書ファイルを閉じました>" << std::endl;
	fclose(fp_stageclear); std::cout << "<info 031: ステージクリア進捗状況ファイルを閉じました>" << std::endl;
	fclose(fp_stage_nolma_info); std::cout << "<info 052: ステージノルマファイルを閉じました>" << std::endl;

	if (flag_SE == true) 
	{ 
		Mix_FreeChunk(SE_select);
		Mix_FreeChunk(SE_back);
		Mix_FreeChunk(SE_batsu);
		Mix_FreeChunk(SE_enter);
		Mix_FreeChunk(SE_hit);
		Mix_FreeChunk(SE_jump);
		Mix_FreeChunk(SE_maru);
		Mix_FreeChunk(SE_shoot);
		Mix_FreeChunk(SE_throw);
		Mix_CloseAudio();
	}

	exit(1);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	int *obbl = &object_block[0][0];
	GameObject *blhr = &block_hiragana[0];
	GameObject *blUI = &block_hiragana_UI[0];
	GameObject *blmn = &block_hiragana_mini[0];
	int *hrrl = &hiragana_roulette[0];
	int *slst = &slot_start[0];
	int *sl = &slot[0];
	int *si = &stage_info[0];
	int *sc = &stage_clear[0][0];
	int *sn = &stage_nolma[0];
	int *stl = &stage_time_limit[0];
	int *stlg = &stage_time_limit_gold[0];
	int *ssc = &stage_slot_constraint[0][0];
	int *mdtn = &made_tango[0][0];


	switch (scene)
	{
	case 0:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		UI_title.SetImage(0, -352);
		UI_quit.SetImage(0, 100);
		UI_gamestart.SetImage(-80, 48);
		UI_pressstart.SetImage(140, -32);

	}break;


	case 1:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		UI_08.SetImage(0, -172);
		UI_13.SetImage(0, 64); //次へ進む

		UI_14.SetImage(0, -444); //are you ready?

	}break;

	case 2: //ポーズ画面
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		UI_pause.SetImage(0, -416);

		switch (mode)
		{
		case 0: {UI_return_0.SetImage(0, 72); } break; //モードによって真ん中の「ステージ選択にもどる」表記が異なる
		case 1: {UI_return_1.SetImage(0, 72); } break;
		}
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
		SetNumImage(-224, -420, 320, 40, score, 0, 4);

		UI_09.SetImage(0, 96);
		UI_rank01.SetImage(204, -228);
		UI_rank02.SetImage(204, -180);
		UI_rank03.SetImage(204, -132);
		UI_rank04.SetImage(204, -84);
		UI_rank05.SetImage(204, -36);

		switch (mode_mojisu)
		{	
		case 3:
		{
			SetNumImage(-244, -244, 320, 40, high_score_3[0], 0, 4);
			SetNumImage(-244, -200, 320, 40, high_score_3[1], 0, 4);
			SetNumImage(-244, -152, 320, 40, high_score_3[2], 0, 4);
			SetNumImage(-244, -104, 320, 40, high_score_3[3], 0, 4);
			SetNumImage(-244, -56, 320, 40, high_score_3[4], 0, 4);
		}break;

		case 4:
		{
			SetNumImage(-244, -244, 320, 40, high_score_4[0], 0, 4);
			SetNumImage(-244, -200, 320, 40, high_score_4[1], 0, 4);
			SetNumImage(-244, -152, 320, 40, high_score_4[2], 0, 4);
			SetNumImage(-244, -104, 320, 40, high_score_4[3], 0, 4);
			SetNumImage(-244, -56, 320, 40, high_score_4[4], 0, 4);
		}break;

		case 5:
		{
			SetNumImage(-244, -244, 320, 40, high_score_5[0], 0, 4);
			SetNumImage(-244, -200, 320, 40, high_score_5[1], 0, 4);
			SetNumImage(-244, -152, 320, 40, high_score_5[2], 0, 4);
			SetNumImage(-244, -104, 320, 40, high_score_5[3], 0, 4);
			SetNumImage(-244, -56, 320, 40, high_score_5[4], 0, 4);
		}break;

		}
		if (ranking >= 0 && ranking <= 4) //ランクインしたら矢印を表示
		{
			arrow.SetImage(334, -224 + 48 * ranking);
		}

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

		SetNumImage(-224, -420, 320, 40, score, 0, 4);

		UI_09.SetImage(0, 96);
		UI_result01.SetImage(128, -232);
		UI_result02.SetImage(128, -184);
		UI_result03.SetImage(128, -136);
		UI_result04.SetImage(128, -88);

		(*(blhr + score_most_hiragana)).SetImage(-224, -132);
		SetNumImage(-244, -244, 320, 40, score_get_hiragana, 0, 4);
		SetNumImage(-244, -200, 320, 40, score_leave_hiragana, 0, 4);

		SetNumImage(-244, -104, 320, 40, score_tango, 0, 4);

	}break;

	case 5:
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y + 128, 0, camera_x, camera_y + 128, 1, 0, 1, 0);

		if (mode == 0 && mode_mojisu == 3 || mode == 1 && *(si+stage_select) % 10 == 3) //3文字モードの時のスロットの背景
		{
			UI_slot_base_3.SetImage(0 + player->center_x, 200);
		}

		else if (mode == 0 && mode_mojisu == 4 || mode == 1 && *(si + stage_select) % 10 == 4) //4文字モードの時のスロットの背景
		{
			UI_slot_base_4.SetImage(0 + player->center_x, 200);
		}

		else if (mode == 0 && mode_mojisu == 5 || mode == 1 && *(si + stage_select) % 10 == 5) //5文字モードの時のスロットの背景
		{
			UI_slot_base_5.SetImage(0 + player->center_x, 200);
		}

		
		UI_11.SetImage(0 + player->center_x, 272); //スロット操作説明
		UI_12.SetImage(-440 + player->center_x, 200); //ポーズ・操作説明


		if (mode == 0)
		{
			UI_10.SetImage(440 + player->center_x, 200); //スコア・ハイスコア・タイマーの文字
			SetNumImage(360 + player->center_x, 132, 160, 20, time / 60, 0, 4); //タイマー
			SetNumImage(360 + player->center_x, 200, 160, 20, score, 0, 4); //スコア
			switch (mode_mojisu)
			{
			case 3: {SetNumImage(360 + player->center_x, 268, 160, 20, high_score_3[0], 0, 4); }break; //ハイスコア
			case 4: {SetNumImage(360 + player->center_x, 268, 160, 20, high_score_4[0], 0, 4); }break; //ハイスコア
			case 5: {SetNumImage(360 + player->center_x, 268, 160, 20, high_score_5[0], 0, 4); }break; //ハイスコア
			}
		}

		else if (mode == 1)
		{
			UI_10_2.SetImage(440 + player->center_x, 200); //スコア・ハイスコア（ベストタイム）・タイマーの文字
			SetNumImage(360 + player->center_x, 128, 160, 20, stage_select , 0, 4); //ステージ番号
			SetNumImage(360 + player->center_x, 176, 160, 20, time / 60, 0, 4); //タイマー
			SetNumImage(444 + player->center_x, 224, 160, 20, score, 0, 4); //スコア（分子）
			UI_num_aslash.SetImage(430 + player->center_x, 236); //分数のスラッシュ（名前忘れた）
			SetNumImage(360 + player->center_x, 224, 160, 20, *(stage_nolma+stage_select), 0, 4); //ノルマ（分母）
			SetNumImage(360 + player->center_x, 272, 160, 20, score_miss, 0, 4); //ミスの回数
		}



		if (mode == 1 && *(si+stage_select) == 23)
		{
			for (i = 0; i <= 2; i++)
			{
				if (*(ssc + stage_select*5+i) != 0)
				{
					UI_slot_same[*(ssc + stage_select * 5 + i)].SetImage(174 - 174 * i + player->center_x, 176);
				}//ステージモードでスロットの文字がABABで固定の場合
			}
		}

		else if (mode == 1 && *(si+stage_select) == 24)
		{
			for (i = 0; i <= 3; i++)
			{
				if (*(ssc + stage_select*5+i) != 0)
				{
					UI_slot_same[*(ssc + stage_select * 5 + i)].SetImage(216 - 144 * i + player->center_x, 176); //ステージモードでスロットの文字ABABで固定の場合
				}
			}
		}

		else if (mode == 1 && *(si+stage_select) == 25)
		{
			for (i = 0; i <= 4; i++)
			{
				if (*(ssc + stage_select * 5 + i) != 0)
				{
					UI_slot_same[*(ssc + stage_select * 5 + i)].SetImage(264 - 132 * i + player->center_x, 176); //ステージモードでスロットの文字ABABで固定の場合
				}
			}
		}


		if (mode == 0 && mode_mojisu == 3 || mode == 1 && *(si + stage_select) % 10 == 3) //3文字モードの時のひらがなスロット
		{
			(*(blUI + *(sl + 0))).SetImage(174 + player->center_x, 176); //ひらがなスロット
			(*(blUI + *(sl + 1))).SetImage(0 + player->center_x, 176);
			(*(blUI + *(sl + 2))).SetImage(-174 + player->center_x, 176);
		}

		else if (mode == 0 && mode_mojisu == 4 || mode == 1 && *(si + stage_select) % 10 == 4) //4文字モードの時ひらがなスロット
		{
			(*(blUI + *(sl + 0))).SetImage(216 + player->center_x, 176); //ひらがなスロット
			(*(blUI + *(sl + 1))).SetImage(72 + player->center_x, 176);
			(*(blUI + *(sl + 2))).SetImage(-72 + player->center_x, 176);
			(*(blUI + *(sl + 3))).SetImage(-216 + player->center_x, 176);
		}

		else if (mode == 0 && mode_mojisu == 5 || mode == 1 && *(si + stage_select) % 10 == 5) //5文字モードの時のひらがなスロット
		{
			(*(blUI + *(sl + 0))).SetImage(264 + player->center_x, 176); //ひらがなスロット
			(*(blUI + *(sl + 1))).SetImage(132 + player->center_x, 176);
			(*(blUI + *(sl + 2))).SetImage(0 + player->center_x, 176);
			(*(blUI + *(sl + 3))).SetImage(-132 + player->center_x, 176);
			(*(blUI + *(sl + 4))).SetImage(-264 + player->center_x, 176);
		}

		if (mode == 0 && mode_mojisu == 3 || mode == 1 && *(si + stage_select) % 10 == 3) //3文字モードの時の語彙スロット固定orスロット制限がある場合
		{
			for (i = 0; i <= 2; i++)
			{
				if (*(ssc + stage_select * 5 + i) != 0)
				{
					if (*(si+stage_select) == 3)
					{
						UI_slot_locked.SetImage(174 - 174 * i + player->center_x, 176); //ステージモードでスロットの文字固定の場合
					}
				}
			}
		}

		else if (mode == 0 && mode_mojisu == 4 || mode == 1 && *(si + stage_select) % 10 == 4) //4文字モードの時の語彙スロット固定orスロット制限がある場合
		{
			for (i = 0; i <= 3; i++)
			{
				if (*(ssc + stage_select * 5 + i) != 0)
				{
					if (*(si+stage_select) == 4)
					{
						UI_slot_locked.SetImage(216 - 144 * i + player->center_x, 176); //ステージモードでスロットの文字固定の場合
					}
				}
			}
		}

		else if (mode == 0 && mode_mojisu == 5 || mode == 1 && *(si + stage_select) % 10 == 5) //5文字モードの時の語彙スロット固定orスロット制限がある場合
		{
			for (i = 0; i <= 4; i++)
			{
				if (*(ssc + stage_select*5+i) != 0)
				{
					if (*(si+stage_select) == 5)
					{
						UI_slot_locked.SetImage(264 - 132 * i + player->center_x, 176); //ステージモードでスロットの文字固定の場合
					}
				}
			}
		}



		if (lamp_timer_01 == 0)
		{
			if (mode == 0 && mode_mojisu == 3 || mode == 1 && *(si + stage_select) % 10 == 3) //3文字モードの時選択箇所
			{
				UI_slot_highlight.SetImage(174 - slot_select * 174 + player->center_x, 176); //スロットの選択箇所
			}

			if (mode == 0 && mode_mojisu == 4 || mode == 1 && *(si + stage_select) % 10 == 4) //4文字モードの時の選択箇所
			{
				UI_slot_highlight.SetImage(216 - slot_select * 144 + player->center_x, 176); //スロットの選択箇所
			}

			if (mode == 0 && mode_mojisu == 5 || mode == 1 && *(si + stage_select) % 10 == 5) //5文字モードの時の選択箇所
			{
				UI_slot_highlight.SetImage(264 - slot_select * 132 + player->center_x, 176); //スロットの選択箇所
			}
		}

		if (lamp_timer_01 % 10 >= 6 && lamp_timer_01 > 0) //Ｋキーを押した後スロットを点滅させる
		{
			switch (word_hit)
			{
			case false:
			{
				if (mode == 0 && mode_mojisu == 3 || mode == 1 && *(si + stage_select) % 10 == 3) //3文字モードの時の語彙スロット点滅
				{
					UI_slot_decision.SetImage(174 + player->center_x, 176);
					UI_slot_decision.SetImage(0 + player->center_x, 176);
					UI_slot_decision.SetImage(-174 + player->center_x, 176);
				}
				else if (mode == 0 && mode_mojisu == 4 || mode == 1 && *(si + stage_select) % 10 == 4) //4文字モードの時のの語彙スロット点滅
				{
					UI_slot_decision.SetImage(216 + player->center_x, 176);
					UI_slot_decision.SetImage(72 + player->center_x, 176);
					UI_slot_decision.SetImage(-72 + player->center_x, 176);
					UI_slot_decision.SetImage(-216 + player->center_x, 176);
				}
				else if (mode == 0 && mode_mojisu == 5 || mode == 1 && *(si + stage_select) % 10 == 5) //5文字モードの時のの語彙スロット点滅
				{
					UI_slot_decision.SetImage(264 + player->center_x, 176);
					UI_slot_decision.SetImage(132 + player->center_x, 176);
					UI_slot_decision.SetImage(0 + player->center_x, 176);
					UI_slot_decision.SetImage(-132 + player->center_x, 176);
					UI_slot_decision.SetImage(-264 + player->center_x, 176);
				}

			}break;

			case true:
			{
				if (mode == 0 && mode_mojisu == 3 || mode == 1 && *(si + stage_select) % 10 == 3) //3文字モードの時の語彙スロット点滅
				{
					UI_slot_decision_green.SetImage(174 + player->center_x, 176);
					UI_slot_decision_green.SetImage(0 + player->center_x, 176);
					UI_slot_decision_green.SetImage(-174 + player->center_x, 176);
				}
				else if (mode == 0 && mode_mojisu == 4 || mode == 1 && *(si + stage_select) % 10 == 4) //4文字モードの時のの語彙スロット点滅
				{
					UI_slot_decision_green.SetImage(216 + player->center_x, 176);
					UI_slot_decision_green.SetImage(72 + player->center_x, 176);
					UI_slot_decision_green.SetImage(-72 + player->center_x, 176);
					UI_slot_decision_green.SetImage(-216 + player->center_x, 176);
				}
				else if (mode == 0 && mode_mojisu == 5 || mode == 1 && *(si + stage_select) % 10 == 5) //4文字モードの時のの語彙スロット点滅
				{
					UI_slot_decision_green.SetImage(264 + player->center_x, 176);
					UI_slot_decision_green.SetImage(132 + player->center_x, 176);
					UI_slot_decision_green.SetImage(0 + player->center_x, 176);
					UI_slot_decision_green.SetImage(-132 + player->center_x, 176);
					UI_slot_decision_green.SetImage(-264 + player->center_x, 176);
				}
			}break;
			}
		}


		for (int i = -6400; i < 300; i++) { //ここから５つ背景を描画
			BG_05.SetImage(i * 1024 + (player->center_x *1.0), -224);
		}

		for (int i = -6400; i < 300; i++) {
			BG_04.SetImage(i * 1024 + (player->center_x *0.75), -224);
		}

		for (int i = -6400; i < 300; i++) {
			BG_03.SetImage(i * 1024 + (player->center_x *0.5), -224);
		}

		for (int i = -6400; i < 300; i++) {
			BG_02.SetImage(i * 1024 + (player->center_x *0.25), -224);
		}

		for (int i = -6400; i < 300; i++) {
			BG_01.SetImage(i * 1024 + (player->center_x *0.0), -224);
		}

		if (gun_timer > 0) //銃を発射しているときのプレイヤーの描画
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
			if (*(obbl + i * 3) != 0 && *(obbl + i * 3) != 77 && *(obbl + i * 3) != 79)
			{
				(*(blhr + *(obbl + i * 3))).SetImage(double(*(obbl + i * 3 + 1)), double(*(obbl + i * 3 + 2)));
			}

			else if (*(obbl + i * 3) == 77) //お題箱描画
			{
				if (*(sl + 0) == 0 && *(sl + 1) == 0 && *(sl + 2) == 0 && *(sl + 3) == 0) //スロットが空っぽの時マ〇オのハテ〇ブロックみたいなテクスチャにする）
				{
					(*(blhr + 77)).SetImage(double(*(obbl + i * 3 + 1)), double(*(obbl + i * 3 + 2)));
				}

				else //スロットが空っぽじゃなかったらお題箱は起動しない（マ〇オのたたいた後のブロックみたいなテクスチャにする）
				{
					(*(blhr + 78)).SetImage(double(*(obbl + i * 3 + 1)), double(*(obbl + i * 3 + 2)));
				}

			}

			else if (*(obbl + i * 3) == 79) //お題箱描画
			{
				//block_hiragana[hiragana_roulette[((hiragana_roulette_timer + (slot_start[i])*60))%(74*60) / 60]].SetImage(double(*(obbl + i * 3 + 1)), double(*(obbl + i * 3 + 2))); //下式が複雑なので一応変形前も載せる

				(*(blhr + *(hrrl + ((hiragana_roulette_timer + *(slst + i) * 60)) % (74 * 60) / 60))).SetImage(double(*(obbl + i * 3 + 1)), double(*(obbl + i * 3 + 2)));

				if (lamp_timer_block % 20 >= 0 && lamp_timer_block % 20 <= 4) { block_light_2.SetImage(double(*(obbl + i * 3 + 1)), double(*(obbl + i * 3 + 2))); }
				if (lamp_timer_block % 20 >= 5 && lamp_timer_block % 20 <= 9) { block_light_1.SetImage(double(*(obbl + i * 3 + 1)), double(*(obbl + i * 3 + 2))); }
				if (lamp_timer_block % 20 >= 10 && lamp_timer_block % 20 <= 14) { block_light_2.SetImage(double(*(obbl + i * 3 + 1)), double(*(obbl + i * 3 + 2))); }
			}
		}




		if (lamp_timer_02 > 0) //Ｋキーを押した後ふきだしとエフェクト点灯
		{
			switch (word_hit)
			{
			case true: //プレイヤーの吹き出しと加点スコアを描画
			{
				player_fukidashi01.SetImage(player->center_x, player->center_y - 80);
				if (score >= 100000) { SetNumImage(player->center_x - 12 * 9, player->center_y - 128, 192, 24, score_word, 1, 4); }
				else if (score >= 10000 && score <= 99999) { SetNumImage(player->center_x - 12 * 8, player->center_y - 128, 192, 24, score_word, 1, 4); }
				else if (score >= 1000 && score <= 9999) { SetNumImage(player->center_x - 12 * 7, player->center_y - 128, 192, 24, score_word, 1, 4); }
				else if (score >= 100 && score <= 999) { SetNumImage(player->center_x - 12 * 6, player->center_y - 128, 192, 24, score_word, 1, 4); }
				else if (score >= 10 && score <= 99) { SetNumImage(player->center_x - 12 * 5, player->center_y - 128, 192, 24, score_word, 1, 4); }
				else if (score >= 0 && score <= 9) { SetNumImage(player->center_x - 12 * 4, player->center_y - 128, 192, 24, score_word, 1, 4); }

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


		if (*(si+stage_select) % 10 == 3) //ステージモードかつ3文字モードのとき，今まで作った単語を左上に表示
		{
			for (i = 0; i <= 100; i++)
			{
				if (*(mdtn + i * 5 + 0) != 0) { (*(blmn + *(mdtn + i * 5 + 0))).SetImage(600 + player->center_x, -400 + i * 35); }
				if (*(mdtn + i * 5 + 1) != 0) { (*(blmn + *(mdtn + i * 5 + 1))).SetImage(568 + player->center_x, -400 + i * 35); }
				if (*(mdtn + i * 5 + 2) != 0) { (*(blmn + *(mdtn + i * 5 + 2))).SetImage(536 + player->center_x, -400 + i * 35); }
			}
		}

		else if (*(si+stage_select) % 10 == 4) //ステージモードかつ4文字モードのとき，今まで作った単語を左上に表示
		{
			for (i = 0; i <= 100; i++) 
			{
				if (*(mdtn + i * 5 + 0) != 0) { (*(blmn + *(mdtn + i * 5 + 0))).SetImage(600 + player->center_x, -400 + i * 35); }
				if (*(mdtn + i * 5 + 1) != 0) { (*(blmn + *(mdtn + i * 5 + 1))).SetImage(568 + player->center_x, -400 + i * 35); }
				if (*(mdtn + i * 5 + 2) != 0) { (*(blmn + *(mdtn + i * 5 + 2))).SetImage(536 + player->center_x, -400 + i * 35); }
				if (*(mdtn + i * 5 + 3) != 0) { (*(blmn + *(mdtn + i * 5 + 3))).SetImage(504 + player->center_x, -400 + i * 35); }
			}
		}

		else if (*(si+stage_select) % 10 == 5) //ステージモードかつ5文字モードのとき，今まで作った単語を左上に表示
		{
			for (i = 0; i <= 100; i++)
			{
				if (*(mdtn + i * 5 + 0) != 0) { (*(blmn + *(mdtn + i * 5 + 0))).SetImage(600 + player->center_x, -400 + i * 35); }
				if (*(mdtn + i * 5 + 1) != 0) { (*(blmn + *(mdtn + i * 5 + 1))).SetImage(568 + player->center_x, -400 + i * 35); }
				if (*(mdtn + i * 5 + 2) != 0) { (*(blmn + *(mdtn + i * 5 + 2))).SetImage(536 + player->center_x, -400 + i * 35); }
				if (*(mdtn + i * 5 + 3) != 0) { (*(blmn + *(mdtn + i * 5 + 3))).SetImage(504 + player->center_x, -400 + i * 35); }
				if (*(mdtn + i * 5 + 4) != 0) { (*(blmn + *(mdtn + i * 5 + 4))).SetImage(472 + player->center_x, -400 + i * 35); }
			}
		}


	}break;

	case 6: //ゲームオーバー画面
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		UI_gameover.SetImage(0, -416);
		UI_tonext.SetImage(0, 64);

	}break;

	case 7: //モード選択画面
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		if (mode == 0)
		{
			UI_mode_description_1.SetImage(-315, -256);
			UI_mode_highscore_1.SetImage(-315, -20);
			UI_mode_waku.SetImage(-315, -20);
			SetNumImage(-520, -80, 400, 50, high_score_3[0], 0, 4);
			SetNumImage(-520, 0, 400, 50, high_score_4[0], 0, 4);
			SetNumImage(-520, 80, 400, 50, high_score_5[0], 0, 4);
		}

		else if (mode == 1)
		{
			UI_mode_description_2.SetImage(-315, -256);
			UI_mode_highscore_2.SetImage(-315, -20);
			UI_mode_waku.SetImage(-315, -20);
			SetNumImage(-350, -80, 300, 50, stage_medal[0], 0, 4);
			SetNumImage(-350, 0, 300, 50, stage_medal[1], 0, 4);
			SetNumImage(-350, 80, 300, 50, stage_medal[2], 0, 4);
			UI_num_aslash_big.SetImage(-375, 25);
			UI_num_aslash_big.SetImage(-375, 105);
			UI_num_aslash_big.SetImage(-375, -55);
			SetNumImage(-520, 80, 300, 50, STAGE_AVAILABLE, 0, 4);
			SetNumImage(-520, 0, 300, 50, STAGE_AVAILABLE, 0, 4);
			SetNumImage(-520, -80, 300, 50, STAGE_AVAILABLE, 0, 4);


		}

		UI_mode_score_attack.SetImage(224, -364);
		UI_mode_stage_clear.SetImage(224, -76);

		UI_mode_highlight.SetImage(224, -364 + mode * 288);
		UI_mode_select_UI.SetImage(224, 112);

	}break;

	case 8: //文字数選択画面
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		UI_mode_3.SetImage(224, -76);
		UI_mode_4.SetImage(0, -76);
		UI_mode_5.SetImage(-224, -76);
		UI_mode_mojisu.SetImage(0, -416);
		UI_mode_mojisu_highlight.SetImage(224 - (mode_mojisu - 3) * 224, -76);
		UI_mode_select_mojisu_UI.SetImage(0, 112);

	}break;

	case 9: //ステージ選択画面
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		UI_mode_stage.SetImage(0, -460);
		UI_block_stage_num_select.SetImage(0, -348);

		if (stage_select >= 2) //ステージ選択の左の矢印
		{
			UI_arrow_2L.SetImage(488, -348);
		}

		if (stage_select <= STAGE_AVAILABLE - 1) //ステージ選択の右の矢印
		{
			UI_arrow_2R.SetImage(-488, -348);
		}

		for (i = -3; i <= 3; i++)
		{
			if (stage_select + i >= 1 && stage_select + i <= STAGE_AVAILABLE) //ステージ選択のブロックの描画
			{
				UI_clear_lamp_off.SetImage(-i * 128, -280);
				UI_block_stage_num.SetImage(-i * 128, -348);
				if (*(si + stage_select + i) == 0)
				{
					UI_coming_soon64.SetImage(-i * 128, -236);
				}
				if (*(sc + stage_select * 5 + 0) == 1) { UI_clear_lamp_on_1.SetImage(-i * 128, -280); } //クリアランプの点灯
				if (*(sc + stage_select * 5 + 1) == 1) { UI_clear_lamp_on_2.SetImage(-i * 128, -280); }
				if (*(sc + stage_select * 5 + 2) == 1) { UI_clear_lamp_on_3.SetImage(-i * 128, -280); }

				if (*(sc + stage_select * 5 + 0) == 1 && *(sc + stage_select * 5 + 1) == 1 && *(sc + stage_select * 5 + 2) == 1) //全点灯アニメーション
				{
					if (lamp_timer_clear % 12 <= 5) {
						UI_clear_lamp_on_1_lux.SetImage(-i * 128, -280);
						UI_clear_lamp_on_2_lux.SetImage(-i * 128, -280);
						UI_clear_lamp_on_3_lux.SetImage(-i * 128, -280);
					}
				}
			}
		}

		for (i = -3; i <= 3; i++)
		{
			if (stage_select + i >= 1 && stage_select + i <= STAGE_AVAILABLE) //ステージ選択の番号の描画
			{
				if (stage_select + i >= 100)  //桁数によってブロックへの数字の画像の納め方が変わるので分けた＆ステージが定義されている場合は明るいフォントで描画
				{
					if ( *(si + stage_select+i) == 0)
					{
						SetNumImage(-38 - i * 128, -372, 288, 48, stage_select + i, 3, -16);
					}

					else
					{
						SetNumImage(-38 - i * 128, -372, 288, 48, stage_select + i, 2, -16);
					}
				}

				else if (stage_select + i >= 10 && stage_select + i <= 99)
				{
					if (*(si + stage_select + i) == 0)
					{
						SetNumImage(-40 - i * 128, -372, 384, 48, stage_select + i, 3, -16);
					}

					else
					{
						SetNumImage(-40 - i * 128, -372, 384, 48, stage_select + i, 2, -16);
					}
				}
				else if (stage_select + i <= 9)
				{
					if (*(si + stage_select + i) == 0)
					{
						SetNumImage(-24 - i * 128, -372, 384, 48, stage_select + i, 3, -16);
					}

					else
					{
						SetNumImage(-24 - i * 128, -372, 384, 48, stage_select + i, 2, -16);
					}
				}
			}
		}

		if (*(si+stage_select) != 0) //ステージが定義されている場合ステージの概要情報と決定ボタンを描画
		{
			UI_time_limit_description.SetImage(0, 32);
			UI_16.SetImage(0, 128); //決定ボタン

			SetNumImage(155, -16, 320, 40, *(stl + stage_select), 0, 4); //ステージの制限時間
			SetNumImage(155, 40, 320, 40, *(stlg + stage_select), 0, 4); //ステージのメダル時間

			if (*(sc + stage_select * 5 + 3) == 0) //今選んでるステージが未クリア（クリアタイム０）のときはーーー表示
			{
				SetNumImage(-376, -16, 160, 40, 1, 6, 4);
				SetNumImage(-426, -16, 160, 40, 1, 6, 4);
				SetNumImage(-352, -16, 160, 40, 1, 6, 4); 
				SetNumImage(-401, -16, 160, 40, 0, 6, 4); //小数点
			}

			else //クリアしたとき
			{
				SetNumImage(-376, -16, 160, 40, *(sc + stage_select * 5 + 3) / 100, 0, 4); //ステージのハイスコア整数部分
				SetNumImage(-426, -16, 160, 40, (int)((*(sc + stage_select * 5 + 3) / 10) % 10), 0, 4); //ステージのハイスコア小数部分
				SetNumImage(-401, -16, 160, 40, 0, 6, 4); //小数点
			}

			switch (*(si+stage_select))//stage_infno情報は1の位でなんもじもーどか決まる
			{
			case 3: 
			{
				if (*(ssc + stage_select * 5 + 0) == 0 && *(ssc + stage_select * 5 + 1) == 0 && *(ssc + stage_select * 5 + 2) == 0)
				{
					UI_mission_description_0.SetImage(0, -145); //スロット固定がない場合「ただのｎ単語作成ミッション」になる
					SetNumImage(80, -166, 320, 40, *(sn+stage_select), 0, 4);
				}
				else
				{
					UI_slot_constraint_description.SetImage(0, -160);
					UI_mission_description_1.SetImage(0, -96); //スロット固定がある場合のn単語作成ミッション
					SetNumImage(-48, -115, 320, 40, *(sn + stage_select), 0, 4);
					(*(blhr + *(ssc + stage_select * 5 + 0))).SetImage(-64, -160); //固定スロットの情報を描画
					(*(blhr + *(ssc + stage_select * 5 + 1))).SetImage(-128, -160);
					(*(blhr + *(ssc + stage_select * 5 + 2))).SetImage(-192, -160);
				}


			}break;

			case 4:
			{
				if (*(ssc + stage_select * 5 + 0) == 0 && *(ssc + stage_select * 5 + 1) == 0 && *(ssc + stage_select * 5 + 2) == 0 && *(ssc + stage_select * 5 + 3) == 0)
				{
					UI_mission_description_0.SetImage(0, -145);//スロット固定がない場合「ただのｎ単語作成ミッション」になる
					SetNumImage(80, -166, 320, 40, *(sn + stage_select), 0, 4);
				}
				else
				{
					UI_slot_constraint_description.SetImage(0, -160);
					UI_mission_description_1.SetImage(0, -96);//スロット固定がある場合のn単語作成ミッション

					SetNumImage(-48, -115, 320, 40, *(sn + stage_select), 0, 4);
					(*(blhr + *(ssc + stage_select * 5 + 0))).SetImage(-64, -160); //固定スロットの情報を描画
					(*(blhr + *(ssc + stage_select * 5 + 1))).SetImage(-128, -160);
					(*(blhr + *(ssc + stage_select * 5 + 2))).SetImage(-192, -160);
					(*(blhr + *(ssc + stage_select * 5 + 3))).SetImage(-256, -160);
				}

			}break;

			case 5:
			{
				if (*(ssc + stage_select * 5 + 0) == 0 && *(ssc + stage_select * 5 + 1) == 0 && *(ssc + stage_select * 5 + 2) == 0 && *(ssc + stage_select * 5 + 3) == 0 && *(ssc + stage_select * 5 + 4) == 0)
				{
					UI_mission_description_0.SetImage(0, -145);//スロット固定がない場合「ただのｎ単語作成ミッション」になる
					SetNumImage(80, -166, 320, 40, *(sn + stage_select), 0, 4);
				}
				else
				{
					UI_slot_constraint_description.SetImage(0, -160);
					UI_mission_description_1.SetImage(0, -96);//スロット固定がある場合のn単語作成ミッショ
					SetNumImage(-48, -115, 320, 40, *(sn + stage_select), 0, 4);
					(*(blhr + *(ssc + stage_select * 5 + 0))).SetImage(-64, -160); //固定スロットの情報を描画
					(*(blhr + *(ssc + stage_select * 5 + 1))).SetImage(-128, -160);
					(*(blhr + *(ssc + stage_select * 5 + 2))).SetImage(-192, -160);
					(*(blhr + *(ssc + stage_select * 5 + 3))).SetImage(-256, -160);
					(*(blhr + *(ssc + stage_select * 5 + 4))).SetImage(-320, -160);
				}

			}break;

			case 13: case 14: case 15: //しりとりミッション
			{
				UI_mission_description_2.SetImage(0, -145);
				SetNumImage(10, -166, 320, 40, stage_nolma[stage_select], 0, 4);

			}break;

			case 23: case 24: case 25: //A〇A〇 Aにはおなじひらがなが入りますミッション
			{
				UI_slot_AB_description.SetImage(-150, -50);

				UI_slot_constraint_description.SetImage(0, -160);
				UI_mission_description_1.SetImage(0, -96);//スロット固定がある場合のn単語作成ミッショ
				SetNumImage(-48, -115, 320, 40, *(sn+stage_select), 0, 4);
				slot_same_block[*(ssc + stage_select * 5 + 0)].SetImage(-64, -160); //スロット制限の情報を描画
				slot_same_block[*(ssc + stage_select * 5 + 1)].SetImage(-128, -160);
				slot_same_block[*(ssc + stage_select * 5 + 2)].SetImage(-192, -160);
				if (*(si+stage_select) >= 24) { slot_same_block[*(ssc + stage_select * 5 + 3)].SetImage(-256, -160); }
				if (*(si+stage_select) >= 25) { slot_same_block[*(ssc + stage_select * 5 + 4)].SetImage(-320, -160); }

			}break;
			}

		}

		UI_17.SetImage(0, 128);

	}break;


	case 10: //ほんとうによろしいですか？
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		UI_confirm.SetImage(0, -416);
		UI_confirm_2.SetImage(0, 64);

	}break;

	case 11: //ステージクリアモードのリザルト
	{
		glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
		gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);

		UI_result_stage.SetImage(0, -160); //
		UI_result_stage_title.SetImage(80, -416); //ステージ
		SetNumImage(-300, -464, 768, 96, stage_select, 2, -16); //ステージ番号
		SetNumImage(-180, -100, 256, 64, (*(stl+stage_select) - (double)(time) / 60) * 100 /100 - score_miss * 30, 0, 4); //整数部分
		SetNumImage(-250, -100, 256, 64, ((int)((*(stl + stage_select) - (double)(time) / 60) * 100 )/10)%10 , 0, 4); //小数点部分
		SetNumImage(-210, -100, 256, 64, 0, 6, 4); //小数点
		SetNumImage(-260, -192, 512, 64, score_miss, 0, 4); //▼
		UI_15.SetImage(0, 64);

		if (before_stageclear_time > (*(stl + stage_select) - (double)(time) / 60) * 100 || before_stageclear_time == 0) //タイム更新 ファイルの0は未プレイを表している（0秒クリアはありえないので）
		{
			UI_newrecord_stage.SetImage(420, -60); //new_Recordの表示
		}

		if (before_stageclear_miss > score_miss )  //ミスの回数更新
		{
			UI_newrecord_stage.SetImage(420, -152); //new_Recordの表示
		}

		if (lamp_timer_clear % 12 >= 6) //リザルト画面でのメダルのきらびやかアニメーション
		{
			UI_result_stage_medal_1.SetImage(0, -160);

			if (score_miss == 0)
			{
				UI_result_stage_medal_2.SetImage(0, -160);
			}

			if (*(stl+stage_select) - time / 60 - score_miss * 30 <= *(stlg + stage_select))
			{
				UI_result_stage_medal_3.SetImage(0, -160);
			}
		}
		else
		{
			UI_result_stage_medal_1_lux.SetImage(0, -160);

			if (score_miss == 0)
			{
				UI_result_stage_medal_2_lux.SetImage(0, -160);
			}

			if (*(stl + stage_select) - time / 60 - score_miss * 30 <= *(stlg + stage_select))
			{
				UI_result_stage_medal_3_lux.SetImage(0, -160);
			}

		}

	}break;

	}
	glutSwapBuffers();
}

void idle(void)
{
	double player_y_before = 0;

	int *obbl = &object_block[0][0];
	int *slst = &slot_start[0];
	int *hrrl = &hiragana_roulette[0];
	int *odhr3 = &odai_hiragana_3[0][0];
	int *odhr4 = &odai_hiragana_4[0][0];
	int *odhr5 = &odai_hiragana_5[0][0];
	int *sl = &slot[0];
	int *si = &stage_info[0];

	// std::cout << <--------------------フレーム開始--------------------->" << std::endl;

	/*
	 std::cout << "<info 007: プレイヤーの座標：[ "<< player->center.x << "],[" <<  player->center.y <<"]" << std::endl; //コピー用 随所で使う
	 std::cout << "<info 007: 見てるブロックの座標：[" << *(obbl + i * 3+1) << "],[" << *(obbl + i * 3+2) <<"]" << std::endl; //コピー用 随所で使う
	 */

	if (player_jump == true && flag_move_y == true && move_lock == false) //ジャンプ
	{
		player_y_before = player->center_y;

		switch (flag_jump_slow)
		{
		case true://ふわふわ落下モード
		{
			player->Move(0, -(15.8 * jump_timer - 9.8* jump_timer * sqrt(jump_timer) *0.5)*0.0015 * 36); //★基本的なジャンプの物理式/最高点到達より後（いろいろ調整したので汚いですが…）
		}break;

		case false:
		{
			player->Move(0, -(15.8 * jump_timer - 9.8* jump_timer * sqrt(jump_timer) *0.5)*0.01 * 128); //★基本的なジャンプの物理式/最高点到達より前（いろいろ調整したので汚いですが…）
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
			if (player->center_y < *(obbl + i * 3 + 2) && *(obbl + i * 3) != 0) //プレイやーがブロックより下側にいる時，かつ比較するオブジェクトブロックが空白でないとき
			{
				if (abs(player->center_x - double(*(obbl + i * 3 + 1))) < 48 && abs(player->center_y - double(*(obbl + i * 3 + 2))) <= 64) //ブロックとの距離がx<48 y<64であるとき
				{
					// std::cout << "<info 002: 下に行こうとしてブロック[ << i <<] に衝突しています>" << std::endl;
					flag_collision_D = true;

					if (height_c > *(obbl + i * 3 + 2))
					{
						height_c = *(obbl + i * 3 + 2); //着地点の座標を記録
					}
				}
			}
		}

		for (i = 0; i < object_on_stage; i++)
		{
			if (player->center_y > *(obbl + i * 3 + 2) && *(obbl + i * 3) != 0) //プレイやーがブロックより上側にいる時，かつ比較するオブジェクトブロックが空白でないとき
			{
				if (abs(player->center_x - double(*(obbl + i * 3 + 1))) < 48 && abs(player->center_y - double(*(obbl + i * 3 + 2))) <= 64) //ブロックとの距離がx<48 y<64であるとき
				{
					// std::cout << "<info 003: 上に行こうとしてブロック[ << i <<] に衝突しています>" << std::endl;
					flag_collision_U = true;
				}
			}
		}

		if (flag_collision_U == true) //天井との衝突を感知したフレームでの処理
		{

			jump_timer = JUMP_HIGHEST;
			player->Move(0, (-((int)(player->center_y))) % 64); //衝突したらそのブロックの下にブロックまで戻す

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

	else if (player_jump == false && flag_move_y == true && move_lock == false) //自由落下の制御
	{
		flag_collision_D = false;

		for (i = 0; i < object_on_stage; i++)
		{
			if (player->center_y < *(obbl + i * 3 + 2) && *(obbl + i * 3) != 0) //プレイやーがブロックより下側にいる時，かつ比較するオブジェクトブロックが空白でないとき
			{
				if (abs(player->center_x - double(*(obbl + i * 3 + 1))) < 48 && abs(player->center_y - double(*(obbl + i * 3 + 2))) <= 64) //ブロックとの距離がx<48 y<64であるとき
				{
					// std::cout << "<info 004: 下に行こうとしてブロック[ << i <<] に衝突しています>" << std::endl;
					flag_collision_D = true;

					if (height_c > *(obbl + i * 3 + 2))
					{
						height_c = *(obbl + i * 3 + 2); //着地点を取得

					}
				}
			}
		}

		if (flag_collision_D == false) //地面との衝突を感知したフレームでの処理
		{
			height_c = 8000; //自分の位置よりも風聞に低い場所に衝突判定に使う数値を戻す
		}

		if (flag_collision_D == false) { jump_timer = JUMP_HIGHEST; player_jump = true; } //足場がなくなると自由落下（タイマー = JUMP_HIGHEST で鉛直投げ上げ最高点）

		flag_move_y = false;
	}

	if (onMoveKeyPress_L == true && flag_move_x == true && move_lock == false) { //左に移動


		camera_x += speed;

		player->Move(speed, 0);

		for (i = 0; i < object_on_stage; i++)
		{
			if (player->center_x < *(obbl + i * 3 + 1) && *(obbl + i * 3) != 0) //プレイやーがブロックより←側にいる時，かつ比較するオブジェクトブロックが空白でないとき
			{
				if (abs(player->center_x - double(*(obbl + i * 3 + 1))) < 48 && abs(player->center_y - double(*(obbl + i * 3 + 2))) < 60
					) //ブロックとの距離がx<48 y<64であるとき
				{
					// std::cout << "<info 005: 左に行こうとしてブロック[ << i <<] に衝突しています>" << std::endl;
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

	if (onMoveKeyPress_R == true && flag_move_x == true && move_lock == false) { //右に移動

		camera_x -= speed;

		player->Move(-speed, 0);

		for (i = 0; i < object_on_stage; i++)
		{
			if (player->center_x > *(obbl + i * 3 + 1) && *(obbl + i * 3) != 0) //プレイやーがブロックより→側にいる時，かつ比較するオブジェクトブロックが空白でないとき
			{
				if (abs(player->center_x - double(*(obbl + i * 3 + 1))) < 48 && abs(player->center_y - double(*(obbl + i * 3 + 2))) < 60) //ブロックとの距離がx<48 y<64であるとき
				{
					// std::cout << "<info 006: 右に行こうとしてブロック[ << i <<] に衝突しています>" << std::endl;
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

	if (flag_bullet_exist == true && flag_move_bullet == true && move_lock == false) //弾丸と語彙ブロックの衝突判定
	{
		switch (bullet->direction) //弾をそのときプレイヤーが向いていた方向に動かす
		{
		case 0:
		{
			bullet->Move(32, 0);
		}break;

		case 1:
		{
			bullet->Move(-32, 0);
		}break;
		}

		for (i = 0; i < object_on_stage; i++)
		{
			if (flag_bullet_exist == true && *(obbl + i * 3) != 0 && abs(bullet->center_x - double(*(obbl + i * 3 + 1))) < 32 && abs(bullet->center_y - double(*(obbl + i * 3 + 2))) < 32) //ブロックとの距離がx<32 y<64で32あるとき（ブロックＩＤ＝０すなわち空気の時はスルー）
			{
				flag_bullet_exist = false;
				if (*(sl + slot_select) == 0 && *(obbl + i * 3) != 49 && *(obbl + i * 3) != 76 && *(obbl + i * 3) != 77 && *(obbl + i * 3) != 78 && *(obbl + i * 3) != 79)//すでにスロットにひらがなが入っている場合は衝突してもブロック消えないしひらがなも保持されない,あと木はスロットには入れられない（当然
				{
					*(sl + slot_select) = *(obbl + i * 3); //弾丸が衝突したブロックをスロットに格納
					play_SE(SE_hit);
					score_get_hiragana++;
		
					if (mode == 1 && *(si+stage_select) >= 23 && *(si+stage_select) <= 25) //A〇A〇のとき
					{
						
						if (stage_slot_constraint[stage_select][slot_select] != 0) //選択しているスロットがＡとかになってるとき
						{
							
							for (k = 0; k <= 4; k++)
							{
								
								if (stage_slot_constraint[stage_select][slot_select] == stage_slot_constraint[stage_select][k])
								{
									*(sl + k) = *(obbl + i * 3);
								}
							}
						}
					}

					*(obbl + i * 3) = 0; //弾丸とブロックが衝突したらそのブロックの情報を０にする

				}
				
				
				else if (*(obbl + i * 3) == 77) //お題箱にヒットしたとき（ごいスロットに何かあるときはＯＦＦ状態になる）
				{
					if (mode_mojisu == 3 && slot[0] == 0 && slot[1] == 0 && slot[2] == 0)//3文字モードの時
					{
						odai = choose_odai(mode_mojisu); //おだい
						*(sl + 0) = *(odhr3 + odai * 5 + 0);
						*(sl + 1) = *(odhr3 + odai * 5 + 1);
						*(sl + 2) = *(odhr3 + odai * 5 + 2);
						play_SE(SE_hit);
						*(obbl + i * 3) = 0; //弾丸とブロックが衝突したらお互いの情報を０にする
					}

					else if (mode_mojisu == 4 && slot[0] == 0 && slot[1] == 0 && slot[2] == 0 && slot[3] == 0)//4文字モードの時
					{
						odai = choose_odai(mode_mojisu); //おだい
						*(sl + 0) = *(odhr4 + odai * 5 + 0);
						*(sl + 1) = *(odhr4 + odai * 5 + 1);
						*(sl + 2) = *(odhr4 + odai * 5 + 2);
						*(sl + 3) = *(odhr4 + odai * 5 + 3);
						play_SE(SE_hit);
						*(obbl + i * 3) = 0; //弾丸とブロックが衝突したらお互いの情報を０にする
					}

					else if (mode_mojisu == 5 && slot[0] == 0 && slot[1] == 0 && slot[2] == 0 && slot[3] == 0 && slot[4] == 0)//5文字モードの時
					{
						odai = choose_odai(mode_mojisu); //おだい
						*(sl + 0) = *(odhr5 + odai * 5 + 0);
						*(sl + 1) = *(odhr5 + odai * 5 + 1);
						*(sl + 2) = *(odhr5 + odai * 5 + 2);
						*(sl + 3) = *(odhr5 + odai * 5 + 3);
						*(sl + 4) = *(odhr5 + odai * 5 + 4);
						play_SE(SE_hit);
						*(obbl + i * 3) = 0; //弾丸とブロックが衝突したらお互いの情報を０にする
					}
				}

				else if (*(obbl + i * 3) == 79 && slot[slot_select] == 0) //ひらがなルーレットにヒットした場合
				{
					*(sl + slot_select) = *(hrrl + ((hiragana_roulette_timer + *(slst + i) * 60)) % (74 * 60) / 60); //弾丸が衝突したブロックをスロットに格納（ランダムで選ばれたスロットの開始位置＋ひらがなの総数の結果ひらがなの総数を超えてしまう場合，ひらがなの総数で割ったあまりを求めることでルーレットの中身がひらがなの総数分から外れることを防いでいる）
					

					if (mode == 1 && *(si+stage_select) >= 23 && *(si+stage_select) <= 25) //A〇A〇のとき
					{

						if (stage_slot_constraint[stage_select][slot_select] != 0) //選択しているスロットがＡとかになってるとき
						{

							for (k = 0; k <= 4; k++)
							{

								if (stage_slot_constraint[stage_select][slot_select] == stage_slot_constraint[stage_select][k])
								{
									*(sl + k) = *(hrrl + ((hiragana_roulette_timer + *(slst + i) * 60)) % (74 * 60) / 60); //ルーレットにあたっても処理は普通の平仮名の時と同じ
								}
							}
						}
					}
					
					
					
					*(obbl + i * 3) = 0; //弾丸とブロックが衝突したらお互いの情報を０にする
					play_SE(SE_hit);
					score_get_hiragana++; //ひらがなを入手した数+1
				}
			}

		}
		flag_move_bullet = false;
	}

	if (scene == 5 && mode == 1 && stage_nolma[stage_select] == score) //ステージクリアモードでノルマを達成した
	{
		scene = 11;
		camera_x = 640; camera_y = -544; player->center_x = 0; player->center_y = 0;

		fclose(fp_stageclear);
		std::cout << "<info 041: ステージクリア進捗状況ファイルを「読み取り用として」閉じました>" << std::endl;

		if ((fopen_s(&fp_stageclear, "./dat/stage_clear.dat", "w")) != 0) //スコアファイルを読み込む
		{
			std::cout << "<info 042:ステージクリア進捗状況ファイルを「書き取り用として」開けませんでした>" << std::endl;
			exit(42);
		}

		std::cout << "<info 043:スステージクリア進捗状況ファイルを「書き取り用として」開きました>\n" << std::endl;

		before_stageclear_miss = stage_clear[stage_select][4]; //リザルト画面でnew recordを表示するために更新前の情報が必要なのでここでとっておく
		before_stageclear_time = stage_clear[stage_select][3];

		if (stage_clear[stage_select][4] > score_miss) //ミス回数のハイスコア更新
		{
			stage_clear[stage_select][4] = score_miss;
		}

		//printf("ttt %d ttt %d ttt\n", stage_clear[stage_select][3], (int)((float)(stage_time_limit[stage_select]) - time/ 60)*100);

	

		if (stage_clear[stage_select][3] > (int)((stage_time_limit[stage_select] - (double)(time) / 60) * 100) - 3000 * score_miss || before_stageclear_time == 0) //タイム更新 ファイルの0は未プレイを表している（0秒クリアはありえないので）
		{
			stage_clear[stage_select][3] = (int)((stage_time_limit[stage_select] - (double)(time) / 60) * 100) - 3000 * score_miss; //◆
			
		}

		stage_clear[stage_select][0] = 1; //ノルマクリアで無条件でメダル一枚目獲得

		if (score_miss == 0)
		{
			stage_clear[stage_select][1] = 1; //ミス０回でメダル二枚目獲得
		}

		if (stage_time_limit[stage_select] - time / 60 - score_miss * 30 <= stage_time_limit_gold[stage_select])
		{
			stage_clear[stage_select][2] = 1; //ゴールドタイマーでメダル3枚目獲得

		}

		stage_medal[0] = 0;
		stage_medal[1] = 0;
		stage_medal[2] = 0;

		for (i = 0; i <= STAGE_LIMIT; i++)
		{
			fprintf(fp_stageclear, "%d,%d,%d,%d,%d\n", stage_clear[i][0], stage_clear[i][1], stage_clear[i][2], stage_clear[i][3], stage_clear[i][4]); //ステージクリア進捗状況ファイルを更新
			
			if (i <= STAGE_AVAILABLE) //ステージのクリア状況の表示は，現在選択できるステージまでの情報のみを参照する（プレイできるステージ以降のステージもテストプレイでクリアデータが入ってる可能性があるため）
			{
				if (stage_clear[i][0] == 1) { stage_medal[0]++; }
				if (stage_clear[i][1] == 1) { stage_medal[1]++; }
				if (stage_clear[i][2] == 1) { stage_medal[2]++; } //メダルを取った数を記録し，「何ステージをどのくらいクリアしているか」の情報を取得する
			}
		}


		fclose(fp_stageclear); //書き取り用として閉じて，もっかい読み取り用として開く

		std::cout << "<info 047: ステージクリア進捗状況ファイルを「書き取り用として」閉じました>" << std::endl;

		if ((fopen_s(&fp_stageclear, "./dat/stage_clear.dat", "r")) != 0) //スコアファイルを読み込む
		{
			std::cout << "<info 048:ステージクリア進捗状況ファイルを「読み取り用として」開けませんでした>" << std::endl;
			exit(48);
		}

		std::cout << "<info 049:ステージクリア進捗状況ファイルを「読み取り用として」開きました>\n" << std::endl;

	}

	if (time < 0 && scene == 5) //タイムアップ時
	{
		time = 0;
		camera_x = 640; camera_y = -544; player->center_x = 0; player->center_y = 0;

		if (mode == 0) //スコアアタックモードのとき，ランキングを更新する
		{
			scene = 6;

			switch (mode_mojisu) //文字数ごとにスコアをランキング化する（なんかもうちょっときれいにできそう）
			{
			case 3:
			{
				fclose(fp_score_3);
				std::cout << "<info 019: スコアファイルを「読み取り用として」閉じました>" << std::endl;

				if ((fopen_s(&fp_score_3, "./dat/score_3.dat", "w")) != 0) //スコアファイルを読み込む
				{
					std::cout << "<info 020:スコアファイルを「書き取り用として」開けませんでした>" << std::endl;
					exit(20);
				}

				std::cout << "<info 021:スコアファイルを「書き取り用として」開きました>\n" << std::endl;

				if (high_score_3[0] < score)
				{
					ranking = 0;
					high_score_3[4] = high_score_3[3];
					high_score_3[3] = high_score_3[2];
					high_score_3[2] = high_score_3[1];
					high_score_3[1] = high_score_3[0];
					high_score_3[0] = score;
				}

				else if (high_score_3[1] < score)
				{
					ranking = 1;
					high_score_3[4] = high_score_3[3];
					high_score_3[3] = high_score_3[2];
					high_score_3[2] = high_score_3[1];
					high_score_3[1] = score;
				}
				else if (high_score_3[2] < score)
				{
					ranking = 2;
					high_score_3[4] = high_score_3[3];
					high_score_3[3] = high_score_3[2];
					high_score_3[2] = score;
				}

				else if (high_score_3[3] < score)
				{
					ranking = 3;
					high_score_3[4] = high_score_3[3];
					high_score_3[3] = score;
				}

				else if (high_score_3[4] < score)
				{
					ranking = 4;
					high_score_3[4] = score;
				}

				else
				{
					ranking = 5; //ランク外
				}

				for (i = 0; i <= 4; i++)
				{
					fprintf(fp_score_3, "%d\n", high_score_3[i]); //スコアファイルを更新
				}

				for (i = 0; i <= 79; i++)
				{
					if (list_most_hiragana[i] > max_most_hiragana)
					{
						max_most_hiragana = list_most_hiragana[i];
						score_most_hiragana = i;
					}
				}

				fclose(fp_score_3); //書き取り用として閉じて，もっかい読み取り用として開く

				std::cout << "<info 044: スコアファイルを「書き取り用として」閉じました>" << std::endl;

				if ((fopen_s(&fp_score_3, "./dat/score_3.dat", "r")) != 0) //スコアファイルを読み込む
				{
					std::cout << "<info 045:スコアファイルを「読み取り用として」開けませんでした>" << std::endl;
					exit(45);
				}

				std::cout << "<info 046:スコアファイルを「読み取り用として」開きました>\n" << std::endl;
			}break;

			case 4:
			{
				fclose(fp_score_4);
				std::cout << "<info 019: スコアファイルを「読み取り用として」閉じました>" << std::endl;

				if ((fopen_s(&fp_score_4, "./dat/score_4.dat", "w")) != 0) //スコアファイルを読み込む
				{
					std::cout << "<info 020:スコアファイルを「書き取り用として」開けませんでした>" << std::endl;
					exit(20);
				}

				std::cout << "<info 021:スコアファイルを「書き取り用として」開きました>\n" << std::endl;

				if (high_score_4[0] < score)
				{
					ranking = 0;
					high_score_4[4] = high_score_4[3];
					high_score_4[3] = high_score_4[2];
					high_score_4[2] = high_score_4[1];
					high_score_4[1] = high_score_4[0];
					high_score_4[0] = score;
				}

				else if (high_score_4[1] < score)
				{
					ranking = 1;
					high_score_4[4] = high_score_4[3];
					high_score_4[3] = high_score_4[2];
					high_score_4[2] = high_score_4[1];
					high_score_4[1] = score;
				}
				else if (high_score_4[2] < score)
				{
					ranking = 2;
					high_score_4[4] = high_score_4[3];
					high_score_4[3] = high_score_4[2];
					high_score_4[2] = score;
				}

				else if (high_score_4[3] < score)
				{
					ranking = 3;
					high_score_4[4] = high_score_4[3];
					high_score_4[3] = score;
				}

				else if (high_score_4[4] < score)
				{
					ranking = 4;
					high_score_4[4] = score;
				}

				else
				{
					ranking = 5; //ランク外
				}

				for (i = 0; i <= 4; i++)
				{
					fprintf(fp_score_4, "%d\n", high_score_4[i]); //スコアファイルを更新
				}

				for (i = 0; i <= 79; i++)
				{
					if (list_most_hiragana[i] > max_most_hiragana)
					{
						max_most_hiragana = list_most_hiragana[i];
						score_most_hiragana = i;
					}
				}

				fclose(fp_score_4); //書き取り用として閉じて，もっかい読み取り用として開く

				std::cout << "<info 044: スコアファイルを「書き取り用として」閉じました>" << std::endl;

				if ((fopen_s(&fp_score_4, "./dat/score_4.dat", "r")) != 0) //スコアファイルを読み込む
				{
					std::cout << "<info 045:スコアファイルを「読み取り用として」開けませんでした>" << std::endl;
					exit(45);
				}

				std::cout << "<info 046:スコアファイルを「読み取り用として」開きました>\n" << std::endl;
			}break;

			case 5:
			{
				fclose(fp_score_5);
				std::cout << "<info 019: スコアファイルを「読み取り用として」閉じました>" << std::endl;

				if ((fopen_s(&fp_score_5, "./dat/score_5.dat", "w")) != 0) //スコアファイルを読み込む
				{
					std::cout << "<info 020:スコアファイルを「書き取り用として」開けませんでした>" << std::endl;
					exit(20);
				}

				std::cout << "<info 021:スコアファイルを「書き取り用として」開きました>\n" << std::endl;

				if (high_score_5[0] < score)
				{
					ranking = 0;
					high_score_5[4] = high_score_5[3];
					high_score_5[3] = high_score_5[2];
					high_score_5[2] = high_score_5[1];
					high_score_5[1] = high_score_5[0];
					high_score_5[0] = score;
				}

				else if (high_score_5[1] < score)
				{
					ranking = 1;
					high_score_5[4] = high_score_5[3];
					high_score_5[3] = high_score_5[2];
					high_score_5[2] = high_score_5[1];
					high_score_5[1] = score;
				}
				else if (high_score_5[2] < score)
				{
					ranking = 2;
					high_score_5[4] = high_score_5[3];
					high_score_5[3] = high_score_5[2];
					high_score_5[2] = score;
				}

				else if (high_score_5[3] < score)
				{
					ranking = 3;
					high_score_5[4] = high_score_5[3];
					high_score_5[3] = score;
				}

				else if (high_score_5[4] < score)
				{
					ranking = 4;
					high_score_5[4] = score;
				}

				else
				{
					ranking = 5; //ランク外
				}

				for (i = 0; i <= 4; i++)
				{
					fprintf(fp_score_5, "%d\n", high_score_5[i]); //スコアファイルを更新
				}

				for (i = 0; i <= 79; i++)
				{
					if (list_most_hiragana[i] > max_most_hiragana)
					{
						max_most_hiragana = list_most_hiragana[i];
						score_most_hiragana = i;
					}
				}

				fclose(fp_score_5); //書き取り用として閉じて，もっかい読み取り用として開く

				std::cout << "<info 044: スコアファイルを「書き取り用として」閉じました>" << std::endl;

				if ((fopen_s(&fp_score_5, "./dat/score_5.dat", "r")) != 0) //スコアファイルを読み込む
				{
					std::cout << "<info 045:スコアファイルを「読み取り用として」開けませんでした>" << std::endl;
					exit(45);
				}

				std::cout << "<info 046:スコアファイルを「読み取り用として」開きました>\n" << std::endl;
			}break;
			}

		}

		else if (mode == 1) //ステージモードのとき（タイムオーバー）
		{
			scene = 6;
		}

	}

	if (lamp_timer_02 <= 0)
	{
		word_hit = false; //正解or間違いの演出のエフェクト終了後，語彙スロット未完成状態に
	}

}

void keyboard(unsigned char key, int x, int y)
{
	int *si = &stage_info[0];
	int *ssc = &stage_slot_constraint[0][0];

	switch (scene)
	{
	case 0:
	{
		switch (key) {

		case 'q': case '\033': game_shutdown(); break;
		case 'g': play_SE(SE_enter); scene = 7;  break; //PRESS GAME START 次の画面へ

		default:
			break;
		}
	}break;

	case 1: //are you ready ? 画面
	{
		switch (key) {

		case '\033': game_shutdown(); break;

		case 'k': { //ゲーム画面へgo
			play_SE(SE_enter);
			if (mode == 0) { game_reset(); scene = 5; } //スコアアタックモード
			else if (mode == 1) { standby_stage(stage_select);  scene = 5; } //ステージクリアモード
		} break;

		default:
			break;
		}
	}break;

	case 2: //ポーズ画面
	{
		switch (key) {
		case 'p': {move_lock = false; camera_x = temp_camera_x; camera_y = temp_camera_y;  scene = 5; } break;//再開 カメラの位置をゲーム中のに戻す
		case 'r': { play_SE(SE_select);  next_scene = 5;  scene = 10; } break;//やりなおす→ほんとうによろしいですか？

		case 's': //ステージ選択に戻る→ほんとうによろしいですか？
		{
			switch (mode)
			{
			case 0: { play_SE(SE_select);  next_scene = 8;  scene = 10; } break; //スコアアタックのときの"ステージ選択に戻る"
			case 1: { play_SE(SE_select);  next_scene = 9;  scene = 10; } break; //ステージクリアモードのときの"ステージ選択に戻る"
			default: break;

			}
		} break;

		case '\033': game_shutdown(); break;
		}
	}break;

	case 3:
	{
		switch (key) {
		case 'l': {scene = 4; } break; //リザルト画面切り替え
		case 'o': {play_SE(SE_enter);  camera_x = 640; camera_y = -544; player->center_x = 0; player->center_y = 0; scene = 5; game_reset(); } break;//リトライ
		case 'p': {play_SE(SE_enter);  camera_x = 640; camera_y = -544; player->center_x = 0; player->center_y = 0; scene = 1; game_reset(); }break;//メニューに戻る
		case '\033':game_shutdown(); break;
		}
	}break;

	case 4:
	{
		switch (key) {
		case 'l': {scene = 3; } break; //リザルト画面切り替え
		case 'o': {play_SE(SE_enter);  camera_x = 640; camera_y = -544; player->center_x = 0; player->center_y = 0; scene = 5; game_reset(); }  break;//リトライ
		case 'p': {play_SE(SE_enter);  camera_x = 640; camera_y = -544; player->center_x = 0; player->center_y = 0; scene = 1; game_reset(); } break;//メニューに戻る

		case '\033': game_shutdown(); break;
		}
	}break;

	case 5: //ゲーム画面
	{
		switch (key) {

		case '\033': game_shutdown(); break;
		case 'a': {onMoveKeyPress_L = true; gun_timer = 0;/*MoveLock_R = true;*/ player->direction = 0; } break; //プレイヤー，銃構えアニメーションを停止して左に移動→キーおしっぱで問題なく動作できるようにkeyboardupfuncで制御してidleで動かす
		case 'd': {onMoveKeyPress_R = true; gun_timer = 0; /*MoveLock_L = true;*/ player->direction = 1; } break;//プレイヤー，銃構えアニメーションを停止して右に移動

		case 'j': //スロットの選択カーソルを左に移動
		{
			if (slot_select > 0 && lamp_timer_01 == 0)
			{
				play_SE(SE_select);  slot_select--;
			}
		} break; 

		case 'l': { //スロットの選択カーソルを→に移動

			if (lamp_timer_01 == 0)
			{
				if (mode == 0 && slot_select < mode_mojisu - 1 || mode == 1 && slot_select < *(si+stage_select) % 10 - 1)
				{
					play_SE(SE_select);  slot_select++;
				}
			}
		} break;

		case 'i': //選択中のスロットの場所をからっぽにする
		{
			if (lamp_timer_02 == 0) //
			{
				if (slot[slot_select] != 0 && *(ssc + stage_select * 5 + slot_select) == 0 || (*(si+stage_select) >= 23 && *(si+stage_select) <= 25))
				{
	
					play_SE(SE_throw);
					score_leave_hiragana++;
					slot[slot_select] = 0;

					if (mode == 1 && *(si+stage_select) >= 23 && *(si+stage_select) <= 25) //A〇A〇のとき
					{
						if (*(ssc + stage_select * 5 + slot_select) != 0) //選択しているスロットがＡとかになってるとき
						{
							for (k = 0; k <= 4; k++)
							{
								if (*(ssc + stage_select * 5 + slot_select) == *(ssc + stage_select * 5 + k)) 
								{
									slot[k] = 0; //スロットを消すときも入れる時と一緒
								}
							}
						}
					}

					
				}

			}
		} break;

		case 'k': //スロットが文字数分満たされたらその単語があるかＧｏ！ってやる
		{
			if (lamp_timer_02 == 0)
			{
				if ((mode == 0 && mode_mojisu == 3 || mode == 1 && *(si+stage_select) % 10 == 3) && slot[0] != 0 && slot[1] != 0 && slot[2] != 0) //3文字モードの時のチェック語彙
				{
					check_goi(slot);
					lamp_timer_02 = 100;
					lamp_timer_01 = 50;

					if (*(si+stage_select) == 13) //しりとりモード
					{
						*(ssc + stage_select * 5) = slot[2];
						slot[0] = slot[2];

						slot[1] = 0;
						slot[2] = 0;
					}

					else if (*(si+stage_select) == 23) //A〇A〇モード
					{
						slot[0] = 0;
						slot[1] = 0;
						slot[2] = 0;
						slot[3] = 0;
						slot[4] = 0;
					}

					else
					{

						if (*(ssc + stage_select * 5 + 0) == 0) { slot[0] = 0; }
						if (*(ssc + stage_select * 5 + 1) == 0) { slot[1] = 0; }
						if (*(ssc + stage_select * 5 + 2) == 0) { slot[2] = 0; }

					}

					slot_select = 0; //スロットの選択位置を左にもどす
				}

				if ((mode == 0 && mode_mojisu == 4 || mode == 1 && *(si+stage_select) % 10 == 4) && slot[0] != 0 && slot[1] != 0 && slot[2] != 0 && slot[3] != 0) //4文字モードの時のチェック語彙
				{
					check_goi(slot);
					lamp_timer_02 = 100;
					lamp_timer_01 = 50;

					if (*(si+stage_select) == 14) //しりとりモード
					{
						*(ssc + stage_select * 5) = slot[3];
						slot[0] = slot[3];

						slot[1] = 0;
						slot[2] = 0;
						slot[3] = 0;
					}

					else if (*(si+stage_select) == 24) //A〇A〇モード
					{
						slot[0] = 0;
						slot[1] = 0;
						slot[2] = 0;
						slot[3] = 0;
						slot[4] = 0;
					}

					else
					{

						if (*(ssc + stage_select * 5 + 0) == 0) { slot[0] = 0; }
						if (*(ssc + stage_select * 5 + 1) == 0) { slot[1] = 0; }
						if (*(ssc + stage_select * 5 + 2) == 0) { slot[2] = 0; }
						if (*(ssc + stage_select * 5 + 3) == 0) { slot[3] = 0; }

					}

					slot_select = 0; //スロットの選択位置を左にもどす
				}

				if ((mode == 0 && mode_mojisu == 5 || mode == 1 && *(si+stage_select) % 10 == 5) && slot[0] != 0 && slot[1] != 0 && slot[2] != 0 && slot[3] != 0 && slot[4] != 0) //5文字モードの時のチェック語彙
				{
					check_goi(slot);
					lamp_timer_02 = 100;
					lamp_timer_01 = 50;

					if (*(si+stage_select) == 15) //しりとりモード
					{
						*(ssc + stage_select * 5) = slot[4];
						slot[0] = slot[4];

						slot[1] = 0;
						slot[2] = 0;
						slot[3] = 0;
						slot[4] = 0;
					}

					else if (*(si+stage_select) == 25) //A〇A〇モード
					{
						slot[0] = 0;
						slot[1] = 0;
						slot[2] = 0;
						slot[3] = 0;
						slot[4] = 0;
					}

					else
					{

						if (*(ssc + stage_select * 5 +0) == 0) { slot[0] = 0; }
						if (*(ssc + stage_select * 5 + 1) == 0) { slot[1] = 0; }
						if (*(ssc + stage_select * 5 + 2) == 0) { slot[2] = 0; }
						if (*(ssc + stage_select * 5 + 3) == 0) { slot[3] = 0; }
						if (*(ssc + stage_select * 5 + 4) == 0) { slot[4] = 0; }

					}

					slot_select = 0; //スロットの選択位置を左にもどす
				}
				
			}
		}  break;//単語チェック

		case 'v': {if (flag_bullet_exist == false) //射撃
		{
			play_SE(SE_shoot);
			bullet->direction = player->direction; bullet_timer = 0; gun_timer = 60; flag_bullet_exist = true;
			if (player->direction == 0) { bullet->center_x = player->center_x + 40; bullet->center_y = player->center_y; }
			else if (player->direction == 1) { bullet->center_x = player->center_x - 40; bullet->center_y = player->center_y; }
		}} break;

		case 'p': {move_lock = true;  scene = 2; temp_camera_x = camera_x; temp_camera_y = camera_y; camera_x = 640; camera_y = -544; } break; //ポーズ カメラの位置をＧＵＩ用の位置にリセット
		//case 't': scene = 6; temp_camera_x = camera_x; temp_camera_y = camera_y; camera_x = 640; camera_y = -544; break; //デバッグ用トリガー1 強制ゲームオーバー
		//case 'b': slot[3] = 19; break; //デバッグ用トリガー2
		//case 'n': slot[0] = 5; slot[1] = 12; slot[2] = 47;  slot[3] = 10; break; //デバッグ用トリガー3（成功時シミュレーション）
		//case 'm': slot[0] = 5; slot[1] = 12; slot[2] = 47;  slot[3] = 6; break; //デバッグ用トリガー4（失敗時シミュレーション）

		case '\040': if (player_jump == false) { play_SE(SE_jump); player_jump = true; flag_collision_D = false; } break;
		default:
			break;
		}
	}break;

	case 6: //ゲームオーバー画面
	{
		switch (key) {
		case 'k':
		{
			play_SE(SE_enter);
			if (mode == 0) { scene = 3; lamp_timer_01 = 0; lamp_timer_02 = 0; }
			else { scene = 9; } //ステージクリアモードのときはステージ選択画面に戻る
		}  break; //リザルト画面へ

		case '\033': game_shutdown(); break;
		}
	}break;

	case 7: //モード選択画面
	{
		switch (key) {
		case 'i': {	play_SE(SE_back);  scene = 0; } break; //タイトル画面に戻る
		case 'k': {	if (mode == 0) { play_SE(SE_enter);  stage_select = 0;  scene = 8; } if (mode == 1) { play_SE(SE_enter);  stage_select = 1;  scene = 9; }} break; //スコアアタックモードはステージ０として扱う
		case 'w': {	play_SE(SE_select); mode = 0; } break;
		case 's': {	play_SE(SE_select); mode = 1; } break;
		case '\033': game_shutdown(); break;
		}
	}break;

	case 8: //スコアアタックモード選択時の次の画面（文字数選択）
	{
		switch (key) {
		case 'i': {	play_SE(SE_back);  scene = 7; } break; //モード選択画面に戻る
		case 'k': { play_SE(SE_enter);	scene = 1; } break;
		case 'a': { if (mode_mojisu >=4) { play_SE(SE_select); mode_mojisu--; }} break; //文字数モード選択切り替え
		case 'd': {	if (mode_mojisu <= 4) { play_SE(SE_select); mode_mojisu++; }} break;
		case '\033': game_shutdown(); break;
		}
	}break;

	case 9: //ステージモード選択時の次の画面　（ステージ選択画面
	{
		switch (key) {
		case 'i': {	play_SE(SE_back);  scene = 7; } break; //モード選択画面に戻る
		case 'k': {if (*(si+stage_select) != 0) { play_SE(SE_enter);  scene = 1; } } break; //ステージが定義されていれば次のシーンへ
		case 'a': { if (stage_select >= 2) { play_SE(SE_select);   stage_select--; }} break; //ステージ番号選択←
		case 'd': {	if (stage_select <= STAGE_AVAILABLE - 1) { play_SE(SE_select); stage_select++; }} break; //ステージ番号選択→
		case 'j': { if (stage_select >= 12) { play_SE(SE_select);  stage_select -= 10; } else { stage_select = 1; }} break;
		case 'l': {	if (stage_select <= STAGE_AVAILABLE - 11) { play_SE(SE_select);  stage_select += 10; } else { stage_select = STAGE_AVAILABLE; }} break;
		case '\033': game_shutdown(); break;
		}
	}break;

	case 10: //ほんとうによろしいですか？ 
	{
		switch (key) {
		case 'y': { //はい
			play_SE(SE_enter);
			move_lock = false;
			camera_x = 640; camera_y = -544; player->center_x = 0; player->center_y = 0;

			if (next_scene == 5) //ゲームをはじめからやりなおすとき，モードに応じてステージを再構築する
			{
				if (mode == 0) { scene = 5; game_reset(); stage_select = 0; }
				else if (mode == 1) { scene = 5; standby_stage(stage_select); }
			}
			scene = next_scene;

		}break;

		case 'n': { //いいえ
			play_SE(SE_back);
			scene = 2;
		}break;

		case '\033': game_shutdown(); break;
		}
	}break;

	case 11:
	{
		switch (key) {
		case 'o': {play_SE(SE_enter);   camera_x = 640; camera_y = -544; player->center_x = 0; player->center_y = 0; scene = 1; } break;//リトライ
		case 'p': {play_SE(SE_enter);   camera_x = 640; camera_y = -544; player->center_x = 0; player->center_y = 0; scene = 9;  }break;//ステージ選択画面に戻る
		case '\033':game_shutdown(); break;
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

	if ((double)h / (double)w <= (double)HEIGHT / (double)WIDTH) //規定のアスペクト比より横長の場合
	{
		glViewport((w - (double)h / 0.5625) / 2, 0, (double)h / 0.5625, h);
	}

	else //規定のアスペクト比より縦長の場合
	{
		glViewport(0, (h - (double)w * 0.5625) / 2, w, (double)w * 0.5625);
	}


	glLoadIdentity();
	glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);

	gluLookAt(camera_x, camera_y, 0, camera_x, camera_y, 1, 0, 1, 0);
	std::cout << "<info 028: 画面のリサイズ>" << std::endl;

}

void Init() {

	int *dic3 = &dic_3moji[0][0];
	int *dic4 = &dic_4moji[0][0];
	int *dic5 = &dic_5moji[0][0];
	int *dics = &dic_sample[0][0];
	const char dics2[40000][25] = {};
	const char *dics2_ = &dics2[0][0];
	int *hw3 = &hiragana_weight_3[0][0];
	int *hw4 = &hiragana_weight_4[0][0];
	int *hw5 = &hiragana_weight_5[0][0];
	float *hs3 = &hiragana_score_3[0][0];
	float *hs4 = &hiragana_score_4[0][0];
	float *hs5 = &hiragana_score_5[0][0];

	int *sc = &stage_clear[0][0];

	// 効果音のロード 
	if (flag_SE == true)
	{
		SE_select = Mix_LoadWAV("./sound/select.wav"); if (SE_select == NULL) { std::cout << "<info 056: 音声ファイルを開けませんでした>" << std::endl; exit(56); }
		SE_back = Mix_LoadWAV("./sound/back.wav"); if (SE_back == NULL) { std::cout << "<info 056: 音声ファイルを開けませんでした>" << std::endl; exit(56); }
		SE_batsu = Mix_LoadWAV("./sound/batsu.wav"); if (SE_batsu == NULL) { std::cout << "<info 056: 音声ファイルを開けませんでした>" << std::endl; exit(56); }
		SE_maru = Mix_LoadWAV("./sound/maru.wav"); if (SE_maru == NULL) { std::cout << "<info 056: 音声ファイルを開けませんでした>" << std::endl; exit(56); }
		SE_enter = Mix_LoadWAV("./sound/enter.wav"); if (SE_enter == NULL) { std::cout << "<info 056: 音声ファイルを開けませんでした>" << std::endl; exit(56); }
		SE_hit = Mix_LoadWAV("./sound/hit.wav"); if (SE_hit == NULL) { std::cout << "<info 056: 音声ファイルを開けませんでした>" << std::endl; exit(56); }
		SE_shoot = Mix_LoadWAV("./sound/shoot.wav"); if (SE_shoot == NULL) { std::cout << "<info 056: 音声ファイルを開けませんでした>" << std::endl; exit(56); }
		SE_jump = Mix_LoadWAV("./sound/jump.wav"); if (SE_jump == NULL) { std::cout << "<info 056: 音声ファイルを開けませんでした>" << std::endl; exit(56); }
		SE_throw = Mix_LoadWAV("./sound/throw.wav"); if (SE_throw == NULL) { std::cout << "<info 056: 音声ファイルを開けませんでした>" << std::endl; exit(56); }
		std::cout << "<info 057: 音声ファイルを読み込みました>" << std::endl;
	}

	if ((fopen_s(&fp_score_3, "./dat/score_3.dat", "r")) != 0) //スコアファイルを読み込む
	{
		std::cout << "<info 015: スコアファイルを開けませんでした>" << std::endl;
		exit(15);
	}

	i = 0;
	while (fscanf_s(fp_score_3, "%d", &high_score_3[i]) != EOF)
	{
		i++;
	}

	std::cout << "<info 016: スコアファイルを読み込みました>" << std::endl;

	if ((fopen_s(&fp_score_4, "./dat/score_4.dat", "r")) != 0) //スコアファイルを読み込む
	{
		std::cout << "<info 015: スコアファイルを開けませんでした>" << std::endl;
		exit(15);
	}

	i = 0;
	while (fscanf_s(fp_score_4, "%d", &high_score_4[i]) != EOF)
	{
		i++;
	}

	std::cout << "<info 016: スコアファイルを読み込みました>" << std::endl;

	if ((fopen_s(&fp_score_5, "./dat/score_5.dat", "r")) != 0) //スコアファイルを読み込む
	{
		std::cout << "<info 015: スコアファイルを開けませんでした>" << std::endl;
		exit(15);
	}

	i = 0;
	while (fscanf_s(fp_score_5, "%d", &high_score_5[i]) != EOF)
	{
		i++;
	}

	std::cout << "<info 016: スコアファイルを読み込みました>" << std::endl;


	if ((fopen_s(&fp_dic_3, "./dat/3moji_dic.dat", "r")) != 0) //辞書ファイルを読み込む
	{
		std::cout << "<info 012: 辞書ファイルを開けませんでした" << std::endl;
		exit(12);
	}

	i = 0;
	while (fscanf_s(fp_dic_3, "%d,%d,%d,", dic3 + i * 3, dic3 + i * 3 + 1, dic3 + i * 3 + 2) != EOF)
	{
		i++;
	}

	dic_3_all = i;

	std::cout << "<info 013: 3文字辞書 " << dic_3_all << "単語を読み込みました>" << std::endl;

	if ((fopen_s(&fp_dic_4, "./dat/4moji_dic.dat", "r")) != 0) //辞書ファイルを読み込む
	{
		std::cout << "<info 012: 辞書ファイルを開けませんでした" << std::endl;
		exit(12);
	}

	i = 0;
	while (fscanf_s(fp_dic_4, "%d,%d,%d,%d,", dic4 + i * 4, dic4 + i * 4 + 1, dic4 + i * 4 + 2, dic4 + i * 4 + 3) != EOF)
	{
		i++;
	}

	dic_4_all = i;

	std::cout << "<info 013: 4文字辞書 " << dic_4_all << "単語を読み込みました>" << std::endl;

	if ((fopen_s(&fp_dic_5, "./dat/5moji_dic.dat", "r")) != 0) //辞書ファイルを読み込む
	{
		std::cout << "<info 012: 辞書ファイルを開けませんでした" << std::endl;
		exit(12);
	}

	i = 0;
	while (fscanf_s(fp_dic_5, "%d,%d,%d,%d,%d,", dic5 + i * 5, dic5 + i * 5 + 1, dic5 + i * 5 + 2, dic5 + i * 5 + 3, dic5 + i * 5 + 4) != EOF)
	{
		i++;
	}

	dic_5_all = i;

	std::cout << "<info 013: 5文字辞書 " << dic_5_all << "単語を読み込みました>" << std::endl;


	if ((fopen_s(&fp_stageclear, "./dat/stage_clear.dat", "r")) != 0) //ステージクリア進捗状況ファイルを読み込んで，ステージ選択モードで「何ステージをどのくらいクリアしているか」の情報を取得する
	{
		std::cout << "<info 032: ステージクリア進捗状況ファイルを開けませんでした>" << std::endl;
		exit(32);
	}

	i = 0;
	while (fscanf_s(fp_stageclear, "%d,%d,%d,%d,%d", sc + i * 5, sc + i * 5 + 1, sc + i * 5 + 2, sc + i * 5 + 3, sc + i * 5 + 4) != EOF)
	{
		i++;
	}

	std::cout << "<info 033: ステージクリア進捗状況(" << i << ")をステージ分を読み込みました>" << std::endl;

	for (i = 0; i <= STAGE_LIMIT; i++)
	{
		//fprintf(fp_stageclear, "%d,%d,%d,%d,%d\n", stage_clear[i][0], stage_clear[i][1], stage_clear[i][2], stage_clear[i][3], stage_clear[i][4]); //ステージクリア進捗状況ファイルを更新
		if (i <= STAGE_AVAILABLE) //ステージのクリア状況の表示は，現在選択できるステージまでの情報のみを参照する（プレイできるステージ以降のステージもテストプレイでクリアデータが入ってる可能性があるため）
		{
			if (stage_clear[i][0] == 1) { stage_medal[0]++; }
			if (stage_clear[i][1] == 1) { stage_medal[1]++; }
			if (stage_clear[i][2] == 1) { stage_medal[2]++; } //メダルを取った数を記録し，「何ステージをどのくらいクリアしているか」の情報を取得する
		}
	}

	if ((fopen_s(&fp_stage_nolma_info, "./dat/stage_nolma_info.dat", "r")) != 0) //ステージのノルマや制限時間などの基本情報を取得する
	{
		std::cout << "<info 050: ステージノルマ情報ファイルを開けませんでした>" << std::endl;
		exit(50);
	}

	i = 0;
	while (fscanf_s(fp_stage_nolma_info, "%d,%d,%d,%d,%d,%d,%d,%d,%d", &stage_info[i], &stage_nolma[i], &stage_time_limit[i], &stage_time_limit_gold[i], &stage_slot_constraint[i][0], &stage_slot_constraint[i][1], &stage_slot_constraint[i][2], &stage_slot_constraint[i][3], &stage_slot_constraint[i][4]) != EOF)
	{
		i++;
	}

	std::cout << "<info 051: ステージノルマ情報ファイルをステージ分を読み込みました>" << std::endl;

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
	GdiplusStartup(&gdiPT, &gdiPSI, NULL);


	LoadImagePNG(L"./pic/num_a0.png", tex_num[0][0]);
	LoadImagePNG(L"./pic/num_a1.png", tex_num[0][1]);
	LoadImagePNG(L"./pic/num_a2.png", tex_num[0][2]);
	LoadImagePNG(L"./pic/num_a3.png", tex_num[0][3]);
	LoadImagePNG(L"./pic/num_a4.png", tex_num[0][4]);
	LoadImagePNG(L"./pic/num_a5.png", tex_num[0][5]);
	LoadImagePNG(L"./pic/num_a6.png", tex_num[0][6]);
	LoadImagePNG(L"./pic/num_a7.png", tex_num[0][7]);
	LoadImagePNG(L"./pic/num_a8.png", tex_num[0][8]);
	LoadImagePNG(L"./pic/num_a9.png", tex_num[0][9]);
	LoadImagePNG(L"./pic/block_null.png", tex_num[0][10]);
	LoadImagePNG(L"./pic/num_b0.png", tex_num[1][0]);
	LoadImagePNG(L"./pic/num_b1.png", tex_num[1][1]);
	LoadImagePNG(L"./pic/num_b2.png", tex_num[1][2]);
	LoadImagePNG(L"./pic/num_b3.png", tex_num[1][3]);
	LoadImagePNG(L"./pic/num_b4.png", tex_num[1][4]);
	LoadImagePNG(L"./pic/num_b5.png", tex_num[1][5]);
	LoadImagePNG(L"./pic/num_b6.png", tex_num[1][6]);
	LoadImagePNG(L"./pic/num_b7.png", tex_num[1][7]);
	LoadImagePNG(L"./pic/num_b8.png", tex_num[1][8]);
	LoadImagePNG(L"./pic/num_b9.png", tex_num[1][9]);
	LoadImagePNG(L"./pic/num_plus.png", tex_num[1][10]);
	LoadImagePNG(L"./pic/num_c0.png", tex_num[2][0]);
	LoadImagePNG(L"./pic/num_c1.png", tex_num[2][1]);
	LoadImagePNG(L"./pic/num_c2.png", tex_num[2][2]);
	LoadImagePNG(L"./pic/num_c3.png", tex_num[2][3]);
	LoadImagePNG(L"./pic/num_c4.png", tex_num[2][4]);
	LoadImagePNG(L"./pic/num_c5.png", tex_num[2][5]);
	LoadImagePNG(L"./pic/num_c6.png", tex_num[2][6]);
	LoadImagePNG(L"./pic/num_c7.png", tex_num[2][7]);
	LoadImagePNG(L"./pic/num_c8.png", tex_num[2][8]);
	LoadImagePNG(L"./pic/num_c9.png", tex_num[2][9]);
	LoadImagePNG(L"./pic/block_null.png", tex_num[2][10]);
	LoadImagePNG(L"./pic/num_d0.png", tex_num[3][0]);
	LoadImagePNG(L"./pic/num_d1.png", tex_num[3][1]);
	LoadImagePNG(L"./pic/num_d2.png", tex_num[3][2]);
	LoadImagePNG(L"./pic/num_d3.png", tex_num[3][3]);
	LoadImagePNG(L"./pic/num_d4.png", tex_num[3][4]);
	LoadImagePNG(L"./pic/num_d5.png", tex_num[3][5]);
	LoadImagePNG(L"./pic/num_d6.png", tex_num[3][6]);
	LoadImagePNG(L"./pic/num_d7.png", tex_num[3][7]);
	LoadImagePNG(L"./pic/num_d8.png", tex_num[3][8]);
	LoadImagePNG(L"./pic/num_d9.png", tex_num[3][9]);
	LoadImagePNG(L"./pic/block_null.png", tex_num[3][10]);
	LoadImagePNG(L"./pic/num_dot.png", tex_num[6][0]); //tex_num[6]は記号群にしたいと思う
	LoadImagePNG(L"./pic/block_null.png", tex_num[6][10]); //tex_num[6]は記号群にしたいと思う [10]はプラス記号なのでえ必要ないフォントは空白にする
	LoadImagePNG(L"./pic/num_hyphen.png", tex_num[6][1]); //[0]小数点 [1]ハイフン

	UI_title.LoadImagePNG2(UI_title.file, UI_title.tex);
	UI_quit.LoadImagePNG2(UI_quit.file, UI_quit.tex);
	UI_gamestart.LoadImagePNG2(UI_gamestart.file, UI_gamestart.tex);
	UI_pressstart.LoadImagePNG2(UI_pressstart.file, UI_pressstart.tex);

	select_highlight.LoadImagePNG2(select_highlight.file, select_highlight.tex);
	UI_pause.LoadImagePNG2(UI_pause.file, UI_pause.tex);
	
	UI_return_0.LoadImagePNG2(UI_return_0.file, UI_return_0.tex);
	UI_return_1.LoadImagePNG2(UI_return_1.file, UI_return_1.tex);
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
	UI_newrecord_stage.LoadImagePNG2(UI_newrecord_stage.file, UI_newrecord_stage.tex);
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
	UI_10_2.LoadImagePNG2(UI_10_2.file, UI_10_2.tex);
	UI_11.LoadImagePNG2(UI_11.file, UI_11.tex);
	UI_12.LoadImagePNG2(UI_12.file, UI_12.tex);
	UI_13.LoadImagePNG2(UI_13.file, UI_13.tex);
	UI_14.LoadImagePNG2(UI_14.file, UI_14.tex);
	UI_15.LoadImagePNG2(UI_15.file, UI_15.tex);
	UI_16.LoadImagePNG2(UI_16.file, UI_16.tex);
	UI_17.LoadImagePNG2(UI_17.file, UI_17.tex);

	UI_slot_base_3.LoadImagePNG2(UI_slot_base_3.file, UI_slot_base_3.tex);
	UI_slot_base_4.LoadImagePNG2(UI_slot_base_4.file, UI_slot_base_4.tex);
	UI_slot_base_5.LoadImagePNG2(UI_slot_base_5.file, UI_slot_base_5.tex);
	UI_slot_same[0].LoadImagePNG2(UI_slot_same[0].file, UI_slot_same[0].tex);
	UI_slot_same[1].LoadImagePNG2(UI_slot_same[1].file, UI_slot_same[1].tex);
	UI_slot_same[2].LoadImagePNG2(UI_slot_same[2].file, UI_slot_same[2].tex);
	slot_same_block[0].LoadImagePNG2(slot_same_block[0].file, slot_same_block[0].tex);
	slot_same_block[1].LoadImagePNG2(slot_same_block[1].file, slot_same_block[1].tex);
	slot_same_block[2].LoadImagePNG2(slot_same_block[2].file, slot_same_block[2].tex);
	slot_same_block_mini[0].LoadImagePNG2(slot_same_block_mini[0].file, slot_same_block_mini[0].tex);
	slot_same_block_mini[1].LoadImagePNG2(slot_same_block_mini[1].file, slot_same_block_mini[1].tex);
	slot_same_block_mini[2].LoadImagePNG2(slot_same_block_mini[2].file, slot_same_block_mini[2].tex);
	UI_slot_highlight.LoadImagePNG2(UI_slot_highlight.file, UI_slot_highlight.tex);
	UI_slot_decision.LoadImagePNG2(UI_slot_decision.file, UI_slot_decision.tex);
	UI_slot_decision_green.LoadImagePNG2(UI_slot_decision_green.file, UI_slot_decision_green.tex);
	player_penalty.LoadImagePNG2(player_penalty.file, player_penalty.tex);
	player_maru.LoadImagePNG2(player_maru.file, player_maru.tex);
	player_batsu.LoadImagePNG2(player_batsu.file, player_batsu.tex);
	player_fukidashi01.LoadImagePNG2(player_fukidashi01.file, player_fukidashi01.tex);
	player_fukidashi02.LoadImagePNG2(player_fukidashi02.file, player_fukidashi02.tex);
	block_light_1.LoadImagePNG2(block_light_1.file, block_light_1.tex);
	block_light_2.LoadImagePNG2(block_light_2.file, block_light_2.tex);
	UI_confirm.LoadImagePNG2(UI_confirm.file, UI_confirm.tex);
	UI_confirm_2.LoadImagePNG2(UI_confirm_2.file, UI_confirm_2.tex);
	UI_mode_score_attack.LoadImagePNG2(UI_mode_score_attack.file, UI_mode_score_attack.tex);
	UI_mode_stage_clear.LoadImagePNG2(UI_mode_stage_clear.file, UI_mode_stage_clear.tex);
	UI_mode_highlight.LoadImagePNG2(UI_mode_highlight.file, UI_mode_highlight.tex);
	UI_mode_description_1.LoadImagePNG2(UI_mode_description_1.file, UI_mode_description_1.tex);
	UI_mode_description_2.LoadImagePNG2(UI_mode_description_2.file, UI_mode_description_2.tex);
	UI_slot_AB_description.LoadImagePNG2(UI_slot_AB_description.file, UI_slot_AB_description.tex);
	UI_mode_select_UI.LoadImagePNG2(UI_mode_select_UI.file, UI_mode_select_UI.tex);
	UI_mode_mojisu.LoadImagePNG2(UI_mode_mojisu.file, UI_mode_mojisu.tex);
	UI_mode_stage.LoadImagePNG2(UI_mode_stage.file, UI_mode_stage.tex);
	UI_mode_3.LoadImagePNG2(UI_mode_3.file, UI_mode_3.tex);
	UI_mode_4.LoadImagePNG2(UI_mode_4.file, UI_mode_4.tex);
	UI_mode_5.LoadImagePNG2(UI_mode_5.file, UI_mode_5.tex);
	UI_mode_highlight.LoadImagePNG2(UI_mode_mojisu_highlight.file, UI_mode_mojisu_highlight.tex);
	UI_mode_select_mojisu_UI.LoadImagePNG2(UI_mode_select_mojisu_UI.file, UI_mode_select_mojisu_UI.tex);


	UI_mode_waku.LoadImagePNG2(UI_mode_waku.file, UI_mode_waku.tex);
	UI_mode_highscore_1.LoadImagePNG2(UI_mode_highscore_1.file, UI_mode_highscore_1.tex);
	UI_mode_highscore_2.LoadImagePNG2(UI_mode_highscore_2.file, UI_mode_highscore_2.tex);
	UI_slot_locked.LoadImagePNG2(UI_slot_locked.file, UI_slot_locked.tex);
	UI_block_stage_num.LoadImagePNG2(UI_block_stage_num.file, UI_block_stage_num.tex);
	UI_block_stage_num_select.LoadImagePNG2(UI_block_stage_num_select.file, UI_block_stage_num_select.tex);
	UI_clear_lamp_on_1.LoadImagePNG2(UI_clear_lamp_on_1.file, UI_clear_lamp_on_1.tex);
	UI_clear_lamp_on_2.LoadImagePNG2(UI_clear_lamp_on_2.file, UI_clear_lamp_on_2.tex);
	UI_clear_lamp_on_3.LoadImagePNG2(UI_clear_lamp_on_3.file, UI_clear_lamp_on_3.tex);
	UI_clear_lamp_on_1_lux.LoadImagePNG2(UI_clear_lamp_on_1_lux.file, UI_clear_lamp_on_1_lux.tex);
	UI_clear_lamp_on_2_lux.LoadImagePNG2(UI_clear_lamp_on_2_lux.file, UI_clear_lamp_on_2_lux.tex);
	UI_clear_lamp_on_3_lux.LoadImagePNG2(UI_clear_lamp_on_3_lux.file, UI_clear_lamp_on_3_lux.tex);
	UI_clear_lamp_off.LoadImagePNG2(UI_clear_lamp_off.file, UI_clear_lamp_off.tex);
	UI_time_limit_description.LoadImagePNG2(UI_time_limit_description.file, UI_time_limit_description.tex);
	UI_coming_soon192.LoadImagePNG2(UI_coming_soon192.file, UI_coming_soon192.tex);
	UI_coming_soon64.LoadImagePNG2(UI_coming_soon64.file, UI_coming_soon64.tex);
	UI_slot_constraint_description.LoadImagePNG2(UI_slot_constraint_description.file, UI_slot_constraint_description.tex);
	UI_mission_description_0.LoadImagePNG2(UI_mission_description_0.file, UI_mission_description_0.tex);
	UI_mission_description_1.LoadImagePNG2(UI_mission_description_1.file, UI_mission_description_1.tex);
	UI_mission_description_2.LoadImagePNG2(UI_mission_description_2.file, UI_mission_description_2.tex);
	UI_num_aslash.LoadImagePNG2(UI_num_aslash.file, UI_num_aslash.tex);
	UI_num_aslash_big.LoadImagePNG2(UI_num_aslash_big.file, UI_num_aslash_big.tex);
	UI_result_stage.LoadImagePNG2(UI_result_stage.file, UI_result_stage.tex);
	UI_result_stage_title.LoadImagePNG2(UI_result_stage_title.file, UI_result_stage_title.tex);
	UI_result_stage_title.LoadImagePNG2(UI_result_stage_title.file, UI_result_stage_title.tex);
	UI_result_stage_medal_1.LoadImagePNG2(UI_result_stage_medal_1.file, UI_result_stage_medal_1.tex);
	UI_result_stage_medal_2.LoadImagePNG2(UI_result_stage_medal_2.file, UI_result_stage_medal_2.tex);
	UI_result_stage_medal_3.LoadImagePNG2(UI_result_stage_medal_3.file, UI_result_stage_medal_3.tex);
	UI_result_stage_medal_1_lux.LoadImagePNG2(UI_result_stage_medal_1_lux.file, UI_result_stage_medal_1_lux.tex);
	UI_result_stage_medal_2_lux.LoadImagePNG2(UI_result_stage_medal_2_lux.file, UI_result_stage_medal_2_lux.tex);
	UI_result_stage_medal_3_lux.LoadImagePNG2(UI_result_stage_medal_3_lux.file, UI_result_stage_medal_3_lux.tex);
	UI_arrow_2L.LoadImagePNG2(UI_arrow_2L.file, UI_arrow_2L.tex);
	UI_arrow_2R.LoadImagePNG2(UI_arrow_2R.file, UI_arrow_2R.tex);
	UI_arrow_2D.LoadImagePNG2(UI_arrow_2D.file, UI_arrow_2D.tex);



	for (i = 0; i <= 79; i++)
	{
		block_hiragana_UI[i].LoadImagePNG2(block_hiragana_UI[i].file, block_hiragana_UI[i].tex);
		block_hiragana[i].LoadImagePNG2(block_hiragana[i].file, block_hiragana[i].tex);
		block_hiragana_mini[i].LoadImagePNG2(block_hiragana_mini[i].file, block_hiragana_mini[i].tex);
	}

	//コピー用
	//.LoadImagePNG2(.file, .tex);

	player = new AnimationChara(0.0, 0.0, 64.0, 64.0, L"./pic/player_walk1.png"); //ゲーム上で移動するオブジェクト
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


	std::cout << "<info 022: 画像を読み込みました>" << std::endl;

	scene = 0;


	i = 0;

	for (k = 0; k < dic_3_all; k++)
	{
		for (j = 0; j <= 2; j++)
		{
			(*(hw3 + (*(dic3 + k * 3 + j)) * 3 + j))++; //ひらがなの登場回数を計算して格納する
		}
	}


	for (k = 0; k < dic_4_all; k++)
	{
		for (j = 0; j <= 3; j++)
		{
			(*(hw4 + (*(dic4 + k * 4 + j)) * 4 + j))++; //ひらがなの登場回数を計算して格納する
		}
	}


	for (k = 0; k < dic_5_all; k++)
	{
		for (j = 0; j <= 4; j++)
		{
			(*(hw5 + (*(dic5 + k * 5 + j)) * 5 + j))++; //ひらがなの登場回数を計算して格納する
		}
	}

	for (k = 0; k < 80; k++) //何文字目の何のひらがなであれば何点，という点数を付与する
	{
		for (j = 0; j <= 2; j++)
		{
			if (*(hw3 + 3 * k + j) >= 2000) { *(hs3 + 3 * k + j) = 2.5; }
			else if (*(hw3 + 3 * k + j) >= 1600) { *(hs3 + 3 * k + j) = 3; }
			else if (*(hw3 + 3 * k + j) >= 1300) { *(hs3 + 3 * k + j) = 4.5; }
			else if (*(hw3 + 3 * k + j) >= 1000) { *(hs3 + 3 * k + j) = 5; }
			else if (*(hw3 + 3 * k + j) >= 750) { *(hs3 + 3 * k + j) = 7.5; }
			else if (*(hw3 + 3 * k + j) >= 500) { *(hs3 + 3 * k + j) = 10; }
			else if (*(hw3 + 3 * k + j) >= 300) { *(hs3 + 3 * k + j) = 12; }
			else if (*(hw3 + 3 * k + j) >= 150) { *(hs3 + 3 * k + j) = 18; }
			else if (*(hw3 + 3 * k + j) >= 75) { *(hs3 + 3 * k + j) = 30; }
			else if (*(hw3 + 3 * k + j) >= 0) { *(hs3 + 3 * k + j) = 45; }
		}

		for (j = 0; j <= 3; j++)
		{
			if (*(hw4 + 4 * k + j) >= 2000) { *(hs4 + 4 * k + j) = 2.5; }
			else if (*(hw4 + 4 * k + j) >= 1600) { *(hs4 + 4 * k + j) = 3; }
			else if (*(hw4 + 4 * k + j) >= 1300) { *(hs4 + 4 * k + j) = 5; }
			else if (*(hw4 + 4 * k + j) >= 1000) { *(hs4 + 4 * k + j) = 7.5; }
			else if (*(hw4 + 4 * k + j) >= 750) { *(hs4 + 4 * k + j) = 10; }
			else if (*(hw4 + 4 * k + j) >= 500) { *(hs4 + 4 * k + j) = 15; }
			else if (*(hw4 + 4 * k + j) >= 300) { *(hs4 + 4 * k + j) = 20; }
			else if (*(hw4 + 4 * k + j) >= 150) { *(hs4 + 4 * k + j) = 30; }
			else if (*(hw4 + 4 * k + j) >= 75) { *(hs4 + 4 * k + j) = 50; }
			else if (*(hw4 + 4 * k + j) >= 0) { *(hs4 + 4 * k + j) = 75; }
		}

		for (j = 0; j <= 4; j++)
		{
			if (*(hw5 + 5 * k + j) >= 2000) { *(hs5 + 5 * k + j) = 2.5; }
			else if (*(hw5 + 5 * k + j) >= 1600) { *(hs5 + 5 * k + j) = 4; }
			else if (*(hw5 + 5 * k + j) >= 1300) { *(hs5 + 5 * k + j) = 6; }
			else if (*(hw5 + 5 * k + j) >= 1000) { *(hs5 + 5 * k + j) = 10; }
			else if (*(hw5 + 5 * k + j) >= 750) { *(hs5 + 5 * k + j) = 12.5; }
			else if (*(hw5 + 5 * k + j) >= 500) { *(hs5 + 5 * k + j) = 20; }
			else if (*(hw5 + 5 * k + j) >= 300) { *(hs5 + 5 * k + j) = 30; }
			else if (*(hw5 + 5 * k + j) >= 150) { *(hs5 + 5 * k + j) = 50; }
			else if (*(hw5 + 5 * k + j) >= 75) { *(hs5 + 5 * k + j) = 75; }
			else if (*(hw5 + 5 * k + j) >= 0) { *(hs5 + 5 * k + j) = 100; }
		}

	}
	std::cout << "<info 023: ひらがなの配点計算が完了しました>" << std::endl;

}


void timer(int value) {

	glutPostRedisplay();
	glutTimerFunc(16, timer, 0); //だいたい60fpsを目指す

	flag_move_x = true; //タイマー関数が呼び出されたのでプレイヤーや弾丸の移動を許可する
	flag_move_y = true;
	flag_move_bullet = true;

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
			lamp_timer_block = 0;
		}

		hiragana_roulette_timer++;
		if (hiragana_roulette_timer > 74 * 60) //ルーレットブロックの中身の遷移のタイマー
		{
			hiragana_roulette_timer = 0;
		}

		if (player_jump == true) //プレイヤーが宙に浮いてる状態で放物運動の経過時間を経過させる
		{
			jump_timer++;
		}
	}

	if (scene == 6) //タイムアップになりゲームオーバーを感知したらカメラの位置をデフォルトに戻す
	{
		temp_camera_x = camera_x; temp_camera_y = camera_y; camera_x = 640; camera_y = -544;
	}

	if (scene == 9 || scene == 11) //クリアメダル全点灯のゴージャスな演出
	{
		lamp_timer_clear++;
		if (lamp_timer_clear > 200) //ルーレットブロックのランプの点灯エフェクト
		{
			lamp_timer_clear = 0;
		}
	}

	if (onMoveKeyPress_L == true || onMoveKeyPress_R == true) //歩きアニメーションのため （画像１→２→３→２というふうに歩き中には４枚の画像を連続で表示する）
	{
		walk_timer++;

		if (speed <= 8) //キャラクターのスピードを加速する
		{
			speed += 1;
		}
	}

	else { speed = 0; walk_timer = 0; } //歩行が止まるとスピードとアニメーションをリセットする
}


int main(int argc, char *argv[])
{

	if (SDL_Init(SDL_INIT_AUDIO) < 0) //SDLのセットアップ
	{
		std::cout << "<info 054: SDLのセットアップに失敗しました>" << std::endl;
		exit(54);
	}

	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 512) == -1) //SDL_mixerのセットアップ
	{
		std::cout << "<info 055: SDL_mixerのセットアップに失敗しました．スピーカーの設定が施されていない場合があります．>" << std::endl;
		flag_SE = false; //以後の効果音をすべて鳴らさないようにする
	}

	atexit(end);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow("goipachi ver.1.5.1");
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