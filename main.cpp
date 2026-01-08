#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
#include "SkyBox.hpp"

// camera
gps::Camera myCamera(
    glm::vec3(-10.0f, 6.0f, 20.0f),
    glm::vec3(0.0f, 0.0f, 10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
// Declare a variable to store the loaded model
gps::Model3D parking;
gps::Model3D oldCar;
gps::Model3D newCar;
gps::Model3D cube;

// Variable to store loaded parts
std::vector<gps::Model3D> modelParts;
GLenum currentRenderMode = GL_FILL;
// Variable to track the last loaded part
int lastLoadedPartIndex = 0;

GLfloat angle;
//Skybox
std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;
// shaders
gps::Shader myBasicShader;
gps::Shader skyboxShader;
gps::Shader lightingShader;

glm::vec3 fogColor(0.7f, 0.7f, 0.7f); // or whatever values you want

gps::SkyBox skybox;

// Define the paths to your skybox textures

GLint cameraPosLoc;

float x = 11.74f;
float y = 1.415f;
float z = 3.19f;
float lampHeight = 4.70f;

// ...
bool isAnimationActive = false;
bool isAutoMoveActive = false;

glm::vec3 oldCarPosition = glm::vec3(0.0f, 0.0f, 0.0f); // Initial position
float carRotationAngle = 0.0f; // Initial rotation angle
glm::vec3 newCarPosition = glm::vec3(1.0f, 0.0f, 0.0f); // Initial position
// Global or class-level variables
glm::mat4 originalNewCarModelMatrix = glm::mat4(1.0f); // Replace with the initial model matrix of your avion object
glm::vec3 circleCenter = glm::vec3(0.0f, 0.0f, 0.0f); // Set the center of the circle
float circleRadius = 5.0f; // Set the radius of the circle
float newCarYPosition = 0.0f; // Set the initial Y position of the avion
float circleAngle = 0.0f; // Angle for the circular motion
glm::mat4 newCarModelMatrix = glm::mat4(1.0f);

glm::mat4 skyboxModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(500.0f, 500.0f, 500.0f)); // Adjusted scale to make it smaller

bool isFogEnabled = true;
GLint lightPosLoc;
GLint viewPosLoc;
GLint objectColorLoc;
bool isGlobalLightOn = true;

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
float lastMouseX, lastMouseY;
bool isFirstMouseMovement = true;
const float MOUSE_SENSITIVITY = 0.05f;

void updateCameraRotationFromMouse(float mouseX, float mouseY, gps::Camera& camera) {
    if (isFirstMouseMovement) {
        lastMouseX = mouseX;
        lastMouseY = mouseY;
        isFirstMouseMovement = false;
    }

    float xOffset = mouseX - lastMouseX;
    float yOffset = lastMouseY - mouseY; // Reversed since y-coordinates go from bottom to top
    lastMouseX = mouseX;
    lastMouseY = mouseY;

    xOffset *= MOUSE_SENSITIVITY;
    yOffset *= MOUSE_SENSITIVITY;

    camera.rotate(yOffset, xOffset); // Assuming rotate method takes pitch (y) and yaw (x)
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d, and height: %d\n", width, height);

    // Ensure height is not zero to prevent division by zero
    if (height == 0) {
        height = 1;
    }

    // Update the viewport to match the new window size
    glViewport(0, 0, width, height);

    // Update the aspect ratio based on the new window dimensions
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    // Check if the aspect ratio is too close to zero, and handle it
    const float minAspectRatio = std::numeric_limits<float>::epsilon();
    if (std::abs(aspectRatio) < minAspectRatio) {
        aspectRatio = (aspectRatio < 0) ? -minAspectRatio : minAspectRatio;
    }

    // TODO: Handle any other updates or calculations related to window resizing

    // For example, you might want to update the projection matrix here
    projection = glm::perspective(glm::radians(45.0f), aspectRatio, 1.0f, 500.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
    if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        // Toggle to GL_FILL (solid) if it's currently GL_LINE (wireframe), and vice versa
        currentRenderMode = (currentRenderMode == GL_FILL) ? GL_LINE : GL_FILL;
    }
    else if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
        // Toggle to GL_POINT (point mode) if it's currently GL_FILL (solid), and vice versa
        currentRenderMode = (currentRenderMode == GL_FILL) ? GL_POINT : GL_FILL;
    }
    if (key == GLFW_KEY_I && action == GLFW_PRESS) {
        // Toggle the animation state
        isAnimationActive = !isAnimationActive;

        if (!isAnimationActive) {
            // Save the current state only when stopping the animation
            originalNewCarModelMatrix = newCarModelMatrix;
        }
    }
    if (key == GLFW_KEY_O && action == GLFW_PRESS) {
        isAutoMoveActive = !isAutoMoveActive; // Toggle auto movement on/off
    }
    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        // Toggle fog state
        isFogEnabled = !isFogEnabled;
    }
    if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        isGlobalLightOn = !isGlobalLightOn; // Toggle the global light
    }

}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    updateCameraRotationFromMouse(xpos, ypos, myCamera);
}
void updateViewMatrix() {
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}


