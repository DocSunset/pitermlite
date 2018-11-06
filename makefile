IDIR = include
SDIR = src
TDIR = test

# note to self: $(patsubst pattern, replacement, texttosearch)
FILES = PiTermLite PiTracker PingPong
HEADERS = $(patsubst %, $(IDIR)/%.h, $(FILES))
OBJECTS = $(patsubst %, $(SDIR)/%.o, $(FILES))
TESTS = parseArgs

CXX = g++
CXXFLAGS = -Wall -std=c++14
INCLUDES = -I$(IDIR) 
LIBS = -lusb-1.0 -lpthread

# note to self: 
# $@ is the name of the file being made e.g. foo.o
# $< is the first prerequisite e.g. foo.cpp
# $^ is all the dependencies of the rule e.g. foo.cpp $(HEADERS)
%.o: %.cpp $(HEADERS)
	$(CXX) -c -o $@ $< $(INCLUDES) $(CXXFLAGS)

pitermlite: $(SDIR)/main.o $(OBJECTS)
	$(CXX) -o $@ $^ $(INCLUDES) $(CXXFLAGS) $(LIBS) 

$(TDIR)/%.test: $(TDIR)/%.cpp $(OBJECTS)
	$(CXX) -g -O0 -o $@ $^ $(INCLUDES) $(CXXFLAGS) $(LIBS) 

.Phony: clean

clean:
	rm $(SDIR)/*.o
	rm $(TDIR)/*.test
	rm pitermlite
