#include <fstream>
#include <iostream>
#include <string>
#include "readData.h"

using namespace std;

double mapdata::getData(int col, int row)
{
	int index;
	if(col < ncols && row < nrows)
	{
		index = col + row * ncols;
	}
	else 
	{
		cerr << "Input does not exist in data." << endl;
		index = 0;
	}
	
	return data[index];
}

mapdata* readDEM(const char* inputfile)
{
	mapdata* retdata = new mapdata();
	
	ifstream infile;
	infile.open(inputfile, ios::in);
	
	string intext;
	double incoord;
	
	if (infile.is_open())
	{
		infile >> intext >> retdata->ncols;
		infile >> intext >> retdata->nrows;
		infile >> intext >> retdata->xllcorner;
		infile >> intext >> retdata->yllcorner;
		infile >> intext >> retdata->cellsize;
		infile >> intext >> retdata->NODATA_value;
	
		int numelem = retdata->ncols * retdata->nrows;
		for (int i = 0; i < numelem; i++)
		{
			infile >> incoord;
			retdata->data.push_back(incoord);
		}
		
    	infile.close();
	}
	else {
		cerr << "Could not open file: " << inputfile << endl;
	}

	return retdata;
}



