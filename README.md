# CPSC 436D: Video game programming

![alt text](https://github.com/esemeniuc/celestial-industries/blob/master/data/textures/Celestial-Industries.png "Celestial Industries Logo")

## Project Info

### Info

#### Pitch
- [OneDrive - DOCX](https://onedrive.live.com/view.aspx?resid=C3F0298314E4ECCD!284946&ithint=file%2cdocx&app=Word&authkey=!AJgdQbFssTUXqhU)

#### Presentation 1
- [OneDrive - PPTX](https://onedrive.live.com/view.aspx?resid=C3F0298314E4ECCD!295036&ithint=file%2cpptx&app=PowerPoint&authkey=!AHZauhwFBn72fPo)

#### Presentation 2
- [OneDrive - PPTX](https://onedrive.live.com/view.aspx?resid=C3F0298314E4ECCD!303764&ithint=file%2cpptx&app=PowerPoint&authkey=!APpASRWldZN18Xs)

### Dependencies
- SDL2
- SDL2_mixer
- OpenGL 4.1
- GLFW3

### How to Download
```bash
git clone https://github.com/ahmdk/celestial-industries.git
git submodule update --init --recursive
```

### Linux Instructions
Requires g++ 6.x/clang++ 4.x or newer

#### Dependency Installation on Ubuntu
```bash
apt-get install -y cmake g++ libsdl2-dev libsdl2-mixer-dev libglfw3-dev libogg libvorbis
```

#### Compiling
```bash
mkdir build && cd build
cmake ..
make -j4
```

#### Running the game
From the git root, run
```bash
build/proj
```

#### Running tests
Catch (from the git root)
```bash
build/test_suite
```

CMake (from within the build folder)
```bash
ctest
```

### Windows Instructions

#### Installation on Windows

You should be able to simply open `visual_studio/Attempt 1.sln` and run it the same way you would any other Visual Studio project.

## Milestone 1

### Requirements

 - [x] Loading and rendering of non‐trivial geometry 
 - [x] Ability to load .obj files and correctly load their material and texture data for use in the shader code
 - [x] Loading and rendering of textured  geometry with correct blending 
 - [x] Shader that implements Blinn-Phong shading and handles textures if present
 - [x] Working 2D Transformations 
 - [x] Response to user input (mouse, keyboard): including changes in the set of rendered objects, object eometry, position, orientation, textures, colors, and other attributes.  
 - [x] Camera supports moving (WASD, ←↑→↓), rotating on the camera's up axis (QE), rotating around the camera's horizontal axis (scroll wheel), zooming and unzooming (Z + scroll wheel)

 - [x] Basic key‐frame/state interpolation (smooth movement from point A to point B in Cartesian space).  
 - [x] Have entities that can move to a given location (with movement happening smoothly between the two points, but not with pathing logic to avoid obstacles)
 - [x] Add a skybox
 - [x] Have the ability to select a tile
