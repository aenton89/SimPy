# ![icon_v3.png](assets/app_icons/icon_v3.png) # SimPy – What is it?

SimPy is an **application with its own Python IDE** and a tool for **block-based modeling of dynamic systems and time-dependent processes**.  
It allows creating simulations of controllers, regulation systems, and measurement setups without the need for specialized graphical software.

---

# Main Project Goals

1. **Designing measurement paths and control systems for ESP32**  
   - Creating schematics and generating code ready to upload to ESP32.  

2. **Real-time data analysis**  
   - Collecting and analyzing data from ESP32 in Python for testing and system optimization.  

3. **Using RL models in dynamic systems**  
   - Training reinforcement learning models considering the system's characteristics to perform better in simulations and on the actual hardware.  

---

# Actual Work

- Integration of the Python IDE with the simulation application to streamline development and testing.

### technicals:
- C++/Python
- based mostly on ImGui for UI

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