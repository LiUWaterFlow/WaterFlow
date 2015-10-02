#include "flowSource.h"

// This file contains the implementation for the methods of the flowSource class
//-----------------------------------------------------------------------------

// This functoin expects pressure and time values in two seperates vectors with a N to N corespondence,
// where N is the vector position.
// For example P = [1.5 , 2.1, 1.7] time = [1, 3, 8] gives that the pressure is 2.1 at ther time 3.
// The time vector most be sorted from smales ellement to largest.
void FlowSource::setPressure(std::vector<float> P, std::vector<int> time){
  pressure = P;
  pressureTime = time;
}

void FlowSource::setNormal(std::vector< std::vector<float> > N, std::vector<int> time){
  normal = N;
  normalTime = time;
}

void FlowSource::setPosition(int x, int y, int z){
  xpos=x;
  ypos=y;
  zpos=z;
}

void FlowSource::setTotalWater(float waterAmount){
  totalWater = waterAmount;
}

void FlowSource::setRadius(float r){
  radius = r;
}

void FlowSource::update(){
  currTime++;
}

float FlowSource::getPressure(){
  int currItr = 0;
  for(auto i = pressureTime.begin(); i !=pressureTime.end(); ++i){
    if( *i <= currTime) currItr++;
    else break;
  }
  return (currItr < pressureTime.size()) ? pressure.at(currItr) : 0;
}

std::vector<float> FlowSource::getNormal(){  
  int currItr = 0;
  std::vector<float> err = {0, 0, 0};
  for(auto i = normalTime.begin(); i !=normalTime.end(); ++i){
    if( *i <= currTime) currItr++;
    else break;
  }
  return (currItr < normalTime.size()) ? normal.at(currItr) : err;
}

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
