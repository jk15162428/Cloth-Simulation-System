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
const glm::vec3 backgroundColor(50.0 / 255, 50.0 / 255, 60.0 / 255);
const glm::vec3 ClothPosition(-8, 9, -4);
const glm::vec2 ClothSize(16, 16);
const int TOTAL_FRAME = 1000; // used for certain frame simulation
const bool Record = false; // true means after TOTAL_FRAME, the simulation will stop immediately
const bool showTime = false; // whether to show time on the left up corner
const float FONT_SIZE = 25;  // displayed UI font size
const int GLFW_INTERVAL = 0; // set interval if needed
/** end of constant variable **/

/** global variable **/
double TIME_STEP = 1.0 / 60.0;
MethodClass Method = M_PPBD;
int isRunning = Record ? TOTAL_FRAME : 0;
int simulationFrame = -1;
glm::vec2 ClothNodesNumber = Method.MethodClothNodesNumber;
int ClothIteration = Method.MethodIteration;
Cloth cloth;
ClothRenderer clothRenderer;
TextRenderer textRenderer;
std::string RECORD_SAVE_PATH = ((std::filesystem::path)std::filesystem::current_path()).string() + "\\exp\\";
std::string TEXT_SAVE_PATH = ((std::filesystem::path)std::filesystem::current_path()).string() + "\\text\\";
int photoCount = 1;
/** end of constant variable **/

/** function statement **/
void Init();
void CallBackFunctionsInit(GLFWwindow* window);
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebufferSizeCallBack(GLFWwindow* window, int width, int height);
void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset);
void savePicture();
void methodInput();
/** end of function statement **/

int main(int argc, const char* argv[]) 
{
    // deal with input
    methodInput();

    // Prepare for rendering
    printf("******************************\n");
    printf("Initializing OpenGL...\n");
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

    // set camera to appropriate position
    if (!showTime)
    {
        camera.Zoom = 40.0f;
    }

    Init();
    printf("******************************\n");
    printf("Building shaders...\n");
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
            //cloth.UpdateVelocity(VEL_BACK, cloth.DEFAULT_FORCE * 0.05);
            //cloth.UpdateVelocity(VEL_DOWN, cloth.DEFAULT_FORCE * 0.05);
            switch (Method.getId())
            {
                case XPBD_SS:
                    for (int subStep = 0; subStep < cloth.Iteration; subStep++) 
                    {
                        cloth.Integrate(subTimeStep);
                    }
                    break;
                default:
                    cloth.Integrate(TIME_STEP);
                    break;
            }
            cloth.computeNormal();
            simulationFrame++;
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
            //savePicture();
            break;
        }
        /** end of display time**/
        
        if (isRunning > 0) isRunning--;
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
    printf("Initializing the cloth...\n");
    printf("");
    cloth.set(ClothPosition, ClothSize, Method);
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
            if (action == GLFW_RELEASE) {
                isRunning = 1;
                std::cout << "Step 1 time." << std::endl;
            }
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
                simulationFrame = 1;
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
        case GLFW_KEY_N:
            if (action == GLFW_RELEASE) {
                isRunning = 1;
                if (!std::filesystem::exists(TEXT_SAVE_PATH))
                    std::filesystem::create_directory(TEXT_SAVE_PATH);
                std::string folderPath = TEXT_SAVE_PATH + "methods=" + Method.getName();
                if (!std::filesystem::exists(folderPath))
                    std::filesystem::create_directory(folderPath);
                folderPath += "/dt=" + std::to_string((int)round(1 / TIME_STEP));
                folderPath += " iteration=" + std::to_string(cloth.Iteration);
                if (!std::filesystem::exists(folderPath))
                    std::filesystem::create_directory(folderPath);
                std::string textPrefix = "/" + Method.getName() + " dt=" + std::to_string((int)round(1 / TIME_STEP)) + " iteration=" + std::to_string(cloth.Iteration);
                std::string photoName = folderPath + textPrefix + " " + std::to_string(simulationFrame) + ".txt";
                std::ofstream clothFile;
                clothFile.open(photoName);
                clothFile << "Nodes Position: " << std::endl;
                for (Node* node : cloth.Nodes) {
                    clothFile << node->Position.x << " " << node->Position.y << " " << node->Position.z << std::endl;
                }
                clothFile.close();
            }
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
    if (!std::filesystem::exists(RECORD_SAVE_PATH))
        std::filesystem::create_directory(RECORD_SAVE_PATH);
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
void methodInput() {
    printf("Please choose the simulation method.\n");
    printf("1. XPBD method with iteration = 10.\n");
    printf("2. PBD method with iteration = 20.\n");
    printf("3. XPBD_SS method with iteration = 10.\n");
    printf("4. Verlet_Intergration with iteration = 40.\n");
    printf("5. Explicit_Euler with iteration = 100.\n");
    printf("6. Semi_Implicit_Euler with iteration = 40.\n");
    printf("Enter the method number: ");
    int inputMethodNum = -1;
    std::cin >> inputMethodNum;
    switch (inputMethodNum)
    {
    case 1:
        Method = M_PPBD;
        break;
    case 2:
        Method = M_PBD;
        break;
    case 3:
        Method = M_PPBD_SS;
        break;
    case 4:
        Method = M_Verlet_Integration;
        break;
    case 5:
        Method = M_Explicit_Euler;
        break;
    case 6:
        Method = M_Semi_Implicit_Euler;
        break;
    Default:
        Method = M_PPBD_SS;
        break;
    }
    int ts;
    printf("Please input the timestep (only input integer, recommended value is 60): 1/");
    std::cin >> ts;
    if (ts < 10 || ts > 10000) {
        ts = 60;
        printf("Invalid timestep, change timestep to default 1/60.0.\n");
    }
    else TIME_STEP = 1.0 / ts;
    printf("Please input the iteration number (default: %d): ", Method.MethodIteration);
    int iter;
    std::cin >> iter;
    if (iter < 3) {
        printf("iteration is too small, use default iteration.\n");
    }
    else if (iter > 1000)
        printf("iteration is too large, use default iteration.\n");
    else Method.MethodIteration = iter;
}