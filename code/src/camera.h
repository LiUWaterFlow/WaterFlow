/// @file camera.h
/// @brief Class for handling the camera.
///
/// Written by Gustav Svensk, free to use as long as this note about the original author remain

#ifndef CAMERA_H
#define CAMERA_H

#ifdef __APPLE__
#	include <OpenGL/gl3.h>
#	include <SDL2/SDL.h>
#else
#	ifdef  __linux__
#		define GL_GLEXT_PROTOTYPES
#		include <GL/gl.h>
#		include <GL/glu.h>
#		include <GL/glx.h>
#		include <GL/glext.h>
#		include <SDL2/SDL.h>
#	else
#		include "glew.h"
#		include "Windows/sdl2/SDL.h"
#	endif
#endif

#include "glm.hpp"
#include "readData.h"

/// @class Camera
/// @brief Pretty general camera class
class Camera {
private:
	int x;						///< Used so mouse will not get stuck at window edges.
	float fi;					///< Camera horizontal rotation angle.
	float theta;				///< Camera vertical rotation angle.
	float rotSpeed;				///< Camera rotation speed.
	int *screenW;				///< Pointer to screen width.
	int *screenH;				///< Pointer to screen height.
	GLfloat speed;				///< Camera movement speed.
	GLfloat drawDistance;		///< Draw distance.
	bool isFrozen;				///< Boolean to switch camera movement on/off.
	int terrW;					///< Terrain data width.
	int terrH;					///< Terrain data height.
	int xzLim;					///< Distance to camera limiting borders around the terrain, world "walls".
	int yLimLo;					///< World "floor".
	int yLimHi;					///< World "ceiling".
	DataHandler* terrain;		///< Pointer to the terrain data.
	glm::vec3 position;			///< Camera position.
	glm::vec3 lookAtPos;		///< The position the camera looks at.
	glm::vec3 up;				///< The up vector.
	glm::mat4 WTVMatrix;		///< World-to-view matrix.
	glm::mat4 VTPMatrix;		///< View-to-projection matrix.
	bool unlocked;				///< Boolean to switch bounding box mode on/off.

	/// @brief Checks whether movement moves the camera outside of the permitted bounding box.
	///
	/// The bounding box is decided by terrW, terrH, xzLim, yLimLo and yLimHi.
	/// @param transVec Vector corresponding to the translation attempted.
	/// @param xz Boolean that decides if a check for xz or y should be made.
	/// @return false if tested movement would move the camera outside of it's bounding box, otherwise true.
	bool isInCollisionBox(glm::vec3 transVec, bool xz);
public:
	/// @brief Constructor to create the camera.
	///
	/// @param startPos Starting camera position.
	/// @param initScreenW Pointer to the initial screen width.
	/// @param initScreenH Pointer to the initial screen height.
	/// @param tH Terrain height.
	/// @param tW Terrain width.
	/// @param xzL Distance to the bounding box in the xz-plane outside the terrain model.
	/// @param yLL Floor of the camera bounding box.
	/// @param yLH Ceiling of the camera bounding box.
	/// @param terr Pointer to the terrain.
	Camera(glm::vec3 startPos, int* initScreenW, int* initScreenH, int tH, int tW, int xzL, int yLL, int yLH, DataHandler* terr);

	/// @brief Unlocks the camera from it's bounding box.
	void unlock();

	/// @brief Rotates the camera.
	///
	/// @param direction Axis to rotate around.
	/// @param angle Angle to rotate.
	void rotate(char direction, GLfloat angle);

	/// @brief Translates the camera position.
	///
	/// @param dx Amount to move the camera along the x axis.
	/// @param dy Amount to move the camera along the y axis.
	/// @param dz Amount to move the camera along the z axis.
	void translate(GLfloat dx, GLfloat dy, GLfloat dz);

	/// @brief Moves the camera forward.
	///
	/// @param d Amount to move the camera forward.
	void forward(GLfloat d);

	/// @brief Moves the camera left or right.
	///
	/// @param d Amount to move the camera to the left (negative moves it to the right).
	void strafe(GLfloat d);

	/// @brief Moves the camera up.
	///
	/// Note that up here means according to the camera view direction,
	/// not necessarily in world coordinates.
	/// @param d Amount to move the camera up.
	void jump(GLfloat d);

	/// @brief Updates the WTV matrix.
	void updateWTV();

	/// @brief Updates the VTP matrix.
	void updateVTP();

	/// @brief Uploads the camera data to the specified shader.
	///
	/// @param program Shader program to upload the camera information to.
	void uploadCamData(GLuint program);

	/// @brief Switches camera movement on/off.
	void toggleFrozen();

	/// @brief Changes the position the camera is looking at when the mouse is moved.
	///
	/// @param xrel Relative mouse motion along the x axis.
	/// @param yrel Relative mouse motion along the y axis.
	void changeLookAtPos(int xrel, int yrel);

	/// @brief Getter for the WTV matrix of the camera.
	/// @return WTV matrix of the camera.
	glm::mat4* getWTV(); 

	/// @brief Getter for the VTP matrix of the camera.
	/// @return VTP matrix of the camera.
	glm::mat4* getVTP();

	/// @brief Getter for the camera position.
	/// @return camera position.
	glm::vec3* getPos();

	/// @brief Getter for the camera movement speed.
	/// @return camera movement speed.
	GLfloat getSpeed();

	/// @brief Getter for the pointer to the camera movement speed.
	/// @return pointer to the camera movement speed.
	GLfloat* getSpeedPtr();

	/// @brief Getter for the camera rotation speed.
	/// @return camera rotation speed.
	GLfloat getRotSpeed();

	/// @brief Getter for the pointer to the camera rotation speed.
	/// @return pointer to the camera rotation speed.
	GLfloat* getRotSpeedPtr();
};

#endif // CAMERA_H