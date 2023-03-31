#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <map>
#include <cmath>
#include <ft2build.h>
#include FT_FREETYPE_H  
#include "headers/renderer.h"

/** constant variable **/
// WIDTH and HEIGHT are set in renderer.h
const double TIME_STEP = 1.0 / 60.0;
const glm::vec3 backgroundColor(50.0 / 255, 50.0 / 255, 60.0 / 255);
const glm::vec3 ClothPosition(-2.5, 5, -4);
const glm::vec2 ClothSize(5, 10);
const glm::vec2 ClothNodesNumber(60, 90); // (w, h)
/** end of constant variable **/

/** global variable **/
int ClothIteration = 10;
int isRunning = 1;
Cloth cloth(ClothPosition, ClothSize, ClothNodesNumber, ClothIteration);
ClothRenderer clothRenderer;
TextRenderer textRenderer;
/** end of constant variable **/

/** function statement **/
void CallBackFunctionsInit(GLFWwindow* window);
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebufferSizeCallBack(GLFWwindow* window, int width, int height);
void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset);
int loadTexture(const std::string texturePath);
/** end of function statement **/

int main(int argc, const char* argv[]) 
{
    // Prepare for rendering
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Try to create a GLFW window
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Cloth Simulation", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create a GLFW window." << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set the context of this window as the main context of current thread
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
    {
        std::cout << "Failed to initialize GLAD." << std::endl;
        return -1;
    }

    // Register all callback functions
    CallBackFunctionsInit(window);

    clothRenderer.init(&cloth);
    textRenderer.init(30);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(3);

    GLdouble subTimeStep = TIME_STEP / cloth.Iteration;
    std::string output;
    float currentFrame, lastFrame, deltaTime;
    while (!glfwWindowShouldClose(window)) 
    {
        /** per-frame time logic **/
        glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        lastFrame = static_cast<float>(glfwGetTime());
        /** end of per-frame time logic **/

        /** simulating & rendering **/
        if (isRunning)
        {
            for (int subStep = 0; subStep < cloth.Iteration; subStep++) {
                cloth.Integrate(subTimeStep);
            }
            cloth.computeNormal();
        }

        clothRenderer.render();
        /** end of simulating & rendering **/
        
        /** post-frame time logic **/
        currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        if (isRunning != 0)
        {
            output = std::to_string(deltaTime * 1000);
            for (int i = 0; i < 4; i++) output.pop_back(); // only display 2 precision
            output += " ms per frame";
        }
        textRenderer.RenderText(output, 25.0f, HEIGHT - 50.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        if (isRunning > 0) isRunning--;
        /* end of post-frame time logic **/
        
        glfwSwapBuffers(window);
        glfwPollEvents(); // Update the status of window
    }
    glfwTerminate();
	return 0;
}

// Register callback functions
void CallBackFunctionsInit(GLFWwindow* window)
{
    glfwSetKeyCallback(window, keyCallBack);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallBack);
    glfwSetScrollCallback(window, scrollCallBack);
}
// utility function for dealing with keyboard control
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (key)
    {
        // Exit: ESC
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, true);
            break;

        // P: switch pause and execute
        case GLFW_KEY_P:
            if (action == GLFW_PRESS)
            {
                if (isRunning == -1)
                {
                    isRunning = 0;
                    std::cout << "Paused." << std::endl;
                }
                else
                {
                    isRunning = -1;
                    std::cout << "Executing." << std::endl;
                }
            }
            break;
        // T: step 1 time
        case GLFW_KEY_T:
            isRunning = 1;
            std::cout << "Step 1 time." << std::endl;
            break;

        // Z, X, C: switch display mode
        case GLFW_KEY_Z:
            cloth.drawMode = Cloth::DRAW_NODES;
            break;
        case GLFW_KEY_X:
            cloth.drawMode = Cloth::DRAW_LINES;
            break;
        case GLFW_KEY_C:
            cloth.drawMode = Cloth::DRAW_FACES;
            break;

        // W, S, A, D: move camera
        case GLFW_KEY_W:
            camera.ProcessKeyboard(DIR_FORWARD);
            break;
        case GLFW_KEY_S:
            camera.ProcessKeyboard(DIR_BACKWARD);
            break;
        case GLFW_KEY_A:
            camera.ProcessKeyboard(DIR_LEFT);
            break;
        case GLFW_KEY_D:
            camera.ProcessKeyboard(DIR_RIGHT);
            break;

        // R: reset the scene and perform 1 render loop
        case GLFW_KEY_R:
            cloth.reset();
            isRunning = 0;
            clothRenderer.ClothObject = &cloth;
            glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            clothRenderer.render();
            glfwSwapBuffers(window);
            glfwPollEvents();
            break;

        // up, down, left and right will pull the cloth with certain force.
        case GLFW_KEY_UP:
            if (action == GLFW_PRESS)
            {
                cloth.UpdateVelocity(VEL_UP);
            }
            break;
        case GLFW_KEY_DOWN:
            if (action == GLFW_PRESS)
            {
                cloth.UpdateVelocity(VEL_DOWN);
            }
            break;
        case GLFW_KEY_LEFT:
            if (action == GLFW_PRESS)
            {
                cloth.UpdateVelocity(VEL_LEFT);
            }
            break;
        case GLFW_KEY_RIGHT:
            if (action == GLFW_PRESS)
            {
                cloth.UpdateVelocity(VEL_RIGHT);
            }
            break;

    };
}
void framebufferSizeCallBack(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
