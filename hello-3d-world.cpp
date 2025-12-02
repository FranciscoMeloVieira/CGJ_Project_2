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

class MyApp : public mgl::App {
public:
  void initCallback(GLFWwindow *win) override;
  void displayCallback(GLFWwindow *win, double elapsed) override;
  void windowCloseCallback(GLFWwindow *win) override;
  void windowSizeCallback(GLFWwindow *win, int width, int height) override;
  void keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) override;
  void mouseButtonCallback(GLFWwindow *win, int button, int action, int mods) override;
  void cursorPosCallback(GLFWwindow* win, double xpos, double ypos);

private:
  const GLuint POSITION = 0, COLOR = 1, UBO_BP = 0;
  GLuint VaoId;

  std::unique_ptr<mgl::ShaderProgram> Shaders = nullptr;
  std::unique_ptr<mgl::Camera> Camera = nullptr;
  GLint ModelMatrixId;

  bool keys[1024]{ false };
  bool rightMouseDown = false;

  double lastMouseX = 0.0f;
  double lastMouseY = 0.0f;
  float mouseSensitivity = 0.1f;


  glm::quat currentRot = glm::quat(1, 0, 0, 0);
  glm::quat targetRot = glm::quat(1, 0, 0, 0);

  float orbitRadius = 15.0f;
  float yawSpeed = 1.0f;
  float pitchSpeed = 1.0f;


  void createShaderProgram();
  void createBufferObjects();
  void destroyBufferObjects();
  void drawScene();
  glm::vec3 updateOrbit(double elapsed);
};

////////////////////////////////////////////////////////////////// VAO, VBO, EBO

typedef struct {
  GLfloat XYZW[4];
  GLfloat RGBA[4];
} Vertex;

const Vertex Vertices[] = {
    // FRONT
    {{-0.5f, -0.5f,  0.5f, 1.0f}, {0.9f, 0.1f, 0.1f, 1.0f}}, // 0
    {{ 0.5f, -0.5f,  0.5f, 1.0f}, {0.9f, 0.1f, 0.1f, 1.0f}}, // 1
    {{ 0.5f,  0.5f,  0.5f, 1.0f}, {0.9f, 0.1f, 0.1f, 1.0f}}, // 2
    {{-0.5f,  0.5f,  0.5f, 1.0f}, {0.9f, 0.1f, 0.1f, 1.0f}}, // 3

    // RIGHT
    {{ 0.5f, -0.5f,  0.5f, 1.0f}, {0.1f, 0.9f, 0.1f, 1.0f}}, // 1
    {{ 0.5f, -0.5f, -0.5f, 1.0f}, {0.1f, 0.9f, 0.1f, 1.0f}}, // 5
    {{ 0.5f,  0.5f, -0.5f, 1.0f}, {0.1f, 0.9f, 0.1f, 1.0f}}, // 6
    {{ 0.5f,  0.5f,  0.5f, 1.0f}, {0.1f, 0.9f, 0.1f, 1.0f}}, // 2

    // TOP
    {{ 0.5f,  0.5f,  0.5f, 1.0f}, {0.1f, 0.1f, 0.9f, 1.0f}}, // 2
    {{ 0.5f,  0.5f, -0.5f, 1.0f}, {0.1f, 0.1f, 0.9f, 1.0f}}, // 6
    {{-0.5f,  0.5f, -0.5f, 1.0f}, {0.1f, 0.1f, 0.9f, 1.0f}}, // 7
    {{-0.5f,  0.5f,  0.5f, 1.0f}, {0.1f, 0.1f, 0.9f, 1.0f}}, // 3

    // BACK
    {{ 0.5f, -0.5f, -0.5f, 1.0f}, {0.1f, 0.9f, 0.9f, 1.0f}}, // 5
    {{-0.5f, -0.5f, -0.5f, 1.0f}, {0.1f, 0.9f, 0.9f, 1.0f}}, // 4
    {{-0.5f,  0.5f, -0.5f, 1.0f}, {0.1f, 0.9f, 0.9f, 1.0f}}, // 7
    {{ 0.5f,  0.5f, -0.5f, 1.0f}, {0.1f, 0.9f, 0.9f, 1.0f}}, // 6

    // LEFT
    {{-0.5f, -0.5f, -0.5f, 1.0f}, {0.9f, 0.1f, 0.9f, 1.0f}}, // 4
    {{-0.5f, -0.5f,  0.5f, 1.0f}, {0.9f, 0.1f, 0.9f, 1.0f}}, // 0
    {{-0.5f,  0.5f,  0.5f, 1.0f}, {0.9f, 0.1f, 0.9f, 1.0f}}, // 3
    {{-0.5f,  0.5f, -0.5f, 1.0f}, {0.9f, 0.1f, 0.9f, 1.0f}}, // 7

    // BOTTOM
    {{-0.5f, -0.5f,  0.5f, 1.0f}, {0.9f, 0.9f, 0.1f, 1.0f}}, // 0
    {{-0.5f, -0.5f, -0.5f, 1.0f}, {0.9f, 0.9f, 0.1f, 1.0f}}, // 4
    {{ 0.5f, -0.5f, -0.5f, 1.0f}, {0.9f, 0.9f, 0.1f, 1.0f}}, // 5
    {{ 0.5f, -0.5f,  0.5f, 1.0f}, {0.9f, 0.9f, 0.1f, 1.0f}}  // 1
};


const unsigned int Indices[] = {
    0,  1,  2,  2,  3,  0,  // FRONT
    4,  5,  6,  6,  7,  4,  // RIGHT
    8,  9,  10, 10, 11, 8,  // TOP
    12, 13, 14, 14, 15, 12, // BACK
    16, 17, 18, 18, 19, 16, // LEFT
    20, 21, 22, 22, 23, 20  // BOTTOM
};

