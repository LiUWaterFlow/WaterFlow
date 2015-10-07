#ifndef XMLPARSING_H
#define XMLPARSING_H
#include "pugixml.h"
#include "flowSource.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

std::vector<float> fStrToVector(std::string str);

std::vector<int> iStrToVector(std::string str);

void parsePressure(FlowSource* obj, pugi::xml_node pres, pugi::xml_node time);

void parsePosition(FlowSource* obj, pugi::xml_node node);

void parseNormal(FlowSource* obj, pugi::xml_node norm, pugi::xml_node time);

void parseTotalWater(FlowSource* obj, pugi::xml_node node);

void parseRadius(FlowSource* obj, pugi::xml_node node);

std::vector<FlowSource*> loadFlows(const char* xmlFile);

void deleteAllFlows(std::vector<FlowSource*> srces);

#endif
