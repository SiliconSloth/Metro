## Building for Linux
 
### 1. Build libssh2
1. Download and extract the [libssh2](https://www.libssh2.org/) source code. Metro has been tested with version 1.9.0.
2. Open a terminal and switch to the libssh2 directory.
3. Run the following commands:
   ```shell
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```
   By this point the file `build/src/libssh2.a` should exist.
   
### 2. Build libgit2
1. Download and extract the [libgit2](https://libgit2.org/) source code. Metro has been tested with version 0.28.4.
3. Open a terminal and switch to the libgit2 directory.
4. Run the following commands:
   ```shell
   mkdir build
   cd build
   cmake -DBUILD_SHARED_LIBS=OFF ..
   cmake --build .
   ```
   By this point the file `build/libgit2.a` should exist.
   
### 3. Build Metro
1. Set the following environment variables, replacing `/.../` with the absolute path of each directory:
   * `LIBGIT_INCLUDE_DIR` to `/.../libgit2-0.28.4/include`
   * `LIBGIT_BUILD_DIR` to `/.../libgit2-0.28.4/build`
   * `LIBSSH_BUILD_DIR` to `/.../libssh2-1.9.0/build/src`
2. Clone the Metro repository by running: \
   `git clone https://github.com/SiliconSloth/Metro`
3. Switch into the `Metro` directory and run the following commands:
   ```shell
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```
   This should create the `metro` file in `build`.
4. If you want to be able to run Metro from any directory on your computer,
   add the directory containing `metro` to your path.