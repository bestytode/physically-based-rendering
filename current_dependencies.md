# Project Dependencies

This document outlines the dependencies for a project configured in Visual Studio 2022 for Debug and Release modes in x64 configuration.

## Graphics Libraries

### GLFW
- **Description**: GLFW is used for creating windows, contexts, and managing input.
- **Website**: [GLFW](https://www.glfw.org/)
- **Integration**:
  - Add the GLFW include directory to 'VC++ Directories' -> 'Include Directories'.
  - Add the GLFW library directory to 'VC++ Directories' -> 'Library Directories'.
  - Add the GLFW library file (e.g., glfw3.lib) to 'Linker' -> 'Input' -> 'Additional Dependencies'.

### GLM
- **Description**: GLM provides mathematics functionality for graphics.
- **Website**: [GLM](https://glm.g-truc.net/)
- **Integration**:
  - Include GLM's header files in your project (no lib directory required for GLM).

### GLEW
- **Description**: GLEW manages OpenGL extensions.
- **Website**: [GLEW](http://glew.sourceforge.net/)
- **Integration**:
  - Define `#STATIC_GLEW` for static linking in your project's preprocessor definitions.
  - Add GLEW's include directory to 'VC++ Directories' -> 'Include Directories'.
  - Add GLEW's library directory to 'VC++ Directories' -> 'Library Directories'.
  - Add GLEW's library file (e.g., glew32s.lib for static linking) to 'Linker' -> 'Input' -> 'Additional Dependencies'.

### Assimp
- **Description**: Assimp for importing various 3D model formats.
- **Website**: [Assimp](https://www.assimp.org/)
- **Integration**:
  - Add Assimp's include directory to 'VC++ Directories' -> 'Include Directories'.
  - Add Assimp's library directory to 'VC++ Directories' -> 'Library Directories'.
  - Add Assimp's library file to 'Linker' -> 'Input' -> 'Additional Dependencies'.
  - In the same directory as the final .exe file, add assimp-vc143-mt.dll.

### ImGui
- **Description**: ImGui for creating immediate-mode graphical user interfaces.
- **Website**: [ImGui](https://github.com/ocornut/imgui)
- **Integration**:
  - Add ImGui source files and include directory to your project (no separate lib file required for ImGui).

### stb_image.h
- **Description**: stb_image.h for image loading.
- **Extra Note**: Use this snippet to avoid multiple inclusions:
  ```c++
  #ifndef STB_IMAGE_IMPLEMENTATION
  #define STB_IMAGE_IMPLEMENTATION
  #include "stb_image.h"
  #endif
