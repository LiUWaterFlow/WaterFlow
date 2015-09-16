ifeq ($(shell uname), Darwin)
  PLATFORM=mac
endif

ifeq ($(shell uname), Linux)
  PLATFORM=linux
endif
ifeq ($(PLATFORM),mac)


#Mac frameworks
LDFLAGS = -framework GLUT -framework OpenGL -framework Cocoa
MICRO = common/mac/MicroGlut.m
INC = -I common/mac -I common/

endif

ifeq ($(PLATFORM),linux)
LDFLAGS = -lGL -lm -DGL_GLEXT_PROTOTYPES -lXt -lX11
MICRO = common/Linux/MicroGlut.c
INC = -I common/Linux
endif


#Compiler flags, enable warnings and c99 standard
G++FLAGS = -Wall -std=c++0x $(INC)
GCCFLAGS = -Wall $(INC)
#SOURCES that are needed for all sub-compilations
SOURCES = common/GL_utilities.c common/VectorUtils3.c common/LoadTGA.c common/loadobj.c
OBJECTS = GL_utilities.o VectorUtils3.o LoadTGA.o loadobj.o MicroGlut.o


all: main
#target: dependencies
#’tab’ 	compilation commands


main: main.cpp $(OBJECTS)
	g++ $(G++FLAGS) $(OBJECTS) main.cpp -o flow.out $(LDFLAGS)

$(OBJECTS):
	g++ -c $(GCCFLAGS) $(SOURCES) $(LDFLAGS)

#compile MicroGlut on it’s own and ignore warnings.
MicroGlut.o: $(MICRO)
	g++ -w -c $(MICRO)

clean:
	rm *.o

system:
	printf $(shell uname)
