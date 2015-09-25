$SOURCEDIR = src

CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES_CPP := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))


C_FILES_COMMON := $(wildcard src/common/*.c)

OBJ_FILES_COMMON := $(addprefix obj/,$(notdir $(C_FILES_COMMON:.c=.o)))

INC = -Isrc/ -Isrc/common/ -Isrc/common/glm -F/Library/Frameworks/
MAIN = main.cpp
OUT = bin/WaterFlow

#if MAC
LINKS =  -framework OpenGL -framework GLUT -framework Cocoa -framework SDL2

GCC = g++ -Wall -pedantic -std=c++0x
CC = gcc

all: $(OBJ_FILES_COMMON) $(OBJ_FILES_CPP)
	$(GCC) obj/*.o $(MAIN) -o $(OUT) $(INC) $(LINKS)

obj/%.o : src/common/%.c
	$(GCC) -c -o $@ $< $(INC)

obj/%.o : src/%.cpp
	$(GCC) -c -o $@ $< $(INC)

clean :
	rm obj/*.o
