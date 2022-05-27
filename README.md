# FVKCore
[![Linux Build](https://github.com/voldien/fvkcore/actions/workflows/linux-build.yml/badge.svg)](https://github.com/voldien/fvkcore/actions/workflows/linux-build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![GitHub release](https://img.shields.io/github/release/voldien/fvkcore.svg)](https://github.com/voldien/fvkcore/releases)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/voldien/fvkcore.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/voldien/fvkcore/context:cpp)

**FVKCore** is a simple library for handling much of the core functionalities associated with a Vulkan-based program. Like Creation of Vulkan Instance, Physical Device, Device Object and etc. Furthermore, with a set of helper methods commonly used for improving the development process.

## Motivation

Create a dedicated library for handling the construction of both instance, physical device and logical device. Additionally, method associated with each object.

## Installation

First clone the project followed by updating the git submodules used in this project, which are required in order to compile the program.

```bash
git clone <this repo url>
cd <git workspace>
git submodule update --init --recursive
```

Afterward, it is as simple as follow the following commands.

```bash
mkdir build && cd build
cmake ..
make
```

## Integration with CMake

The idea is to be able to integrate this library with another project easily. With CMake, it basically requires 2 lines. One for adding the project and the second for adding it as a dependent linked library target.

```cmake
ADD_SUBDIRECTORY(fvkcore EXCLUDE_FROM_ALL)
```

```cmake
TARGET_LINK_LIBRARIES(myTarget PUBLIC fvkcore)
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
