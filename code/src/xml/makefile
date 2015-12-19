IDIR = ../common/xml
CC = g++
CFLAGS = -I$(IDIR) -std=c++0x

make: testing.cpp xmlParsing.cpp flowSource.cpp $(IDIR)/pugixml.cpp
	g++ -o test testing.cpp xmlParsing.cpp flowSource.cpp $(IDIR)/pugixml.cpp $(CFLAGS)
  clean:
	$(RM) test