void autoMoveAndRotateFunc(float rotationDecrement) {
    static float totalRotation = 60.0f; // Start with a full rotation
    myCamera.move(gps::MOVE_FORWARD, 0.3f); // Forward movement speed


    // Check if there's still rotation to be done
    if (totalRotation > 0.0f) {
        // Subtract the decrement from the total rotation
        totalRotation -= rotationDecrement;

        // Rotate the camera by the updated total rotation
        myCamera.rotate(0.0f, -totalRotation);

        // Update the view matrix with the new camera parameters
        glm::mat4 view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Compute and update the normal matrix
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    } else {
        // Reset total rotation for the next activation and stop auto movement

        totalRotation = 360.0f;
        isAutoMoveActive = false;
    }

}





void processMovement() {
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_UP, cameraSpeed); // Adjust if your Camera class has a MOVE_UP enum
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed); // Adjust if your Camera class has a MOVE_DOWN enum
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // Also update model matrix for avion
        newCarModelMatrix = glm::rotate(originalNewCarModelMatrix, glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        newCarModelMatrix = glm::rotate(originalNewCarModelMatrix, glm::radians(angle), glm::vec3(0, 1, 0));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (pressedKeys[GLFW_KEY_Z]) {
        myCamera.scale(0.99f);
        updateViewMatrix();
    }

    if (pressedKeys[GLFW_KEY_X]) {
        myCamera.scale(1.01f); 
        updateViewMatrix();
    }
    float movementSpeed = 0.1f;
    float rotationSpeed = 2.0f;

    if (pressedKeys[GLFW_KEY_UP]) {
        oldCarPosition += movementSpeed * glm::vec3(cos(glm::radians(carRotationAngle)), 0.0f, sin(glm::radians(carRotationAngle)));
    }

    if (pressedKeys[GLFW_KEY_DOWN]) {
        oldCarPosition -= movementSpeed * glm::vec3(cos(glm::radians(carRotationAngle)), 0.0f, sin(glm::radians(carRotationAngle)));
    }

    float rotationSpeed1= 0.5f; 

    if (pressedKeys[GLFW_KEY_LEFT]) {
        carRotationAngle += rotationSpeed1;
    }

    if (pressedKeys[GLFW_KEY_RIGHT]) {
        carRotationAngle -= rotationSpeed1;
    }


}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); 
	glEnable(GL_CULL_FACE); 
	glCullFace(GL_BACK); 
	glFrontFace(GL_CCW); 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void initModels() {
    parking.LoadModel("models/city/Parking.obj");
    oldCar.LoadModel("models/city/OldCar.obj");
    newCar.LoadModel("models/city/NewCar.obj");
    cube.LoadModel("models/city/cube.obj");

}

void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
     skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "fogColor"), 1, glm::value_ptr(fogColor));
    lightingShader.loadShader(
            "shaders/lighting.vert",
            "shaders/lighting.frag");

}

void initUniforms() {
	myBasicShader.useShaderProgram();
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");
    projection = glm::perspective(glm::radians(45.0f),
                                  (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                                  0.1f, 20.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
	lightColor = glm::vec3(1.0f, 0.8f, 0.5f);
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "useFog"), false);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity"), 0.01f); 
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "fogColor"), 1, glm::value_ptr(fogColor));
    cameraPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "cameraPos");


}


void initSkybox() {
    std::vector<const GLchar*> skyboxFaces = {
            "models/skybox2/right.jpg",
            "models/skybox2/left.jpg",
            "models/skybox2/top.jpg",
            "models/skybox2/bottom.jpg",
            "models/skybox2/back.jpg",
            "models/skybox2/front.jpg"
    };
    mySkyBox.Load(skyboxFaces);
    skyboxModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f)); // Adjusted scale
}


