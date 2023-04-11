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
#if __has_include(<FreeImage.h>)
#define FREEIMAGE
#include <FreeImage.h>
#endif

/** constant variable **/
// WIDTH and HEIGHT are set in renderer.h
const double TIME_STEP = 1.0 / 60.0;
const glm::vec3 backgroundColor(50.0 / 255, 50.0 / 255, 60.0 / 255);
const glm::vec3 ClothPosition(-8, 9, -4);
const glm::vec2 ClothSize(16, 16);
const int TOTAL_FRAME = 400; // used for certain frame simulation
const bool Record = false; // true means after TOTAL_FRAME, the simulation will stop immediately
const bool showTime = false; // whether to show time on the left up corner
const float FONT_SIZE = 25;  // displayed UI font size
const int GLFW_INTERVAL = 0; // set interval if needed
/** end of constant variable **/

/** global variable **/
MethodClass Method = M_PBD;
int isRunning = Record ? TOTAL_FRAME : -1;
glm::vec2 ClothNodesNumber = Method.MethodClothNodesNumber;
int ClothIteration = Method.MethodIteration;
int constraintLevel = 3; // 0: no bending constraint, 1: only diagonal bending constraint, 2: only edge bending constraint, 3: all bending constraint
Cloth cloth;
ClothRenderer clothRenderer;
TextRenderer textRenderer;
std::string RECORD_SAVE_PATH = "C:/Users/jk151/Desktop/实验结果/实验1. 调整迭代步数与时间步长/";
int photoCount = 1;
/** end of constant variable **/

/** function statement **/
void Init();
void CallBackFunctionsInit(GLFWwindow* window);
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebufferSizeCallBack(GLFWwindow* window, int width, int height);
void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset);
void savePicture();
/** end of function statement **/

