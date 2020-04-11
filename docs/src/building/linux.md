# Building for Linux
 
### 1. Clone Metro
Clone using the command `git clone --recursive https://github.com/SiliconSloth/Metro`
   
Alternatively you can clone normally and use `git submodule update --init --recursive` to get dependancies
   
### 2. Build Metro
Switch into the `Metro` directory and run the following commands:
```shell
mkdir build
cd build
cmake ..
cmake --build .
```
This should create the `metro` file in `build`.

### 3. Add to PATH
If you want to be able to run Metro from any directory on your computer, add the directory containing `metro` to your path.

You can do this by adding `export PATH=PATH:/path-to-metro/build` to `~/.bashrc` or similar