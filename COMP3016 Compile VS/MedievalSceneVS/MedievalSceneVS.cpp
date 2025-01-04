#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>  // For std::vector

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// House and Castle model load
#pragma region Model Loading

// Vertex structure
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

// Mesh class to handle rendering
class Mesh {
public:
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, unsigned int textureID)
        : vertices(vertices), indices(indices), textureID(textureID) {
        setupMesh();
    }
    void draw() {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;
    unsigned int textureID;

    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);

        // Texture coord attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }
};
// Function prototypes
void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes, const std::string& directory);
Mesh processMesh(aiMesh* mesh, const aiScene* scene, const std::string& directory);
GLuint loadTexture(const char* path);

// Load Model using Assimp
void loadModel(const std::string& path, std::vector<Mesh>& meshes) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        return;
    }
    std::string directory = path.substr(0, path.find_last_of('/'));

    // Correctly pass meshes as a reference to processNode
    processNode(scene->mRootNode, scene, meshes, directory);
}

void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes, const std::string& directory) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene, directory));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, meshes, directory);
    }
}

Mesh processMesh(aiMesh* mesh, const aiScene* scene, const std::string& directory) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int textureID = 0;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

        if (mesh->mTextureCoords[0]) {
            vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Process materials
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        aiString texturePath;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
            std::string fullPath = directory + "/" + texturePath.C_Str();
            textureID = loadTexture(fullPath.c_str());
        }
    }

    return Mesh(vertices, indices, textureID);
}


#pragma endregion
#pragma region Camera Settings
// Camera settings
glm::vec3 cameraPos = glm::vec3(-15.0f, 1.0f, 0.0f); // Start further back on the Z axis
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Camera looks down the -Z axis by default
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); // Up is along the Y axis

// Camera boundries (matches the road)
const glm::vec3 minBoundary = glm::vec3(-15.0f, 1.0f, -3.0f); // Minimum x, y, z values
const glm::vec3 maxBoundary = glm::vec3(5.0f, 1.0f, 3.0f);  // Maximum x, y, z values

float cameraYaw = 0.0f;  // Yaw angle, starting at 0
float cameraPitch = 0.0f;    // Pitch angle, starting at 0 degrees
float rotationSpeed = 0.2f;   // Rotation speed (sensitivity)

// View matrix
glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

// Projection matrix
glm::mat4 projection = glm::perspective(glm::radians(60.0f), 800.0f / 600.0f, 0.1f, 50.0f);

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of the last frame
#pragma endregion
#pragma region Camera update and Input
// Function to update the camera's front vector based on yaw (and pitch if necessary)
void updateCameraFront() {
    // Calculate the new front vector based on the yaw (and pitch)

    const float yawLimit = 45.0f;

    // Clamp yaw to be within [-yawLimit, yawLimit]

    if (cameraYaw > yawLimit) {
        cameraYaw = yawLimit;
    }
    if (cameraYaw < -yawLimit) {
        cameraYaw = -yawLimit;
    }

    glm::vec3 front;
    front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    front.y = sin(glm::radians(cameraPitch));  // Optional: use pitch for vertical rotation
    front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    cameraFront = glm::normalize(front);  // Normalize to ensure consistent movement
}

// Process input
void processInput(GLFWwindow* window) {
    float cameraSpeed = 2.5f * deltaTime; // Adjust the speed based on frame time

    glm::vec3 newCameraPos = cameraPos; // Temporary variable for new position


    // Move forward and backward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        newCameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        newCameraPos -= cameraSpeed * cameraFront;

    // Move left and right
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        newCameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        newCameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    // Clamp the new position to stay within the boundaries
    newCameraPos.x = glm::clamp(newCameraPos.x, minBoundary.x, maxBoundary.x);
    newCameraPos.y = glm::clamp(newCameraPos.y, minBoundary.y, maxBoundary.y);
    newCameraPos.z = glm::clamp(newCameraPos.z, minBoundary.z, maxBoundary.z);

    // Update the camera position
    cameraPos = newCameraPos;

    // Rotation controls (Q and E)
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraYaw -= rotationSpeed;  // Rotate counterclockwise
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraYaw += rotationSpeed;  // Rotate clockwise

    // Update camera front based on yaw (and pitch if needed)
    updateCameraFront();

    // Update the view matrix based on camera position
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}
#pragma endregion
#pragma region LOAD FUNCTIONS
// Function to load a texture
GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    std::cout << "Loading texture: " << path << std::endl; // Debug log

    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cerr << "Failed to load texture: " << path << std::endl; // Error log
        std::cerr << "STB Reason: " << stbi_failure_reason() << std::endl; // Log reason
        stbi_image_free(data);
    }

    return textureID;
}

