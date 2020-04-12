# Building for Windows

These instructions assume you already have Visual Studio installed, and therefore have access to the
Developer Command Prompt, or the x64 Native Tools Command Prompt on 64-bit systems.

### 1. Build OpenSSL
1. Install [Perl](http://strawberryperl.com/) and [NASM](https://www.nasm.us/). Make sure that they are both on the PATH, such that the `perl` and `nasm` commands
   can be run from the build directory.
2. Download and extract the [OpenSSL](https://www.openssl.org/) source distribution. Metro has been tested with version 1.1.1f.
3. If you want to build a 32-bit version of Metro, open the **Developer Command Prompt**.
   If you want to build a 64-bit version of Metro, open the **x64 Native Tools Command Prompt**.
   It is essential that you use the right command prompt, otherwise you will experience build errors.
4. Switch to the OpenSSL source directory with `cd`.
5. If you want to build 32-bit Metro, run `perl Configure -static VC-WIN32`.
   If you want to build 64-bit Metro, run `perl Configure -static VC-WIN64A`.
   Make sure that the version you choose matches the command prompt you have open from step 3.
6. Run `nmake`. This might take some time, but once done there should be two files called
   `libssl.lib` and `libcrypto.lib` in the OpenSSL directory. There should be no `.dll` files;
   if there are you likely forgot the `-static` flag above.
7. Set the `OPENSSL_ROOT_DIR` environment variable to the path of the OpenSSL directory,
   which contains the `.lib` files.
   
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
This should create the `metro.exe` file in `build`.

### 4. Add to PATH
If you want to be able to run Metro from any directory on your computer, add the directory containing `metro.exe` to your path.

You can do this by going to Windows Explorer, right clicking This PC, choosing properties, choosing Advanced System Settings, choosing Environmental Variables, clicking on PATH in either User or System, choosing Edit and adding an entry with `C:/path-to-metro/build`