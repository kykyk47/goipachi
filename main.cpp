#include "pch.h"
#include <iostream>
#include <stdio.h>
#include <gl/glut.h>
#include <math.h>
#include <IL/il.h>
#include <IL/ilut.h>


void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
}

void resize(int w, int h)
{
	/* ウィンドウ全体をビューポートにする */
	glViewport(0, 0, w, h);

	/* 変換行列の初期化 */
	glLoadIdentity();

	/* スクリーン上の表示領域をビューポートの大きさに比例させる */
	glOrtho(-w / 200.0, w / 200.0, -h / 200.0, h / 200.0, -1.0, 1.0);
}

void init(void)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_TEXTURE_2D); // 2Dテクスチャを有効にする

}

int main(int argc, char *argv[])
{
	ilInit();
	ilutRenderer(ILUT_OPENGL);
	wchar_t unko[24] = L"start.png";
	
	GLuint texture = ilutGLLoadImage(unko);

	glutInitWindowPosition(200, 100);
	glutInitWindowSize(800, 450);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutCreateWindow(argv[0]);
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	init();
	glutMainLoop();
	return 0;
}
