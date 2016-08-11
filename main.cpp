#include <iostream>
#include <cmath>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>

// GLM Mathemtics
#define GLM_FORCE_RADIANS // force everything in radian
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Config
#include "Config.h"

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#include <SOIL.h>

#define RAND_MAX 1

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void do_movement();
void RenderScene(Model &ourModel, Shader &shader);
GLuint loadTexture(GLchar* path);

// Window dimensions
const GLuint WIDTH = 1280, HEIGHT = 720;

// Camera
Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX  =  WIDTH  / 2.0;
GLfloat lastY  =  HEIGHT / 2.0;
bool    keys[1024];

// Light
glm::vec3 lightPos(-2.0f, 12.0f, -3.0f);
glm::vec3 lightInvDir(-2, 12, -11);
// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

int main()
{
    // Init windows
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create window object
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Final Project", nullptr, nullptr);
    // Check if window is null or else terminate
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Make window
    glfwMakeContextCurrent(window);

    // Set callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Init GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // OpenGL options
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    // Build and compile our shader program
    std::string vert_path;
    vert_path.append(FILE_PATH);
    vert_path.append("shader/nvidia.vert");
    std::string frag_path;
    frag_path.append(FILE_PATH);
    frag_path.append("shader/nvidia.frag");

    std::string simple_depth_vert_path;
    simple_depth_vert_path.append(FILE_PATH);
    simple_depth_vert_path.append("shader/shadow_mapping_depth.vert");
    std::string simple_depth_frag_path;
    simple_depth_frag_path.append(FILE_PATH);
    simple_depth_frag_path.append("shader/shadow_mapping_depth.frag");

    Shader shader(vert_path.c_str(), frag_path.c_str());
    Shader simpleDepthShader(simple_depth_vert_path.c_str(), simple_depth_frag_path.c_str());

    // Load models

    std::string model_obj_path;
    model_obj_path.append(FILE_PATH);
    model_obj_path.append("ground/sphere.obj");
    char *cstrModel = &model_obj_path[0u];

    std::string solid_obj_path;
    solid_obj_path.append(FILE_PATH);
    solid_obj_path.append("ground/hamburger.obj");
    char *cstrSolidModel = &solid_obj_path[0u];

    string floor_obj_path;
    floor_obj_path.append(FILE_PATH);
    floor_obj_path.append("ground/ground.obj");
    char *cstrFloorModel = &floor_obj_path[0u];

    Model ourModel(cstrModel);
    Model floorModel(cstrFloorModel);
    Model solidModel(cstrSolidModel);

    // Load and create a texture
    GLuint oceanHeightTexture, oceanNormalTexture, lightMapTexture;
    int textureWidth, textureHeight;
    std::string textureOceanHeight;
    textureOceanHeight.append(FILE_PATH);
    textureOceanHeight.append("maps/sine_wave_height.png");
    char *cstrTextureOceanHeight = &textureOceanHeight[0u];
    std::string textureOceanNormal;
    textureOceanNormal.append(FILE_PATH);
    textureOceanNormal.append("maps/sine_wave_normal.png");
    char *cstrtextureOceanNormal = &textureOceanNormal[0u];
    std::string lightMapFile;
    lightMapFile.append(FILE_PATH);
    lightMapFile.append("maps/light_map.png");
    char *cstrlightMapFile = &lightMapFile[0u];

    oceanHeightTexture = loadTexture(cstrTextureOceanHeight);
    oceanNormalTexture = loadTexture(cstrtextureOceanNormal);
    lightMapTexture = loadTexture(cstrlightMapFile);

    // Draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Set texture samples

    //Don't understand why arragmenet of code screws it up. DO NOT TOUCH
    simpleDepthShader.Use();
    glUniform1i(glGetUniformLocation(shader.Program, "shadowMap"), 28);
    GLuint DepthBiasID = glGetUniformLocation(shader.Program, "DepthBiasMVP");
    shader.Use();
    // Get a handle for our "MVP" uniform
    GLuint depthMatrixID = glGetUniformLocation(simpleDepthShader.Program, "depthMVP");


    // Configure depth map FBO
    const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    // - Create depth texture
    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
//    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
//    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);

    // Always check that our framebuffer is ok
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return false;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clear the colorbuffer
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    // The loop
    glfwSetTime(0);
    while(!glfwWindowShouldClose(window))
    {
        // Set frame time
        double currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check and call events
        glfwPollEvents();
        do_movement();

        // Compute the MVP matrix from the light's point of view
        glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10,10,-10,10,-10,20);
        glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));

        glm::mat4 depthModelMatrix = glm::mat4(1.0);
        glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(depthMatrixID, 1, GL_FALSE, &depthMVP[0][0]);

        glViewport(0, 0, 1024, 1024);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        // Draw the loaded model
        glm::mat4 model_solid;
        model_solid = glm::translate(model_solid, glm::vec3(0.0f, -1.75f, 1.0f)); // Translate it down a bit so it's at the center of the scene
        model_solid = glm::scale(model_solid, glm::vec3(0.2, 0.2, 0.2));	// It's a bit too big for our scene, so scale it down
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model_solid));
        glUniform3f(glGetUniformLocation(shader.Program, "objectColor"),  1.0f, 0.0f, 0.0f);
        solidModel.Draw(shader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. Render scene as normal
        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //shader.Use();   // <-- Don't forget this one!
        //simpleDepthShader.Use();
        // Transformation matrices
        glm::mat4 projection = glm::perspective(camera.Zoom, (float)WIDTH/(float)HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 biasMatrix(
            0.5, 0.0, 0.0, 0.0,
            0.0, 0.5, 0.0, 0.0,
            0.0, 0.0, 0.5, 0.0,
            0.5, 0.5, 0.5, 1.0
        );

        glm::mat4 depthBiasMVP = biasMatrix * depthMVP;

        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(DepthBiasID, 1, GL_FALSE, &depthBiasMVP[0][0]);

        GLint lightColorLoc  = glGetUniformLocation(shader.Program, "lightColor");
        GLint lightPosLoc    = glGetUniformLocation(shader.Program, "lightPos");
        GLint viewPosLoc     = glGetUniformLocation(shader.Program, "viewPos");
        glUniform3f(lightColorLoc,  0.0f, 0.0f, 0.0f);
        glUniform3f(lightPosLoc,    lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(viewPosLoc,     camera.Position.x, camera.Position.y, camera.Position.z);

        // Bind Textures using texture units
        glActiveTexture(GL_TEXTURE28);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glActiveTexture(GL_TEXTURE29);
        glBindTexture(GL_TEXTURE_2D, oceanHeightTexture);
        glUniform1i(glGetUniformLocation(shader.Program, "oceanHeight"), 29);
        glActiveTexture(GL_TEXTURE30);
        glBindTexture(GL_TEXTURE_2D, oceanNormalTexture);
        glUniform1i(glGetUniformLocation(shader.Program, "oceanNormal"), 30);
        glActiveTexture(GL_TEXTURE31);
        glBindTexture(GL_TEXTURE_2D, lightMapTexture);
        glUniform1i(glGetUniformLocation(shader.Program, "lightMap"), 31);

        // Draw the loaded model
        glm::mat4 model;
        model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// It's a bit too big for our scene, so scale it down
        glUniform3f(glGetUniformLocation(shader.Program, "objectColor"),  1.0f, 1.0f, 1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        floorModel.Draw(shader);
        // Draw the loaded model
        glUniform3f(glGetUniformLocation(shader.Program, "objectColor"),  1.0f, 0.0f, 0.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model_solid));
        solidModel.Draw(shader);

        glUniform3f(glGetUniformLocation(shader.Program, "objectColor"),  1.0f, 1.0f, 1.0f);
        // Draw the loaded model
        glm::mat4 model_sphere;
        model_sphere = glm::translate(model_sphere, glm::vec3(cos(currentFrame)/10+0.1f, currentFrame/13 -2.0f, -0.05f)); // Translate it down a bit so it's at the center of the scene
        model_sphere = glm::scale(model_sphere, glm::vec3(0.1f, 0.1f, 0.1f));	// It's a bit too big for our scene, so scale it down
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model_sphere));
        ourModel.Draw(shader);

        glm::mat4 model_sphere2;
        model_sphere2 = glm::translate(model_sphere2, glm::vec3(cos(currentFrame)/10-0.5f, currentFrame/16 -0.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
        model_sphere2 = glm::scale(model_sphere2, glm::vec3(0.05f, 0.05f, 0.05f));	// It's a bit too big for our scene, so scale it down
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model_sphere2));
        ourModel.Draw(shader);

        glm::mat4 model_sphere3;
        model_sphere3 = glm::translate(model_sphere3, glm::vec3(cos(currentFrame)/10-0.58f, currentFrame/16 -0.78f, 0.0f)); // Translate it down a bit so it's at the center of the scene
        model_sphere3 = glm::scale(model_sphere3, glm::vec3(0.03f, 0.03f, 0.03f));	// It's a bit too big for our scene, so scale it down
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model_sphere3));
        ourModel.Draw(shader);

        // Swap the buffers
        glfwSwapBuffers(window);
    }

    // Terminate at the end
    glfwTerminate();
    return 0;
}

// This function loads a texture from file. Note: texture loading functions like these are usually
// managed by a 'Resource Manager' that manages all resources (like textures, models, audio).
// For learning purposes we'll just define it as a utility function.
GLuint loadTexture(GLchar* path)
{
    // Generate texture ID and load texture data
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width, height;
    unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);
    // Assign texture to ID
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);
    return textureID;

}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void do_movement()
{
    // Camera controls
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (keys[GLFW_KEY_LEFT])
        lightInvDir.x -= 1;
    if (keys[GLFW_KEY_RIGHT])
        lightInvDir.x += 1;
    if (keys[GLFW_KEY_DOWN])
        lightInvDir.z -= 1;
    if (keys[GLFW_KEY_UP])
        lightInvDir.z += 1;
    if (keys[GLFW_KEY_MINUS])
        lightInvDir.y -= 1;
    if (keys[GLFW_KEY_EQUAL])
        lightInvDir.y += 1;

    //cout << lightInvDir.x << ", " << lightInvDir.y << ", " << lightInvDir.z << "\n";
}

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
