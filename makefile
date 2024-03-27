# Define compiler
CXX = g++

# Define any compile-time flags
CXXFLAGS = -Wall -g

# Define the source files
SOURCES = Email.cpp User.cpp System.cpp myserver.cpp 

# Define the object files from the source files
OBJECTS = $(SOURCES:.cpp=.o)

# Define the output binary
EXECUTABLE = myserver

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
