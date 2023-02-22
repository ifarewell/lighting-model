#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <vector>
#include "shader.h"
#include "camera.h"
#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
// continuous input
void processInput(GLFWwindow *window);
// discrete input
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void generateSphere(int nSegments, std::vector<float> &vertices, std::vector<int> &indices);
void generateCone(int nSegments, std::vector<float> &vertices, std::vector<int> &indices);
void generateCylinder(int nSegments, std::vector<float> &vertices, std::vector<int> &indices);
void generatePolyhedron(int nSegments, std::vector<float> &vertices, std::vector<int> &indices);
void renderObjects(Shader &shader, unsigned int objVAO[], unsigned int planeVAO, int target=-1);

// settings
// const unsigned int SCR_WIDTH = 1280;
// const unsigned int SCR_HEIGHT = 1024;
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const int NUM_LIGHTS = 2;
bool blinn = false;

int global_nSegments[4] = {50, 50, 50, 4};
int prev_nSegments[4] = {50, 50, 50, 4};

int controlTarget = 0;

const GLfloat PI = 3.14159265358979323846f;

// camera
// Camera camera(glm::vec3(-3.0f, 3.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -45.0f, -35.0f);
Camera camera(glm::vec3(-5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), -30.0f, -35.0f);

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos[NUM_LIGHTS]={
    glm::vec3(3.0f, 3.0f, 4.0f),
    glm::vec3(3.0f, 3.0f, -4.0f)
};

