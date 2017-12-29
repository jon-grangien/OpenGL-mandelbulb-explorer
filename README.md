# Mandelbulb renderer
Render the mandelbulb fractal patterns interactively, with PostFX.

![full screenshot](https://github.com/codingInSpace/OpenGL-mandelbulb-explorer/blob/master/screenshots/full_shot2.png)
<img src="https://github.com/codingInSpace/OpenGL-mandelbulb-explorer/blob/master/screenshots/2017-12-29-115615_1920x1080_scrot.png" width="400">
<img src="https://github.com/codingInSpace/OpenGL-mandelbulb-explorer/blob/master/screenshots/2017-12-29-183140_1920x1080_scrot.png" width="450">
<img src="https://github.com/codingInSpace/OpenGL-mandelbulb-explorer/blob/master/screenshots/2017-12-29-120439_1920x1080_scrot.png" width="300">
<img src="https://github.com/codingInSpace/OpenGL-mandelbulb-explorer/blob/master/screenshots/2017-12-29-182019_1920x1080_scrot.png" width="300">

## Usage
```
Usage: ./mandelbulb -c
Options:
	-h,--help		Show this message
	-w,--weak 		Lower settings for weak computer i.e. shitty Intel HD graphics laptop
	-c,--coordinates 	Log coordinates in console every frame 

Controls:
	Q 	Quit the program
	L 	Reload shaders
	WASD 	Movement around center
	Z 	Zoom in
	X 	Zoom out
	R 	Reset position

```

Tick the "FREE MODE" box to enter free roaming mode, where wasd changes view direction relative to position.

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
cmake .. -DCMAKE_BUILD_TYPE=Release
make
./mandelbulb
```

Or use CLion with working directory as build folder (run configurations, edit, set working directory)

## License
MIT

