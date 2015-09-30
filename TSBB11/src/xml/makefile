IDIR = ../common/xml
CC = g++
CFLAGS = -I$(IDIR) -std=c++0x

make: xmlParsing.cpp flowSource.cpp $(IDIR)/pugixml.cpp
	g++ -o test xmlParsing.cpp flowSource.cpp $(IDIR)/pugixml.cpp $(CFLAGS)
  clean:
	$(RM) test