void MyApp::createBufferObjects() {
  GLuint boId[2];

  glGenVertexArrays(1, &VaoId);
  glBindVertexArray(VaoId);
  {
    glGenBuffers(2, boId);
    glBindBuffer(GL_ARRAY_BUFFER, boId[0]);
    {
      glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

      glEnableVertexAttribArray(POSITION);
      glVertexAttribPointer(POSITION, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                            reinterpret_cast<void *>(0));

      glEnableVertexAttribArray(COLOR);
      glVertexAttribPointer(
          COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
          reinterpret_cast<GLvoid *>(sizeof(Vertices[0].XYZW)));
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boId[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices,
                 GL_STATIC_DRAW);
  }
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDeleteBuffers(2, boId);
}

void MyApp::destroyBufferObjects() {
  glBindVertexArray(VaoId);
  glDisableVertexAttribArray(POSITION);
  glDisableVertexAttribArray(COLOR);
  glDeleteVertexArrays(1, &VaoId);
  glBindVertexArray(0);
}

///////////////////////////////////////////////////////////////////////// SHADER

void MyApp::createShaderProgram() {
  Shaders = std::make_unique<mgl::ShaderProgram>();
  Shaders->addShader(GL_VERTEX_SHADER, "color-vs.glsl");
  Shaders->addShader(GL_FRAGMENT_SHADER, "color-fs.glsl");

  Shaders->addAttribute(mgl::POSITION_ATTRIBUTE, POSITION);
  Shaders->addAttribute(mgl::COLOR_ATTRIBUTE, COLOR);
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
    glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));

// Eye(-5,-5,-5) Center(0,0,0) Up(0,1,0)
const glm::mat4 ViewMatrix2 =
    glm::lookAt(glm::vec3(-5.0f, -5.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));

// Orthographic LeftRight(-2,2) BottomTop(-2,2) NearFar(1,10)
const glm::mat4 ProjectionMatrix1 =
    glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 5.0f, 100.0f);

// Perspective Fovy(30) Aspect(640/480) NearZ(1) FarZ(10)
const glm::mat4 ProjectionMatrix2 =
    glm::perspective(glm::radians(30.0f), 4.0f/3.0f, 0.1f, 100.0f);


void MyApp::drawScene() {
  Camera->setProjectionMatrix(ProjectionMatrix2);

  glBindVertexArray(VaoId);
  Shaders->bind();

  glUniformMatrix4fv(ModelMatrixId, 1, GL_FALSE, glm::value_ptr(ModelMatrix));
  glDrawElements(GL_TRIANGLES, sizeof(Indices), GL_UNSIGNED_INT, nullptr);

  Shaders->unbind();
  glBindVertexArray(0);
}

glm::vec3 MyApp::updateOrbit(double elapsed) {
	float dt = static_cast<float>(elapsed);

	// keyboard input for orbit control. Only here for debug, remove later.
    /*
    if (keys[GLFW_KEY_LEFT]) {
		glm::quat q = glm::angleAxis(-yawSpeed * dt, glm::vec3(0.0f, 1.0f, 0.0f));
		targetRot = q * targetRot;
    }
    if (keys[GLFW_KEY_RIGHT]) {
		glm::quat q = glm::angleAxis(yawSpeed * dt, glm::vec3(0.0f, 1.0f, 0.0f));
		targetRot = q * targetRot;
    }
	if (keys[GLFW_KEY_UP]) {
		glm::vec3 localRight = currentRot * glm::vec3(1.0f, 0.0f, 0.0f);
		glm::quat q = glm::angleAxis(-pitchSpeed * dt, localRight);
		targetRot = q * targetRot;
	}
	if (keys[GLFW_KEY_DOWN]) {
		glm::vec3 localRight = currentRot * glm::vec3(1.0f, 0.0f, 0.0f);
		glm::quat q = glm::angleAxis(pitchSpeed * dt, localRight);
		targetRot = q * targetRot;
	}
    */
    currentRot = glm::slerp(currentRot, targetRot, 0.1f);
	glm::vec3 base(0.0f, 0.0f, orbitRadius);

	
    return currentRot * base;
}

////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow *win) {
  createBufferObjects();
  createShaderProgram();
  Camera = std::make_unique<mgl::Camera>(UBO_BP);
  Camera->setViewMatrix(ViewMatrix1);
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

void MyApp::windowCloseCallback(GLFWwindow *win) { destroyBufferObjects(); }

void MyApp::windowSizeCallback(GLFWwindow *win, int winx, int winy) {
  glViewport(0, 0, winx, winy);
}

void MyApp::displayCallback(GLFWwindow *win, double elapsed) { 
	
	glm::vec3 eye = updateOrbit(elapsed);
	glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 up = currentRot * glm::vec3(0.0f, 1.0f, 0.0f);

	glm::mat4 viewMatrix = glm::lookAt(eye, center, up);
	Camera->setViewMatrix(viewMatrix);
    
    drawScene(); 
}

void MyApp::keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) {
	MyApp* app = static_cast<MyApp*>(mgl::Engine::getInstance().getApp());

    if (action == GLFW_PRESS) {
        app->keys[key] = true;
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
        glm::vec3 localRight = currentRot * glm::vec3(1.0f, 0.0f, 0.0f);
        glm::quat qPitch = glm::angleAxis(glm::radians(dy), localRight);

        targetRot = qYaw * qPitch * targetRot;
        lastMouseX = xpos;
        lastMouseY = ypos;
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
