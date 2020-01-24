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
For more detailed installation instructions, see the [wiki](https://siliconsloth.github.io/Metro/installation.html).

## Status and Future
The basic functionality of Metro, including syncing, has been implemented, however it is not yet stable or user-friendly.
We are currently working on fixing bugs and generally improving the user experience.

## Information
See [wiki](https://siliconsloth.github.io/Metro/) for details and info.

## Download
Not yet released.
