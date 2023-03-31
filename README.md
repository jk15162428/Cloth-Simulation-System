#### Usage
* Escape/ESC: Exit
* W, A, S, D: move camera
* P: Execute/Pause the simulation
* T: Step 1 time, this key will unconditionally pause the simulation
* Z, X, C: Switch the render mode as DRAW_NODES, DRAW_LINES and DRAW_FACES
* R: Reset the scene
* Up, Down, Left, Right: Adding force to the cloth.

#### Environment

* Windows 10
* Visual Studio 2022
* OpenGL >= 3.3

#### Dependencies

* glm
* glad
* glfw
* glTools
* freetype

Note: **opengl32.lib**, **glfw3.lib** and **freetype.lib** should be linked, and besides including corresponding library, you should also add **glad.c** into your project.
