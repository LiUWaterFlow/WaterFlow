###########################################################
###														###
###				          MAKEFILE			  			###
###											  			###
### 		         Project in TSBB11	      			###
###											  			###
###########################################################

CXX=g++
RM=rm -f

CPPFLAGS = -Wall -pedantic -std=c++0x
FLAGS= -lm -DGL_GLEXT_PROTOTYPES

LFLAG= -lGL
WFLAG= -lfreeglut -lglew32 -lopengl32 -L$(COM)/Windows/lib

# Utilities
COM=common
LINC= -I$(COM) -I$(COM)/Linux -Iinc
WINC= -I$(COM) -I$(COM)/Windows -Iinc

# Folders
SRC=src
BIN=bin
NAME=main.cpp

# Compile with
UTILS=$(COM)/*.c $(COM)/*.cpp $(SRC)/*.cpp
LUTILS=$(COM)/Linux/MicroGlut.c $(UTILS)

all: main

main: $(NAME)
	$(CXX) $(CPPFLAGS) -o $(BIN)/WaterFlow $(NAME) $(LUTILS) $(FLAGS) $(LFLAG) $(LINC)
	
clean :
	@ $(RM) $(BIN)/*

