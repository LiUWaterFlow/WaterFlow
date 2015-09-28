
// This is a test file
#include <src/pugixml.hpp>
#include <iostream>
int main(){

  pugi::xml_document doc;

  pugi::xml_parse_result result = doc.load_file("docs/samples/tree.xml");

  std::cout << "Load result: " << result.description() << ", mesh name: " << doc.child("mesh").attribute("name").value() << std::endl;
}
