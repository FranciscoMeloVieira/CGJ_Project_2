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

////////////////////////////////////////////////////////////////////////// MYAPP

typedef struct CameraData {
    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;
    glm::quat currentRot = glm::quat(1, 0, 0, 0);
    glm::quat targetRot = glm::quat(1, 0, 0, 0);
    float orbitRadius = 15.0f;
} CameraData;

class MyApp : public mgl::App {
public:
  void initCallback(GLFWwindow *win) override;
  void displayCallback(GLFWwindow *win, double elapsed) override;
  void windowSizeCallback(GLFWwindow *win, int width, int height) override;
  void keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) override;
  void mouseButtonCallback(GLFWwindow *win, int button, int action, int mods) override;
  void cursorPosCallback(GLFWwindow* win, double xpos, double ypos);
  void scrollCallback(GLFWwindow* win, double xoffset, double yoffset) override;

private:
    const GLuint UBO_BP = 0;
    mgl::ShaderProgram* Shaders = nullptr;
	mgl::Camera* Camera = nullptr;
	std::vector<CameraData> Cameras;
    int currentCamera = 0;
    GLint ModelMatrixId;
    mgl::Mesh* Mesh = nullptr;

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
  void updateOrbit();
};

////////////////////////////////////////////////////////////////// VAO, VBO, EBO

void MyApp::createMeshes() {
    std::string mesh_dir = "./assets/";
    // std::string mesh_file = "cube-v.obj";
    // std::string mesh_file = "cube-vn-flat.obj";
    // std::string mesh_file = "cube-vn-smooth.obj";
    // std::string mesh_file = "cube-vt.obj";
    // std::string mesh_file = "cube-vt2.obj";
    // std::string mesh_file = "torus-vtn-flat.obj";
    // std::string mesh_file = "torus-vtn-smooth.obj";
    // std::string mesh_file = "suzanne-vtn-flat.obj";
    // std::string mesh_file = "suzanne-vtn-smooth.obj";
    // std::string mesh_file = "teapot-vn-flat.obj";
    // std::string mesh_file = "teapot-vn-smooth.obj";
    std::string mesh_file = "bunny-vn-flat.obj";
    // std::string mesh_file = "bunny-vn-smooth.obj";
    //std::string mesh_file = "monkey-torus-vtn-flat.obj";
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
    Shaders->addUniformBlock(mgl::CAMERA_BLOCK, UBO_BP);
    Shaders->create();

    ModelMatrixId = Shaders->Uniforms[mgl::MODEL_MATRIX].index;
}

////////////////////////////////////////////////////////////////////////// SCENE

const glm::mat4 ModelMatrix =
    glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 3.0f));

// Eye(5,5,5) Center(0,0,0) Up(0,1,0)
const glm::mat4 ViewMatrix1 =
    glm::lookAt(glm::vec3(0.0f, 0.0f, 15.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));

// Eye(-5,-5,-5) Center(0,0,0) Up(0,1,0)
const glm::mat4 ViewMatrix2 =
    glm::lookAt(glm::vec3(0.0f, 15.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, -1.0f));

// Orthographic LeftRight(-2,2) BottomTop(-2,2) NearFar(1,10)
const glm::mat4 ProjectionMatrix1 =
    glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 5.0f, 100.0f);

// Perspective Fovy(30) Aspect(640/480) NearZ(1) FarZ(10)
const glm::mat4 ProjectionMatrix2 =
    glm::perspective(glm::radians(30.0f), 4.0f/3.0f, 0.1f, 100.0f);

void MyApp::createCamera() {
	CameraData camera;
	camera.ViewMatrix = ViewMatrix1;
	camera.ProjectionMatrix = ProjectionMatrix2;
	Cameras.push_back(camera);

	CameraData camera2;
	camera2.ProjectionMatrix = ProjectionMatrix2;
	camera2.ViewMatrix = ViewMatrix2;
	Cameras.push_back(camera2);

    Camera = new mgl::Camera(UBO_BP);
    Camera->setViewMatrix(Cameras[currentCamera].ViewMatrix);
	Camera->setProjectionMatrix(Cameras[currentCamera].ProjectionMatrix);
}

void MyApp::drawScene() {
    updateOrbit();
    Shaders->bind();
    glUniformMatrix4fv(ModelMatrixId, 1, GL_FALSE, glm::value_ptr(ModelMatrix));
    Mesh->draw();
    Shaders->unbind();
}

void MyApp::updateOrbit() {
	Cameras[currentCamera].currentRot = glm::slerp(Cameras[currentCamera].currentRot, Cameras[currentCamera].targetRot, 0.1f);
    Cameras[currentCamera].ViewMatrix = glm::lookAt(
        Cameras[currentCamera].currentRot * glm::vec3(0.0f, 0.0f, Cameras[currentCamera].orbitRadius),
        glm::vec3(0.0f, 0.0f, 0.0f),
        Cameras[currentCamera].currentRot * glm::vec3(0.0f, 1.0f, 0.0f)
	);

	Camera->setViewMatrix(Cameras[currentCamera].ViewMatrix);

}

////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow *win) {
  createMeshes();
  createShaderPrograms();
  createCamera();
  glfwSetWindowUserPointer(win, this);
  glfwSetKeyCallback(win, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
      MyApp* app = static_cast<MyApp*>(glfwGetWindowUserPointer(win));
      if (app) {
          app->keyCallback(win, key, scancode, action, mods);
      }
      });
  glfwSetMouseButtonCallback(win, [](GLFWwindow* win, int button, int action, int mods) {
      MyApp* app = static_cast<MyApp*>(glfwGetWindowUserPointer(win));
      if (app) app->mouseButtonCallback(win, button, action, mods);
      });

  glfwSetCursorPosCallback(win, [](GLFWwindow* win, double xpos, double ypos) {
      MyApp* app = static_cast<MyApp*>(glfwGetWindowUserPointer(win));
      if (app) app->cursorPosCallback(win, xpos, ypos);
      });

}

void MyApp::windowSizeCallback(GLFWwindow *win, int winx, int winy) {
  glViewport(0, 0, winx, winy);
}

void MyApp::displayCallback(GLFWwindow *win, double elapsed) { 
    drawScene(); 
}

void MyApp::keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) {
	MyApp* app = static_cast<MyApp*>(mgl::Engine::getInstance().getApp());

    if (action == GLFW_PRESS) {
        app->keys[key] = true;
        if (key == GLFW_KEY_C) {
			app->currentCamera = (app->currentCamera + 1) % app->Cameras.size();
        }
    }
    else if (action == GLFW_RELEASE) {
        app->keys[key] = false;
	}
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

void MyApp::cursorPosCallback(GLFWwindow* win, double xpos, double ypos) {
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
    }
}

void MyApp::scrollCallback(GLFWwindow* win, double xoffset, double yoffset) {
    Cameras[currentCamera].orbitRadius -= static_cast<float>(yoffset);
    if (Cameras[currentCamera].orbitRadius < 2.0f) {
        Cameras[currentCamera].orbitRadius = 2.0f;
    }
    if (Cameras[currentCamera].orbitRadius > 50.0f) {
        Cameras[currentCamera].orbitRadius = 50.0f;
	}
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
