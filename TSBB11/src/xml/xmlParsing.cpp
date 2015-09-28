#include "pugixml.hpp"

#include <string.h>
#include <iostream>
#include <vector>

class FlowSource {
  std::vector<float> normal;
  std::vector<float> normalTime;
  std::vector<float> pressureTime;
  std::vector<float> pressure;
public:
  FlowSource();
  int xpos, ypos, zpos;
  float totalWater;
  float radius;
  float getPressure(time);
  std::vector<float> getNormal(time); 
};

std::vector<FlowSource> loadFlows(string xmlFile) {
  pugi::xml_document doc;
  if (!doc.load_file(xmlFile)) return -1;
  pugi::xml_node sources = doc.child("Profile").child("Sources");

  for (pugi::xml_node_iterator it = sources.begin(); it != sources.end(); ++it)
    
    {
      std::cout << "Sources:";

      for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
  	{
  	  std::cout << " " << ait->name() << "=" << ait->value();
  	}

      std::cout << std::endl;
    }


int main()
{
  pugi::xml_document doc;
  if (!doc.load_file("xgconsole.xml")) return -1;
    
  pugi::xml_node tools = doc.child("Profile").child("Tools");

  // tag::basic[]
  for (pugi::xml_node tool = tools.first_child(); tool; tool = tool.next_sibling())
    {
      std::cout << "Tool:";

      for (pugi::xml_attribute attr = tool.first_attribute(); attr; attr = attr.next_attribute())
	{
	  std::cout << " " << attr.name() << "=" << attr.value();
	}

      std::cout << std::endl;
    }
  // end::basic[]

  std::cout << std::endl;

  // tag::data[]
  for (pugi::xml_node tool = tools.child("Tool"); tool; tool = tool.next_sibling("Tool"))
    {
      std::cout << "Tool " << tool.attribute("Filename").value();
      std::cout << ": AllowRemote " << tool.attribute("AllowRemote").as_bool();
      std::cout << ", Timeout " << tool.attribute("Timeout").as_int();
      std::cout << ", Description '" << tool.child_value("Description") << "'\n";
    }
  // end::data[]

  std::cout << std::endl;

  // tag::contents[]
  std::cout << "Tool for *.dae generation: " << tools.find_child_by_attribute("Tool", "OutputFileMasks", "*.dae").attribute("Filename").value() << "\n";

  for (pugi::xml_node tool = tools.child("Tool"); tool; tool = tool.next_sibling("Tool"))
    {
      std::cout << "Tool " << tool.attribute("Filename").value() << "\n";
    }
  // end::contents[]



  for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it)
    {
      std::cout << "Tool:";

      for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
	{
	  std::cout << " " << ait->name() << "=" << ait->value();
	}

      std::cout << std::endl;
    }


}

// vim:et
