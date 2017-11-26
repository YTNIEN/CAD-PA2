
CXX		= g++
LINK		= g++
CXXFLAGS	+=
INCPATH		= 
LIBS		= 
DEL_FILE	= rm -fv

##### Files
HEADERS = pause.h
SOURCES = main.cc
OBJECTS = main.o
TARGET = hw2

##### Implicit rules
.SUFFIXES: .o .cpp .cc .cxx

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

##### Build rules
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LINK) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS) $(INCPATH)

$(OBJECTS): $(HEADERS) Makefile

clean: 
	@$(DEL_FILE) $(OBJECTS) $(TARGET)

