## Building for Windows

These instructions assume you already have Visual Studio installed, and therefore have access to the
Developer Command Prompt, or the x64 Native Tools Command Prompt on 64-bit systems.

### 1. Build OpenSSL
1. Install [Perl](http://strawberryperl.com/) and [NASM](https://www.nasm.us/). Make sure that they are both on the PATH, such that the `perl` and `nasm` commands
   can be run from the build directory.
2. Download and extract the [OpenSSL](https://www.openssl.org/) source distribution. Metro has been tested with version 1.1.1e.
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
   
### 2. Build libssh2
1. Download and extract the [libssh2](https://www.libssh2.org/) source code. Metro has been tested with version 1.9.0.
2. Open Command Prompt and switch to the libssh2 directory.
3. Run the following commands:
   ```batch
   mkdir build
   cd build
   cmake -DCRYPTO_BACKEND=OpenSSL ..
   cmake --build .
   ```
   By this point the file `build/src/Debug/libssh2.lib` should exist.
   
### 3. Build libgit2
1. Download and extract the [libgit2](https://libgit2.org/) source code. Metro has been tested with version 0.28.4.
3. Open Command Prompt and switch to the libgit2 directory.
4. Run the following commands, where `C:/.../libssh2-1.9.0` is the path of your libssh2 directory.
   Note that all backslashes in the path must be replaced with forward slashes to appease CMake.
   ```batch
   mkdir build
   cd build
   cmake -DBUILD_SHARED_LIBS=OFF -DBUILD_CLAR=OFF -DUSE_SSH=OFF -DLIBSSH2_FOUND=1 -DLIBSSH2_INCLUDE_DIRS=C:/.../libssh2-1.9.0/include -DLIBSSH2_LIBRARIES=C:/.../libssh2-1.9.0/build/src/Debug/libssh2.lib -DLIBSSH2_LDFLAGS="" ..
   cmake --build .
   ```
   By this point the file `build/Debug/git2.lib` should exist.
   
### 4. Build Metro
1. Set the following environment variables, replacing `C:\...\ ` with the absolute path of each directory:
   * `LIBGIT_INCLUDE_DIR` to `C:\...\libgit2-0.28.4\include`
   * `LIBGIT_BUILD_DIR` to `C:\...\libgit2-0.28.4\build\Debug`
   * `LIBSSH_BUILD_DIR` to `C:\...\libssh2-1.9.0\build\src\Debug`
   
   `OPENSSL_ROOT_DIR` should still be set from building OpenSSL.
2. Clone the Metro repository by running: \
   `git clone https://github.com/SiliconSloth/Metro`
3. Switch into the `Metro` directory and run the following commands:
   ```batch
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```
   This should create the `metro.exe` file in `build\Debug`.
4. If you want to be able to run Metro from any directory on your computer,
   add the directory containing `metro.exe` to your PATH variable.