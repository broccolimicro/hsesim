SOURCES	    :=  $(shell find src -name '*.cpp') $(shell find src -name '*.c')
OBJECTS	    :=  $(subst .c,.o,$(subst .cpp,.o,$(subst src/,build/,$(SOURCES))))
DIRECTORIES :=  $(sort $(dir $(OBJECTS)))

CXX				= g++
CC				= gcc
CFLAGS			= -O2 -g -fmessage-length=0
INCLUDE_PATHS	= -I../../lib/common -I../../lib/parse -I../../lib/parse_boolean -I../../lib/parse_dot -I../../lib/parse_hse -I../../lib/boolean -I../../lib/hse -I../../lib/interpret_boolean -I../../lib/interpret_hse -I../../lib/interpret_dot
LIBRARY_PATHS	= -LC:\MinGW\bin -L../../lib/common -L../../lib/parse -L../../lib/parse_boolean -L../../lib/parse_dot -L../../lib/parse_hse -L../../lib/boolean -L../../lib/hse -L../../lib/interpret_boolean -L../../lib/interpret_hse -L../../lib/interpret_dot
LIBRARIES		= -linterpret_dot -linterpret_hse -linterpret_boolean -lhse -lboolean -lparse_hse -lparse_dot -lparse_boolean -lparse -lcommon -lcgraph -lgvc
TARGET			= hsesim

all: build $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LIBRARY_PATHS) $(CFLAGS) $(OBJECTS) -o $(TARGET) $(LIBRARIES)

build/%.o: src/%.cpp 
	$(CXX) $(INCLUDE_PATHS) $(CFLAGS) -c -o $@ $<

build/%.o: src/%.c 
	$(CC) $(INCLUDE_PATHS) $(CFLAGS) -c -o $@ $<

build:
	mkdir $(DIRECTORIES)

clean:
	rm -f $(OBJECTS) $(TARGET) $(TARGET).exe
