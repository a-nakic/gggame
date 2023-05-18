default: main

CXX = g++

OBJ = graphGeneration.o textProcessing.o treeAlgorithms.o userInput.o glfwCallbacks.o graphicAlgorithms.o turnComputeAlgorithms.o statusAlgorithms.o main.o

FLAGS = -lGLEW -lGL -lglfw -lfreetype -I /usr/include/freetype2 -I include

clean:
	rm *.o main

main: $(OBJ)
	$(CXX) -o main $(OBJ) $(FLAGS)

graphGeneration.o: graphGeneration.cpp
	$(CXX) -c -o $@ $< $(FLAGS)
textProcessing.o: textProcessing.cpp
	$(CXX) -c -o $@ $< -I textProcessing.hpp $(FLAGS)
treeAlgorithms.o: treeAlgorithms.cpp
	$(CXX) -c -o $@ $< -I treeAlgorithms.hpp $(FLAGS)
userInput.o: userInput.cpp
	$(CXX) -c -o $@ $< -I userInput.hpp $(FLAGS)
glfwCallbacks.o: glfwCallbacks.cpp
	$(CXX) -c -o $@ $< -I glfwCallbacks.hpp $(FLAGS)
graphicAlgorithms.o: graphicAlgorithms.cpp
	$(CXX) -c -o $@ $< -I graphicAlgorithms.hpp $(FLAGS)
turnComputeAlgorithms.o: turnComputeAlgorithms.cpp
	$(CXX) -c -o $@ $< -I turnComputeAlgorithms.hpp $(FLAGS)
statusAlgorithms.o: statusAlgorithms.cpp
	$(CXX) -c -o $@ $< -I statusAlgorithms.hpp $(FLAGS)


main.o: main.cpp
	$(CXX) -c -o $@ $< $(FLAGS)
