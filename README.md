# Mandelbulb renderer
Render the mandelbulb fractal patterns interactively, with PostFX.

![screenshot](https://github.com/codingInSpace/OpenGL-mandelbulb-explorer/blob/master/screenshots/purple_with_gui.png)

## Usage
```
Usage: ./mandelbulb -c
Options:
	-h,--help		Show this message
	-c,--coordinates 	Log coordinates in console every frame 

Controls:
	Q 	Quit the program
	L 	Reload shaders
	WASD 	Movement around center
	Z 	Zoom in
	X 	Zoom out
	R 	Reset position

```

## Environment
Built for Linux and tested on Arch Linux. Mac support limitedly implemented. Windows not implemented due to lack of interest. PR required.

## Dependencies and installing
- OpenGL 3.3+ (GLSL 330)
- GLFW (included submodule)
- GLEW
- GLM
- imgui (included submodule)

To get the applied versions of GLFW and imgui, *clone this repository recursively*

```sh
git clone git@github.com:codingInSpace/OpenGL-mandelbulb-explorer.git --recursive
```

GLFW needs to be built for the hardware:
```sh
cd ext/glfw
cmake .
make
```

If you didn't clone the repo recursively you can get the submodules like this:
```sh
git submodule update --init --recursive
```

## Running
```sh
cd build
cmake ..
make
./mandelbulb
```

Or use CLion with working directory as build folder (run configurations, edit, set working directory)

## License
MIT