void renderSkyBox() {
    glDepthMask(GL_FALSE);
    skyboxShader.useShaderProgram();
    glm::mat4 skyboxView = glm::mat4(glm::mat3(myCamera.getViewMatrix()));
    glm::mat4 skyboxTranslation = glm::translate(glm::mat4(7.0f), glm::vec3(100.0f, -200.0f, -100.0f));
    glm::mat4 modifiedView = skyboxTranslation * skyboxView;
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(modifiedView));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glm::mat4 skyboxModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(skyboxModelMatrix));
    mySkyBox.Draw(skyboxShader, view, projection);
    glDepthMask(GL_TRUE);
}




void renderParking(gps::Shader shader) {
    shader.useShaderProgram();
    glPolygonMode(GL_FRONT_AND_BACK, currentRenderMode);
    glm::mat4 parkingModel = model;
    parkingModel = glm::scale(parkingModel, glm::vec3(5.0f, 5.0f, 5.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(parkingModel));
    glm::mat3 parkingNormalMatrix = glm::mat3(glm::inverseTranspose(view * parkingModel));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(parkingNormalMatrix));
    glUniform1i(glGetUniformLocation(shader.shaderProgram, "useFog"), true);
    glUniform1f(glGetUniformLocation(shader.shaderProgram, "fogDensity"), 0.01f);
    glUniform3fv(glGetUniformLocation(shader.shaderProgram, "fogColor"), 1, glm::value_ptr(fogColor));

    parking.Draw(shader);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
void renderOldCar(gps:: Shader shader){
    shader.useShaderProgram();
    glm::mat4 carModelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
    carModelMatrix = glm::translate(carModelMatrix, oldCarPosition);
    carModelMatrix = glm::rotate(carModelMatrix, glm::radians(carRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    carModelMatrix = glm::scale(carModelMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(carModelMatrix));
    oldCar.Draw(shader);
};
void renderNewCar(gps::Shader shader) {
    shader.useShaderProgram();
    glm::mat4 carModelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
    carModelMatrix = glm::translate(carModelMatrix, newCarPosition);
    carModelMatrix = glm::rotate(carModelMatrix, glm::radians(carRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    carModelMatrix = glm::scale(carModelMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(carModelMatrix));
    newCar.Draw(shader);
};


void makeCeata(gps::Shader shader) {
    if (isFogEnabled) {
        glm::vec3 cameraPosition = myCamera.getPosition();
        glm::vec3 fogColor = glm::vec3(0.7f, 0.7f, 0.7f);
        float fogDensity = 0.02f; 
        shader.useShaderProgram();
        glUniform3fv(glGetUniformLocation(shader.shaderProgram, "cameraPos"), 1, glm::value_ptr(cameraPosition));
        glUniform3fv(glGetUniformLocation(shader.shaderProgram, "fogColor"), 1, glm::value_ptr(fogColor));
        glUniform1f(glGetUniformLocation(shader.shaderProgram, "fogDensity"), fogDensity);
    }
}


void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "isGlobalLightOn"), isGlobalLightOn);
    renderSkyBox();
    makeCeata(myBasicShader);
	renderParking(myBasicShader);
    renderOldCar(myBasicShader);
    renderNewCar(myBasicShader);
    view = myCamera.getViewMatrix();
    glm::vec3 cameraPosition = myCamera.getPosition();
    glm::vec3 globalLightColor = glm::vec3(1.0f, 1.0f, 0.5f);
    glm::vec3 pointLightColor = glm::vec3(1.0f, 0.0f, 0.0f); 
    myBasicShader.useShaderProgram();
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightColor"), 1, glm::value_ptr(globalLightColor));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColor"), 1, glm::value_ptr(pointLightColor));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "cameraPos"), 1, glm::value_ptr(cameraPosition));
}

void cleanup() {
    myWindow.Delete();
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    glm::mat4 viewMatrix = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f) 
    );


    float farPlane = 100.0f; 
    projection = glm::perspective(glm::radians(60.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.5f, 200.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    initOpenGLState();
   // loadSkyBox();
	initModels();
	initShaders();
	initUniforms();
    setWindowCallbacks();
    initSkybox();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
        renderScene();
		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());
        if (isAutoMoveActive) {
            autoMoveAndRotateFunc(0.5f);
        }
		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
