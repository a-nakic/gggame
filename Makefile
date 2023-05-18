CXX = g++

OBJ = graphGeneration.o\
	  textProcessing.o\
	  treeAlgorithms.o\
	  userInput.o\
	  glfwCallbacks.o\
	  graphicAlgorithms.o\
	  turnComputeAlgorithms.o\
	  statusAlgorithms.o main.o

FLAGS_LINUX = -lGLEW -lGL -lglfw -lfreetype -I /usr/include/freetype2 -I include

FLAGS_WWINDOWSS = -lGLEW -lGL -lglfw -lfreetype -I /usr/include/freetype2 -I include

clean:
	rm *.o

linux: $(OBJ)
	$(CXX) -o build/linux/gggame $(OBJ) $(FLAGS_LINUX)

windows: $(OBJ)
	$(CXX) -o build/windows/gggame $(OBJ) $(FLAGS_WINDOWS)

graphGeneration.o: graphGeneration.cpp
	$(CXX) -c -o $@ $< $(FLAGS_LINUX)
textProcessing.o: textProcessing.cpp
	$(CXX) -c -o $@ $< $(FLAGS_LINUX)
treeAlgorithms.o: treeAlgorithms.cpp
	$(CXX) -c -o $@ $< $(FLAGS_LINUX)
userInput.o: userInput.cpp
	$(CXX) -c -o $@ $< $(FLAGS_LINUX)
glfwCallbacks.o: glfwCallbacks.cpp
	$(CXX) -c -o $@ $< $(FLAGS_LINUX)
graphicAlgorithms.o: graphicAlgorithms.cpp
	$(CXX) -c -o $@ $< $(FLAGS_LINUX)
turnComputeAlgorithms.o: turnComputeAlgorithms.cpp
	$(CXX) -c -o $@ $< $(FLAGS_LINUX)
statusAlgorithms.o: statusAlgorithms.cpp
	$(CXX) -c -o $@ $< $(FLAGS_LINUX)
main.o: main.cpp
	$(CXX) -c -o $@ $< $(FLAGS_LINUX)
