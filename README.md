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

## Code Snippets

You can find all related code snippets in the [code snippets](./code%20snippets/) folder.

## Dependencies Used

- **GL (OpenGL)**: Provides the graphics rendering pipeline used for drawing the visual elements such as the road, houses, trees, and castle in the scene.
- **GLFW**: A library that helps handle window creation, user input (e.g., camera movement), and managing the OpenGL context.
- **GLM (OpenGL Mathematics)**: A library for handling transformations, vectors, matrices, and other math functions used in 3D graphics programming. It was key for managing camera movements and object transformations in the scene.
- **ASSIMP (Open Asset Import Library)**: Used for loading and importing 3D model files, such as the meshes for the trees, houses, and castle. It allows the conversion of various model file formats into a usable format for rendering.
- **STB_IMAGE.h**: A header for loading image files. This was particularly useful for texture loading in the scene, such as applying textures to the ground, houses, and other objects.

### Code Snippet for Dependencies
![Dependencies](./code%20snippets/Dependencies.PNG)

## Game Programming Patterns Used

- **Component-based architecture**: The project was designed to separate the rendering logic from the game objects themselves. For instance, each object like the house, trees, and castle could have components for their meshes, textures, and transformations.
- **Event-driven programming**: The camera controls were set up as event handlers where input from the user (via the keyboard) triggers specific actions like moving or rotating the camera.

### Code Snippet for Program Structure
![Structure](./code%20snippets/CodeStructure.PNG)

## Game Mechanics and How They Are Coded

- **Camera Control**: The user can move the camera using the **W**, **A**, **S**, and **D** keys for forward, left, back, and right movement, respectively. The **Q** and **E** keys allow for camera rotation.
- **Scene Composition**: The scene consists of several objects, including a dirt road, grass, two houses, a wheat field, trees, and a castle. The castle and the houses are placed using 3D models loaded from external files.
- **Fog Effect**: A fog effect was added to create a mystical atmosphere for the medieval setting, which is implemented using OpenGL shaders.

### Code Snippet for Camera Control
![Dependencies](./code%20snippets/Camera.PNG)

### Snippet of Assets used in the Scene
![Dependencies](./code%20snippets/Assets.PNG)

## Software Engineering Issues

- **Performance vs. Good Practice**: A key issue was the trade-off between performance and clarity in the code. For example, individual trees were rendered separately, which could have been optimized through batching. However, rendering each tree separately allowed for easier debugging during development.
- **Texture Optimization**: The textures used might not be the best fit for the scene's aesthetic. In future versions, more appropriate textures could be chosen to enhance the visual quality and match the medieval theme better.
  
## UML Design Diagram

_No UML diagram was provided, but one could be created to show how different components like meshes, textures, camera, and objects (houses, trees, etc.) interact with each other._

## Sample Screens

1. **Scene Overview**: A shot of the full scene showing the dirt road leading to the castle.
2. **Close-up of House**: A close-up rendering showing the house and surrounding trees.
3. **Fog Effect**: A screenshot illustrating how the fog enhances the atmosphere in the scene.

### Snippet for Scene Overview
![Dependencies](./code%20snippets/SceneOverview.PNG)

### Snippet for Close-up of House
![Dependencies](./code%20snippets/House.PNG)

### Snippet for Fog Effect
![Dependencies](./code%20snippets/Fog.PNG)

## Exception Handling and Test Cases

- **Manual Testing**: Due to the visual nature of the project, manual testing was used to ensure that objects were being loaded correctly and the scene was rendered as expected.
- **Exception Handling**: The project includes basic exception handling to account for errors such as failing to load models or textures. For example, an error message would be displayed if a model file couldn’t be loaded or if there was an issue with a shader.

## Further Details on How the Prototype Works

