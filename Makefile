default: linux

CXX = g++

BUILD_LINUX = build/linux

BUILD_WINDOWS = build/windows

OBJ = graphGeneration.o\
	  textProcessing.o\
	  treeAlgorithms.o\
	  userInput.o\
	  glfwCallbacks.o\
	  graphicAlgorithms.o\
	  turnComputeAlgorithms.o\
	  statusAlgorithms.o main.o


FLAGS_LINUX = -lGLEW -lGL -lglfw -lfreetype -I /usr/include/freetype2

FLAGS_WINDOWS = -lGLEW -lGL -lglfw -lfreetype -I /usr/include/freetype2

OBJ_LINUX := $(foreach obj,$(OBJ),$(BUILD_LINUX)/$(obj))

OBJ_WINDOWS := $(foreach obj,$(OBJ),$(BUILD_WINDOWS)/$(obj))

clean:
	rm build/linux/*.o build/windows/*.o

linux: $(OBJ_LINUX)
	$(CXX) -o build/linux/gggame $(OBJ_LINUX) $(FLAGS_LINUX)

windows: $(OBJ_WINDOWS)
	$(CXX) -o build/windows/gggame $(OBJ_WINDOWS) $(FLAGS_WINDOWS)


$(BUILD_LINUX)/%.o: %.cpp
	$(CXX) -c -o $@ $< $(FLAGS_LINUX)

$(BUILD_WINDOWS)/%.o: %.cpp
	$(CXX) -c -o $@ $< $(FLAGS_WINDOWS)
