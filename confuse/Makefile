FLAGS = -Wall -std=gnu++0x -O3

LIBS = -lGL -lglut -lGLU -lglui -lGLEW -ljpeg
OBJ = imageloader.o dof.o

all: main.cpp $(OBJ)
	gcc $^ -o main $(LIBS) $(FLAGS)

%.o: %.cpp %.h
	gcc -c $< -o $@ $(FLAGS) $(LIBS)
