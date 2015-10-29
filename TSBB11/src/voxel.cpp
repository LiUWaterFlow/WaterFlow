
#include "voxel.h"

#include <iostream>
#include "gtc/type_ptr.hpp"

/* -----------------------------------------------------------------
Voxelgrid - Create the initial vector strutcture.
*/

Voxelgrid::Voxelgrid(DataHandler* dataHandler){

  this->voxels  = new std::vector<std::vector<std::vector<voxel*>*>*>;

  this->datahandler = dataHandler;
}


/* -----------------------------------------------------------------
Voxelgrid - Destructor, ensure destruction of all pointer structures
*/

Voxelgrid::~Voxelgrid(){
  for (GLuint x = 0; x < voxels->size(); x++) {
    if(voxels->at(x) != nullptr){
		for (GLuint y = 0; y < voxels->at(x)->size(); y++)
		{
        if(voxels->at(x)->at(y) != nullptr){
			for (GLuint z = 0; z < voxels->at(x)->at(y)->size(); z++)
			{
            if( voxels->at(x)->at(y)->at(z) != nullptr){
              delete voxels->at(x)->at(y)->at(z);
            }
          }
          delete voxels->at(x)->at(y);
        }
      }
      delete voxels->at(x);
    }
  }
delete voxels;
delete voxelPositions;
}



/* -----------------------------------------------------------------
setVoxel take a x,y,z coordinate where the voxel will be created (or modified)
with the values ax, bx.
*/

void Voxelgrid::setVoxel(GLuint x, GLuint y, GLuint z, bool filledx, float ax, float bx)
{

  //if x is not in table. Create y and z tables, resize x, and
  //point to children (y,z);
  if(voxels->size() < x+1){
    std::vector<voxel*>* tempZ = new std::vector<voxel*>(z+1);
    std::vector<std::vector<voxel*>*>* tempY = new std::vector<std::vector<voxel*>*>(y+1);
    voxels->resize(x+1,nullptr);
    voxels->at(x) = tempY;
    voxels->at(x)->at(y) = tempZ;

  }
  //if y is not in table. Create z table, resize y, and
  //point to childtable z; Note that existence of y table is
  //managed by the first part of the if-statement
  else if(voxels->at(x) != nullptr && voxels->at(x)->size() < y+1){

    std::vector<voxel*>* tempZ = new std::vector<voxel*>(z+1);
    voxels->at(x)->resize(y+1,nullptr);
    voxels->at(x)->at(y) = tempZ;

  }
  //if z is not large enough resize. Note that existence of z table is
  //managed by the first two parts of the if-statement
  else if(voxels->at(x) != nullptr &&  voxels->at(x)->at(y) != nullptr && voxels->at(x)->at(y)->size() < z+1){
    voxels->at(x)->at(y)->resize(z+1,nullptr);

  }

  //If x is large enough but does not contain a table at position x
  //create and insert relevant tables.
  if(voxels->at(x) == nullptr){
    std::vector<voxel*>* tempZ = new std::vector<voxel*>(z+1);
    std::vector<std::vector<voxel*>*>* tempY = new std::vector<std::vector<voxel*>*>(y+1);
    voxels->at(x) = tempY;
    voxels->at(x)->at(y) = tempZ;
  }
  //If y is large enough but does not contain a table at position y
  //create and insert relevant tables.
  else if(voxels->at(x)->at(y) == nullptr){
    std::vector<voxel*>* tempZ = new std::vector<voxel*>(z+1);
    voxels->at(x)->at(y) = tempZ;
  }
  //If there is already a voxel at position x,y,z, delete this in
  //preperation for new insertion.
  else if(voxels->at(x)->at(y)->at(z)  != nullptr){
    delete voxels->at(x)->at(y)->at(z);
  }

  //create and insert the new voxel.
  voxel* temp = new voxel;
  temp->filled = filledx;
  temp->a = ax;
  temp->b = bx;
  voxels->at(x)->at(y)->at(z) = temp;

}

/* -----------------------------------------------------------------
Get voxel at x,y,z. This function returns a pointer to the struct.
If no voxel is present it returns a nullptr.
*/

voxel* Voxelgrid::getVoxel(GLuint x, GLuint y, GLuint z)
{
  //std::cout << "In getVoxel, size of vector voxels is: " << voxels->size() << std::endl;

  //std::cout << "Voxels at x is empty" << std::endl;

  //ensure table existance and table size, if either fails return nullptr.
  if(voxels->size() < x+1 || voxels->at(x) == nullptr){
    //std::cout << "In first if in get_Voxel" << std::endl;
    return nullptr;
  }
  //ensure table existance and table size, if either fails return nullptr.
  else if(voxels->at(x)->size() < y+1 || voxels->at(x)->at(y) == nullptr){
    //std::cout << "In second if in get_Voxel" << std::endl;
    return nullptr;
  }
  //ensure table existance and table size, if either fails return nullptr.
  else if(voxels->at(x)->at(y)->size() < z+1 || voxels->at(x)->at(y)->at(z) == nullptr){
    return nullptr;
  }

  //Existance is ensured, return pointer at location x,y,z
  //std::cout << "Just before returning voxel in get_Voxel" << std::endl;
  return voxels->at(x)->at(y)->at(z);
}


