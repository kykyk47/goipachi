//ゲーム風なエディタ画面を目指す

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

using namespace Gdiplus;

#define WIDTH 1280 //デフォルトの画面のサイズ
#define HEIGHT 720

#define OBJECT_LIMIT 10000 //ブロックの制限
#define STAGE_LIMIT 518 //ステージモードの上限

bool flag_delete = false; //選択中のブロックが消された場合，その後のブロックの情報の移動をさせるトリガー
int timer_cursor_lux = 0; //選択中のカーソルの点滅

int stage_select_edit = 0; //編集中ステージの番号
int stage_select_confirm = 1; //インポートorエクスポートするステージの番号

int stage_clear[STAGE_LIMIT + 1][5] = { {} }; //ステージクリアモードの進捗状況  0~2 メダル 3:クリア時間ハイスコア 4:ミス回数ハイスコア
int stage_info[STAGE_LIMIT + 1] = {}; //0:未定義（カミングスーン) 1:何単語作るかミッション それ以外：面白そうなのがあったら追加
int stage_nolma[STAGE_LIMIT + 1] = {}; //例：ステージ１で10単語作れミッション→ [1]=10
int stage_time_limit[STAGE_LIMIT + 1] = {}; //例：ステージ1は200秒以内 [1]=200
int stage_time_limit_gold[STAGE_LIMIT + 1] = {}; //例：ステージ1のメダル獲得タイム50秒以内→[1]=50
int stage_slot_constraint[STAGE_LIMIT + 1][5] = { {} }; //語彙スロット固定（例：ステージ1で[][][い][ろ]のときは [1][0]～[1][3]：＿＿いろになる
int stage_block_info[OBJECT_LIMIT][3] = { {} }; //ステージモードの時に配置されるブロックの情報

int stage_info_edit = 3; //編集中のステージの情報
int stage_nolma_edit = 0;
int stage_time_limit_edit = 0;
int stage_time_limit_gold_edit = 0;
int stage_slot_constraint_edit[5] = {};

int mode_edit = 1; //0=上半分カーソル選択，1=下半分カーソル選択

int select_hiragana = 1; //下画面で選択しているひらがな


int displayGUI = 0; //1はステージインポート画面 //2はステージエクスポート画面

class CURSOR
{
public:
	int x = 0;
	int y = 0;

};

int object_block[OBJECT_LIMIT][3] = { {} }; //衝突判定に使う，プレイヤーとの距離の条件を満たすためにこの配列にオブジェクトの情報を記載（ブロックの種類,中心座標（ｘとｙ））

GdiplusStartupInput gdiPSI;
ULONG_PTR gdiPT;


GLuint tex_num[7][11] = { {} }; //[0][]：数字フォント（赤） //[1][]：数字フォント（頭上のスコアの数字・黄緑） [10]は「＋」

FILE *fp_stage_structure_info; //ステージの情報
FILE *fp_stage_nolma_info;

int i, j, k;
int object_on_stage = 0;

double camera_x = 0; //カメラの位置
double camera_y = 0;


class GameObject 
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

