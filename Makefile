# Compiler
CC = g++

# Compiler Flags (Manually set GLFW if pkg-config fails)
CFLAGS = -Wall -std=c++17 -I. -Iimgui -Iimgui/backends -I/usr/include

# Libraries (Link GLFW Manually if Needed)
LIBS = -lglfw -lvulkan -ldl -lpthread

# Source Files
SOURCES = \
    src/main.cpp \
    imgui/imgui_demo.cpp \
    src/dice.cpp \
    src/character.cpp \
    imgui/imgui.cpp \
    imgui/imgui_draw.cpp \
    imgui/imgui_widgets.cpp \
    imgui/imgui_tables.cpp \
    imgui/backends/imgui_impl_glfw.cpp \
    imgui/backends/imgui_impl_vulkan.cpp

# Object Files (Convert .cpp -> .o)
OBJECTS = $(SOURCES:.cpp=.o)

# Output Executable
EXECUTABLE = downndirtyDnD

# Default Target: Compile Everything
all: $(EXECUTABLE)

# Linking Step
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o $(EXECUTABLE)

# Compile Each .cpp File to .o
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean Build Files
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
