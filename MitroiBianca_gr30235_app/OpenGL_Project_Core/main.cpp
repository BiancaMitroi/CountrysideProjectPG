#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Skybox.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat4 lightRotation;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// fog parameters
int fog = 1;
GLfloat fogLoc;
GLfloat fogSkyboxLoc;
bool drawFog = true;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

float firefly1param = 0.0f;
float firefly2param = 0.2f;
float firefly3param = 0.4f;
float firefly4param = 0.6f;
float firefly5param = 0.8f;

bool firefly1up = false;
bool firefly2up = false;
bool firefly3up = false;
bool firefly4up = false;
bool firefly5up = false;

GLfloat firefly1paramLoc;
GLfloat firefly1modelLoc;
GLfloat firefly1viewLoc;
GLfloat firefly1projectionLoc;
GLfloat firefly2paramLoc;
GLfloat firefly3paramLoc;
GLfloat firefly4paramLoc;
GLfloat firefly5paramLoc;

// camera
gps::Camera myCamera(
    glm::vec3(-9.0f, 2.0f, 4.0f),
    glm::vec3(0.0f, 2.0f, 4.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

// movement parameters
float moveLeftRight, moveForwardBackward = 0.0f;
float lastX, lastY;
float pitch;
float yaw;
bool firstMouse = true;
float sensitivity = 0.15f;
GLfloat angle;

// light
GLfloat lightAngle;


// presenting and animation parameters
float count = 0.0f;
bool up = false;
int presentValue = 0;
bool present = true;
bool drawFireFly = true;

// projection parameters
const GLfloat near_plane = 0.1f, far_plane = 100.0f;
int retina_width, retina_height;

GLboolean pressedKeys[1024];

// models

gps::Model3D baloon;
gps::Model3D bridge;
gps::Model3D chimney;
gps::Model3D door;
gps::Model3D ground;
gps::Model3D pillar;
gps::Model3D roof;
gps::Model3D tree;
gps::Model3D walls;
gps::Model3D water;
gps::Model3D window;
gps::Model3D firefly1;
gps::Model3D firefly2;
gps::Model3D firefly3;
gps::Model3D firefly4;
gps::Model3D firefly5;


gps::Model3D light;
gps::Model3D screenQuad;

// skybox
gps::SkyBox mySkyBox;
std::vector<const GLchar*> faces;

// shaders
gps::Shader myBasicShader;
gps::Shader skyboxShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

gps::Shader firefly1Shader;
gps::Shader firefly2Shader;
gps::Shader firefly3Shader;
gps::Shader firefly4Shader;
gps::Shader firefly5Shader;

//shadows
GLuint shadowMapFBO;
GLuint depthMapTexture;
bool showDepthMap;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // wireframe
    if (pressedKeys[GLFW_KEY_Z]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    // poly
    if (pressedKeys[GLFW_KEY_X]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    // normal
    if (pressedKeys[GLFW_KEY_C]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    // fog
    if (key == GLFW_KEY_V && action == GLFW_RELEASE) {
        fog = 1 - fog;
        drawFog = !drawFog;

        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

        glUniform1i(fogLoc, fog);
        skyboxShader.useShaderProgram();
        glUniform1i(fogSkyboxLoc, fog);
    }

    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        showDepthMap = !showDepthMap;

    if (pressedKeys[GLFW_KEY_F]) {
        drawFireFly = !drawFireFly;
        firefly1Shader.useShaderProgram();
        glUniform1f(firefly1paramLoc, firefly1param);
        firefly2Shader.useShaderProgram();
        glUniform1f(firefly2paramLoc, firefly2param);
        firefly3Shader.useShaderProgram();
        glUniform1f(firefly3paramLoc, firefly3param);
        firefly4Shader.useShaderProgram();
        glUniform1f(firefly4paramLoc, firefly4param);
        firefly5Shader.useShaderProgram();
        glUniform1f(firefly5paramLoc, firefly5param);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //TODO
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }
    float xDiff = (float)xpos - lastX;
    float yDiff = (float)ypos - lastY;
    lastX = (float)xpos;
    lastY = (float)ypos;

    xDiff *= sensitivity;
    yDiff *= sensitivity;

    yaw += xDiff;
    pitch -= yDiff;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_J]) {
        lightAngle -= 0.5f;
    }

    if (pressedKeys[GLFW_KEY_L]) {
        lightAngle += 0.5f;
    }
}

void initOpenGLWindow() {
    myWindow.Create(1920, 1080, "OpenGL Project Core");
    glfwGetFramebufferSize(myWindow.getWindow(), &retina_width, &retina_height);
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    //glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    // glEnable(GL_CULL_FACE); // cull face
    // glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initSkyBox() {
    faces.push_back("models/skybox/majesty_rt.tga");
    faces.push_back("models/skybox/majesty_lf.tga");
    faces.push_back("models/skybox/majesty_up.tga");
    faces.push_back("models/skybox/majesty_dn.tga");
    faces.push_back("models/skybox/majesty_bk.tga");
    faces.push_back("models/skybox/majesty_ft.tga");

    mySkyBox.Load(faces);
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");

    skyboxShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
        glm::value_ptr(view));

    projection = glm::perspective(glm::radians(90.0f), (float)retina_width / (float)retina_height, 0.1f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initModels() {
    initSkyBox();
    baloon.LoadModel("models/baloon/baloon.obj");
    bridge.LoadModel("models/bridge/bridge.obj");
    chimney.LoadModel("models/chimney/chimney.obj");
    door.LoadModel("models/door/door.obj");
    ground.LoadModel("models/ground/ground.obj");
    light.LoadModel("models/light/light.obj");
    pillar.LoadModel("models/pillar/pillar.obj");
    roof.LoadModel("models/roof/roof.obj");
    screenQuad.LoadModel("models/quad/quad.obj");
    tree.LoadModel("models/tree/tree.obj");
    walls.LoadModel("models/walls/walls.obj");
    water.LoadModel("models/water/water.obj");
    window.LoadModel("models/window/window.obj");
    firefly1.LoadModel("models/firefly/firefly1.obj");
    firefly2.LoadModel("models/firefly/firefly2.obj");
    firefly3.LoadModel("models/firefly/firefly3.obj");
    firefly4.LoadModel("models/firefly/firefly4.obj");
    firefly5.LoadModel("models/firefly/firefly5.obj");
}

void initShaders() {
	myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");

    firefly1Shader.loadShader("shaders/firefly1.vert", "shaders/firefly1.frag");
    firefly2Shader.loadShader("shaders/firefly2.vert", "shaders/firefly2.frag");
    firefly3Shader.loadShader("shaders/firefly3.vert", "shaders/firefly3.frag");
    firefly4Shader.loadShader("shaders/firefly4.vert", "shaders/firefly4.frag");
    firefly5Shader.loadShader("shaders/firefly5.vert", "shaders/firefly5.frag");
}

void initUniforms() {
    myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // create projection matrix
    projection = glm::perspective(glm::radians(90.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 100.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(-4.0f, 3.0f, 0.0f);
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
    
    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    fogLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fog");
    glUniform1i(fogLoc, fog);
   
    skyboxShader.useShaderProgram();
    fogSkyboxLoc = glGetUniformLocation(skyboxShader.shaderProgram, "fog");
    glUniform1i(fogSkyboxLoc, fog);

    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    ///////////////////////////////////////////////////////////////////
    firefly1Shader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(firefly1Shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    firefly1paramLoc = glGetUniformLocation(firefly1Shader.shaderProgram, "param");
    glUniform1f(firefly1paramLoc, firefly1param);


    firefly2Shader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(firefly2Shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    firefly2paramLoc = glGetUniformLocation(firefly2Shader.shaderProgram, "param");
    glUniform1f(firefly2paramLoc, firefly2param);


    firefly3Shader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(firefly3Shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    firefly3paramLoc = glGetUniformLocation(firefly3Shader.shaderProgram, "param");
    glUniform1f(firefly3paramLoc, firefly3param);


    firefly4Shader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(firefly4Shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    firefly4paramLoc = glGetUniformLocation(firefly4Shader.shaderProgram, "param");
    glUniform1f(firefly4paramLoc, firefly4param);


    firefly5Shader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(firefly5Shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    firefly5paramLoc = glGetUniformLocation(firefly5Shader.shaderProgram, "param");
    glUniform1f(firefly5paramLoc, firefly5param);
}

void initFBO() {
    //TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
    //generate FBO ID
    glGenFramebuffers(1, &shadowMapFBO);

    //create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    //attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
    //TODO - Return the light-space transformation matrix
    glm::mat4 lightView = glm::lookAt(5.0f * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = 0.1f, far_plane = 100.0f; 
    glm::mat4 lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
}

void draw(gps::Shader shader, bool depthPass) {
    // select active shader program
    shader.useShaderProgram();

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    
    // desenarea obiectelor statice
    bridge.Draw(shader);
    chimney.Draw(shader);
    door.Draw(shader);
    pillar.Draw(shader);
    roof.Draw(shader);
    tree.Draw(shader);
    walls.Draw(shader);
    window.Draw(shader);
    ground.Draw(shader);
    water.Draw(shader);

    // animatia balonului

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, count, 0.0f));
    //model = glm::scale(model, glm::vec3(0.5f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    
    //glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    baloon.Draw(shader);
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -count, 0.0f));
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

}

void renderScene() {
    // depth maps creation pass
    //TODO - Send the light-space transformation matrix to the depth map creation shader and
    //		 render the scene in the depth map
    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    if (!drawFog)
        draw(depthMapShader, true);
    else
        draw(myBasicShader, false);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // render depth map on screen - toggled with the M key
    if (showDepthMap) {
        glViewport(0, 0, retina_width, retina_height);

        glClear(GL_COLOR_BUFFER_BIT);

        screenQuadShader.useShaderProgram();

        //bind the depth map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

        glDisable(GL_DEPTH_TEST);
        screenQuad.Draw(screenQuadShader);
        glEnable(GL_DEPTH_TEST);
    }
    else {

        // final scene rendering pass (with shadows)

        glViewport(0, 0, retina_width, retina_height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myBasicShader.useShaderProgram();
        if (present) {
            
            if (presentValue >= 0 && presentValue < 100)
                myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
            else if (presentValue >= 100 && presentValue < 200) {
                if (yaw > -90.0f) {
                    yaw -= 1.0f;
                    myCamera.rotate(pitch, yaw);
                }
            }
            else if (presentValue >= 200 && presentValue < 400)
                myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
            else if (presentValue >= 400 && presentValue < 500) {
                if (yaw > -180.0f) {
                    yaw -= 1.0f;
                    myCamera.rotate(pitch, yaw);
                }

            }
            else if (presentValue >= 500 && presentValue < 750)
                myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
            else if (presentValue >= 750 && presentValue < 850) {
                if (yaw > -270.0f) {
                    yaw -= 1.0f;
                    myCamera.rotate(pitch, yaw);
                }

            }
            else if (presentValue >= 850 && presentValue < 1000)
                myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        }


            view = myCamera.getViewMatrix();
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

            lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));


            //bind the shadow map
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, depthMapTexture);
            glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

            glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

            draw(myBasicShader, false);

            //draw a white cube around the light

            lightShader.useShaderProgram();

            glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
            model = lightRotation;
            glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

            mySkyBox.Draw(skyboxShader, view, projection);
            light.Draw(lightShader);

            //////////////////////////////////////////
            if (drawFireFly) {
                if (firefly1param > 0.9f) {
                    firefly1up = true;
                }
                if (firefly1param < 0.1f) {
                    firefly1up = false;
                }

                if (!firefly1up) {
                    firefly1param += 0.005f;
                }
                if (firefly1up) {
                    firefly1param -= 0.005f;
                }
                firefly1Shader.useShaderProgram();

                glUniformMatrix4fv(glGetUniformLocation(firefly1Shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
                model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, firefly1param, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(firefly1Shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                glUniform1f(firefly1paramLoc, firefly1param);

                firefly1.Draw(firefly1Shader);
                model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -firefly1param, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(firefly1Shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                ///////////////////////////////////////////////////////////////////////

                if (firefly2param > 0.9f) {
                    firefly2up = true;
                }
                if (firefly2param < 0.1f) {
                    firefly2up = false;
                }

                if (!firefly2up) {
                    firefly2param += 0.005f;
                }
                if (firefly2up) {
                    firefly2param -= 0.005f;
                }
                firefly2Shader.useShaderProgram();

                glUniformMatrix4fv(glGetUniformLocation(firefly2Shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
                model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, firefly2param, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(firefly2Shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                glUniform1f(firefly2paramLoc, firefly2param);

                firefly2.Draw(firefly2Shader);
                model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -firefly2param, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(firefly2Shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                ////////////////////////////////////////////////////////////
                if (firefly3param > 0.9f) {
                    firefly3up = true;
                }
                if (firefly3param < 0.1f) {
                    firefly3up = false;
                }

                if (!firefly3up) {
                    firefly3param += 0.005f;
                }
                if (firefly3up) {
                    firefly3param -= 0.005f;
                }
                firefly3Shader.useShaderProgram();

                glUniformMatrix4fv(glGetUniformLocation(firefly3Shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
                model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, firefly3param, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(firefly3Shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                glUniform1f(firefly3paramLoc, firefly3param);

                firefly3.Draw(firefly3Shader);
                model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -firefly3param, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(firefly3Shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                ////////////////////////////////////////////////////
                if (firefly4param > 0.9f) {
                    firefly4up = true;
                }
                if (firefly4param < 0.1f) {
                    firefly4up = false;
                }

                if (!firefly4up) {
                    firefly4param += 0.005f;
                }
                if (firefly4up) {
                    firefly4param -= 0.005f;
                }
                firefly4Shader.useShaderProgram();

                glUniformMatrix4fv(glGetUniformLocation(firefly4Shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
                model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, firefly4param, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(firefly4Shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                glUniform1f(firefly4paramLoc, firefly4param);

                firefly4.Draw(firefly4Shader);
                model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -firefly4param, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(firefly4Shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                ///////////////////////////////////////////////
                if (firefly5param > 0.9f) {
                    firefly5up = true;
                }
                if (firefly5param < 0.1f) {
                    firefly5up = false;
                }

                if (!firefly5up) {
                    firefly5param += 0.005f;
                }
                if (firefly5up) {
                    firefly5param -= 0.005f;
                }
                firefly5Shader.useShaderProgram();

                glUniformMatrix4fv(glGetUniformLocation(firefly5Shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
                model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, firefly5param, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(firefly5Shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                glUniform1f(firefly5paramLoc, firefly1param);

                firefly5.Draw(firefly5Shader);
                model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -firefly5param, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(firefly5Shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            }

        if (count > 7.5f) {
            up = true;
        }
        if (count < 0.1f) {
            up = false;
        }

        if (!up) {
            count += 0.1f;
        }
        if (up) {
            count -= 0.1f;
        }
    }
}
void cleanup() {
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
    myWindow.Delete();
    //cleanup code for your own data
    glfwTerminate();
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
    initFBO();
	initUniforms();

	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    if(present)
            if (presentValue == 1000) {
                setWindowCallbacks();
                present = false;
                //yaw = 0.0f;
            }
            else
                presentValue += 1;
        renderScene();
		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());
	}

	cleanup();

    return EXIT_SUCCESS;
}
