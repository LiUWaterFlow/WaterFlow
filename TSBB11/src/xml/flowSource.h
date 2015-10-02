#ifndef FLOWSOURCE_H
#define FLOWSOURCE_H
#include <vector>

class FlowSource {
  std::vector<std::vector<float>> normal;
  std::vector<int> normalTime;
  std::vector<int> pressureTime;
  std::vector<float> pressure;
  int xpos, ypos, zpos;
  float totalWater;
  float radius;
  int currTime;
public:
  FlowSource() {currTime = 0;};
  ~FlowSource() = default;
  void setPressure(std::vector<float> P, std::vector<int> t);
  void setNormal(std::vector< std::vector<float> > N, std::vector<int> t);
  void setPosition(int x, int y, int z);
  void setTotalWater(float tot);
  void setRadius(float r);
  void update();
  float getPressure();
  std::vector<float> getNormal();
  std::vector<int> getPosition();
  float getWaterLeft();
  float getRadius();
};

#endif
