#include "pugixml.h"
#include "flowSource.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


std::vector<float> fStrToVector(std::string str){
  std::vector<float> valueVector;
  std::stringstream ss(str);
  std::string fValue;
  
  while(std::getline(ss, fValue, ',')){
    std::stringstream fs(fValue);
    float f = 0.0; 
    fs >> f;
    valueVector.push_back(f);
  }
  return valueVector;
}

std::vector<int> iStrToVector(std::string str){
  std::vector<int> valueVector;
  std::stringstream ss(str);
  std::string iValue;
  
  while(std::getline(ss, iValue, ',')){
    std::stringstream is(iValue);
    int i = 0; 
    is >> i;
    valueVector.push_back(i);
  }
  return valueVector;
}

void parsePressure(FlowSource* obj, pugi::xml_node pres, pugi::xml_node time){
  std::string timestr = time.attribute("t").value();
  std::string pstr = pres.attribute("p").value();
  std::vector<int> timevec = iStrToVector(timestr);
  std::vector<float> pvec = fStrToVector(pstr);
  obj->setPressure(pvec, timevec); 
}

void parsePosition(FlowSource* obj, pugi::xml_node node){
  int x, y, z;
  x = node.attribute("x").as_int();
  y = node.attribute("y").as_int();
  z = node.attribute("z").as_int();
  obj->setPosition(x,y,z);
}

void parseNormal(FlowSource* obj, pugi::xml_node node){

}

void parseTotalWater(FlowSource* obj, pugi::xml_node node){

}

void parseRadius(FlowSource* obj, pugi::xml_node node){

}

std::vector<FlowSource*> loadFlows(const char* xmlFile) {
  pugi::xml_document doc;
  std::vector<FlowSource*> srces;
  if (!doc.load_file(xmlFile)) return srces;
  std::cout << "Load success" << std::endl;
  pugi::xml_node sources = doc.child("Profile").child("Sources");
  for (pugi::xml_node_iterator it = sources.begin(); it != sources.end(); ++it){
      FlowSource* fsp;
      fsp = new FlowSource;
      parsePressure(fsp, it->child("Pressure"), it->child("TimeP"));
      parsePosition(fsp, it->child("Position"));
      srces.push_back(fsp);
    }
  return srces;
}


int main()
{
  std::vector<int> pos1, pos2;
  std::vector<FlowSource*> sfv = loadFlows("xgconsole.xml");
  FlowSource first = *sfv[0];
  for(int i = 0; i < 12; i++){
    std::cout << first.getPressure() << std::endl;
    first.update();
  }

  pos1 = sfv[0]->getPosition();
  pos2 = sfv[1]->getPosition();
  std::cout << "Postion given by xyz: ";
  for (auto i = pos1.begin(); i != pos1.end(); ++i){
    std::cout << *i << ' ';
  }
  std::cout << std::endl;
  std::cout << "Postion given by xyz: ";
  for (auto i = pos2.begin(); i != pos2.end(); ++i){
    std::cout << *i << ' ';
  }
  std::cout << std::endl;

  return 1;
}

// vim:et
