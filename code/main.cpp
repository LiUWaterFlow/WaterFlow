/// @file main.cpp
/// @brief Simple main file, most things are located in Program.h
///
/// The code below is based upon Ingemar Ragnemalm's code provided for the
/// course TSBK03 at Link�ping University. The link to the original shell is
/// http://www.ragnemalm.se/lightweight/psychteapot+MicroGlut-Windows-b1.zip
/// which according to the web site was updated 2015-08-17.
///
/// SDL functions written by Gustav Svensk, acquired with permissions from
/// https://github.com/DrDante/TSBK03Project/ 2015-09-24. Some related code
///
/// Notes:
/// * Use glUniform1fv instead of glUniform1f, since glUniform1f has a bug under Linux.

#include "program.h"

int main(int argc, char *argv[]) {

	std::cout << "Which simulation do you want to run?\n";
	std::cout << "1. Heightfield based water simulation (with xml specified data) \n";
	std::cout << "2. Navier-Stokes based shallow water simulation on a predefined testcase. (showcase demo). \n";
	int choice;
	
	std::cin >> choice;

	if(choice != 1 && choice != 2)
	{
		std::cout << "Invalid option" << std::endl;
		return -1;
	}
	
	Program program(choice);
	
	return program.exec();
}
