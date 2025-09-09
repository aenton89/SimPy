# ![icon_v3.png](assets/app_icon/icon_v3.png) SimPy - what is it?
### patryk wymyśl co tu dać
- graphical based system??? dunno
- with data flow
- integrates with python code???
### technicals:
- C++
- based mostly on ImGui
- uses Conan for dependencies
- uses CMake for building

# References
- [conan tutorial](https://docs.conan.io/2/tutorial/consuming_packages/the_flexibility_of_conanfile_py.html)
- [imgui tutorial](https://thescienceofcode.com/imgui-quickstart/)

# Getting started and so on
## 1. install cmake and conan
### 1.1. install cmake
```
choco install cmake
```
> [!TIP]
> in conanfile.py change cmake version i guess? idk

### 1.2. install conan
```
choco install conan
```
> [!IMPORTANT]
> or you can use pip
```
pip install conan
```

### 1.3. detect conan profile
```
conan profile detect --force
```

## 2. install dependencies
### 2.1. download imgui backends (my python script)
```
python3 download_backends.py
```
### 2.2. install rest of dependencies by conan
```
conan install . --output-folder=build --build=missing
```
```
cd build
```
```
.\conanbuild.bat
```

## 3. build
```
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake"
```
```
cmake --build . --config Release
```

## 4. run
### 4.1. run main program
```
.\Release\patrykcpp.exe
```
### 4.2. run tests
```
.\Release\tests.exe
```

## 5. deactivate conan
```
.\deactivate_conanbuild.bat
```
> [!WARNING]
> idk what this does; to find out later