#include "flowSource.h"

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
    if( *i < currTime) currItr++;
    else break;
  }
  return pressure[currItr];
}

std::vector<float> FlowSource::getNormal(){  
  int currItr = 0;
  for(auto i = normalTime.begin(); i !=normalTime.end(); ++i){
    if( *i < currTime) currItr++;
    else break;
  }
  return normal[currItr];
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
