all: simulation

simulation: main.cpp fluidsolver.h fluidsolver.cpp
	g++ -g -std=c++0x -pedantic -Wall -Wextra -Werror -o simulation fluidsolver.h fluidsolver.cpp main.cpp

clean:
	rm simulation
