////////////////////////////////////////////////////////////////////////////////
//
// Adding a camera and moving to the 3rd dimension.
// A "Hello 3D World" of Modern OpenGL.
//
// Copyright (c) 2013-25 by Carlos Martinho
//
// INTRODUCES:
// VIEW PIPELINE, UNIFORM BUFFER OBJECTS, mglCamera.hpp
//
////////////////////////////////////////////////////////////////////////////////

#include <memory>

#include "../mgl/mgl.hpp"
#include "ScenegraphNode.h"

////////////////////////////////////////////////////////////////////////// MYAPP

typedef struct CameraData {
    glm::mat4 ViewMatrix;
    glm::mat4 PerspectiveMatrix;
    glm::mat4 OrthoProjectionMatrix;
	bool isPerspective = true;
    glm::quat currentRot;
    glm::quat targetRot;
    float orbitRadius = 50.0f;
} CameraData;

class MyApp : public mgl::App {
public:
  void initCallback(GLFWwindow *win) override;
  void displayCallback(GLFWwindow *win, double elapsed) override;
  void windowSizeCallback(GLFWwindow *win, int width, int height) override;
  void keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) override;
  void mouseButtonCallback(GLFWwindow *win, int button, int action, int mods) override;
  void cursorCallback(GLFWwindow* win, double xpos, double ypos) override;
  void scrollCallback(GLFWwindow* win, double xoffset, double yoffset) override;

private:
    const GLuint UBO_BP = 0, COLOR = 5;
    mgl::ShaderProgram* Shaders = nullptr;
	mgl::Camera* Camera = nullptr;
	std::vector<CameraData> Cameras;
    int currentCamera = 0;
    GLint ModelMatrixId, ColorId;
    mgl::Mesh* Mesh = nullptr;
	ScenegraphNode* Root = nullptr;

  bool keys[1024]{ false };
  bool rightMouseDown = false;

  double lastMouseX = 0.0f;
  double lastMouseY = 0.0f;
  float mouseSensitivity = 0.1f;

  
  float yawSpeed = 1.0f;
  float pitchSpeed = 1.0f;

  void createMeshes();
  void createShaderPrograms();
  void createCamera();
  void drawScene();
  void updateCamera();
  void createScenegraph();
};

////////////////////////////////////////////////////////////////// VAO, VBO, EBO

void MyApp::createMeshes() {
    std::string mesh_dir = "./shapes/";
    std::string mesh_file = "Shape1_2_BigTriangle.obj";
    std::string mesh_fullname = mesh_dir + mesh_file;

    Mesh = new mgl::Mesh();
    Mesh->joinIdenticalVertices();
    Mesh->create(mesh_fullname);
}

///////////////////////////////////////////////////////////////////////// SHADER

void MyApp::createShaderPrograms() {
    Shaders = new mgl::ShaderProgram();
    Shaders->addShader(GL_VERTEX_SHADER, "cube-vs.glsl");
    Shaders->addShader(GL_FRAGMENT_SHADER, "cube-fs.glsl");

    Shaders->addAttribute(mgl::POSITION_ATTRIBUTE, mgl::Mesh::POSITION);
    if (Mesh->hasNormals()) {
        Shaders->addAttribute(mgl::NORMAL_ATTRIBUTE, mgl::Mesh::NORMAL);
    }
    if (Mesh->hasTexcoords()) {
        Shaders->addAttribute(mgl::TEXCOORD_ATTRIBUTE, mgl::Mesh::TEXCOORD);
    }
    if (Mesh->hasTangentsAndBitangents()) {
        Shaders->addAttribute(mgl::TANGENT_ATTRIBUTE, mgl::Mesh::TANGENT);
    }

    Shaders->addUniform(mgl::MODEL_MATRIX);
	Shaders->addUniform(mgl::COLOR_ATTRIBUTE);
    Shaders->addUniformBlock(mgl::CAMERA_BLOCK, UBO_BP);
    Shaders->create();

    ModelMatrixId = Shaders->Uniforms[mgl::MODEL_MATRIX].index;
	ColorId = Shaders->Uniforms[mgl::COLOR_ATTRIBUTE].index;
}

////////////////////////////////////////////////////////////////////////// SCENE

const glm::mat4 ModelMatrix =
    glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 3.0f));

// Eye(0,0,50) Center(0,0,0) Up(0,1,0)
const glm::mat4 ViewMatrix1 =
    glm::lookAt(glm::vec3(0.0f, 0.0f, 50.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));

// Eye(0,50,0) Center(0,0,0) Up(0,0,-1)
const glm::mat4 ViewMatrix2 =
    glm::lookAt(glm::vec3(0.0f, 50.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, -1.0f));

// Orthographic LeftRight(-2,2) BottomTop(-2,2) NearFar(1,100)
const glm::mat4 ProjectionMatrix1 =
    glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 1.0f, 100.0f);

// Perspective Fovy(30) Aspect(640/480) NearZ(1) FarZ(100)
const glm::mat4 ProjectionMatrix2 =
    glm::perspective(glm::radians(30.0f), 4.0f/3.0f, 1.0f, 200.0f);

