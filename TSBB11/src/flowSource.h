/// @file flowSource.h
/// @brief Contains class specifing a flow source and its parameters.
#ifndef FLOWSOURCE_H
#define FLOWSOURCE_H
#include <vector>

/// @class FlowSource
/// @brief Contains the parameters of a flow source..
///
/// This class is a container for parameters of a flow source.
/// Parameters can be added and read using public functions.


class FlowSource {
  std::vector<std::vector<float>> normal; ///< Vector of vectors containing normal direction as xyz
  std::vector<int> normalTime; ///< Vector containing time instances specifying change of normal.
  ///< "-1" specifies an infinite source.
  std::vector<float> pressureTime; ///<  Vector containing time instances specifying change of pressure, "-1" is the only negativ vaslue allowed and must be at the end.
  ///< "-1" specifies an infinite source.
  std::vector<float> pressure; ///< Vector containing pressure values
  int xpos, ypos, zpos; ///< The position of the source given by the parameters x, y and z/
  float totalWater; ///< The total water of the source.
  float radius; ///< The radius of the source.
  float currTime; ///< The current internal source time, this should be linear maped to simulation time.

public:
  float currPres;
  /// @brief Initializes an epty source with time set to zero, it sould be populated with data using the set* functions.
  FlowSource() {currTime = 0; currPres = 0;};
  /// @brief default destructor for now.
  ~FlowSource() = default;
  /// @brief Sets the pressure vector and the pressure time vector. This functoin expects pressure and time values in two seperates vectors with a N to N corespondence, where N is the vector position.
  /// For example P = [1.5 , 2.1, 1.7] time = [1, 3, 8] gives that the pressure is 2.1 between the time 1 and 3.
  /// @param P the pressure vector containing floats to be set.
  /// @param t the time vector containing ints to be set, this must be sorted from smallest to largest and only using values larger thamn 0 with the exception of "-1" that must be at the end if included.
  void setPressure(std::vector<float> P, std::vector<float> t);
  /// @brief This function expects a vector filled with vectors that hold 3 values, xyz, and a time vector that specifies when to change the Normal direction. this time vector can be independent from the one used for pressure. For vector correspondences and time vetor specifications see setPressure()
  /// @param N is a vector of vectors containig the xyz values.
  /// @param t time vector see getPressure().
  /// @see setPressure()
  void setNormal(std::vector< std::vector<float> > N, std::vector<int> t);
  /// @brief This function expects three ints symbolizing the 3D space cordinates of the source expressed in voxels.
  /// @param x is the x value of the source in voxels.
  /// @param y is the y value of the source in voxels.
  /// @param z is the z value of the source in voxels.
  void setPosition(int x, int y, int z);
  /// @brief This sets a float symbolising the total water in a source.
  /// @todo  Implement method changing this parameter.
  /// @param tot Is the total water at the start of the simulation in the source.
  void setTotalWater(float tot);
  /// @brief Sets the radius of the source.
  /// @param r The radius of the source to be set.
  void setRadius(float r);
  /// @brief This function updates the source by progressing the internal source time. this should be called regularly by the simulation.
  /// @todo make this function also recalculate the total water left.
  void update(float dt);
  /// @brief This function finds the curent pressure depandeing on the internal time set by the update function.
  /// @return The current pressure
  float getPressure();
  /// @brief This function finds a thee valued vector with xyz defining the Normal direction at the internal time specified by the update function.
  /// @return The flow normal as xyz vector 
  std::vector<float> getNormal();
  /// @brief Gets the position of the flow source.
  /// @return The position given in voxels by a vector with xyz values.
  std::vector<int> getPosition();
  /// @brief Gets the water left in the flow source as descriebed by totalWater.
  /// @returns Water left in source, @warning this value never changes in current implementation 
  float getWaterLeft();
  /// @brief Gets the radius of the flow source.
  /// @returns Radius of the source, @warning this value never changes in current implementation   
  float getRadius();

  bool getChange(float dt);
};

#endif
