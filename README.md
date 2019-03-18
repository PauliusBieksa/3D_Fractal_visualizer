# 3D_Fractal_visualizer
music visualization using 3D fractals


This program renders 3D fractals based on sound attributes extracted from selected input.
This program is a proof of concept and therefore does not have a GUI or some quality of life features.

Compiling on Windows:
1. Clone the repo.
2. Update submodules (git submodule uptate --init --recursive).
3. Use CMake to build a solution.
4. Set working directory in project settings to $(TargetDir). Right-click Visualiser project in Visual Studio -> properties -> Debugging -> Working Directory
5. Build the copy_res project.
6. Build the Visualiser project.
