IDIR =../../lib/PugiXML/src
CC = g++
CFLAGS = -I$(IDIR) -std=c++0x

make: xmlParsing.cpp ../../lib/PugiXML/src/pugixml.cpp 
	g++ -o test xmlParsing.cpp ../../lib/PugiXML/src/pugixml.cpp $(CFLAGS)