/* FloodFill function creates a vector queue. Tests if the first voxel coordinates are above land, if so its coordinates are added to the queue
vector and the struct for the voxel is creatd using setVoxel. While there are still coordinates in the queue, the neighboring voxels'
coordinates relative to the current coordinates (last position in queue) are added to the queue and a corresponding struct is created with setVoxel.
As each element in the queue is processed the voxels beneath the current voxel are filled.
*/

void Voxelgrid::FloodFill(int init_x, int height, int init_z){

  std::vector<std::vector<int>> queue;

  if (datahandler->getCoord(init_x, init_z) < height) {
    queue.push_back({init_x, init_z});
    setVoxel(init_x, height, init_z, true, 0, 0);
  }

  int temp_x, temp_z;
  int height_test;
  int terrain_height;

/* While queue is not empty, keep processing queue from back to front.
*/
  while(queue.size() > 0){

    temp_x  = queue.back().at(0);
    temp_z = queue.back().at(1);

    queue.pop_back();

    /* Fill voxels beneath current voxel
    */
    height_test = height;
    terrain_height = (int)datahandler->getCoord(temp_x, temp_z);

    while(height_test > terrain_height && height_test >= 0){

      setVoxel(temp_x, height_test, temp_z, true, 0, 0);
      height_test--;
    }

/* Checking voxels adjacent to current voxel and adding their coordinates to the queue if they are inside the terrain,
above land and have not yet been added to the queue. Before coordina are added the struct is created. Struct existance
(!= nullptr) thus equivalent to voxel added to cue as used in if-statement.
*/

    if(temp_x + 1 < datahandler->getDataWidth() &&
        datahandler->getCoord(temp_x + 1, temp_z) < height &&
          getVoxel(temp_x + 1, height, temp_z) == nullptr){
      setVoxel(temp_x + 1, height, temp_z, true, 0, 0);
      queue.push_back({temp_x + 1, temp_z});
    }
    if(temp_x - 1 >= 0 &&
        datahandler->getCoord(temp_x - 1, temp_z) < height &&
          getVoxel(temp_x - 1, height, temp_z) == nullptr){
      setVoxel(temp_x - 1, height, temp_z, true, 0, 0);
      queue.push_back({temp_x - 1, temp_z});
    }
    if(temp_z < datahandler->getDataWidth() &&
        datahandler->getCoord(temp_x, temp_z + 1) < height &&
          getVoxel(temp_x, height, temp_z + 1) == nullptr){
      setVoxel(temp_x, height, temp_z + 1, true, 0, 0);
      queue.push_back({temp_x, temp_z + 1});
    }
    if(temp_z <= 0 &&
        datahandler->getCoord(temp_x, temp_z - 1) < height &&
          getVoxel(temp_x, height, temp_z - 1) == nullptr){
      setVoxel(temp_x, height, temp_z - 1, true, 0, 0);
      queue.push_back({temp_x, temp_z - 1});
    }
  }
}

std::vector<GLuint> *Voxelgrid::getVoxelPositions() {
	std::vector<GLuint> *positions = new std::vector<GLuint>;

	for (GLuint x = 0; x < voxels->size(); x++) {
		if (voxels->at(x) != nullptr) {
			for (GLuint y = 0; y < voxels->at(x)->size(); y++) {
				if (voxels->at(x)->at(y) != nullptr) {
					for (GLuint z = 0; z < voxels->at(x)->at(y)->size(); z++) {
						if (voxels->at(x)->at(y)->at(z) != nullptr && voxels->at(x)->at(y)->at(z)->filled) {
							positions->push_back(x);
							positions->push_back(y);
							positions->push_back(z);
						}
					}
				}
			}
		}
	}
	return positions;
}

void Voxelgrid::initDraw() {
	voxelPositions = getVoxelPositions();
	numVoxels = voxelPositions->size() / 3;
	voxelShader = loadShadersG("src/shaders/simplevoxels.vert", "src/shaders/simplevoxels.frag", "src/shaders/simplevoxels.geom");

	glGenBuffers(1, &voxelBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, voxelBuffer);
	glBufferData(GL_ARRAY_BUFFER, numVoxels * 3 * sizeof(GLuint), voxelPositions->data(), GL_STATIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, voxelBuffer);
	glGenVertexArrays(1, &voxelVAO);
	glBindVertexArray(voxelVAO);

	GLuint posAttrib = glGetAttribLocation(voxelShader, "posValue");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_UNSIGNED_INT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Voxelgrid::updateVoxelrender() {
	delete voxelPositions;
	voxelPositions = getVoxelPositions();
	numVoxels = voxelPositions->size() / 3;

	glBindBuffer(GL_ARRAY_BUFFER, voxelBuffer);
	glBufferData(GL_ARRAY_BUFFER, numVoxels * 3 * sizeof(GLuint), voxelPositions->data(), GL_STATIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Voxelgrid::drawVoxels(glm::mat4 projectionMatrix, glm::mat4 viewMatrix) {
	glUseProgram(voxelShader);
	glBindBuffer(GL_ARRAY_BUFFER, voxelBuffer);
	glBindVertexArray(voxelVAO);
	glUniformMatrix4fv(glGetUniformLocation(voxelShader, "VTPMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(glGetUniformLocation(voxelShader, "WTVMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));

	if (numVoxels > 0) {
		glDrawArrays(GL_POINTS, 0, numVoxels);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	printError("Voxel Draw Billboards");
}