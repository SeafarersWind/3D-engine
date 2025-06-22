BUILD_NAME := game.exe
SRC_NAME := main.c

BUILD_DIR := 
SRC_DIR := 

BUILD := $(BUILD_DIR)$(BUILD_NAME)
SRC := $(SRC_DIR)$(SRC_NAME)

$(BUILD): $(SRC) $(BUILD_DIR)
	@gcc $(SRC) -o $(BUILD) -Iinclude -Iinclude/GLFW/include -Iinclude/GLFW/deps glad_gl.c -Linclude/GLFW/build -lglfw3 -lgdi32 && \
	echo $(BUILD_NAME) rebuilt!

$(BUILD_DIR):
	@mkdir $@

include/GLFW/build:
	cd include/GLFW && \
	gcc -c src/context.c src/init.c src/win32_thread.c src/window.c src/input.c src/win32_monitor.c src/monitor.c src/vulkan.c src/win32_init.c src/win32_window.c src/win32_time.c src/win32_joystick.c src/wgl_context.c src/egl_context.c src/osmesa_context.c -I include -I deps -I deps/glad -I deps/mingw -D_GLFW_WIN32 && rd /s /q build && mkdir build && mv *.o build && ar rcs build/libglfw3.a build/*.o && \
	echo GLFW rebuilt!

.PHONY: clean
clean:

.PHONY: play
play: $(BUILD)
	@$(BUILD)
