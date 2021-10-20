# README

## Compilation
The project is provided as a CMake project.
In order to compile it create a build folder, run cmake and then make.
If nothing is visible on the screen make sure that the current working directory is pointing to the root directory or copy the models and shaders directories into the build folder.

## Menus

### Performance Statistics Tab
Shows the current fps and the number of objects rendered.

### Settings Tab
Controls the algorithm that renders the scene.
Debug mode shows the bounding boxes of the objects for debugging purposes.
Path mode only shows the bounding boxes of the objects for easier recording of a path.
### Record Path Tab
Provides the functionality to record a path, specifying the duration of it in seconds and the name of the output file where the path will be stored.
Once the record is started, the position and orientation of the camera is tracked and stored.

### Replay Path Tab
Provides the functionality to replay a path, specifying the name of the input file and the name of the output file where the fps recorded along the path will be stored.

## Implementation

### Frustum Culling
For the frustum culling implementation, the relevant functions are Camera::updateFrustum() and Scene::insideFrustum(const AABB &aabb).

### Occlusion Culling
For the occlusion culling implementation, the relevant functions are Scene::renderBasic(), Scene::renderStopAndWait(), Scene::renderAdvanced() and Scene::renderCHC().

## Experiment Setup
The file path.dat contains the camera path that I have used for the fps measurements.
