#include "xmlParsing.h"

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
  deleteAllFlows(sfv);
  std::cout << "Delete succesfull" << std::endl;
  return 1;
}
