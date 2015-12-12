/// @file xmlParsing.h
/// @brief This file implement all the parsing functions for the XML file contents and an generating function for the Flowsource class.
#ifndef XMLPARSING_H
#define XMLPARSING_H
#include "pugixml.h"
#include "flowSource.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

/// @struct Flood_Fill_data
/// @brief Struct that contains information about a location to be filled
struct Flood_Fill_data{
	/// @brief Simple constructor for initialization
	Flood_Fill_data(int, int, float);
	int x;			///< X position of fill to start
	int z;			///< Z position of fill to start
	float height;	///< Height of the completed fill
};

/// @struct init_Data_struct
/// @brief Encapsulating struct for all objects read from XML file.
struct init_Data_struct {
  /// @brief Simple constructor for initialization
  init_Data_struct(const char* XMLfile);
  std::string data_filename;	///< path and name of dem data to be used for terrain generation.
  std::vector<FlowSource*> Flowsources; ///< Vector containing pointers to the Sources used for generating time varying water flows.
  std::vector<Flood_Fill_data*> FFData; ///< Vctor containing pionters to flood fill structs used for creating large scale lakes. 
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
/// @param norm xml node with child nodes containing normals defined by the attributes: "x", "y" and "z".
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


/// @brief This function progresses the pressure time for the flowsources provided and checks if there has ocured a change in pressure.
/// @param Vector with pointers to flowSource objects to be updated and and checked for changes.
/// @param Time unit to advance the internal source time with given as a foat.
/// @return If any source has changed is pressure this function will return true. Otherwise the return will be false.
bool flowChange(std::vector<FlowSource*> flows, float dt);

/// @brief Loads a path given in the data node from a XML file.
/// @param relative path to the XML file containing the terrain map to be loaded.
/// @return string containing relative path to the dem data file used for terrain generatrion.
std::string loadMapPath(const char* xmlFile);

/// @brief Loads all flood objects given in the floods node from a XML file to a vector.
/// @param Relative path to the XML file containing the floods to fill the terrain with.
/// @return A vector containing pointers to Flood_Fill_Data objects created from flood nodes.
std::vector<Flood_Fill_data*> loadFFData(const char* xmlFile);

/// @brief This function removes flowSources to free up memory
/// @param Vector with pointers to flowSource objects to be deleted.
void deleteAllFlows(std::vector<FlowSource*> srces);

#endif
