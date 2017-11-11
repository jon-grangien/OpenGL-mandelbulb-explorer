# Mandelbulb renderer
Render the mandelbulb (mandelbrot 3D) fractal patterns interactively, with shadow mapping and SSAO.

## Environment
Build for Linux and tested on Arch Linux. Mac support limited. Windows not implemented due to lack of interest.

## Dependencies and installing
- OpenGL
- GLFW (included)
- GLEW
- GLM

To get GLFW, clone the repository recursively

```sh
git clone <this-repo> --recursive
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

