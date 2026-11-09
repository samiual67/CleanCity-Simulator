# Linux / MSYS2 / MinGW build.  Needs freeglut + OpenGL.
CXX      ?= g++
CXXFLAGS ?= -std=c++11 -O2 -Wall -Iinclude
SRC      := $(wildcard src/*.cpp)
ifeq ($(OS),Windows_NT)
  LIBS := -lfreeglut -lopengl32 -lglu32
  TARGET := CleanCitySimulator.exe
else
  LIBS := -lglut -lGL -lGLU
  TARGET := CleanCitySimulator
endif
$(TARGET): $(SRC) $(wildcard include/*.h)
	$(CXX) $(CXXFLAGS) $(SRC) -o $@ $(LIBS)
clean:
	rm -f CleanCitySimulator CleanCitySimulator.exe
