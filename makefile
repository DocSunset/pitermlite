IDIR = include
SDIR = src

# note to self: $(patsubst pattern, replacement, texttosearch)
FILES = PiTermLite PiTracker PingPong
HEADERS = $(patsubst %, $(IDIR)/%.h, $(FILES))
OBJECTS = $(patsubst %, $(SDIR)/%.o, $(FILES))

CXX = g++
CXXFLAGS = -Wall -std=c++14
INCLUDES = -I$(IDIR) 
LIBS = -lusb-1.0

# note to self: 
# $@ is the name of the file being made e.g. foo.o
# $< is the first prerequisite e.g. foo.cpp
# $^ is all the dependencies of the rule e.g. foo.cpp $(HEADERS)
$(SDIR)/%.o: $(SDIR)/%.cpp $(HEADERS)
	$(CXX) -c -o $@ $< $(INCLUDES) $(CXXFLAGS)

pitermlite: $(OBJECTS)
	$(CXX) -o $@ $^ $(INCLUDES) $(CXXFLAGS) $(LIBS) 

echoheaders:
	echo $(HEADERS)

echoobjects:
	echo $(OBJECTS)
