IDIR = include
SDIR = src
TDIR = test

# note to self: $(patsubst pattern, replacement, texttosearch)
NAMES = PiTermLite PiTracker PingPong
HEADERS = $(patsubst %, $(IDIR)/%.h, $(NAMES))
OBJECTS = $(patsubst %, $(SDIR)/%.o, $(NAMES))
TESTNAMES = argumentsParser commandParser writeThread
TESTS = $(patsubst %, $(TDIR)/%.test, $(TESTNAMES))

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

.Phony: clean test runtests

clean:
	rm $(SDIR)/*.o
	rm $(TDIR)/*.test
	rm pitermlite

$(TDIR)/%.test: $(TDIR)/%.cpp $(OBJECTS)
	$(CXX) -o $@ $^ $(INCLUDES) $(CXXFLAGS) $(LIBS) 

runtests:
	for i in $(TESTS); do echo Running $$i; if ! ./$$i; then exit 1; fi; done

test: $(TESTS) runtests

