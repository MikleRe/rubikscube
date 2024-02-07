#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <UTIL/UtilGLSL.cpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

using namespace cv;

// index of our texture / camera feed
GLuint textureID;

// index of our shaders
GLuint shaderProgram;

/*
 * VBO: Vertex Buffer Object    ->
 * VAO: Vertex Array Object     ->
 * EBO: Element Buffer Object   -> Used to say which vertex goes to which triangle
 */
unsigned int VBO, VAO, EBO;

/*
 * Generate the texture and set the texture's parameter.
 */
void initTexture() {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}

/*
 * Initialize the background mesh
 */
void initBackground() {
    float vertices[] = {
            // positions                        // colors                       // texture coords
            1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
            1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
            -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
            -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
    };

    unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

void imageProcessing(VideoCapture *cap) {
    Mat toTexture;

    // Read camera
    Mat currentframe;
    cap->read(currentframe);

    // Image Processing
    Mat blurredframe;
    cv::GaussianBlur(currentframe, blurredframe, cv::Size(0,0), 1.6, 0);

    // Convert Mat from opencv to OpenGl's Texture2D
    cv::absdiff(blurredframe, currentframe, toTexture);

    /* CHATGPT:
    To find and track a Rubik's Cube in a camera feed and determine its colors, you can use a combination of computer
     vision techniques. Here's a high-level overview of the process:

    Object Detection: Use a pre-trained object detection model (like YOLO, SSD, or Faster R-CNN) to detect the Rubik's
     Cube in the camera feed. Train the model on images of Rubik's Cubes to recognize its shape and appearance.

    Feature Extraction: Once the cube is detected, extract its features such as corners or edges using techniques like
     image segmentation or contour detection. This step helps in accurately identifying the cube's boundaries.

    Cube Orientation Estimation: Determine the orientation of the cube in the camera feed to ensure consistent color
     detection. This might involve perspective transformation or geometric calculations to correct for tilts or rotations.

    Color Detection: Divide the cube's surface into individual squares and analyze each square to determine its color.
     Techniques like color thresholding, clustering, or neural networks can be used for color detection.

    Color Classification: Classify the detected colors into the standard Rubik's Cube color set (e.g., white, yellow,
     red, orange, blue, green). This could be achieved using machine learning classifiers or lookup tables.

    Cube State Representation: Represent the detected colors as a 3D matrix or array, mimicking the layout of a solved
     Rubik's Cube. This representation will be used for solving the cube algorithmically.

    Continuous Tracking: Implement a tracking algorithm to continuously monitor the cube's movement within the camera
     feed. This could involve techniques like Kalman filtering, optical flow, or feature tracking to predict and
     update the cube's position in subsequent frames.

    Feedback and Calibration: Implement mechanisms for user feedback and calibration to refine the detection and
     tracking accuracy. This could involve manual corrections or adaptive algorithms that learn from user interactions.

    By combining these image processing techniques, you can create a robust system for detecting, tracking, and
     analyzing a Rubik's Cube through a camera feed. However, keep in mind that lighting conditions, occlusions,
     and other environmental factors may affect the system's performance, so robustness testing and optimization are
     crucial.
     */

    cv::flip(toTexture, toTexture, 0);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, toTexture.cols, toTexture.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, toTexture.ptr());
    glGenerateMipmap(GL_TEXTURE_2D);
}

/*
 * Render Loop
 */
void render() {
    // Clear Screen
    glClearColor(.5, .5, .5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Shader
    glUseProgram(shaderProgram);

    // Draw triangles
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "rubikscube", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    shaderProgram = createShaderProgram("glsl/background.vert", "glsl/background.frag");
    if (!shaderProgram) {
        return -1;
    }

    // Access Camera
    VideoCapture cap;
    int deviceID = 0;           // 0 = open default camera
    int apiID = cv::CAP_ANY;    // 0 = autodetect default API
    // open selected camera using selected API
    cap.open(deviceID, apiID);
    // check if we succeeded
    if (!cap.isOpened()) {
        std::cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    initTexture();

    initBackground();

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // camera
        // ------
        imageProcessing(&cap);

        // do the rendering
        render();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
