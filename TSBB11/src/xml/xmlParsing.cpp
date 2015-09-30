#include "pugixml.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "flowSource.h"



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

std::vector<FlowSource*> loadFlows(const char* xmlFile) {
  pugi::xml_document doc;
  std::vector<FlowSource*> srces;
  if (!doc.load_file(xmlFile)) return srces;
  std::cout << "Load success" << std::endl;
  pugi::xml_node sources = doc.child("Profile").child("Sources");
  for (pugi::xml_node_iterator it = sources.begin(); it != sources.end(); ++it){
      std::cout << "Sources:";
      for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait){
  	  std::cout << " " << ait->name() << "=" << ait->value();
  	}

      std::cout << std::endl;
      FlowSource* fsp;
      fsp = new FlowSource;
      int x, y, z;
      pugi::xml_node pos = it->child("Position"); 
      std::string timestr = it->child("Time").attribute("t").value();
      std::string pstr = it->child("Pressure").attribute("p").value();
      std::vector<int> timevec = iStrToVector(timestr);
      std::vector<float> pvec = fStrToVector(pstr);
      x = pos.attribute("x").as_int();
      y = pos.attribute("y").as_int();
      z = pos.attribute("z").as_int();

      fsp->setPosition(x,y,z);
      fsp->setPressure(pvec, timevec);
      srces.push_back(fsp);

    }
  return a;
}


int main()
{
  std::vector<int> pos1, pos2;
  //   std::cout << "HA";
  std::vector<FlowSource*> sfv = loadFlows("xgconsole.xml");
  FlowSource first = *sfv[0];
  for(int i = 0; i < 12; i++){
    std::cout << first.getPressure() << std::endl;
    first.update();
    // std::cout << first.update();
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
