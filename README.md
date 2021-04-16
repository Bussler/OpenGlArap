# OpenGlArap
Implementation of the As-Rigid-As-Possible Surface Modeling (ARAP) algorithm by Sorkine and Alexa with OpenGL.
ARAP is an algorithm for mesh deformations in a 3-dimensional environment.
Depending on a user’s set of control points and predefined constraints, it is possible to render a visually appealing mesh deformation.
The mesh itself can be freely rigged in real-time by enabling and disabling vertices as control points and then dragging them to a desired location.
The ARAP modelling algorithm is an iterative scheme which preserves local mesh shapes as much as possible and thereby enforces realisitic rigid deformations of the base mesh.

## Dependencies
- Eigen
- OpenMesh
- Assimp
- GLM
- GLAD
- GLFW

## How to use
After linking the dependencies and compiling the program is used with the following 2 arguments:

`./ARAPImplementation filenameModel filenameShader`

The program can also be executed without arguments, in which case it loads the default cactus model and shader.

`./ARAPImplementation`

### Controls
- Right-click on a vertex: Selecting a vertex as a constraint. These are excluded from the ARAP-Solving and will remain at their original position. Clicking a vertex the first time will register it as a static constraint, clicking it a second time as a dynamic constraint. Dynamic constraints are draggable by user input. Clicking the vertex a third time will remove the constraint-status from the vertex.
- Pressing the left mouse-button and dragging the mouse: All dynamic constraints are dragged according to the user input. The rest of the mesh is deformed as rigid as possible according to the ARAP algorithm. This feature enables the animation of the input mesh.
- Pressiong the middle mouse-button and dragging the mouse: Rotates the loaded mesh around the Y-Axis.
