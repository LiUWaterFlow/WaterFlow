#include <vector>
#include "flowSource.hpp"


void FlowSource::setPosition(int x, int y, int z){
  xpos=x;
  ypos=y;
  zpos=z;
}

float FlowSource::getPressure(){
  // Implementation here
  return 1.0;
}

std::vector<float> FlowSource::getNormal(){  
// Implementation here
  std::vector<float> a;
  return a;
}

std::vector<int> FlowSource::getPosition(){
  std::vector<int> pos;
  pos.push_back(xpos);
  pos.push_back(ypos);
  pos.push_back(zpos);
  return pos;
}
