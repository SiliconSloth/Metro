# Building for macOS

### 1. Install OpenSSL
1. Install [Homebrew](https://brew.sh/) if you do not have it already.
2. Run `brew install openssl`.
3. Set the `OPENSSL_ROOT_DIR` environment variable to `$(brew --prefix openssl)`.
4. Set the `LDFLAGS` environment variable to `-L$OPENSSL_ROOT_DIR/lib`.

### 2. Clone Metro
Clone using the command `git clone --recursive https://github.com/SiliconSloth/Metro`
   
Alternatively you can clone normally and use `git submodule update --init --recursive` to get dependancies
   
### 3. Build Metro
Switch into the `Metro` directory and run the following commands:
```shell
mkdir build
cd build
cmake ..
cmake --build .
```
This should create the `metro` file in `build`.

### 4. Add to PATH
If you want to be able to run Metro from any directory on your computer, add the directory containing `metro` to your path.

You can do this by adding `export PATH=PATH:/path-to-metro/build` to `$HOME/.bash_profile` or similar