![Banner](banner.png)
# Simple and flexible version control and code syncing
# WORK IN PROGRESS

## Development Setup
Metro is written in C++ and uses CMake. It requires that `libgit2-dev` is installed via your system package manager.
Alternatively, the `LIBGIT_INCLUDE_DIR` and `LIBGIT_BUILD_DIR` environment variables can be set to the include and build
directories of your own source build of libgit2. Build Metro with the following commands:
```
mkdir build && cd build
cmake ..
make metro
```
For more detailed installation instructions, see the [wiki](https://github.com/SiliconSloth/Metro/wiki/Install).

## Status and Future
Currently, some basic features are working, but not enough to be able to use in practice.
We are still working on various crucial features such as syncing and traversal.

Once it's in a useable state, we plan to put work into a graphical Metro UI that allows easy control over repositories.

## Information
See wiki for details and info.

## Download
Not yet released.
