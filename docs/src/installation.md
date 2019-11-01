# Installation

Currently, Metro can only be installed by building it from source. Luckily, it's not that difficult!

## Windows

- Download `cmake` from [cmake.org](https://cmake.org/download/)
- Build `libgit2` from source, following the [libgit2 build instructions](https://github.com/libgit2/libgit2#building)
- Set the environment variables `LIBGIT_INCLUDE_DIR` and `LIBGIT_BUILD_DIR` to point to your `libgit2` include and build directories
- Build Metro:
  ```
  mkdir build && cd build
  cmake ..
  make metro
  ```

## Linux
### Ubuntu

- Download `cmake` and `libgit2` using the following commands:
  ```
  sudo apt-get install cmake
  sudo apt-get install libgit2-dev
  ```
- Clone Metro and build:
  ```
  git clone https://github.com/SiliconSloth/Metro.git
  mkdir build && cd build
  cmake ..
  make metro
  ```

### Arch

- Download `cmake` and `libgit2` using the following command:
  ```
  pacman -S cmake-git libgit2-git
  ```
- Clone Metro and build: 
  ```
  git clone https://github.com/SiliconSloth/Metro.git
  mkdir build && cd build
  cmake ..
  make metro
  ```

### NixOS

- Clone Metro:
  ```
  git clone https://github.com/SiliconSloth/Metro.git
  ```
- Create a `shell.nix` file with the following:
  ```
  with import <nixpkgs> {};
  stdenv.mkDerivation {
    name = "Metro";
    buildInputs = [ cmake libgit2 openssl zlib ];
    shellHook = "mkdir build; cd build && cmake ..";
  } 
  ```
- Build Metro in the `nix-shell`:
  ```
  nix-shell
  make metro
  ```

