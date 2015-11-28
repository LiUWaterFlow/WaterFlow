/// @file xmlparsing.h
/// @brief This file implement all the parsing functions for the XML file contents and an generating function for the Flowsource class.
#ifndef XMLPARSING_H
#define XMLPARSING_H
#include "pugixml.h"
#include "flowSource.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

struct Flood_Fill_data{
 Flood_Fill_data(int, int, float);
 int x;
 int z;
 float height;
};

struct init_Data_struct {
  init_Data_struct(const char* XMLfile);
  /* const char* data_filename; */
  std::string data_filename;
  std::vector<FlowSource*> Flowsources;
  std::vector<Flood_Fill_data*> FFData;
};

/// @brief Converts a string with CSV to a vector with floats.
/// @param str a string with comma separated float values e.g. "1 , 2.5, -8.3"
/// @return vector of floats with the values from the input string.
std::vector<float> fStrToVector(std::string str);

/// @brief Converts a string with CSV to a vector with int.
/// @param str a string with comma separated int values e.g. "1 , 2, 8"
/// @return vector of ints with the values from the input string.
std::vector<int> iStrToVector(std::string str);

/// @brief This function parses the pressure data in the given xml nodes.
/// @param obj pointer to FlowSource object that pressure data will be writen to.
/// @param pres xml node containing pressure values as CSV in attribute "p".
/// @param time xml node containing time values as CSV in attribute "t".
void parsePressure(FlowSource* obj, pugi::xml_node pres, pugi::xml_node time);

/// @brief This function parses the position data in the given xml node.
/// @param obj pointer to FlowSource object that position data will be writen to.
/// @param node xml node containing xyz positions in attributes: "x", "y" and "z".
void parsePosition(FlowSource* obj, pugi::xml_node node);

/// @brief This function parses the normals data in the given xml nodes.
/// @param obj pointer to FlowSource object that normal data will be writen to.
/// @param norm xml node with child nodes containing normals defined by the attributes: "x", "y" and " "z".
/// @param time xml node containing time values as CSV in attribute "t".
void parseNormal(FlowSource* obj, pugi::xml_node norm, pugi::xml_node time);

/// @brief This function parses the total water value in the given xml node.
/// @param obj pointer to FlowSource object that totalWater value will be writen to.
/// @param node xml node containing the totalWater value in the "tot" attribute.
void parseTotalWater(FlowSource* obj, pugi::xml_node node);

/// @brief This function parses the radius value in the given xml node.
/// @param obj pointer to FlowSource object that Radius value will be writen to.
/// @param node xml node containing the Radius value in the "r" attribute.
void parseRadius(FlowSource* obj, pugi::xml_node node);

/// @brief This function creates and populates FlowSource objects from a given XML file.
/// The XML file is expected to be formated and contain pre defined nodes and attributes.
/// @param xmlFile relative path to an xml file containing flow sources data.
/// @return A vector filled with pointers to flowSource objects containing data given by an XML file.
std::vector<FlowSource*> loadFlows(const char* xmlFile);


std::string loadMapPath(const char* xmlFile);

std::vector<Flood_Fill_data*> loadFFData(const char* xmlFile);

/// @brief This function removes flowSources to free up memory
/// @param Vector with pointers to flowSource objects to be deleted.
void deleteAllFlows(std::vector<FlowSource*> srces);

#endif