- **Loading and Rendering Objects**: 3D models are loaded using ASSIMP and their corresponding textures are loaded using STB_IMAGE. The objects are rendered using OpenGL with transformations handled by GLM to position, rotate, and scale the objects.
- **Camera Logic**: The camera movement is managed through keyboard inputs (WASD for movement and QE for turning). The camera is updated based on user input, and the scene is re-rendered accordingly.

## Evaluation

The project demonstrates several key achievements in incorporating diverse rendering techniques and implementing a visually cohesive medieval-themed scene. Below is an in-depth evaluation of the successes, challenges, and areas for improvement.

### Achievements

1. **Multiple Rendering Techniques**  
   - The project successfully integrates various rendering methods, showcasing a deep understanding of OpenGL:  
     - **Manually Written Vertices**: Simple geometric shapes were defined directly in the source code to construct parts of the scene, demonstrating a fundamental grasp of vertex and buffer management.  
     - **Loop-based Vertex Generation**: The wheat field was efficiently generated using a loop-based approach, reusing and repeating vertex data to create a natural, field-like appearance without redundant code.  
     - **Mesh Loading**: Complex models such as the houses and the castle were loaded from `.obj` files using the ASSIMP library, reflecting proficiency in handling external assets.  
   - These techniques come together to create a cohesive and immersive scene that leverages the strengths of both procedural generation and external asset management.

2. **Fog Implementation**  
   - A fog effect was added to enhance the scene's atmosphere. The implementation is particularly notable for its flexibility, allowing easy adjustments to fog density and color directly within the source code.

3. **Code Readability and Documentation**  
   - The source code is well-documented with comments throughout, explaining the purpose and functionality of each section.  
   - The use of **regions** to group related parts of the code improves readability and maintainability, making it easier for future developers to navigate and understand the project structure.

### Challenges and Shortcomings

1. **Mesh and Texture Loading**  
   - The process of loading a mesh and texture from external files posed significant challenges during development.  
     - Issues such as managing file paths, binding textures correctly, and integrating them with the rendering pipeline required considerable time and effort to resolve.  
     - While the final result is functional, the process highlights an area where further experience and practice would reduce development time and improve confidence.  

2. **Texture Quality and Aesthetic Consistency**  
   - Although the project successfully applies textures to models, the chosen textures do not perfectly align with the medieval theme.  
     - The textures were sourced from free online assets, reflecting a reliance on external resources for visual quality.  
     - Improving skills in creating custom meshes and textures, or sourcing more thematically consistent assets, would greatly enhance the scene's aesthetic and immersion.

3. **Asset Creation Limitations**  
   - The reliance on free online assets underscores a gap in skills related to creating 3D models and textures.  
     - While this approach was necessary to complete the project within the given constraints, developing skills in modeling and texture creation would enable greater customisation and control over future projects.

### Areas for Improvement

1. **Optimisations**  
   - The current approach to rendering trees and other repeating objects involves treating each as a separate entity, which could be optimised using batching techniques.  
     - Implementing instanced rendering for repetitive assets like trees would improve performance and reduce the overhead of individual draw calls.

2. **Asset Management**  
   - Enhancing familiarity with tools like Blender for model creation and image editing software for texture design would allow for more cohesive and tailored visual assets.  
   - Developing a better workflow for managing and integrating external assets would streamline the process of importing and rendering models.

3. **Thematic Enhancements**  
   - Incorporating higher-quality, thematic assets and experimenting with advanced rendering techniques (e.g., dynamic lighting, shadows, and reflections) would elevate the scene's visual fidelity.

### Summary

The project successfully achieves its primary goals of creating a visually engaging medieval-themed scene while incorporating various rendering techniques. It demonstrates a strong foundation in OpenGL programming, from vertex management to asset integration and visual effects like fog. However, challenges in mesh and texture handling, as well as reliance on external assets, highlight areas for growth.  

If given the opportunity to revisit this project, the focus would be on improving asset creation skills, optimising rendering performance, and further aligning the textures and models with the intended theme. Despite these areas for improvement, the project stands as a robust and well-documented prototype that effectively showcases the current skill set.
