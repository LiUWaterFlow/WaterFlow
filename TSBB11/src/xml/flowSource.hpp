#ifndef FLOWSOURCE_H
#define FLOWSOURCE_H

class FlowSource {
  std::vector<float> normal;
  std::vector<int> normalTime;
  std::vector<int> pressureTime;
  std::vector<float> pressure;
  int xpos, ypos, zpos;
  float totalWater;
  float radius;
public:
  FlowSource() { };
  void setPreassure(std::vector<float> P, std::vector<int> t);
  void setNormal(std::vector<float> N, std::vector<int> t);
  void setPosition(int x, int y, int z);
  void setTotalWater(float tot);
  void setRadius(float r);
  float getPressure();
  std::vector<float> getNormal();
  std::vector<int> getPosition();
  float getWaterLeft();
  float getRadius();
};

#endif
