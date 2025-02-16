# Simple JSON parser, builder and formatter C++ library

Started a C++ project to develop a simple json parser, json builder and json formatter header only library.

### 1. Building the library:

First download or clone this github repository using the following command:
```
git clone --recurse-submodules https://github.com/atib80/simple_json_library.git
```
Enter the simple_json_library folder, create a new build folder and change into it.

```
cd simple_json_library 
mkdir build
cd build
```
To build the simple_json static library together with the tests type in the following commands in the terminal:
```
cmake -B . -S ../ -DCMAKE_BUILD_TYPE=Release
make
```

### 2. Running the tests

To run the tests run the following command in the terminal:

```
./tests/simple_json_parser_tests
```
or
```
ctest --test-dir . --output-on-failure -j12
```


### 3. Usage of library

To use the static library simply copy or add the simple_json.h header file, copy libsimple_json_library.a to your C++ project's dependencies folder or add the build folder to its additional c++ libraries' path (-Ldependencies/libs) and link your C++ project with the simple_json_library static library file (-lsimple_json_library).