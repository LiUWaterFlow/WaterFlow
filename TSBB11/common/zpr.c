#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>

#include "zpr.h"

#define bool int
#define true 1
#define false 0

// 130831: Ingemar converted to VU3. Nuked superfluous projection matrix info.
// Also nuked the shader reference - the host program should pass the matrix
// to whatever shaders it is using (often more than one).
// 130907: The Mac version of MicroGlut got its Y axis right, and then I could
// get this right, too.

//---------------------Globals------------------------------------
static int  _mouseX      = 0;
static int  _mouseY      = 0;
static bool _mouseLeft   = false;
static bool _mouseMiddle = false;
static bool _mouseRight  = false;

mat4 *_cameraMatrix; // , _projectionMatrix;

mat4 tMat, rotMatX, rotMatY, rotMat, tmpMat1, tmpMat2;
//char *viewMatName; //, *projMatName;
static vec3 camera, lookAtPoint,
        up, fwd, right;

static GLfloat angle_y = 0.0, angle_x = 0.0;
static int lastX = 300, lastY = 300;

//------------------------------------------------------------------------------

void updateCameraMatrix(mat4 *matrix)
{
    if (matrix != NULL)
    {
        camera = MultVec3(*matrix, camera);
        lookAtPoint = MultVec3(*matrix, lookAtPoint);
    }

    *_cameraMatrix = lookAt(camera.x, camera.y, camera.z,
    					lookAtPoint.x, lookAtPoint.y, lookAtPoint.y, up.x, up.y, up.z);
    right = SetVector(_cameraMatrix->m[0], _cameraMatrix->m[1], _cameraMatrix->m[2]);
    up = SetVector(_cameraMatrix->m[4], _cameraMatrix->m[5], _cameraMatrix->m[6]);
    fwd = SetVector(_cameraMatrix->m[8], _cameraMatrix->m[9], _cameraMatrix->m[10]);

//    glUniformMatrix4fv(glGetUniformLocation(shader, viewMatName), 1, GL_TRUE, _cameraMatrix.m);
}

void zprInit(mat4 *viewMatrix, vec3 cam, vec3 point)
{
//    shader = s; // EN shader?! Orimligt!
//    viewMatName = vMatName;
//    projMatName = pMatName;
    _cameraMatrix = viewMatrix;
//    _projectionMatrix = projMatrix; // ???
    camera = cam;
    lookAtPoint = point;
    up = SetVector(0, 1, 0);

    updateCameraMatrix(NULL);

    glutMouseFunc(zprMouse);
    glutKeyboardFunc(zprKey);
    glutMotionFunc(zprMouseFunc);
}

void zprMouse(int button, int state, int x, int y)
{
  _mouseX = x;
  _mouseY = y;

  if (state==GLUT_UP)
    switch (button)
      {
      case GLUT_LEFT_BUTTON:   _mouseLeft   = false; break;
      case GLUT_MIDDLE_BUTTON: _mouseMiddle = false; break;
      case GLUT_RIGHT_BUTTON:  _mouseRight  = false; break;
      }
  else
    switch (button)
      {
      case GLUT_LEFT_BUTTON:
      {
          if(!_mouseLeft)
            {
                lastX = x;
                lastY = y;
            }
          _mouseLeft   = true;
          break;
      }
      case GLUT_MIDDLE_BUTTON: _mouseMiddle = true; break;
      case GLUT_RIGHT_BUTTON:  _mouseRight  = true; break;
      }
}

void zprMouseFunc(int x, int y)
{
    if(_mouseLeft)
    {
        angle_y = (float)(x - lastX) *0.002;
        angle_x = (float)(y - lastY) *0.002;
//        tmpMat1 = ArbRotate(right, angle_x);
        tmpMat1 = ArbRotate(right, -angle_x); // Reversed, also looks more reasonable (but wasn't interesting before the Mac version got isFlipped.)
        tmpMat2 = ArbRotate(up, -angle_y); // Reversed; I think this feels better /Ingemar 130901

        rotMat = Mult(tmpMat2, tmpMat1);
        updateCameraMatrix(&rotMat);

        lastX = x;
        lastY = y;
    }
}

void zprKey(unsigned char key, int x, int y)
{
  float speed = 0.3, rotSpeed = 0.3;
    GLfloat speedX = 0, speedY = 0, speedZ = 0,
    angle = 0.0,
    dX = 0, dY = 0, dZ = 0;

    bool turnedX = false, turnedY = false;

  switch (key)
   {
   case 'i':
     speedY = speed;
     break;
   case 'k':
     speedY = -speed;
     break;
   case 'j':
     speedX = -speed;
     break;
   case 'l':
     speedX = speed;
     break;

   case 'd':
        angle = -rotSpeed;
        turnedY = true;
     break;
   case 'a':
        angle = rotSpeed;
        turnedY = true;
     break;
   case 'w':
     speedZ = -speed;
     break;
   case 's':
     speedZ = speed;
     break;

   case 'q':
    angle = -rotSpeed;
    turnedX = true;
     break;
   case 'e':
    angle = rotSpeed;
    turnedX = true;
     break;
   }

    dX = (GLfloat) _cameraMatrix->m[0] * speedX + _cameraMatrix->m[4] * speedY + _cameraMatrix->m[8] * speedZ;
    dY = (GLfloat) _cameraMatrix->m[1] * speedX + _cameraMatrix->m[5] * speedY + _cameraMatrix->m[9] * speedZ;
    dZ = (GLfloat) _cameraMatrix->m[2] * speedX + _cameraMatrix->m[6] * speedY + _cameraMatrix->m[10] * speedZ;

    camera.x += dX;
    camera.y += dY;
    camera.z += dZ;

    lookAtPoint.x += dX;
    lookAtPoint.y += dY;
    lookAtPoint.z += dZ;

    if(turnedY)
    {
        tMat = T(-camera.x, -camera.y, -camera.z);
        rotMat = ArbRotate(up, angle);
        lookAtPoint = MultVec3(tMat, lookAtPoint);
        lookAtPoint = MultVec3(rotMat, lookAtPoint);
        tMat = T(camera.x, camera.y, camera.z);
        lookAtPoint = MultVec3(tMat, lookAtPoint);
    }
    if(turnedX)
    {
        tMat = T(-lookAtPoint.x, -lookAtPoint.y, -lookAtPoint.z);
        rotMat = ArbRotate(right, angle);
        camera = MultVec3(tMat, camera);
        camera = MultVec3(rotMat, camera);
        tMat = T(lookAtPoint.x, lookAtPoint.y, lookAtPoint.z);
        camera = MultVec3(tMat, camera);
    }
    updateCameraMatrix(0);
}

