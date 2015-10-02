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

void parseNormal(FlowSource* obj, pugi::xml_node norm, pugi::xml_node time){
  std::string timestr = time.attribute("t").value();
  std::vector<float> ndir;
  std::vector<std::vector<float> > nvec;
  float x, y, z;
  for (pugi::xml_node_iterator it = norm.begin(); it != norm.end(); ++it){
  x = it->attribute("x").as_float();
  y = it->attribute("y").as_float();
  z = it->attribute("z").as_float();
  ndir = {x, y, z};
  nvec.push_back(ndir);
  }
  std::vector<int> timevec = iStrToVector(timestr);
  obj->setNormal(nvec, timevec); 
}

void parseTotalWater(FlowSource* obj, pugi::xml_node node){
  float tot; 
  tot = node.attribute("tot").as_float();
  obj->setTotalWater(tot);
}

void parseRadius(FlowSource* obj, pugi::xml_node node){
  float r; 
  r = node.attribute("r").as_float();
  obj->setRadius(r);
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
      parseNormal(fsp, it->child("Normals"), it->child("TimeN"));
      parseTotalWater(fsp, it->child("TotWater"));
      parseRadius(fsp, it->child("Radius"));
      srces.push_back(fsp);
  }
  return srces;
}


int main()
{
  float tw, r;
  std::vector<int> pos1, pos2;
  std::vector<float> ndirec;
  std::vector<FlowSource*> sfv = loadFlows("xgconsole.xml");
  FlowSource first = *sfv[0];
  for(int i = 0; i < 12; i++){
    std::cout << "Pressure: " << first.getPressure() << std::endl;
    ndirec = first.getNormal();
    std::cout << "Normal given by xyz: ";
    for (auto j = ndirec.begin(); j != ndirec.end(); ++j){
      std::cout << *j << ' ';
    }
    std::cout << std::endl;
    first.update();
  }

  pos1 = sfv[0]->getPosition();
  pos2 = sfv[1]->getPosition();
  tw = sfv[0]->getWaterLeft();
  r = sfv[0]->getRadius();
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
  std::cout << "TotWater: " << tw << std::endl;
  std::cout << "Radius: " << r << std::endl;
  return 1;
}

// vim:et