// Function to load a texture with transparent background
GLuint loadTreeTexture(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha); // Force RGBA (with alpha channel)

    if (data) {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Upload texture data to OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        return texture;
    }
    else {
        std::cerr << "Failed to load texture" << std::endl;
        return 0;
    }
}

// Load images to skybox Cubemap
unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            // Check if the image has 3 or 4 channels and adjust the format
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
#pragma endregion
#pragma region Shaders
// Shader code
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;
out vec3 FragPos;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;

    FragPos = vec3(model * vec4(aPos, 1.0));
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

in vec3 FragPos; // Pass the fragment position from the vertex shader

uniform sampler2D texture1;
uniform vec3 cameraPos; // Camera position
uniform float fogStart; // Fog start distance
uniform float fogEnd;   // Fog end distance
uniform vec4 fogColor;  // Fog color

void main() {
    // Fetch texture color
    vec4 texColor = texture(texture1, TexCoord);

    // Skip fog blending for fully transparent pixels
    if (texColor.a < 0.1) {
        discard;
    }
    
    // Calculate distance from camera to the fragment
    float distance = length(cameraPos - FragPos);
    
    // Calculate fog factor (linear fade)
    float fogFactor = clamp((fogEnd - distance) / (fogEnd - fogStart), 0.0, 1.0);
    
    // Blend the fog color with the texture color based on fog factor
    vec4 color = mix(texColor, fogColor, fogFactor);

    FragColor = color;
}
)";

const char* skyboxVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main() {
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww; // Keep depth at maximum to prevent z-fighting
}
)";

const char* skyboxFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

uniform vec3 cameraPos; // Camera position
uniform float fogStart; // Fog start distance
uniform float fogEnd;   // Fog end distance
uniform vec4 fogColor;  // Fog color

void main() {
    // Sample the skybox texture
    vec4 skyboxColor = texture(skybox, TexCoords);

    // Calculate the distance from the camera to the "infinity"
    // For a skybox, use a fixed distance to simulate infinity, e.g., fogEnd + buffer
    float fogDistance = fogEnd + 10.0;

    // Calculate fog factor (linear fade)
    float fogFactor = clamp((fogEnd - fogDistance) / (fogEnd - fogStart), 0.0, 1.0);
    fogFactor = pow(fogFactor, 2.0); // Exponential curve for denser fog

    // Blend the fog color with the skybox color
    vec4 color = mix(fogColor, skyboxColor, fogFactor);

    FragColor = color;
}
)";


#pragma endregion
#pragma region Create Shader Program
// Compile and link shaders
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
#pragma endregion
#pragma region Vertices
// Vertices for the road (centered)
float roadVertices[] = {
    // Positions         // Texture Coords
    -3.0f,  0.0f, -25.0f,  0.0f, 0.0f,  // Left side, far end
    -3.0f,  0.0f,  25.0f,  0.0f, 5.0f,  // Left side, near end
     3.0f,  0.0f, -25.0f,  1.0f, 0.0f,  // Right side, far end
     3.0f,  0.0f,  25.0f,  1.0f, 5.0f   // Right side, near end
};

// Vertices for the grass (a simple grid around the road)
float grassVertices[] = {
    // Positions         // Texture Coords
    -100.0f,  0.0f, -25.0f,  0.0f, 0.0f,
    -100.0f,  0.0f,  50.0f,  0.0f, 10.0f,
     100.0f,  0.0f, -25.0f,  10.0f, 0.0f,
     100.0f,  0.0f,  50.0f,  10.0f, 10.0f
};

// Vertices for the wheat
float wheatVertices[] = {
    // Positions          // Texture Coords
    -0.1f, 0.0f,  0.0f,   0.0f, 0.0f,  // Bottom-left
     0.1f, 0.0f,  0.0f,   1.0f, 0.0f,  // Bottom-right
    -0.1f, 1.0f,  0.0f,   0.0f, 1.0f,  // Top-left
     0.1f, 1.0f,  0.0f,   1.0f, 1.0f   // Top-right
};

// Vertices for a simple quad (for tree sprite)
float treeVertices[] = {
    // Positions         // Texture Coords
    -0.5f,  0.0f, -0.5f,   0.0f, 0.0f,  // Bottom-left
     0.5f,  0.0f, -0.5f,   1.0f, 0.0f,  // Bottom-right
    -0.5f,  1.0f, -0.5f,   0.0f, 1.0f,  // Top-left
     0.5f,  1.0f, -0.5f,   1.0f, 1.0f   // Top-right
};

