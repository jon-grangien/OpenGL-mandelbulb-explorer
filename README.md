# Mandelbulb renderer
Render the mandelbulb fractal patterns interactively, with shadow mapping and SSAO.

## Usage
```bash
Usage: ./mandelbulb -p -g 1
Options:
	-h,--help		Show this message
	-g,--graphics 		Specify graphics setting: 0 low, 1 medium (default), 2 high
	-p,--performance 	Log performance measures every frame during run
	-c,--coordinates 	Log coordinates every frame during run

```

## Environment
Built for Linux and tested on Arch Linux. Mac support limitedly implemented. Windows not implemented due to lack of interest. PR required.

## Dependencies and installing
- OpenGL 3.3+ (GLSL 330)
- GLFW (included submodule)
- GLEW
- GLM

To install the applied version of GLFW, clone the repository recursively

```sh
git clone <this-repo> --recursive
```

GLFW needs to be built for the hardware:
```sh
cd ext/glfw
cmake .
make
```

If you didn't clone the repo recursively you can get the submodule like this:
```sh
git submodule update --init --recursive
```

## Running
```sh
cd build
cmake ..
make
./mandelbulb -g 2
```

Or use CLion with working directory as build folder (run configurations, edit, set working directory)

## License
MIT

