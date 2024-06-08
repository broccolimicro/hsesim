NAME          = hsesim
DEPEND        = interpret_hse interpret_boolean interpret_ucs hse petri boolean ucs parse_chp parse_astg parse_dot parse_expression parse_ucs parse common

SRCDIR        = src
TESTDIR       = tests
GTEST        := ../../googletest
GTEST_I      := -I$(GTEST)/googletest/include -I.
GTEST_L      := -L$(GTEST)/build/lib -L.

INCLUDE_PATHS = $(DEPEND:%=-I../../lib/%) -I.
LIBRARY_PATHS = $(DEPEND:%=-L../../lib/%) -I.
LIBRARIES     = $(DEPEND:%=-l%)
CXXFLAGS	    = -std=c++14 -O2 -g -Wall -fmessage-length=0
LDFLAGS		    =  

SOURCES	     := $(shell mkdir -p $(SRCDIR); find $(SRCDIR) -name '*.cpp')
OBJECTS	     := $(SOURCES:%.cpp=build/%.o)
DEPS         := $(shell mkdir -p build/$(SRCDIR); find build/$(SRCDIR) -name '*.d')
TARGET		    = $(NAME)

TESTS        := $(shell mkdir -p tests; find $(TESTDIR) -name '*.cpp')
TEST_OBJECTS := $(TESTS:%.cpp=build/%.o) build/$(TESTDIR)/gtest_main.o
TEST_DEPS    := $(shell mkdir -p build/$(TESTDIR); find build/$(TESTDIR) -name '*.d')
TEST_TARGET   = test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LIBRARY_PATHS) $(CXXFLAGS) $(OBJECTS) -o $(TARGET) $(LIBRARIES)

build/$(SRCDIR)/%.o: $(SRCDIR)/%.cpp 
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) $(INCLUDE_PATHS) -MM -MF $(patsubst %.o,%.d,$@) -MT $@ -c $<
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(INCLUDE_PATHS) -c -o $@ $<

$(TEST_TARGET): $(TEST_OBJECTS) $(filter-out build/$(SRCDIR)/main.o, $(OBJECTS))
	$(CXX) $(LIBRARY_PATHS) $(CXXFLAGS) $(GTEST_L) $(INCLUDE_PATHS) $^ -pthread -lgtest -o $(TEST_TARGET) $(LIBRARIES)

build/$(TESTDIR)/%.o: $(TESTDIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(GTEST_I) $(INCLUDE_PATHS) -MM -MF $(patsubst %.o,%.d,$@) -MT $@ -c $<
	$(CXX) $(CXXFLAGS) $(GTEST_I) $(INCLUDE_PATHS) $< -c -o $@

build/$(TESTDIR)/gtest_main.o: $(GTEST)/googletest/src/gtest_main.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(GTEST_I) $< -c -o $@

include $(DEPS) $(TEST_DEPS)

clean:
	rm -rf build $(TARGET) $(TEST_TARGET)