int main(int argc, const char* argv[]) 
{
    // Prepare for rendering
    printf("******************************\n");
    printf("Initializing OpenGL.\n");
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
    printf("OpenGL initialized with no error.\n");
    printf("******************************\n");
    // TODO:input

    // set camera to appropriate position
    if (!showTime)
    {
        camera.Zoom = 40.0f;
    }

    Init();
    printf("******************************\n");
    printf("Building shaders.\n");
    clothRenderer.init(&cloth);
    textRenderer.init(FONT_SIZE);
    printf("Shaders built with no error.\n");
    printf("******************************\n");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(3);

    std::string outputFrameTime, outputTotalTime;
    GLdouble subTimeStep = TIME_STEP / cloth.Iteration;
    float currentFrame, lastFrame, deltaTime; // count every frame time
    float beginTime = static_cast<float>(glfwGetTime()), endTime, averageTime; // count total simulation time
    int simulationFrame = 0;
    glfwSwapInterval(GLFW_INTERVAL);
    cloth.UpdateVelocity(VEL_BACK, cloth.DEFAULT_FORCE * 0.02);
    while (!glfwWindowShouldClose(window)) 
    {
        /** per-frame time logic **/
        glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (showTime)
            lastFrame = static_cast<float>(glfwGetTime());  
        /** end of per-frame time logic **/
        
        /** simulating & rendering **/
        if (isRunning)
        {
            if (Record)
            {
                if (simulationFrame % 20 == 0) cloth.UpdateVelocity(VEL_BACK, cloth.DEFAULT_FORCE); // make the cloth has y velocity
                cloth.UpdateVelocity(VEL_DOWN, cloth.DEFAULT_FORCE * 0.1);
            }
            switch (Method.getId())
            {
                case PPBD:
                case PBD:
                    cloth.Integrate(TIME_STEP);
                    break;
                case PPBD_SS:
                    for (int subStep = 0; subStep < cloth.Iteration; subStep++) 
                    {
                        cloth.Integrate(subTimeStep);
                    }
                    break;
            }
            cloth.computeNormal();
        }

        clothRenderer.render();
        /** end of simulating & rendering **/
        
        /** post-frame time logic **/
        /** display time**/
        if (showTime)
        {
            currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;

            if (isRunning != 0)
            {
                outputFrameTime = std::to_string(deltaTime * 1000);
                for (int i = 0; i < 4; i++) outputFrameTime.pop_back(); // only display 2 precision
                outputFrameTime += " ms per frame";
            }
            textRenderer.RenderText(outputFrameTime, 25.0f, HEIGHT - 40.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
            endTime = static_cast<float>(glfwGetTime());
            outputTotalTime = std::to_string(endTime);
            for (int i = 0; i < 4; i++) outputTotalTime.pop_back(); // only display 2 precision
            outputTotalTime += "s in total.";
            textRenderer.RenderText(outputTotalTime, 25.0f, HEIGHT - 80.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        }
        if (Record && isRunning == 0)
        {
            endTime = static_cast<float>(glfwGetTime());
            averageTime = (endTime - beginTime) / TOTAL_FRAME;
            printf("The total simulation time of %d frames is: %.2f ms, average time per frame is: %.2f ms\n", TOTAL_FRAME, (endTime - beginTime) * 1000, averageTime * 1000);
            savePicture();
            break;
        }
        /** end of display time**/
        
        if (isRunning > 0) isRunning--;
        simulationFrame++;
        /* end of post-frame time logic **/

        glfwSwapBuffers(window);
        glfwPollEvents(); // Update the status of window
    }
    glfwTerminate();
	return 0;
}
// Init cloth and other things using input and default value
void Init()
{
    printf("******************************\n");
    printf("Initializing the cloth.\n");
    printf("");
    cloth.set(ClothPosition, ClothSize, ClothNodesNumber, Method.getId(), ClothIteration, constraintLevel);
    printf("Cloth initialized with no error.\n");
    printf("******************************\n");
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
            if (action == GLFW_PRESS)
            {
                cloth.reset();
                cloth.UpdateVelocity(VEL_BACK, cloth.DEFAULT_FORCE * 0.02);
                if (!Record)
                {
                    isRunning = 0;
                    clothRenderer.ClothObject = &cloth;
                    glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    clothRenderer.render();
                    glfwSwapBuffers(window);
                    glfwPollEvents();
                }
                else
                    isRunning = TOTAL_FRAME;
            }
            break;

        // up, down, left and right will pull the cloth with certain force.
        case GLFW_KEY_UP:
            if (action == GLFW_PRESS)
                cloth.UpdateVelocity(VEL_FRONT);
            break;
        case GLFW_KEY_DOWN:
            if (action == GLFW_PRESS)
                cloth.UpdateVelocity(VEL_BACK);
            break;
        case GLFW_KEY_LEFT:
            if (action == GLFW_PRESS)
                cloth.UpdateVelocity(VEL_LEFT_AND_UP);
            break;
        case GLFW_KEY_RIGHT:
            if (action == GLFW_PRESS)
                cloth.UpdateVelocity(VEL_RIGHT_AND_UP);
            break;
#ifdef FREEIMAGE
        // press M to screenshot
        case GLFW_KEY_M:
            if (action == GLFW_PRESS)
            {
                savePicture();
            }
#endif
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
void savePicture()
{
    BYTE* pixels = new BYTE[3 * WIDTH * HEIGHT]; // BGR
    glReadPixels(0, 0, WIDTH, HEIGHT, GL_BGR, GL_UNSIGNED_BYTE, pixels);
    // Convert to FreeImage format & save to file
    FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, WIDTH, HEIGHT, 3 * WIDTH, 24, 0x0000FF, 0xFF0000, 0x00FF00, false);
    std::string folderPath = RECORD_SAVE_PATH + "methods=" + Method.getName();
    if (!std::filesystem::exists(folderPath))
        std::filesystem::create_directory(folderPath);
    folderPath += "/dt=" + std::to_string((int)round(1 / TIME_STEP));
    folderPath += " iteration=" + std::to_string(cloth.Iteration);
    if (!std::filesystem::exists(folderPath))
        std::filesystem::create_directory(folderPath);
    std::string photoPrefix = "/" + Method.getName() + " dt=" + std::to_string((int)round(1 / TIME_STEP)) + " iteration=" + std::to_string(cloth.Iteration);
    std::string photoName = folderPath + photoPrefix + " " + std::to_string(photoCount) + ".png";
    FreeImage_Save(FIF_PNG, image, photoName.c_str(), 0);
    std::cout << "Save screenshot as " + photoName << std::endl;
    photoCount++;
    // Free resources
    FreeImage_Unload(image);
    delete[] pixels;
}