ifeq ($(shell uname), Darwin)
	PLATFORM=mac
	ANT=ant
endif
ifeq ($(shell uname), Linux)
	PLATFORM=linux
endif
$SOURCEDIR = src

CPP_FILES := $(wildcard src/*.cpp)
CPP_FILES += src/common/xml/pugixml.cpp

OBJ_FILES_CPP := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))


C_FILES_COMMON := $(wildcard src/common/*.c)

CPP_FILES_COMMON := $(wildcard src/common/*.cpp)


OBJ_FILES_COMMON := $(addprefix obj/,$(notdir $(C_FILES_COMMON:.c=.o)))
OBJ_FILES_COMMON += $(addprefix obj/,$(notdir $(CPP_FILES_COMMON:.cpp=.o)))


INC = -Isrc/ -Isrc/common/ -Isrc/common/glm -Isrc/common/AntTweak -Isrc/common/xml
MAIN = main.cpp
OUT = bin/WaterFlow

ifeq ($(PLATFORM),mac)
INC += -F/Library/Frameworks/
LINKS =  -framework OpenGL -framework GLUT -Isrc/common/xml -framework Cocoa -framework SDL2 -L/usr/local/lib -lAntTweakBar
endif

GCC = g++ -Wall -pedantic -std=c++0x

#if Linux
ifeq ($(PLATFORM),linux)
INC += $(shell pkg-config --cflags sdl2)
LINKS = $(shell pkg-config --libs sdl2)
LINKS += -lGL -lGLU -lglut -lm -lXt -lX11 -Wl,-rpath,./lib/AntTweakLinux -Llib/AntTweakLinux -lAntTweakBar  
GCC += -std=gnu++0x -Dnullptr=NULL
endif



all: $(OBJ_FILES_COMMON) $(OBJ_FILES_CPP)
	mkdir -p $(CURDIR)/bin
	$(GCC) obj/*.o $(MAIN) -o $(OUT) $(INC) $(LINKS)

obj/%.o : src/common/%.c
	mkdir -p $(CURDIR)/obj
	$(GCC) -c -o $@ $< $(INC)

obj/%.o : src/common/%.cpp
	mkdir -p $(CURDIR)/obj
	$(GCC) -c -o $@ $< $(INC)

obj/%.o : src/%.cpp
	mkdir -p $(CURDIR)/obj
	$(GCC) -c -o $@ $< $(INC)

obj/%.o : src/common/xml/%.cpp
	$(GCC) -c -o $@ $< $(INC)

TWtest:
	$(GCC) TwSimpleSDL.c -o $(OUT) $(INC) $(LINKS)

clean :
	rm obj/*.o

docs:
	doxygen resources/Doxyfile
