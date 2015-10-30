SOURCES	    :=  $(shell find src -name '*.cpp') $(shell find src -name '*.c')
OBJECTS	    :=  $(subst .c,.o,$(subst .cpp,.o,$(subst src/,build/,$(SOURCES))))
DIRECTORIES :=  $(sort $(dir $(OBJECTS)))

CXX				= g++
CC				= gcc
CFLAGS			= -O2 -g -fmessage-length=0
INCLUDE_PATHS	= -I../../lib/common -I../../lib/parse -I../../lib/parse_ucs -I../../lib/parse_expression -I../../lib/parse_astg -I../../lib/parse_dot -I../../lib/parse_chp -I../../lib/ucs -I../../lib/boolean -I../../lib/petri -I../../lib/hse -I../../lib/interpret_ucs -I../../lib/interpret_boolean -I../../lib/interpret_hse
LIBRARY_PATHS	= -L../../lib/common -L../../lib/parse -L../../lib/parse_ucs -L../../lib/parse_expression -L../../lib/parse_astg -L../../lib/parse_dot -L../../lib/parse_chp -L../../lib/ucs -L../../lib/boolean -L../../lib/petri -L../../lib/hse -L../../lib/interpret_ucs -L../../lib/interpret_boolean -L../../lib/interpret_hse
LIBRARIES		= -linterpret_hse -linterpret_boolean -linterpret_ucs -lhse -lpetri -lboolean -lucs -lparse_chp -lparse_astg -lparse_dot -lparse_expression -lparse_ucs -lparse -lcommon
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
