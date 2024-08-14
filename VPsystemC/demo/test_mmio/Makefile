# This Makefile will assume that these two command
# export SYSTEMC=/path/to/install/systemc
# export ARCH=linux64 (or other computer architecture)
# have been put into .bashrc

# for compilation
CXXFLAGS = -MMD -MP
CPPFLAGS = -I$(SYSTEMC_ROOT)/include -I ../../
LDFLAGS  = -L$(LD_LIBRARY_PATH) -Wl,-rpath,$(LD_LIBRARY_PATH)
LDLIBS = -lsystemc
# for project
SRCS = $(wildcard *.cpp) ../../vpipc/vpipc.cpp
OBJS = $(patsubst %.cpp, %.o, $(SRCS))
DEPS = $(patsubst %.cpp, %.d, $(SRCS))
# phony target name
.PHONY = all debug release clean help

all: debug ## compile in debug mode by default

debug: CXXFLAGS += -g -Wall -Werror -O0 -DDEBUG
debug: main ## debug mode setup

release: CXXFLAGS += -O2
release: main ## release mode setup

main: $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) $^ -o main

-include $(DEPS)

clean: ## clean all file except for source codes
	$(RM) -rv main $(OBJS) $(DEPS)

help:
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-15s\033[0m %s\n", $$1, $$2}'