std::vector<float> objectVertices[4];
std::vector<int> objectIndices[4];
std::vector<float> lightVertices;
std::vector<int> lightIndices;
std::vector<float> planeVertices = {
    20.0f, 0.0f, 20.0f, 0.0f, 1.0f, 0.0f,
    20.0f, 0.0f, -20.0f, 0.0f, 1.0f, 0.0f,
    -20.0f, 0.0f, -20.0f, 0.0f, 1.0f, 0.0f,
    -20.0f, 0.0f, -20.0f, 0.0f, 1.0f, 0.0f,
    -20.0f, 0.0f, 20.0f, 0.0f, 1.0f, 0.0f,
    20.0f, 0.0f, 20.0f, 0.0f, 1.0f, 0.0f,
};
glm::vec3 materialAmbient[5]={
    glm::vec3(0.0f,0.1f,0.06f),
    glm::vec3(0.0f,0.0f,0.0f),
    glm::vec3(0.0f,0.0f,0.0f),
    glm::vec3(0.2f,0.0f,0.0f),
    glm::vec3(0.0f,0.0f,0.0f)
};
glm::vec3 materialDiffuse[5]={
    glm::vec3(0.0f,0.5098f,0.5098f),
    glm::vec3(0.1f,0.35f,0.1f),
    glm::vec3(0.5f,0.5f,0.0f),
    glm::vec3(0.8f,0.0f,0.0f),
    
    glm::vec3(0.8f,0.8f,0.8f)
};
glm::vec3 materialSpecular[5]={
    glm::vec3(0.5020f,0.5020f,0.5020f),
    glm::vec3(0.45f,0.55f,0.45f),
    glm::vec3(0.6f,0.6f,0.5f),
    glm::vec3(0.7f,0.6f,0.6f),
    
    glm::vec3(0.5f,0.5f,0.5f)
};
float objScale = 0.5f;
glm::vec3 objPosition[5]={
    glm::vec3(-1.0f, 0.0f, -1.0f),
    glm::vec3(1.0f, objScale, -1.0f),
    glm::vec3(-1.0f, 0.0f, 1.0f),
    glm::vec3(1.0f, 0.0f, 1.0f),
    
};
float materialAlpha[5]={ 1.0f, 1.0f, 1.0f, 0.8f, 1.0f};
void (*generateObject[4])(int, std::vector<float> &, std::vector<int> &) = {
    generateCylinder, 
    generateSphere, 
    generateCone,
    generatePolyhedron
};

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
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Local illumination models", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightingShader("object.vs", "object.fs");
    Shader lightSourceShader("light.vs", "light.fs");
    Shader simpleDepthShader("shadow.vs", "shadow.fs");
    Shader outlineShader("outline.vs", "outline.fs");

    // generate objects
    for (int i = 0; i < 4; ++i)
    {
        generateObject[i](global_nSegments[i], objectVertices[i], objectIndices[i]);
    }

    // generate sphere light source 
    generateSphere(50, lightVertices, lightIndices);

    unsigned int objVAO[4], objVBO[4], objEBO[4];
    glGenVertexArrays(4, objVAO);
    glGenBuffers(4, objVBO);
    glGenBuffers(4, objEBO);

    for (int i = 0; i < 4; ++i)
    {
        glBindVertexArray(objVAO[i]);

        glBindBuffer(GL_ARRAY_BUFFER, objVBO[i]);
        glBufferData(GL_ARRAY_BUFFER, objectVertices[i].size() * sizeof(float), &objectVertices[i][0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objEBO[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, objectIndices[i].size() * sizeof(int), &objectIndices[i][0], GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        // normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    unsigned int lightVAO, lightVBO, lightEBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    glGenBuffers(1, &lightEBO);

    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, lightVertices.size() * sizeof(float), &lightVertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, lightIndices.size() * sizeof(int), &lightIndices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);

    glBindVertexArray(planeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, planeVertices.size() * sizeof(float), &planeVertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // configure depth map FBO
    // -----------------------
    const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
    unsigned int depthMapFBO[NUM_LIGHTS];
    glGenFramebuffers(NUM_LIGHTS, depthMapFBO);
    // create depth texture
    unsigned int depthMap[NUM_LIGHTS];
    glGenTextures(NUM_LIGHTS, depthMap);
    for(int i=0;i<NUM_LIGHTS;++i){
        glBindTexture(GL_TEXTURE_2D, depthMap[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    lightingShader.use();
    for(int i=0;i<NUM_LIGHTS;++i){
        lightingShader.setInt("shadowMaps["+std::to_string(i)+"]", i);
    }

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        lightPos[NUM_LIGHTS-1].x = 5.0f * cos(glfwGetTime());
        lightPos[NUM_LIGHTS-1].z = 5.0f * sin(glfwGetTime());
        // input
        // -----
        processInput(window);
        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // process segment changes
        if (prev_nSegments[controlTarget] != global_nSegments[controlTarget])
        {
            prev_nSegments[controlTarget] = global_nSegments[controlTarget];

            generateObject[controlTarget](global_nSegments[controlTarget], objectVertices[controlTarget], objectIndices[controlTarget]);

            glBindVertexArray(objVAO[controlTarget]);

            glBindBuffer(GL_ARRAY_BUFFER, objVBO[controlTarget]);
            glBufferData(GL_ARRAY_BUFFER, objectVertices[controlTarget].size() * sizeof(float), &objectVertices[controlTarget][0], GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objEBO[controlTarget]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, objectIndices[controlTarget].size() * sizeof(int), &objectIndices[controlTarget][0], GL_STATIC_DRAW);
        }

        // 1. render depth of scene to texture (from light's perspective)
        // --------------------------------------------------------------
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrixs[NUM_LIGHTS];
        float near_plane = 1.0f, far_plane = 25.0f;
        lightProjection = glm::perspective(90.0f, (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT,  near_plane, far_plane);
        for(int i=0;i<NUM_LIGHTS;++i){
            lightView = glm::lookAt(lightPos[i], glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
            lightSpaceMatrixs[i] = lightProjection * lightView;
            // render scene from light's point of view
            simpleDepthShader.use();
            simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrixs[i]);
            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]);
            glClear(GL_DEPTH_BUFFER_BIT);

            // render objects
            renderObjects(simpleDepthShader, objVAO, planeVAO);

            // reset viewport
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        lightingShader.use();
        for(int i=0;i<NUM_LIGHTS;++i){
            lightingShader.setMat4("lightSpaceMatrixs["+std::to_string(i)+"]", lightSpaceMatrixs[i]);
            glActiveTexture(GL_TEXTURE0+i);
            glBindTexture(GL_TEXTURE_2D, depthMap[i]);
            // light properties
            lightingShader.setVec3("lights["+std::to_string(i)+"].position", lightPos[i]);
            lightingShader.setVec3("lights["+std::to_string(i)+"].ambient", 0.2f, 0.2f, 0.2f);
            lightingShader.setVec3("lights["+std::to_string(i)+"].diffuse", 0.8f, 0.8f, 0.8f);
            lightingShader.setVec3("lights["+std::to_string(i)+"].specular", 1.0f, 1.0f, 1.0f);
            lightingShader.setFloat("lights["+std::to_string(i)+"].constant", 1.0f);
            lightingShader.setFloat("lights["+std::to_string(i)+"].linear", 0.09f);
            lightingShader.setFloat("lights["+std::to_string(i)+"].quadratic", 0.032f);
        }
        
        lightingShader.setVec3("viewPos", camera.Position);
        lightingShader.setFloat("material.shininess", 32.0f);
        lightingShader.setBool("blinn", blinn);
        
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        outlineShader.use();
        outlineShader.setMat4("projection", projection);
        outlineShader.setMat4("view", view);

        // render the plane and objects
        renderObjects(lightingShader, objVAO, planeVAO);    

        // render select outlines
        glCullFace(GL_FRONT);
        renderObjects(outlineShader, objVAO, planeVAO, controlTarget);
        glCullFace(GL_BACK);

        // also draw the light source object
        lightSourceShader.use();
        lightSourceShader.setMat4("projection", projection);
        lightSourceShader.setMat4("view", view);
        for(int i=0;i<NUM_LIGHTS;++i){
            glm::mat4 lightModel = glm::mat4(1.0f);
            lightModel = glm::translate(lightModel, lightPos[i]);
            lightModel = glm::scale(lightModel, glm::vec3(0.2f));
            lightSourceShader.setMat4("model", lightModel);

            glBindVertexArray(lightVAO);
            glDrawElements(GL_TRIANGLES, lightIndices.size(), GL_UNSIGNED_INT, 0);
        }
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------   
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(4, objVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(4, objVBO);
    glDeleteBuffers(4, objEBO);
    glDeleteBuffers(1, &lightVBO);
    glDeleteBuffers(1, &lightEBO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteFramebuffers(NUM_LIGHTS, depthMapFBO);
    glDeleteTextures(NUM_LIGHTS, depthMap);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process continuous key event
// ----------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// process discrete key event
// --------------------------
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    switch (key)
    {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, true);
        break;
    case GLFW_KEY_1:
    case GLFW_KEY_2:
    case GLFW_KEY_3:
    case GLFW_KEY_4:
        controlTarget = key - GLFW_KEY_1;
        break;
    case GLFW_KEY_B:
        if(action==GLFW_PRESS)
            blinn=!blinn;
        break;
    case GLFW_KEY_UP:
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            prev_nSegments[controlTarget] = global_nSegments[controlTarget]++;
        break;
    case GLFW_KEY_DOWN:
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            if (global_nSegments[controlTarget] > 3)
                prev_nSegments[controlTarget] = global_nSegments[controlTarget]--;
        }
        break;
    default:
        break;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void generateSphere(int nSegments, std::vector<float> &vertices, std::vector<int> &indices)
{
    vertices.clear();
    indices.clear();
    vertices.reserve(6 * (nSegments * nSegments + 2 * nSegments + 1));
    indices.reserve(3 * (2 * nSegments * nSegments));
    for (int y = 0; y <= nSegments; y++)
    {
        for (int x = 0; x <= nSegments; x++)
        {
            float xSegment = (float)x / (float)nSegments;
            float ySegment = (float)y / (float)nSegments;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);
            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);
        }
    }

    for (int i = 0; i < nSegments; i++)
    {
        for (int j = 0; j < nSegments; j++)
        {
            indices.push_back(i * (nSegments + 1) + j);
            indices.push_back((i + 1) * (nSegments + 1) + j + 1);
            indices.push_back((i + 1) * (nSegments + 1) + j);
            indices.push_back(i * (nSegments + 1) + j);
            indices.push_back(i * (nSegments + 1) + j + 1);
            indices.push_back((i + 1) * (nSegments + 1) + j + 1);
        }
    }
}

void generateCone(int nSegments, std::vector<float> &vertices, std::vector<int> &indices)
{
    vertices.clear();
    indices.clear();
    vertices.reserve(6 * (2 * nSegments + 4));
    indices.reserve(3 * (2 * nSegments));
    float h = 2.0f;
    // 侧面
    vertices.push_back(0.0f);
    vertices.push_back(h);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(1.0f);
    vertices.push_back(0.0f);
    for (int x = 0; x <= nSegments; x++)
    {
        float xSegment = (float)x / (float)nSegments;
        float xPos = std::cos(xSegment * 2.0f * PI);
        float yPos = 0.0f;
        float zPos = std::sin(xSegment * 2.0f * PI);
        vertices.push_back(xPos);
        vertices.push_back(yPos);
        vertices.push_back(zPos);

        glm::vec3 n = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(xPos, yPos, zPos));
        glm::vec3 norm = glm::normalize(glm::cross(n, glm::vec3(-xPos, h, -zPos)));

        vertices.push_back(norm.x);
        vertices.push_back(norm.y);
        vertices.push_back(norm.z);
    }
    for (int i = 1; i <= nSegments; i++)
    {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(i);
    }

    // 底面
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(-1.0f);
    vertices.push_back(0.0f);
    for (int x = 0; x <= nSegments; x++)
    {
        float xSegment = (float)x / (float)nSegments;
        float xPos = std::cos(xSegment * 2.0f * PI);
        float yPos = 0.0f;
        float zPos = std::sin(xSegment * 2.0f * PI);
        vertices.push_back(xPos);
        vertices.push_back(yPos);
        vertices.push_back(zPos);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
        vertices.push_back(0.0f);
    }
    for (int i = 1; i <= nSegments; i++)
    {
        indices.push_back(nSegments + 2);
        indices.push_back(i + nSegments + 2);
        indices.push_back(i + 1 + nSegments + 2);
    }
}

void generateCylinder(int nSegments, std::vector<float> &vertices, std::vector<int> &indices)
{
    vertices.clear();
    indices.clear();
    vertices.reserve(6 * (4 * nSegments + 6));
    indices.reserve(3 * (4 * nSegments));
    float h = 2.0f;
    // 侧面
    for (int x = 0; x <= nSegments; x++)
    {
        float xSegment = (float)x / (float)nSegments;
        float xPos = std::cos(xSegment * 2.0f * PI);
        float yPos = 0.0f;
        float zPos = std::sin(xSegment * 2.0f * PI);
        vertices.push_back(xPos);
        vertices.push_back(yPos);
        vertices.push_back(zPos);
        vertices.push_back(xPos);
        vertices.push_back(0.0f);
        vertices.push_back(zPos);

        vertices.push_back(xPos);
        vertices.push_back(yPos + h);
        vertices.push_back(zPos);
        vertices.push_back(xPos);
        vertices.push_back(0.0f);
        vertices.push_back(zPos);
    }
    for (int i = 0; i < nSegments; i++)
    {
        indices.push_back(2 * i);
        indices.push_back(2 * i + 1);
        indices.push_back(2 * i + 2);
        indices.push_back(2 * i + 3);
        indices.push_back(2 * i + 2);
        indices.push_back(2 * i + 1);
    }

    // 底面
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(-1.0f);
    vertices.push_back(0.0f);
    for (int x = 0; x <= nSegments; x++)
    {
        float xSegment = (float)x / (float)nSegments;
        float xPos = std::cos(xSegment * 2.0f * PI);
        float yPos = 0.0f;
        float zPos = std::sin(xSegment * 2.0f * PI);
        vertices.push_back(xPos);
        vertices.push_back(yPos);
        vertices.push_back(zPos);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
        vertices.push_back(0.0f);
    }
    for (int i = 1; i <= nSegments; i++)
    {
        indices.push_back(2 * nSegments + 2);
        indices.push_back(i + 2 * nSegments + 2);
        indices.push_back(i + 1 + 2 * nSegments + 2);
    }

    // 顶面
    vertices.push_back(0.0f);
    vertices.push_back(h);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(1.0f);
    vertices.push_back(0.0f);
    for (int x = 0; x <= nSegments; x++)
    {
        float xSegment = (float)x / (float)nSegments;
        float xPos = std::cos(xSegment * 2.0f * PI);
        float yPos = h;
        float zPos = std::sin(xSegment * 2.0f * PI);
        vertices.push_back(xPos);
        vertices.push_back(yPos);
        vertices.push_back(zPos);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);
    }
    for (int i = 1; i <= nSegments; i++)
    {
        indices.push_back(3 * nSegments + 4);
        indices.push_back(i + 1 + 3 * nSegments + 4);
        indices.push_back(i + 3 * nSegments + 4);
    }
}

void generatePolyhedron(int nSegments, std::vector<float> &vertices, std::vector<int> &indices)
{
    vertices.clear();
    indices.clear();
    vertices.reserve(6 * (6 * nSegments + 4));
    indices.reserve(3 * (4 * nSegments));
    float h = 3.0f;
    // 侧面
    float delta = 1.0f / nSegments * 2.0f * PI;
    for (int x = 0; x < nSegments; x++)
    {
        float xPos = std::cos(x * delta);
        float zPos = std::sin(x * delta);
        float xNorm = std::cos((x + 0.5f) * delta);
        float zNorm = std::sin((x + 0.5f) * delta);

        vertices.push_back(xPos);
        vertices.push_back(0.0f);
        vertices.push_back(zPos);
        vertices.push_back(xNorm);
        vertices.push_back(0.0f);
        vertices.push_back(zNorm);

        vertices.push_back(xPos);
        vertices.push_back(h);
        vertices.push_back(zPos);
        vertices.push_back(xNorm);
        vertices.push_back(0.0f);
        vertices.push_back(zNorm);

        xPos = std::cos((x + 1) * delta);
        zPos = std::sin((x + 1) * delta);

        vertices.push_back(xPos);
        vertices.push_back(0.0f);
        vertices.push_back(zPos);
        vertices.push_back(xNorm);
        vertices.push_back(0.0f);
        vertices.push_back(zNorm);

        vertices.push_back(xPos);
        vertices.push_back(h);
        vertices.push_back(zPos);
        vertices.push_back(xNorm);
        vertices.push_back(0.0f);
        vertices.push_back(zNorm);
    }
    for (int i = 0; i < nSegments; i++)
    {
        indices.push_back(4 * i);
        indices.push_back(4 * i + 1);
        indices.push_back(4 * i + 2);

        indices.push_back(4 * i + 1);
        indices.push_back(4 * i + 3);
        indices.push_back(4 * i + 2);
    }

    // 底面
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(-1.0f);
    vertices.push_back(0.0f);
    for (int x = 0; x <= nSegments; x++)
    {
        float xSegment = (float)x / (float)nSegments;
        float xPos = std::cos(xSegment * 2.0f * PI);
        float yPos = 0.0f;
        float zPos = std::sin(xSegment * 2.0f * PI);
        vertices.push_back(xPos);
        vertices.push_back(yPos);
        vertices.push_back(zPos);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
        vertices.push_back(0.0f);
    }
    for (int i = 1; i <= nSegments; i++)
    {
        indices.push_back(4 * nSegments);
        indices.push_back(i + 4 * nSegments);
        indices.push_back(i + 1 + 4 * nSegments);
    }

    // 顶面
    vertices.push_back(0.0f);
    vertices.push_back(h);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(1.0f);
    vertices.push_back(0.0f);
    for (int x = 0; x <= nSegments; x++)
    {
        float xSegment = (float)x / (float)nSegments;
        float xPos = std::cos(xSegment * 2.0f * PI);
        float yPos = h;
        float zPos = std::sin(xSegment * 2.0f * PI);
        vertices.push_back(xPos);
        vertices.push_back(yPos);
        vertices.push_back(zPos);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);
    }
    for (int i = 1; i <= nSegments; i++)
    {
        indices.push_back(5 * nSegments + 2);
        indices.push_back(i + 1 + 5 * nSegments + 2);
        indices.push_back(i + 5 * nSegments + 2);
    }
}

// render the plane and objects
void renderObjects(Shader &shader, unsigned int objVAO[], unsigned int planeVAO, int target)
{
    shader.use();
    glm::mat4 model = glm::mat4(1.0f);

    if(target!=-1){
        model = glm::translate(model, objPosition[target]);
        model = glm::scale(model, glm::vec3(objScale));
        shader.setVec3("material.ambient", materialAmbient[target]);
        shader.setVec3("material.diffuse", materialDiffuse[target]);
        shader.setVec3("material.specular", materialSpecular[target]);
        shader.setFloat("material.alpha", materialAlpha[target]);
        shader.setMat4("model", model);
        glBindVertexArray(objVAO[target]);
        glDrawElements(GL_TRIANGLES, objectIndices[target].size(), GL_UNSIGNED_INT, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // render plane
    model = glm::translate(model, glm::vec3(0.0f, -0.12f, 0.0f));
    shader.setVec3("material.ambient", materialAmbient[4]);
    shader.setVec3("material.diffuse", materialDiffuse[4]);
    shader.setVec3("material.specular", materialSpecular[4]);
    shader.setMat4("model", model);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, planeVertices.size());

    // render objects
    for (int i = 0; i < 4; ++i)
    {
        // material properties
        model = glm::mat4(1.0f);
        model = glm::translate(model, objPosition[i]);
        model = glm::scale(model, glm::vec3(objScale));
        shader.setVec3("material.ambient", materialAmbient[i]);
        shader.setVec3("material.diffuse", materialDiffuse[i]);
        shader.setVec3("material.specular", materialSpecular[i]);
        shader.setFloat("material.alpha", materialAlpha[i]);
        shader.setMat4("model", model);
        glBindVertexArray(objVAO[i]);
        glDrawElements(GL_TRIANGLES, objectIndices[i].size(), GL_UNSIGNED_INT, 0);
    }
        
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}