// Skybox vertices
float skyboxVertices[] = {
    // positions         // 2 Triangles to make a Square 
    -1.0f,  1.0f, -1.0f, // Triangle 1
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f, // Triangle 2
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};
#pragma endregion
#pragma region FRAMBUFFER and Render tree function
// GLFW framebuffer size callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Adjust the viewport to match the new window dimensions
    glViewport(0, 0, width, height);
}

void renderTree(GLuint treeVAO, GLuint treeTexture, GLint modelLoc, glm::vec3 translation) {
    // Create the tree model matrix
    glm::mat4 treeModel = glm::mat4(1.0f);
    treeModel = glm::translate(treeModel, translation); // Apply translation
    treeModel = glm::scale(treeModel, glm::vec3(4.0f, 4.0f, 4.0f)); // Apply scaling
    treeModel = glm::scale(treeModel, glm::vec3(1.0f, -1.0f, 1.0f)); // Flip along y-axis (if needed)

    // Set the model matrix uniform
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(treeModel));

    // Bind the tree VAO and texture, then draw the tree
    glBindVertexArray(treeVAO);
    glBindTexture(GL_TEXTURE_2D, treeTexture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
#pragma endregion
#pragma region Main Render Function
int main() {
    // GLFW initialization
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Medieval Scene", nullptr, nullptr);

    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glewInit();

    // Shader program
    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    GLuint skyboxShaderProgram = createShaderProgram(skyboxVertexShaderSource, skyboxFragmentShaderSource);

    // Road and grass texture loading
    GLuint roadTexture = loadTexture("../assets/textures/road.jpg");
    GLuint grassTexture = loadTexture("../assets/textures/grass-texture.jpg");

    GLuint wheatTexture = loadTexture("../assets/textures/wheat-texture.png");

    GLuint treeTexture = loadTreeTexture("../assets/textures/tree-texture.png");

    // House load
    std::vector<Mesh> meshes;
    loadModel(R"(../assets/house/medieval house.obj)", meshes);
    std::vector<Mesh> castleMeshes;
    loadModel(R"(../assets/castle/Palace.obj)", castleMeshes);

    // Skybox texture loading
    std::vector<std::string> faces = {
        R"(..\assets\skybox\nx.png)",
        R"(..\assets\skybox\px.png)",
        R"(..\assets\skybox\ny.png)",
        R"(..\assets\skybox\py.png)",
        R"(..\assets\skybox\nz.png)",
        R"(..\assets\skybox\pz.png)"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    // Set up road and grass VAOs and VBOs
    GLuint roadVAO, roadVBO;
    glGenVertexArrays(1, &roadVAO);
    glGenBuffers(1, &roadVBO);
    glBindVertexArray(roadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, roadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(roadVertices), roadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLuint grassVAO, grassVBO;
    glGenVertexArrays(1, &grassVAO);
    glGenBuffers(1, &grassVBO);
    glBindVertexArray(grassVAO);
    glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grassVertices), grassVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Set up skybox VBO and VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    // Set up wheat and trees
    GLuint wheatVAO, wheatVBO;
    glGenVertexArrays(1, &wheatVAO);
    glGenBuffers(1, &wheatVBO);
    glBindVertexArray(wheatVAO);
    glBindBuffer(GL_ARRAY_BUFFER, wheatVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wheatVertices), wheatVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    GLuint treeVAO, treeVBO;
    glGenVertexArrays(1, &treeVAO);
    glGenBuffers(1, &treeVBO);
    glBindVertexArray(treeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, treeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(treeVertices), treeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Enable depth test for regular objects
        glEnable(GL_DEPTH_TEST);

        // Clear the color and depth buffers
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glUniform3fv(glGetUniformLocation(shaderProgram, "cameraPos"), 1, glm::value_ptr(cameraPos)); // Camera position
        glUniform1f(glGetUniformLocation(shaderProgram, "fogStart"), 30.0f);  // Fog start distance
        glUniform1f(glGetUniformLocation(shaderProgram, "fogEnd"), 5.0f);    // Fog end distance
        glUniform4fv(glGetUniformLocation(shaderProgram, "fogColor"), 1, glm::value_ptr(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f))); // Light gray fog color

        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Render the skybox (with depth testing but no depth writes)
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);  // Skybox should be rendered behind everything
        glUseProgram(skyboxShaderProgram);

        // Set fog parameters
        glUniform3fv(glGetUniformLocation(skyboxShaderProgram, "cameraPos"), 1, glm::value_ptr(cameraPos));
        glUniform1f(glGetUniformLocation(skyboxShaderProgram, "fogStart"), 50.0f);  // Fog start distance
        glUniform1f(glGetUniformLocation(skyboxShaderProgram, "fogEnd"), 5.0f);    // Fog end distance
        glUniform4fv(glGetUniformLocation(skyboxShaderProgram, "fogColor"), 1, glm::value_ptr(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f))); // Fog color

        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));  // Remove translation from view matrix
        glUniformMatrix4fv(glGetUniformLocation(skyboxShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(skyboxView));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Bind and render the skybox
        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // Reset depth function to GL_LESS for rendering other objects
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        glUseProgram(shaderProgram);

        // Render the road (centered)
        glm::mat4 roadModel = glm::mat4(1.0f);
        roadModel = glm::rotate(roadModel, glm::radians(-270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        roadModel = glm::translate(roadModel, glm::vec3(0.0f, 0.05f, 0.0f));
        glBindVertexArray(roadVAO);
        glBindTexture(GL_TEXTURE_2D, roadTexture);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(roadModel));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Render the grass
        glm::mat4 grassModel = glm::mat4(1.0f);
        grassModel = glm::rotate(grassModel, glm::radians(-270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        grassModel = glm::translate(grassModel, glm::vec3(0.0f, 0.0f, 0.0f));
        glBindVertexArray(grassVAO);
        glBindTexture(GL_TEXTURE_2D, grassTexture);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(grassModel));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Render the wheat fields (with a grid pattern)
        float wheatOffset = 40.0f;
        glBindVertexArray(wheatVAO);
        glBindTexture(GL_TEXTURE_2D, wheatTexture);

        // Grid of wheat
        for (int x = -100; x <= 100; ++x) {
            for (int z = -20; z <= 20; ++z) {
                glm::mat4 wheatModel = glm::mat4(1.0f);
                wheatModel = glm::translate(wheatModel, glm::vec3(x * 0.25f, 0.0f, wheatOffset + z * 0.25f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(wheatModel));
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
        }

        // Set transformation matrices
        glm::mat4 houseModel1 = glm::mat4(1.0f);
        houseModel1 = glm::translate(houseModel1, glm::vec3(5.0f, 0.0f, 15.0f));
        houseModel1 = glm::rotate(houseModel1, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        houseModel1 = glm::scale(houseModel1, glm::vec3(0.5, 0.5, 0.5));

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(houseModel1));

        for (auto& mesh : meshes) {
            mesh.draw();
        }

        glm::mat4 houseModel2 = glm::mat4(1.0f);
        houseModel2 = glm::translate(houseModel2, glm::vec3(8.0f, 0.0f, -30.0f));
        houseModel2 = glm::rotate(houseModel2, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        houseModel2 = glm::scale(houseModel2, glm::vec3(0.5, 0.5, 0.5));

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(houseModel2));

        for (auto& mesh : meshes) {
            mesh.draw();
        }

        // Set up castle model transformation
        glm::mat4 castleModel = glm::mat4(1.0f);
        castleModel = glm::translate(castleModel, glm::vec3(35.0f, 0.0f, 0.0f));
        castleModel = glm::scale(castleModel, glm::vec3(1.20, 1.20, 1.20));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(castleModel));
        for (auto& mesh : castleMeshes) {
            mesh.draw();
        }

        // Enable blending for transparent objects
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Render trees (furthest back rendered first to not overlap and hide trees behind)
        renderTree(treeVAO, treeTexture, modelLoc, glm::vec3(0.0f, 4.0f, -30.0f));
        renderTree(treeVAO, treeTexture, modelLoc, glm::vec3(15.0f, 4.0f, -27.0f));
        renderTree(treeVAO, treeTexture, modelLoc, glm::vec3(5.0f, 4.0f, -25.0f));
        renderTree(treeVAO, treeTexture, modelLoc, glm::vec3(-11.0f, 4.0f, -25.0f));
        renderTree(treeVAO, treeTexture, modelLoc, glm::vec3(-5.0f, 4.0f, -20.0f));
        renderTree(treeVAO, treeTexture, modelLoc, glm::vec3(15.0f, 4.0f, -20.0f));
        renderTree(treeVAO, treeTexture, modelLoc, glm::vec3(20.0f, 4.0f, -15.0f));
        renderTree(treeVAO, treeTexture, modelLoc, glm::vec3(-10.0f, 4.0f, -14.0f));
        // Disable blend
        glDisable(GL_BLEND);

        glBindTexture(GL_TEXTURE_2D, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // Clean up

    glDeleteVertexArrays(1, &roadVAO);
    glDeleteBuffers(1, &roadVBO);
    glDeleteVertexArrays(1, &treeVAO);
    glDeleteBuffers(1, &treeVBO);
    glDeleteVertexArrays(1, &wheatVAO);
    glDeleteBuffers(1, &wheatVBO);
    glDeleteVertexArrays(1, &grassVAO);
    glDeleteBuffers(1, &grassVBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(skyboxShaderProgram);

    glfwTerminate();
    return 0;
}
#pragma endregion