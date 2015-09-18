###########################################################
###														###
###				          MAKEFILE			  			###
###											  			###
### 		         Project in TSBB11	      			###
###											  			###
###########################################################

CXX=g++
RM=rm -f

ifeq ($(shell uname), Darwin)
  PLATFORM=mac
endif

ifeq ($(shell uname), Linux)
  PLATFORM=linux
endif


# Utilities
COM=common

# Folders
SRC=src
BIN=bin
NAME=main.cpp


#windows settings
FLAGS = -lm -DGL_GLEXT_PROTOTYPES -lfreeglut -lglew32 -lopengl32 -L$(COM)/Windows/lib
INC = -I$(COM) -I$(COM)/Windows -Iinc

# Compile with
UTILS=$(COM)/*.c $(COM)/*.cpp $(SRC)/*.cpp
MICRO = $(COM)/Windows/MicroGlut.c


LUTILS=	MicroGlut.o $(UTILS)

#mac settings
ifeq ($(PLATFORM),mac)
#Mac frameworks
FLAGS = -framework GLUT -framework OpenGL -framework Cocoa
MICRO = $(COM)/mac/MicroGlut.m
INC = -I$(COM) -I$(COM)/mac -Iinc

endif

#linux settings
ifeq ($(PLATFORM),linux)

FLAGS= -lGL -lm -DGL_GLEXT_PROTOTYPES
#-lXt -lX11
INC = -I$(COM) -I$(COM)/Linux -Iinc
MICRO = $(COM)/Linux/MicroGlut.c

endif

CPPFLAGS = -Wall -pedantic -std=c++0x


all: main

main: $(NAME) MicroGlut.o
	$(CXX) $(CPPFLAGS) -o $(BIN)/WaterFlow $(NAME) $(LUTILS) $(FLAGS) $(INC) $(MIRCO)

MicroGlut.o: $(MICRO)
	$(CXX) -c $(MICRO)

clean :
	@ $(RM) $(BIN)/*
