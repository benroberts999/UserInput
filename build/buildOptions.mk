# Options + settings for makefile for each 'Main'

## will return the Operating system name
detected_OS := $(shell uname -s)
$(info )
$(info Detected operating system: $(detected_OS))

# runs make in //
ifeq ($(Build),debug)
  ParallelBuild=1
endif
ifneq ($(ParallelBuild),)
ifneq ($(ParallelBuild),0)
ifneq ($(ParallelBuild),1)
  MAKEFLAGS += -j$(ParallelBuild)
  $(info Compiling in parallel with N=$(ParallelBuild) cores)
endif
endif
endif
$(info )

#Warnings:
WARN=-Wall -Wpedantic -Wextra -Wdouble-promotion -Wconversion -Wshadow \
-Weffc++ -Wsign-conversion
# -Wfloat-equal

# Changes to warning based on compiler:
ifeq ($(findstring clang++,$(CXX)),clang++)
  WARN += -Wheader-hygiene -Wno-unused-function
# have to put in 'else' block, since clanG++ contains g++
else
ifeq ($(findstring g++,$(CXX)),g++)
  WARN += -Wsuggest-override -Wnon-virtual-dtor -Wcast-align \
  -Woverloaded-virtual -Wduplicated-cond -Wduplicated-branches \
  -Wnull-dereference -Wuseless-cast
endif
endif

# Changes to optimisation based on build setting:
OPT=-O3
ifeq ($(Build),release)
  WARN=-w
  OPT+=-g0 -DNDEBUG
endif

ifeq ($(Build),debug)
  UseOpenMP=no
  WARN+=-Wno-unknown-pragmas
  OPT=-O0 -g3 -fno-omit-frame-pointer
endif

# If not using openMP, turn off 'unkown pragmas' warning.
OMP=-fopenmp
ifneq ($(UseOpenMP),yes)
  OMP=
  WARN+=-Wno-unknown-pragmas
endif

################################################################################
# Linking + Compiling:

CXXFLAGS= $(CXXSTD) $(OPT) $(OMP) $(WARN) -I$(SD)
LIBS=
#LIBS=-lgsl -lgslcblas

# GSL location (if different from assumed default)
ifneq ($(PathForGSL),)
  CXXFLAGS+=-I$(PathForGSL)/include/
  LIBS+=-L$(PathForGSL)/lib/
endif

# Any other "extra" includes:
ifneq ($(ExtraInclude),)
  tmpInc = $(addprefix -I,$(ExtraInclude))
  CXXFLAGS+= $(tmpInc)
endif
ifneq ($(ExtraLink),)
  tmpLink = $(addprefix -L,$(ExtraLink))
  LIBS+= $(tmpLink)
endif
ifneq ($(ExtraFlags),)
  CXXFLAGS+= $(ExtraFlags)
endif

#Command to compile objects and link them
COMP=$(CXX) -MMD -c -o $@ $< $(CXXFLAGS)
ifeq ($(ParallelBuild),1)
	COMP=time $(CXX) -MMD -c -o $@ $< $(CXXFLAGS)
endif
LINK=$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)
