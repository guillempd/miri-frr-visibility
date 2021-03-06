# MIRI FRR Visibility Project (Fast Realistic Rendering)

## Project Description
This projects aims to measure and compare the performance of rendering a scene using different visibility culling strategies.

For more detailed information see [REPORT.pdf](https://github.com/guillempd/miri-frr-visibility/blob/master/REPORT.pdf).

## Demo Video
Notice how the fps and instances rendered vary on the bottom left corner depending on the occlusion culling strategy and whether frustum culling is enabled or disabled.

https://user-images.githubusercontent.com/15831136/138293187-ed5d893c-79b2-4646-953f-fa6e4361c3ae.mp4

## Building the Project
The project is provided as a CMake project. In order to build it (Linux) execute the following commands at the root directory (where the `CMakeLists.txt` file is located):

```
mkdir build
cd build
cmake ..
make
```

This series of commands will generate the `BaseCode` executable.

## Using the interface

By default the mouse input is captured and so the interface **can't** be used. To stop capturing the mouse and be able to use the interface (as well as resizing the screen) use the key `i`.

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
For the frustum culling implementation, the relevant functions are:
```c++
Camera::updateFrustum();
Scene::insideFrustum(const AABB &aabb);
```

### Occlusion Culling
For the occlusion culling implementation, the relevant functions are:
```c++
Scene::renderBasic();
Scene::renderStopAndWait();
Scene::renderAdvanced();
Scene::renderCHC();
```

## Project Results
See [REPORT.pdf](https://github.com/guillempd/miri-frr-visibility/blob/master/REPORT.pdf) for the full results and conclusions of the project.
