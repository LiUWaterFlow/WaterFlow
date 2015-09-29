// Written by Gustav Svensk, acquired from
// https://github.com/DrDante/TSBK03Project/
// with permission, 2015-09-24.
#include "camera.h"
#include <cmath>
#include <iostream>

Camera::Camera(int program, glm::mat4 *matrix)
{
    this->matrix = matrix;
    this->program = program;
    *matrix = glm::lookAt(position, look_at_pos, up);
    upload();
}

Camera::Camera()
{}


void Camera::upload()
{
    // Upload camera matrix here
    /* glUniformMatrix4fv(glGetUniformLocation(program, "WTVMatrix"), 1, GL_FALSE, glm::value_ptr(*matrix)); */
    glUniform3fv(glGetUniformLocation(program, "camPos"), 1, glm::value_ptr(position));
    
}
void Camera::update()
{
    *matrix = glm::lookAt(position, look_at_pos, up);
    upload();
}

void Camera::rotate(char direction, float angle)
{
    switch (direction){
        case 'x':
            look_at_pos = glm::rotate(look_at_pos, angle, glm::vec3(1,0,0));
            break;
        case 'y':
            look_at_pos = glm::rotate(look_at_pos, angle, glm::vec3(0,1,0));
            break;
        case 'z':
            look_at_pos = glm::rotate(look_at_pos, angle, glm::vec3(0,0,1));
            break;
    }
    *matrix = glm::lookAt(position, look_at_pos, up);
    upload();
}

void Camera::translate(float dx, float dy, float dz)
{
    position = glm::vec3(glm::translate(glm::mat4(1.0f), glm::vec3(dx,dy,dz)) * glm::vec4(position, 1));
    look_at_pos = glm::vec3(glm::translate(glm::mat4(1.0f), glm::vec3(dx,dy,dz)) * glm::vec4(look_at_pos, 1));
    *matrix = glm::lookAt(position, look_at_pos, up);
    upload();
}

void Camera::forward(float d)
{
    glm::vec3 forward_vec = look_at_pos - position;
    forward_vec = d * forward_vec;
    translate(forward_vec.x, forward_vec.y, forward_vec.z);
}

void Camera::strafe(float d)
{
    glm::vec3 strafe_vec = look_at_pos - position;
    strafe_vec = glm::cross(up, strafe_vec);
    strafe_vec = glm::normalize(strafe_vec);
    strafe_vec = d * strafe_vec;
    translate(strafe_vec.x, strafe_vec.y, strafe_vec.z);
}

void Camera::change_look_at_pos(int xrel, int y, int width, int height)
{
    if(y==0){
        y = 1;
    }
    x += xrel;
    float fi = ((float)x) / ((float)width) * 2.0f * M_PI;
    float theta = ((float)y) / ((float)height) * M_PI;

    look_at_pos.x = -sin(theta)*sin(fi) + position.x;
    look_at_pos.y = cos(theta) + position.y;
    look_at_pos.z = sin(theta)*cos(fi) + position.z;
    *matrix = glm::lookAt(position, look_at_pos, up);
    upload();
}

void Camera::print_matrix(glm::mat4 m)
{
    std::cout << m[0][0] << ", "  << m[0][1] << ", " << m[0][2] << ", " << m[0][3] << std::endl;
    std::cout << m[1][0] << ", "  << m[1][1] << ", " << m[1][2] << ", " << m[1][3] << std::endl;
    std::cout << m[2][0] << ", "  << m[2][1] << ", " << m[2][2] << ", " << m[2][3] << std::endl;
    std::cout << m[3][0] << ", "  << m[3][1] << ", " << m[3][2] << ", " << m[3][3] << std::endl;
    std::cout << std::endl;
}
