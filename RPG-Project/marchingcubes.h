#pragma once
#include <glad/glad.h>

GLvoid vSetTime(GLfloat fTime);
GLfloat fSample1(GLfloat fX, GLfloat fY, GLfloat fZ);
GLfloat fSample2(GLfloat fX, GLfloat fY, GLfloat fZ);
GLfloat fSample3(GLfloat fX, GLfloat fY, GLfloat fZ);
extern GLfloat (*fSample)(GLfloat fX, GLfloat fY, GLfloat fZ);

GLvoid vMarchingCubes();
GLvoid vMarchCube1(GLfloat fX, GLfloat fY, GLfloat fZ, GLfloat fScale);
GLvoid vMarchCube2(GLfloat fX, GLfloat fY, GLfloat fZ, GLfloat fScale);