GameObject block_hiragana_mini[80] = {
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

GameObject UI_01 = GameObject(0, 0, 384, 192, L"./pic/UI_STAGE_INFO.png");
GameObject UI_02 = GameObject(0, 0, 384, 192, L"./pic/UI_file_io.png");
GameObject UI_confirm01 = GameObject(0, 0, 768, 384, L"./pic/import_confirm.png");
GameObject UI_confirm02 = GameObject(0, 0, 768, 384, L"./pic/export_confirm.png");
GameObject UI_confirm03 = GameObject(0, 0, 768, 384, L"./pic/renew_confirm.png");
GameObject UI_start= GameObject(0, 0, 64, 64, L"./pic/player_walk2.png");
GameObject UI_cursor_A = GameObject(0, 0, 64, 64, L"./pic/block_select.png"); //エディタ上画面でのブロック選択
GameObject UI_cursor_Bm = GameObject(0, 0, 200, 50, L"./pic/select_koumooku.png"); //エディタ下画面でのメニュー選択
GameObject UI_cursor_Bb = GameObject(0, 0, 34, 34, L"./pic/block_select.png"); //エディタ下画面でのブロック選択

CURSOR cursorA; //上半分（ステージ部分のカーソル
CURSOR cursorB; //下半分（メニュー部分のカーソル



void end() {
	GdiplusShutdown(gdiPT);
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

	if (num >= 100000) //10万の位
	{
		glBindTexture(GL_TEXTURE_2D, tex_num[font][num / 100000]);

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
		glBindTexture(GL_TEXTURE_2D, tex_num[font][(num / 10000) % 10]);

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
		glBindTexture(GL_TEXTURE_2D, tex_num[font][(num / 1000) % 10]);

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
		glBindTexture(GL_TEXTURE_2D, tex_num[font][(num / 100) % 10]);

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
		glBindTexture(GL_TEXTURE_2D, tex_num[font][(num / 10) % 10]);

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

	glBindTexture(GL_TEXTURE_2D, tex_num[font][num % 10]);

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
		glBindTexture(GL_TEXTURE_2D, tex_num[font][10]);
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
		glBindTexture(GL_TEXTURE_2D, tex_num[font][10]);
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
		glBindTexture(GL_TEXTURE_2D, tex_num[font][10]);
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
		glBindTexture(GL_TEXTURE_2D, tex_num[font][10]);
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
		glBindTexture(GL_TEXTURE_2D, tex_num[font][10]);
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
		glBindTexture(GL_TEXTURE_2D, tex_num[font][10]);
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


void new_stage() //新規ステージ作成
{
	for (i = 0; i <= OBJECT_LIMIT; i++) //ファイルの情報をobject_blockに書き出していく
	{
		object_block[i][0] = 0;
		object_block[i][1] =0;
		object_block[i][2] = 0;
	}

	stage_info_edit = 0;
	stage_nolma_edit = 0;
	stage_time_limit_edit = 0;
	stage_time_limit_gold_edit = 0;
	stage_slot_constraint_edit[0] = 0;
	stage_slot_constraint_edit[1] = 0;
	stage_slot_constraint_edit[2] = 0;
	stage_slot_constraint_edit[3] = 0;
	stage_slot_constraint_edit[4] = 0;

	displayGUI = 0;
	stage_select_edit = 0;
}

void import_stage(int stage_num) //ステージファイルを読み込む
{
	object_on_stage = 0;

	char file_path[32]; //ステージファイルを読み込むときのステージ名


	fclose(fp_stage_nolma_info);
	std::cout << "<info 094: ステージノルマファイルを「読み込み用として」閉じました>" << std::endl;


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


	sprintf_s(file_path, "./dat/stage_%03d.dat", stage_num); //ステージファイルを読み込む
	if ((fopen_s(&fp_stage_structure_info, file_path, "r")) != 0)
	{
		printf("%s\n", file_path);
		std::cout << "<info 088: ステージファイルを開けませんでした" << std::endl;	exit(88);

	}

	i = 0;

	while (fscanf_s(fp_stage_structure_info, "%d,%d,%d", &stage_block_info[i][0], &stage_block_info[i][1], &stage_block_info[i][2]) != EOF)
	{
		i++;
	}

	object_on_stage = i;

	std::cout << "<info 089: ステージファイルを読み込みました>" << std::endl;


	for (i = 0; i <= object_on_stage; i++) //ファイルの情報をobject_blockに書き出していく
	{
		object_block[i][0] = stage_block_info[i][0];
		object_block[i][1] = stage_block_info[i][1] * (-64);
		object_block[i][2] = stage_block_info[i][2] * (-64);
	}

	stage_info_edit = stage_info[stage_num];
	stage_nolma_edit = stage_nolma[stage_num];
	stage_time_limit_edit = stage_time_limit[stage_num];
	stage_time_limit_gold_edit = stage_time_limit_gold[stage_num];
	stage_slot_constraint_edit[0] = stage_slot_constraint[stage_num][0];
	stage_slot_constraint_edit[1] = stage_slot_constraint[stage_num][1];
	stage_slot_constraint_edit[2] = stage_slot_constraint[stage_num][2];
	stage_slot_constraint_edit[3] = stage_slot_constraint[stage_num][3];
	stage_slot_constraint_edit[4] = stage_slot_constraint[stage_num][4];


	fclose(fp_stage_structure_info);
	std::cout << "<info 090: ステージファイルを閉じました>" << std::endl;

	displayGUI = 0;
	stage_select_edit = stage_num;
}

void export_stage(int stage_num) //ステージファイルを書き出す
{
	char file_path[32]; //ステージファイルを読み込むときのステージ名


	stage_info[stage_num] = stage_info_edit;
	stage_nolma[stage_num] = stage_nolma_edit;
	stage_time_limit[stage_num] = stage_time_limit_edit;
	stage_time_limit_gold[stage_num] = stage_time_limit_gold_edit;
	stage_slot_constraint[stage_num][0] = stage_slot_constraint_edit[0];
	stage_slot_constraint[stage_num][1] = stage_slot_constraint_edit[1];
	stage_slot_constraint[stage_num][2] = stage_slot_constraint_edit[2];
	stage_slot_constraint[stage_num][3] = stage_slot_constraint_edit[3];
	stage_slot_constraint[stage_num][4] = stage_slot_constraint_edit[4];

	sprintf_s(file_path, "./dat/stage_%03d.dat", stage_num); //ステージファイルを読み込む
	if ((fopen_s(&fp_stage_structure_info, file_path, "w")) != 0)
	{
		printf("%s\n", file_path);
		std::cout << "<info 091: ステージファイルを開けませんでした" << std::endl;	exit(91);

	}

	for (i = 0; i < object_on_stage; i++)
	{
		fprintf(fp_stage_structure_info, "%d,%d,%d\n", object_block[i][0], (object_block[i][1])/(-64), object_block[i][2]/(-64));
	}

	fclose(fp_stage_structure_info);
	std::cout << "<info 092: ステージファイルを閉じました>" << std::endl;


	fclose(fp_stage_nolma_info);
	std::cout << "<info 094: ステージノルマファイルを「読み込み用として」閉じました>" << std::endl;

	if ((fopen_s(&fp_stage_structure_info, "./dat/stage_nolma_info.dat", "w")) != 0)
	{
		std::cout << "<info 095: ステージファイルを開けませんでした" << std::endl;	exit(95);
	}


	for (i = 0; i < STAGE_LIMIT; i++)
	{
		fprintf(fp_stage_nolma_info, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n", stage_info[i], stage_nolma[i], stage_time_limit[i], stage_time_limit_gold[i], stage_slot_constraint[i][0], stage_slot_constraint[i][1], stage_slot_constraint[i][2], stage_slot_constraint[i][3], stage_slot_constraint[i][4]);
	}

	fclose(fp_stage_nolma_info);
	std::cout << "<info 096: ステージノルマファイルを「書き込み用として」閉じました>" << std::endl;


	if ((fopen_s(&fp_stage_nolma_info, "./dat/stage_nolma_info.dat", "r")) != 0) //ステージのノルマや制限時間などの基本情報を取得する
	{
		std::cout << "<info 097: ステージノルマ情報ファイルを開けませんでした>" << std::endl;
		exit(97);
	}

	displayGUI = 0;
	stage_select_edit = stage_num;
}
void shutdown(void)
{
	fclose(fp_stage_nolma_info); std::cout << "<info 098: ステージノルマファイルを閉じました>" << std::endl;
	exit(98);
}

int def_hiragana(int x, int y) //cursorBの値によって上画面で設置するひらがなを選ぶ
{
	if (x == 0 && y == 0) { return 1; }
	if (x == 0 && y == 1) { return 2; }
	if (x == 0 && y == 2) { return 3; }
	if (x == 0 && y == 3) { return 4; }
	if (x == 0 && y == 4) { return 5; }
	if (x == 1 && y == 0) { return 6; }
	if (x == 1 && y == 1) { return 7; }
	if (x == 1 && y == 2) { return 8; }
	if (x == 1 && y == 3) { return 9; }
	if (x == 1 && y == 4) { return 10; }
	if (x == 2 && y == 0) { return 11; }
	if (x == 2 && y == 1) { return 12; }
	if (x == 2 && y == 2) { return 13; }
	if (x == 2 && y == 3) { return 14; }
	if (x == 2 && y == 4) { return 15; }
	if (x == 3 && y == 0) { return 16; }
	if (x == 3 && y == 1) { return 17; }
	if (x == 3 && y == 2) { return 18; }
	if (x == 3 && y == 3) { return 19; }
	if (x == 3 && y == 4) { return 20; }
	if (x == 4 && y == 0) { return 21; }
	if (x == 4 && y == 1) { return 22; }
	if (x == 4 && y == 2) { return 23; }
	if (x == 4 && y == 3) { return 24; }
	if (x == 4 && y == 4) { return 25; }
	if (x == 5 && y == 0) { return 26; }
	if (x == 5 && y == 1) { return 27; }
	if (x == 5 && y == 2) { return 28; }
	if (x == 5 && y == 3) { return 29; }
	if (x == 5 && y == 4) { return 30; }
	if (x == 6 && y == 0) { return 31; }
	if (x == 6 && y == 1) { return 32; }
	if (x == 6 && y == 2) { return 33; }
	if (x == 6 && y == 3) { return 34; }
	if (x == 6 && y == 4) { return 35; }
	if (x == 7 && y == 0) { return 36; }
	if (x == 7 && y == 2) { return 37; }
	if (x == 7 && y == 4) { return 38; }
	if (x == 8 && y == 0) { return 39; }
	if (x == 8 && y == 1) { return 40; }
	if (x == 8 && y == 2) { return 41; }
	if (x == 8 && y == 3) { return 42; }
	if (x == 8 && y == 4) { return 43; }
	if (x == 9 && y == 0) { return 44; }
	if (x == 9 && y == 1) { return 45; }
	if (x == 9 && y == 2) { return 46; }
	if (x == 9 && y == 3) { return 47; }
	if (x == 9 && y == 4) { return 50; }
	if (x == 10 && y == 0) { return 79; }
	if (x == 10 && y == 2) { return 48; }
	if (x == 10 && y == 3) { return 49; }
	if (x == 10 && y == 4) { return 76; }
	if (x == 11 && y == 0) { return 51; }
	if (x == 11 && y == 1) { return 52; }
	if (x == 11 && y == 2) { return 53; }
	if (x == 11 && y == 3) { return 54; }
	if (x == 11 && y == 4) { return 55; }
	if (x == 12 && y == 0) { return 56; }
	if (x == 12 && y == 1) { return 57; }
	if (x == 12 && y == 2) { return 58; }
	if (x == 12 && y == 3) { return 59; }
	if (x == 12 && y == 4) { return 60; }
	if (x == 13 && y == 0) { return 61; }
	if (x == 13 && y == 1) { return 62; }
	if (x == 13 && y == 2) { return 63; }
	if (x == 13 && y == 3) { return 64; }
	if (x == 13 && y == 4) { return 65; }
	if (x == 14 && y == 0) { return 66; }
	if (x == 14 && y == 1) { return 67; }
	if (x == 14 && y == 2) { return 68; }
	if (x == 14 && y == 3) { return 69; }
	if (x == 14 && y == 4) { return 70; }
	if (x == 15 && y == 0) { return 71; }
	if (x == 15 && y == 1) { return 72; }
	if (x == 15 && y == 2) { return 73; }
	if (x == 15 && y == 3) { return 74; }
	if (x == 15 && y == 4) { return 75; }
	else return 0;
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
	gluLookAt(camera_x, camera_y+128, 0, camera_x, camera_y+128, 1, 0, 1, 0);

	int *obbl = &object_block[0][0];
	GameObject *blhr = &block_hiragana[0];
	GameObject *blmn = &block_hiragana_mini[0];

	UI_02.SetImage(-400-640 + camera_x,194);
	UI_01.SetImage(400-640 + camera_x,194);


	for (i = 0; i < object_on_stage; i++) //オブジェクトとなるブロックここで全部描画
	{
		if (*(obbl + i * 3) != 0)
		{
			(*(blhr + *(obbl + i * 3))).SetImage(double(*(obbl + i * 3 + 1) - 640), double(*(obbl + i * 3 + 2)));
		}
	}
	UI_start.SetImage(0, 0);

	if (stage_select_edit != 0)
	{
		SetNumImage(-320 + camera_x, 110, 192, 24, stage_select_edit, 0, 4); //ステージ番号
	}

	SetNumImage(-320 + camera_x, 136, 192, 24, stage_info_edit, 0, 4); //みしょん情報の番号
	SetNumImage(-320 + camera_x, 166, 192, 24, stage_nolma_edit, 0, 4); //ノルマ数の情報
	SetNumImage(-320 + camera_x, 196, 192, 24, stage_time_limit_edit, 0, 4); //制限時間
	SetNumImage(-320 + camera_x, 226, 192, 24, stage_time_limit_gold_edit, 0, 4); //勲章の制限時間


	block_hiragana_mini[stage_slot_constraint_edit[0]].SetImage(-930 + camera_x + 34 * 20, 266); //固定スロット情報
	block_hiragana_mini[stage_slot_constraint_edit[1]].SetImage(-930 + camera_x + 34 * 19, 266);
	block_hiragana_mini[stage_slot_constraint_edit[2]].SetImage(-930 + camera_x + 34 * 18, 266);
	block_hiragana_mini[stage_slot_constraint_edit[3]].SetImage(-930 + camera_x + 34 * 17, 266);
	block_hiragana_mini[stage_slot_constraint_edit[4]].SetImage(-930 + camera_x + 34 * 16, 266);

	for (k = 0; k <= 6; k++) //メニュー あ～も
	{
		for (i = 0; i <= 4; i++)
		{
			block_hiragana_mini[k*5+i+1].SetImage(-960+34*k + camera_x , 120+34*i);
		}
	}

	for (i = 0; i <= 2; i++) //やゆよ
	{
		block_hiragana_mini[36+i].SetImage(-960 + 34 * 7 + camera_x, 120 + 68 * i);
	}

	for (i = 0; i <= 4; i++) //らりるれろ
	{
		block_hiragana_mini[39+i].SetImage(-960 + 34 * 8 + camera_x, 120 + 34 * i);
	}
	for (i = 0; i <= 3; i++) //わをんっー
	{
		block_hiragana_mini[44 + i].SetImage(-960 + 34 * 9 + camera_x, 120 +34* i);
	}
	block_hiragana_mini[50].SetImage(-960 + 34 * 9+ camera_x , 120 + 34 * 4);

	block_hiragana_mini[79].SetImage(-960 + 34 * 10 + camera_x, 120 + 34 * 0); //ヴ
	block_hiragana_mini[48].SetImage(-960 + 34 * 10 + camera_x, 120 + 34 * 2); //ルーレット
	block_hiragana_mini[49].SetImage(-960 + 34 * 10 + camera_x, 120 + 34 * 3); //木箱
	block_hiragana_mini[76].SetImage(-960 + 34 * 10 + camera_x, 120 + 34 * 4); //地面


	for (k = 10; k <= 14; k++) //濁点
	{
		for (i = 0; i <= 4; i++)
		{
			block_hiragana_mini[k * 5 + i + 1].SetImage(-960 + 34 * (k+1) + camera_x , 120 + 34 * i);
		}
	}

	if (timer_cursor_lux % 10 >=2 && mode_edit == 0)
	{
		UI_cursor_A.SetImage(cursorA.x * (-64), cursorA.y * (-64)); //ステージ編集（上画面）でのカーソル描画(点滅）
	}

	if (mode_edit == 1)
	{
		UI_cursor_A.SetImage(cursorA.x * (-64), cursorA.y * (-64)); //ステージ編集（上画面）でのカーソル描画
	}

	if (timer_cursor_lux % 10 >= 2 && mode_edit == 1 && cursorB.x >= 0 && cursorB.x <= 15 && cursorB.y >= 0 && cursorB.y <= 4) //下画面の設置したいひらがなのカーソル描画（点滅）
	{
		UI_cursor_Bb.SetImage(-960 + cursorB.x * (34) + camera_x, 120+cursorB.y * (34));
	}

	if (timer_cursor_lux % 10 >= 2 && mode_edit == 1 && cursorB.x >=16 && cursorB.y >=0 && cursorB.y <= 3) //下画面でのステージ情報編集のカーソル描画（固定スロット情報以外）点滅）
	{
		UI_cursor_Bm.SetImage(-280 + camera_x, 148+cursorB.y * 30);
	}

	if (timer_cursor_lux % 10 >= 2 && mode_edit == 1 && cursorB.x >=17 || timer_cursor_lux % 10 >= 2 && mode_edit == 1 && cursorB.x == 16 && cursorB.y == 4) //下画面でのステージ情報編集のカーソル描画（固定スロット情報）点滅）
	{
		UI_cursor_Bb.SetImage(-930 + camera_x + 34 * cursorB.x, 266);
	}

	if (mode_edit == 0 && cursorB.x >= 0 && cursorB.x <= 15 && cursorB.y >= 0 && cursorB.y <= 4) //下画面の設置したいひらがなのカーソル描画
	{
		UI_cursor_Bb.SetImage(-960 + cursorB.x * (34) + camera_x, 120 + cursorB.y * (34));
	}

	if ( mode_edit == 0 && cursorB.x >= 16 && cursorB.y >= 0 && cursorB.y <= 3) //下画面でのステージ情報編集のカーソル描画（固定スロット情報以外）
	{
		UI_cursor_Bm.SetImage(-280 + camera_x, 148 + cursorB.y * 30);

	}

	if (mode_edit == 0 && cursorB.x <= 20 && cursorB.x >= 17 || mode_edit == 0 && cursorB.x == 16 && cursorB.y == 4) //下画面でのステージ情報編集のカーソル描画（固定スロット情報）
	{
		UI_cursor_Bb.SetImage(-930 + camera_x + 34 * cursorB.x, 266);
	}

	if (displayGUI == 1) //ステージを読み込むのを確認する画面
	{
		UI_confirm01.SetImage(camera_x-640, -72);
		SetNumImage(-232 + camera_x - 640,-216,640,80,stage_select_confirm,2,-24);
	}

	if (displayGUI == 2) //ステージをデータに書き込むのを確認する画面
	{
		UI_confirm02.SetImage(camera_x - 640, -72);
		SetNumImage(-232 + camera_x - 640, -216, 640, 80, stage_select_confirm, 2, -24);
	}

	if (displayGUI == 3) //ステージを白紙にする確認の画面
	{
		UI_confirm03.SetImage(camera_x - 640, -72);
	}


	glutSwapBuffers();
}

void idle(void)
{

}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q': case '\033': shutdown(); break;
	}

	if (displayGUI == 0)
	{
		if (mode_edit == 0)
		{
			switch (key) {

			case 'a':
			{
				if (cursorA.x >= -13) { cursorA.x--; camera_x += 64; } //上半分で変更するブロックを選択
			}break;

			case 'd':
			{
				if (cursorA.x <= 3000) { cursorA.x++; camera_x -= 64; }
			}break;

			case 'w':
			{
				if (cursorA.y <= 12) { cursorA.y++; }
			}break;

			case 's':
			{
				if (cursorA.y >= 0) { cursorA.y--;  }
			}break;

			case 'k': //下画面で選択したブロックを配置
			{
				std::cout << "<info 100: ブロックID:<<" << object_on_stage << " として追加しました" << std::endl;
				if (select_hiragana != 0)
				{
					object_block[object_on_stage][0] = select_hiragana ;
					object_block[object_on_stage][1] = cursorA.x * (-64) +640;
					object_block[object_on_stage][2] = cursorA.y * (-64);
					object_on_stage++;
				}
			}break;

			case 'i': //選択中のブロックを削除
			{
				for (i = 0; i<object_on_stage; i++)
				{
					if (object_block[i][1] == cursorA.x * (-64)+640 && object_block[i][2] == cursorA.y*(-64) && object_block[i][0] != 0)
					{
						object_block[i][0] = 0;
						object_block[i][1] = 0;
						object_block[i][2] = 0;
						std::cout << "<info 099: ブロックID: " << i << " を削除しました" << std::endl;
						flag_delete = true;
						break;
					}
				}

				if (flag_delete == true)
				{
					for (k = i; k < object_on_stage - 1; k++) //消えた分を前に詰める
					{
						object_block[k][0] = object_block[k + 1][0];
						object_block[k][1] = object_block[k + 1][1];
						object_block[k][2] = object_block[k + 1][2];
					}
					object_block[object_on_stage - 1][0] = 0;
					object_block[object_on_stage - 1][1] = 0;
					object_block[object_on_stage - 1][2] = 0;

					object_on_stage--;
					
					flag_delete = false;
				}
				
			}break;

			case 't': //操作画面切り替え
			{
				if (mode_edit == 0) { mode_edit++; }
				else if (mode_edit == 1) { mode_edit--; }
			}break;

			case 'n': //操作画面切り替え
			{
				displayGUI = 1;
			}break;

			case 'm': //操作画面切り替え
			{
				displayGUI = 2;
			}break;

			case 'b': //操作画面切り替え
			{
				displayGUI = 3;
			}break;

			default:
				break;

			}
		}

		else if (mode_edit == 1)
		{
			switch (key) {

			case 'a':
			{
				if (cursorB.x <= 15 && cursorB.y <= 3 || cursorB.x <= 19 && cursorB.y == 4) { cursorB.x++; select_hiragana = def_hiragana(cursorB.x, cursorB.y); printf("%d\n", select_hiragana); }
			}break;

			case 'd':
			{
				if (cursorB.x >= 1) { cursorB.x--; select_hiragana = def_hiragana(cursorB.x, cursorB.y); printf("%d\n", select_hiragana);
				} //下半分で変更するブロックを選択 or B.xが16のときはステージ情報を選択
			}break;

			case 'w':
			{
				if (cursorB.x >= 17) { cursorB.x = 16; cursorB.y = 3; }
				else if (cursorB.y >= 1) {
					cursorB.y--; select_hiragana = def_hiragana(cursorB.x, cursorB.y); printf("%d\n", select_hiragana);
				}
			}break;

			case 's':
			{
				if (cursorB.y <= 3) { cursorB.y++; select_hiragana = def_hiragana(cursorB.x, cursorB.y); printf("%d\n", select_hiragana);
				}
			}break;



			case 'j':
			{
				if (cursorB.x == 16 && cursorB.y == 0) //ステージ情報
				{
					if (stage_info_edit % 10 == 3 && stage_info_edit >=13) { stage_info_edit -= 8; }
					else if (stage_info_edit >= 4) { stage_info_edit--; }
				}

				if (cursorB.x == 16 && cursorB.y == 1) //ノルマ情報
				{
					if (stage_nolma_edit >= 2) { stage_nolma_edit--; }
				}

				if (cursorB.x == 16 && cursorB.y == 2) //タイマー情報
				{
					if (stage_time_limit_edit >= 10) { stage_time_limit_edit-=5; }
				}

				if (cursorB.x == 16 && cursorB.y == 3) //タイマー情報
				{
					if (stage_time_limit_gold_edit >= 6) { stage_time_limit_gold_edit --; }
				}
			}break;

			case 'l':
			{
				if (cursorB.x == 16 && cursorB.y == 0) //ステージ情報
				{
					if (stage_info_edit % 10 == 5 && stage_info_edit <=5) { stage_info_edit += 8; }
					else if (stage_info_edit <= 14) { stage_info_edit++; }
				}

				if (cursorB.x == 16 && cursorB.y == 1) //ノルマ情報
				{
					if (stage_nolma_edit <= 99) { stage_nolma_edit++; }
				}

				if (cursorB.x == 16 && cursorB.y == 2) //タイマー情報
				{
					if (stage_time_limit_edit <= 995) { stage_time_limit_edit += 5; }
				}

				if (cursorB.x == 16 && cursorB.y == 3) //タイマー情報
				{
					if (stage_time_limit_gold_edit <= 994) { stage_time_limit_gold_edit++; }
				}
			}break;

			case 'i':
			{
				if (cursorB.x == 20 && cursorB.y == 4) //スロット0
				{
					if (stage_slot_constraint_edit[0] <= 74) { stage_slot_constraint_edit[0]++; }
				}
				if (cursorB.x == 19 && cursorB.y == 4) //スロット1
				{
					if (stage_slot_constraint_edit[1] >= 1) { stage_slot_constraint_edit[1]--; }
				}

				if (cursorB.x == 18 && cursorB.y == 4) //スロット2
				{
					if (stage_slot_constraint_edit[2] >= 1) { stage_slot_constraint_edit[2]--; }
				}

				if (cursorB.x == 17 && cursorB.y == 4) //スロット3
				{
					if (stage_slot_constraint_edit[3] >= 1) { stage_slot_constraint_edit[3]--; }
				}

				if (cursorB.x == 16 && cursorB.y == 4) //スロット4
				{
					if (stage_slot_constraint_edit[4] >= 1) { stage_slot_constraint_edit[4]--; }
				}
				
			}break;

			case 'k':
			{
				if (cursorB.x == 20 && cursorB.y == 4) //スロット0
				{
					if (stage_slot_constraint_edit[0] <= 74) { stage_slot_constraint_edit[0]++; }
				}

				if (cursorB.x == 19 && cursorB.y == 4) //スロット1
				{
					if (stage_slot_constraint_edit[1] <= 74) { stage_slot_constraint_edit[1]++; }
				}

				if (cursorB.x == 18 && cursorB.y == 4) //スロット2
				{
					if (stage_slot_constraint_edit[2] <= 74) { stage_slot_constraint_edit[2]++; }
				}

				if (cursorB.x == 17 && cursorB.y == 4) //スロット3
				{
					if (stage_slot_constraint_edit[3]  <= 74) { stage_slot_constraint_edit[3]++; }
				}

				if (cursorB.x == 16 && cursorB.y == 4) //スロット4
				{
					if (stage_slot_constraint_edit[4] <= 74) { stage_slot_constraint_edit[4]++; }
				}

			}break;

			case 't': //操作画面切り替え
			{
				if (mode_edit == 0) { mode_edit++; }
				else if (mode_edit == 1) { mode_edit--; }
			}break;

			case 'n': //操作画面切り替え
			{
				displayGUI = 1;
			}break;

			case 'm': //操作画面切り替え
			{
				displayGUI = 2;
			}break;

			case 'b': //操作画面切り替え
			{
				displayGUI = 3;
			}break;

			default:
				break;

			}
		}
	}

	if (displayGUI == 1)
	{
		switch (key) {

		case 'a':
		{
			if (stage_select_confirm >= 2) { stage_select_confirm--; }
		}break;

		case 'd':
		{
			if (stage_select_confirm <= STAGE_LIMIT-1) { stage_select_confirm++; }
		}break;

		case 'j':
		{
			if (stage_select_confirm >= 12) { stage_select_confirm -=10; }
			else if (stage_select_confirm >= 2) { stage_select_confirm =1; }
		}break;

		case 'i':
		{
			displayGUI = 0;
		}break;

		case 'l':
		{
			if (stage_select_confirm <= STAGE_LIMIT - 1) { stage_select_confirm +=10; }
			else if (stage_select_confirm <= STAGE_LIMIT - 11) { stage_select_confirm = STAGE_LIMIT; }
		}break;

		case 'k':
		{
			import_stage(stage_select_confirm);
		}break;

		default:
			break;

		}
	}

	if (displayGUI == 2)
	{
		switch (key) {

		case 'a':
		{
			if (stage_select_confirm >= 2) { stage_select_confirm--; }
		}break;

		case 'd':
		{
			if (stage_select_confirm <= STAGE_LIMIT - 1) { stage_select_confirm++; }
		}break;

		case 'j':
		{
			if (stage_select_confirm >= 12) { stage_select_confirm -= 10; }
			else if (stage_select_confirm >= 2) { stage_select_confirm = 1; }
		}break;

		case 'l':
		{
			if (stage_select_confirm <= STAGE_LIMIT - 1) { stage_select_confirm++; }
			else if (stage_select_confirm <= STAGE_LIMIT - 11) { stage_select_confirm = STAGE_LIMIT; }
		}break;

		case 'i':
		{
			displayGUI = 0;
		}break;

		case 'k':
		{
			export_stage(stage_select_confirm);
		}break;

		default:
			break;

		}
	}

	if (displayGUI == 3)
	{
		switch (key) {

		case 'i':
		{
			displayGUI = 0;
		}break;

		case 'k':
		{
			new_stage();
		}break;

		default:
			break;

		}
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

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
	GdiplusStartup(&gdiPT, &gdiPSI, NULL);

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


	for (i = 0; i <= 79; i++)
	{
		block_hiragana[i].LoadImagePNG2(block_hiragana[i].file, block_hiragana[i].tex);
		block_hiragana_mini[i].LoadImagePNG2(block_hiragana_mini[i].file, block_hiragana_mini[i].tex);
	}

	UI_01.LoadImagePNG2(UI_01.file, UI_01.tex);
	UI_02.LoadImagePNG2(UI_02.file, UI_02.tex);
	UI_confirm01.LoadImagePNG2(UI_confirm01.file, UI_confirm01.tex);
	UI_confirm02.LoadImagePNG2(UI_confirm02.file, UI_confirm02.tex);
	UI_confirm03.LoadImagePNG2(UI_confirm03.file, UI_confirm03.tex);
	UI_start.LoadImagePNG2(UI_start.file, UI_start.tex);
	UI_cursor_A.LoadImagePNG2(UI_cursor_A.file, UI_cursor_A.tex);
	UI_cursor_Bm.LoadImagePNG2(UI_cursor_Bm.file, UI_cursor_Bm.tex);
	UI_cursor_Bb.LoadImagePNG2(UI_cursor_Bb.file, UI_cursor_Bb.tex);

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
}


void timer(int value) {

	glutPostRedisplay();
	glutTimerFunc(16, timer, 0); //だいたい60fpsを目指す
		
	if(timer_cursor_lux >= 1000)
	{
		timer_cursor_lux = 0;

	}
	timer_cursor_lux++;

}


int main(int argc, char *argv[])
{
	atexit(end);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow("goipachi editor ver.1.0.0");
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutTimerFunc(16, timer, 0);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	Init();
	glutMainLoop();

	return 0;
}