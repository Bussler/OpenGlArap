# OpenGlArap
Implementation of the As-Rigid-As-Possible Surface Modeling (ARAP) algorithm by Sorkine and Alexa with OpenGL.
ARAP is an algorithm for mesh deformations in a 3-dimensional environment.
Depending on a user’s set of control points and predefined constraints, it is possible to render a visually appealing mesh deformation.
The mesh itself can be freely rigged in real-time by enabling and disabling vertices as control points and then dragging them to a desired location.
The ARAP modelling algorithm is an iterative scheme which preserves local mesh shapes as much as possible and thereby enforces realisitic rigid deformations of the base mesh.

### Dependencies
- Eigen
- OpenMesh
- Assimp
- GLM
- GLAD
- GLFW

### How to use
