#include <vector>

struct mapdata {
	int	ncols;
	int nrows;
	double xllcorner;
	double yllcorner;
	double cellsize;
	double NODATA_value;
	std::vector<double> data; 
	
	double getData(int col, int row);
};

mapdata* readDEM(const char* inputfile);
