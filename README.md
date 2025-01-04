# COMP3016-CW2

# OpenGL Project Report

## Table of Contents
1. [Dependencies Used](#dependencies-used)
2. [Game Programming Patterns Used](#game-programming-patterns-used)
3. [Game Mechanics and How They Are Coded](#game-mechanics-and-how-they-are-coded)
4. [Software Engineering Issues](#software-engineering-issues)
5. [UML Design Diagram](#uml-design-diagram)
6. [Sample Screens](#sample-screens)
7. [Exception Handling and Test Cases](#exception-handling-and-test-cases)
8. [Further Details on How the Prototype Works](#further-details-on-how-the-prototype-works)
9. [Evaluation](#evaluation)

## Dependencies Used

- **GL (OpenGL)**: Provides the graphics rendering pipeline used for drawing the visual elements such as the road, houses, trees, and castle in the scene.
- **GLFW**: A library that helps handle window creation, user input (e.g., camera movement), and managing the OpenGL context.
- **GLM (OpenGL Mathematics)**: A library for handling transformations, vectors, matrices, and other math functions used in 3D graphics programming. It was key for managing camera movements and object transformations in the scene.
- **ASSIMP (Open Asset Import Library)**: Used for loading and importing 3D model files, such as the meshes for the trees, houses, and castle. It allows the conversion of various model file formats into a usable format for rendering.
- **STB_IMAGE.h**: A header for loading image files. This was particularly useful for texture loading in the scene, such as applying textures to the ground, houses, and other objects.

## Game Programming Patterns Used

- **Component-based architecture**: The project was designed to separate the rendering logic from the game objects themselves. For instance, each object like the house, trees, and castle could have components for their meshes, textures, and transformations.
- **Event-driven programming**: The camera controls were set up as event handlers where input from the user (via the keyboard) triggers specific actions like moving or rotating the camera.

## Game Mechanics and How They Are Coded

- **Camera Control**: The user can move the camera using the **W**, **A**, **S**, and **D** keys for forward, left, back, and right movement, respectively. The **Q** and **E** keys allow for camera rotation.
- **Scene Composition**: The scene consists of several objects, including a dirt road, grass, two houses, a wheat field, trees, and a castle. The castle and the houses are placed using 3D models loaded from external files.
- **Fog Effect**: A fog effect was added to create a mystical atmosphere for the medieval setting, which is implemented using OpenGL shaders.

## Software Engineering Issues

- **Performance vs. Good Practice**: A key issue was the trade-off between performance and clarity in the code. For example, individual trees were rendered separately, which could have been optimized through batching. However, rendering each tree separately allowed for easier debugging during development.
- **Texture Optimization**: The textures used might not be the best fit for the scene's aesthetic. In future versions, more appropriate textures could be chosen to enhance the visual quality and match the medieval theme better.
  
## UML Design Diagram

_No UML diagram was provided, but one could be created to show how different components like meshes, textures, camera, and objects (houses, trees, etc.) interact with each other._

## Sample Screens

1. **Scene Overview**: A shot of the full scene showing the dirt road leading to the castle.
2. **Close-up of House**: A close-up rendering showing the house and surrounding trees.
3. **Fog Effect**: A screenshot illustrating how the fog enhances the atmosphere in the scene.

## Exception Handling and Test Cases

- **Manual Testing**: Due to the visual nature of the project, manual testing was used to ensure that objects were being loaded correctly and the scene was rendered as expected.
- **Exception Handling**: The project includes basic exception handling to account for errors such as failing to load models or textures. For example, an error message would be displayed if a model file couldn’t be loaded or if there was an issue with a shader.

## Further Details on How the Prototype Works

- **Loading and Rendering Objects**: 3D models are loaded using ASSIMP and their corresponding textures are loaded using STB_IMAGE. The objects are rendered using OpenGL with transformations handled by GLM to position, rotate, and scale the objects.
- **Camera Logic**: The camera movement is managed through keyboard inputs (WASD for movement and QE for turning). The camera is updated based on user input, and the scene is re-rendered accordingly.

## Evaluation

Overall, the project was successful in achieving a simple 3D scene with basic camera controls and atmospheric effects. However, there are areas for improvement:
- **Performance Optimization**: The individual rendering of trees could be optimized by batching them together for better performance.
- **Texture Quality**: The textures used in the scene could be upgraded to better fit the medieval aesthetic.
  
In future projects, I would focus on optimising asset rendering and texture management, as well as incorporating more advanced features like lighting and shadow effects to enhance the visual fidelity of the scene.
