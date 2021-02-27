## Introduction

This is a simple 3D file viewer. Currently, only .obj files are supported but more formats will be added in the future.

## Build Instructions

### Linux

#### Install Vulkan SDK

The first step is to download and install the latest Vulkan SDK from the LunarG web site: https://vulkan.lunarg.com/sdk/home

To ensure that Vulkan is working with your hardware, use the vulkaninfo command to pull up relevant information about your system:

```console
$ vulkaninfo
```

#### Install Cmake

```console
$ sudo apt install cmake
```

Once cmake is installed, ensure that its version is 3.14 or higher:

```console
$ cmake --version
cmake version 3.16.3
```

#### Install GLFW Dependencies

Vega links against GLFW library, an API for creating windows, contexts and surfaces, reading input, handling events, etc.

##### Linux and X11

 To compile GLFW for X11, you need to have the X11 packages installed. For example, on Ubuntu and other distributions based on Debian GNU/Linux, you need to install the xorg-dev package:

```console
$ sudo apt install xorg-dev
```

##### Linux and Wayland

 To compile GLFW for Wayland, you need to have the wayland packages installed. For example, on Ubuntu and other distributions based on Debian GNU/Linux, you need to install the libwayland-dev package:

```console
$ sudo apt install libwayland-dev
```

#### Install GCC

Vega uses some C++20 features and thus requires gcc version 10 or higher:

```console
$ sudo apt install gcc-10 g++-10
```

#### Install Git

```console
$ sudo apt install git
```

#### Download Source Files

In a directory of your choosing execute:

```console
$ git clone https://github.com/guybrush77/Vega.git
```

#### Generate Build Files

Use cmake to generate build files:

```console
$ cd Vega
$ mkdir build && cd build
$ cmake -DCMAKE_C_COMPILER=gcc-10 -DCMAKE_CXX_COMPILER=g++-10 -DCMAKE_BUILD_TYPE=Release ..
```

Once the cmake command is executed, cmake will download the sources of all the dependencies and generate build files. This step can take some time.

#### Generate an Executable

Assuming that cmake generated the build files without errors, we can finally build the Vega executable:

```console
$ make -j4
```

This command will start a parallel build process using 4 CPU cores. If your machine has more than 4 cores, you can increase this number to speed up the building process.

If the build completed without errors, you can now run the executable:

```console
$ cd src/vega
$ ./vega
```
