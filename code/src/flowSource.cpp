/// @file flowSource.cpp
/// @brief This file contains the implementation for the methods of the flowSource.h file

#include "flowSource.h"

// This functoin expects pressure and time values in two seperates vectors with a N to N corespondence,
// where N is the vector position.
// For example P = [1.5 , 2.1, 1.7] time = [1, 3, 8] gives that the pressure is 2.1 between the time 1 and 3.
// The time vector most be sorted from smalest element to largest.
void FlowSource::setPressure(std::vector<float> P, std::vector<float> time){
  pressure = P;
  pressureTime = time;
}

// This function expects a vector filled with vectors that hold 3 values, xyz, and a time vector that specifies when to change the Normal direction. this time vector can be independent from the one used for pressure.
void FlowSource::setNormal(std::vector< std::vector<float> > N, std::vector<int> time){
  normal = N;
  normalTime = time;
}

// This function expects three ints symbolizing the 3D space cordinates of the source expressed in voxels
void FlowSource::setPosition(int x, int y, int z){
  xpos=x;
  ypos=y;
  zpos=z;
}

// This simply sets a float symbolising the total water in a source, no method changing this parameter has been implemented yet.
void FlowSource::setTotalWater(float waterAmount){
  totalWater = waterAmount;
}


void FlowSource::setRadius(float r){
  radius = r;
}

// This function updates the internal time of the source idealy this should be called att constant intervals e.g. onece evry loop of simulation. It could be expanded with calculating remaining water.
void FlowSource::update(float dt){
  currTime = currTime + dt;
}

// This function returns the curent pressure depending on the internal time set by the update function.
float FlowSource::getPressure(){
 unsigned int currItr = 0;
  for(auto i = pressureTime.begin(); i !=pressureTime.end(); ++i){
    if( *i < currTime) currItr++;
    else break;
  }
  if (currItr < pressureTime.size())
    return pressure.at(currItr);
  else if (pressureTime.back() == -1)
    return pressure.back();
  else
    return 0;
}

// This function returns a thee valued vector with xyz defining the Normal direction at the internal time specified by the update function.
std::vector<float> FlowSource::getNormal(){  
  unsigned int currItr = 0;
  std::vector<float> err = {0, 0, 0};
  for(auto i = normalTime.begin(); i !=normalTime.end(); ++i){
    if( *i <= currTime) currItr++;
    else break;
  }
  if (currItr < normalTime.size())
    return normal.at(currItr);
  else if (normalTime.back() == -1)
    return normal.back();
  else
    return err;
}

// This returns the position as a three valued vector containing xyz.
std::vector<int> FlowSource::getPosition(){
  std::vector<int> pos;
  pos.push_back(xpos);
  pos.push_back(ypos);
  pos.push_back(zpos);
  return pos;
}


float FlowSource::getWaterLeft(){
  return totalWater;
}

float FlowSource::getRadius(){
  return radius;
}


bool FlowSource::getChange(float dt) {
  //currPres = FlowSource::getPressure();
  FlowSource::update(dt);
  float newPre = FlowSource::getPressure();
  if(newPre == currPres){
    return false;
  }
  else {
    currPres = newPre;
    return true;
  }  
}
