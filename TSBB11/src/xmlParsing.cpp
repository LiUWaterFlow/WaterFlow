#include "xmlParsing.h"

// This file implement all the parsing functions for the XML file contents and an generating function for the Flowsource class.


Flood_Fill_data::Flood_Fill_data(int in_x, int in_z, int in_height)
{
  this->x = in_x;
  this->z = in_z;
  this->height = in_height;
}

init_Data_struct::init_Data_struct(const char* XMLfile)
{
  this->data_filename = loadMapPath(XMLfile);
  this->Flowsources = loadFlows(XMLfile);
  this->FFData = loadFFData(XMLfile);
}

// Converts a string with CSV to a vector with floats.
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

//Converts a string with CSV to a vector with ints.
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

// This function parses the pressure values from the XML file by takeing the Pressure node, the PressureTime node and a pointer to a FlowSource object as input and adding the contents to the provided Object.
void parsePressure(FlowSource* obj, pugi::xml_node pres, pugi::xml_node time){
  std::string timestr = time.attribute("t").value();
  std::string pstr = pres.attribute("p").value();
  std::vector<int> timevec = iStrToVector(timestr);
  std::vector<float> pvec = fStrToVector(pstr);
  obj->setPressure(pvec, timevec);
}

// This function parses the Position node and adds it to by pointer provided FlowSource Object.
void parsePosition(FlowSource* obj, pugi::xml_node node){
  int x, y, z;
  x = node.attribute("x").as_int();
  y = node.attribute("y").as_int();
  z = node.attribute("z").as_int();
  obj->setPosition(x,y,z);
}

// This function parses the xyz direction of A Normal node and the time of a NormalTime node, then adds the result to an FlowSource Object given by pointer.
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

// This Function transverces all child nodes to a Sources node in a (by input) specified XML file. Fore every source specifed it adds a new FlowSource object and populates it with the contents in the XML file coresponding to the source. It returns a vector with pointers to the objects it creates.
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

std::string loadMapPath(const char* xmlFile){
  pugi::xml_document doc;
  if(!doc.load_file(xmlFile)) return "No XML file";
  std::cout << "Load success" << std::endl;
  std::string path = doc.child("Profile").child("Data").child("MapName").attribute("path").value();
  return path;
}

std::vector<Flood_Fill_data*> loadFFData(const char* xmlFile){
  pugi::xml_document doc;
  doc.load_file(xmlFile);

  std::vector<Flood_Fill_data*> out_data;
  pugi::xml_node floods = doc.child("Profile").child("Floods");

  for (pugi::xml_node_iterator it = floods.begin(); it != floods.end(); ++it){
    Flood_Fill_data* Flood_Fill = new Flood_Fill_data(it->attribute("x").as_int(), it->attribute("z").as_int(), it->attribute("height").as_int());

    out_data.push_back(Flood_Fill);
  }
  return out_data;
}


void deleteAllFlows(std::vector<FlowSource*> srces){
  for(auto it = srces.begin(); it != srces.end(); ++it){
    delete *it;
  }
}

// vim:et
