# OpenGL Scene Project

This project is an OpenGL scene built with `GL`, `GLFW`, `GLM`, `Assimp`, and `stb_image.h`. It includes basic setup instructions to get everything running using `vcpkg` for dependency management.

---

## Prerequisites

Before you can build and run the project, ensure you have the following installed:

**vcpkg**: Dependency manager for C++.

---

## Setting Up vcpkg (Windows)

1. **Clone vcpkg Repository**:
   ```bash
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat

2. **Integrate vcpkg with VS**:

   ```bash
   .\vcpkg integrate install

3. **Install Required Libraries**:

   ```bash
   .\vcpkg install glfw3:x64-windows 
   .\vcpkg install glew:x64-windows
   .\vcpkg install glm:x64-windows
   .\vcpkg install assimp:x64-windows

4. **Build the Project in VS**

After installing the required libraries using vcpkg, you are ready to build the project and run it. You don't have to install the stb_image.h since the header should already be in the source code.