void MyApp::createCamera() {
	CameraData camera;
	camera.ViewMatrix = ViewMatrix1;
	camera.PerspectiveMatrix = ProjectionMatrix2;
	camera.OrthoProjectionMatrix = ProjectionMatrix1;
	camera.currentRot = glm::quat(1,0,0,0);
	camera.targetRot = glm::quat(1,0,0,0);
	Cameras.push_back(camera);

	camera.ViewMatrix = ViewMatrix2;
	camera.currentRot = glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0));
	camera.targetRot = glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0));
	Cameras.push_back(camera);

    Camera = new mgl::Camera(UBO_BP);
    Camera->setViewMatrix(Cameras[currentCamera].ViewMatrix);
	Camera->setProjectionMatrix(Cameras[currentCamera].PerspectiveMatrix);
}

void MyApp::drawScene() {
	Root->draw();
}

void MyApp::updateCamera() {
	// Projection matrix update
    if (Cameras[currentCamera].isPerspective) {
        Camera->setProjectionMatrix(Cameras[currentCamera].PerspectiveMatrix);
    }
    else {
        Camera->setProjectionMatrix(Cameras[currentCamera].OrthoProjectionMatrix);
	}

	// View matrix update
	Cameras[currentCamera].currentRot = glm::slerp(Cameras[currentCamera].currentRot, Cameras[currentCamera].targetRot, 0.1f);
    Cameras[currentCamera].ViewMatrix = glm::lookAt(
        Cameras[currentCamera].currentRot * glm::vec3(0.0f, 0.0f, Cameras[currentCamera].orbitRadius),
        glm::vec3(0.0f, 0.0f, 0.0f),
        Cameras[currentCamera].currentRot * glm::vec3(0.0f, 1.0f, 0.0f)
	);
	Camera->setViewMatrix(Cameras[currentCamera].ViewMatrix);

	// Redraw scene
	drawScene();
}

void MyApp::createScenegraph() {
	Root = new ScenegraphNode(Mesh, Shaders, ModelMatrix, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow *win) {
  createMeshes();
  createShaderPrograms();
  createCamera();
  createScenegraph();
}

void MyApp::windowSizeCallback(GLFWwindow *win, int winx, int winy) {
  glViewport(0, 0, winx, winy);
  float aspect = static_cast<float>(winx) / static_cast<float>(winy);
  Cameras[0].PerspectiveMatrix =
      glm::perspective(glm::radians(30.0f), aspect, 1.0f, 500.0f);
  Cameras[1].PerspectiveMatrix =
      glm::perspective(glm::radians(30.0f), aspect, 1.0f, 500.0f);
  updateCamera();
}

void MyApp::displayCallback(GLFWwindow *win, double elapsed) { 
    drawScene(); 
}

void MyApp::keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) {

    if (action == GLFW_PRESS) {
        keys[key] = true;
        if (key == GLFW_KEY_C) {
			currentCamera = (currentCamera + 1) % Cameras.size();
			Camera->setViewMatrix(Cameras[currentCamera].ViewMatrix);
        }
        if (key == GLFW_KEY_P) {
            if (Cameras[currentCamera].isPerspective) {
                Cameras[currentCamera].isPerspective = false;
            }
            else {
                Cameras[currentCamera].isPerspective = true;
			}
        }
    }
    else if (action == GLFW_RELEASE) {
        keys[key] = false;
	}
	updateCamera();
}

void MyApp::mouseButtonCallback(GLFWwindow *win, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            rightMouseDown = true;
            glfwGetCursorPos(win, &lastMouseX, &lastMouseY);
        }
        else if (action == GLFW_RELEASE) {
            rightMouseDown = false;
        }
    }
}

void MyApp::cursorCallback(GLFWwindow* win, double xpos, double ypos) {
    if (rightMouseDown) {
        float dx = static_cast<float>(xpos - lastMouseX);
        float dy = static_cast<float>(lastMouseY - ypos);

        dx *= mouseSensitivity;
        dy *= mouseSensitivity;

        glm::quat qYaw = glm::angleAxis(glm::radians(-dx), glm::vec3(0.0f, 1.0f, 0.0f));
		Cameras[currentCamera].targetRot = qYaw * Cameras[currentCamera].targetRot;
        glm::vec3 localUp = Cameras[currentCamera].targetRot * glm::vec3(1.0f, 0.0f, 0.0f);
        glm::quat qPitch = glm::angleAxis(glm::radians(dy), localUp);

        Cameras[currentCamera].targetRot = qPitch * Cameras[currentCamera].targetRot;
        lastMouseX = xpos;
        lastMouseY = ypos;

		updateCamera();
    }
}

void MyApp::scrollCallback(GLFWwindow* win, double xoffset, double yoffset) {
    Cameras[currentCamera].orbitRadius -= static_cast<float>(yoffset) * 2;
    if (Cameras[currentCamera].orbitRadius < 30.0f) {
        Cameras[currentCamera].orbitRadius = 30.0f;
    }
    if (Cameras[currentCamera].orbitRadius > 100.0f) {
        Cameras[currentCamera].orbitRadius = 100.0f;
	}
	    updateCamera();
}

/////////////////////////////////////////////////////////////////////////// MAIN

int main(int argc, char *argv[]) {
  mgl::Engine &engine = mgl::Engine::getInstance();
  engine.setApp(new MyApp());
  engine.setOpenGL(4, 6);
  engine.setWindow(800, 600, "Hello Modern 3D World", 0, 1);
  engine.init();
  engine.run();
  exit(EXIT_SUCCESS);
}

////////////////////////////////////////////////////////////////////////////////
