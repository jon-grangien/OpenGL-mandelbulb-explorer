# Mandelbulb renderer
Render the mandelbulb (mandelbrot 3D) fractal patterns interactively, with shadow mapping and SSAO.

## Installing
This project uses glfw as a dependency, managed as a git submodule to minimize overhead

```sh
git clone <this-repo> --recursive
```

Glfw needs to be built for the hardware:  
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

