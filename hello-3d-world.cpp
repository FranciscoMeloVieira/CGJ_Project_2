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
#include <unordered_map>
#include <iostream>

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
    float orbitRadius = 25.0f;
} CameraData;

class MyApp : public mgl::App {
public:
    void initCallback(GLFWwindow* win) override;
    void displayCallback(GLFWwindow* win, double elapsed) override;
    void windowSizeCallback(GLFWwindow* win, int width, int height) override;
    void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) override;
    void mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) override;
    void cursorCallback(GLFWwindow* win, double xpos, double ypos) override;
    void scrollCallback(GLFWwindow* win, double xoffset, double yoffset) override;

private:
    const GLuint UBO_BP = 0, COLOR = 5;
    mgl::ShaderProgram* Shaders = nullptr;
    mgl::Camera* Camera = nullptr;
    std::vector<CameraData> Cameras;
    int currentCamera = 0;
    GLint ModelMatrixId, ColorId;
    std::unordered_map<std::string, std::shared_ptr<mgl::Mesh>> Meshes;
    ScenegraphNode* Root = nullptr;
	std::unordered_map<std::string, TransformTRS> Transforms;

    bool keys[1024]{ false };
    bool rightMouseDown = false;
	bool leftMouseDown = false;

    double lastMouseX = 0.0f;
    double lastMouseY = 0.0f;
    float mouseSensitivity = 0.1f;

    float yawSpeed = 1.0f;
    float pitchSpeed = 1.0f;

    float animationT = 0.0f;
    float animationSpeed = 0.75f;
    int animationDirection = 0; // -1 backward, +1 forward

    void createMeshes();
    mgl::ShaderProgram* createShaderPrograms(mgl::Mesh* Mesh);
    void createCamera();
    void drawScene();
    void updateCamera();
    void createScenegraph();
    void transformations();
    void processInput();
};

////////////////////////////////////////////////////////////////// VAO, VBO, EBO

void MyApp::createMeshes() {
    std::string mesh_dir = "./shapes/";
    std::vector<std::string> mesh_files = {
        "BigTriangle.obj", "Parallelogram.obj", "TallSmallTriangle.obj",
		"Square.obj", "ShortSmallTriangle.obj", "MediumTriangle.obj"
    };

    for (const auto& file : mesh_files) {
        std::shared_ptr<mgl::Mesh> mesh = std::make_shared<mgl::Mesh>();
		mesh->joinIdenticalVertices();
        mesh->create(mesh_dir + file);
		Meshes.insert({ file.substr(0, file.find_last_of('.')), mesh });
	}
}

///////////////////////////////////////////////////////////////////////// SHADER

mgl::ShaderProgram* MyApp::createShaderPrograms(mgl::Mesh* Mesh) {
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

    return Shaders;
}

///////////////////////////////////////////////////////////////////////// SCENEGRAPH

const glm::mat4 I = glm::mat4(1.0f);

const float global_scale = 0.1f;

void MyApp::transformations() {

    // Pickagram Root
    Transforms.insert({ "PickagramRoot_Start", TransformTRS() });
    Transforms.insert({ "PickagramRoot_End" , TransformTRS(glm::vec3(0.0f, 10.0f, 0.0f)) });

	// Big Triangle 1
	const float hypotenuse = 89.0f * global_scale;
	const float side_length = glm::sqrt(glm::pow(hypotenuse, 2) * 2) / 2;
    const float centroid = side_length / 3;
	const float centroid_diagonal = glm::sqrt(glm::pow(centroid, 2) * 2);
	const float height = side_length * glm::sin(glm::radians(45.0f));

	// Square
    const float square_side = side_length / 2;
	const float square_diagonal = glm::sqrt(2 * glm::pow(square_side, 2));

    Transforms.insert({ "Square_Start", TransformTRS(glm::vec3(square_diagonal / 2, 0.0f, 0.0f))});
    Transforms.insert({ "BigTriangle1_Start", TransformTRS(glm::vec3(-(centroid_diagonal + square_diagonal / 2), 0.0f, 0.0f))});

    
	// Tall Small Triangle
    const float tall_small_hypotenuse = square_diagonal;
    const float tall_small_side = square_side;
	const float tall_small_centroid = tall_small_side / 3;
	const float tall_small_centroid_diagonal = glm::sqrt(glm::pow(tall_small_centroid, 2) * 2);

    Transforms.insert({ "TallSmallTriangle_Start", TransformTRS(glm::vec3(centroid_diagonal, 0.0f, tall_small_centroid_diagonal))});

	// Big Triangle 2
	const float large_triangle2_centroid_diagonal = centroid_diagonal;

    Transforms.insert({ "BigTriangle2_Start", TransformTRS(glm::vec3(-(square_diagonal / 2), 0.0f, -large_triangle2_centroid_diagonal),
                                                (glm::angleAxis(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))))});

	// Medium Triangle
	const float medium_side = 89.0f * global_scale / 2;
	const float medium_centroid = medium_side / 3;

    Transforms.insert({ "MediumTriangle_Start", TransformTRS(glm::vec3(square_diagonal / 2 - medium_centroid, 0.0f, medium_side - medium_centroid))});

	// Short Small Triangle
	const float short_small_hypotenuse = medium_side;
    const float short_small_side = square_side;
	const float short_small_centroid = short_small_side / 3;
	const float short_small_centroid_diagonal = glm::sqrt(glm::pow(short_small_centroid, 2) * 2);
	const float short_small_height = short_small_side * glm::sin(glm::radians(45.0f));

    Transforms.insert({ "ShortSmallTriangle_Start", TransformTRS(glm::vec3(medium_centroid - (short_small_height - short_small_centroid_diagonal), 0.0f,
                                                                                -(medium_side + (short_small_hypotenuse / 2 - medium_centroid)))) });

	// Parallelogram
	const float parallelogram_side = side_length / 2;
    const float parallelogram_base = 89.0f * global_scale / 2;
	const float parallelogram_height = parallelogram_side * glm::sin(glm::radians(45.0f));
    const float parallelogram_centroid_x = (parallelogram_base + parallelogram_side * glm::sin(glm::radians(45.0f))) / 2;
    

    Transforms.insert({ "Parallelogram_Start", TransformTRS(glm::vec3(parallelogram_centroid_x - (height - centroid_diagonal), 0.0f,
                                                                            hypotenuse / 2 - parallelogram_height / 2))});

}

void MyApp::createScenegraph() {

	Root = new ScenegraphNode();

	// Pickagram
	ScenegraphNode* pickagramRoot = new ScenegraphNode();
    pickagramRoot->setAnimation(Transforms.at("PickagramRoot_Start"), Transforms.at("PickagramRoot_End"));
	Root->addChild(pickagramRoot);

    ScenegraphNode* square = new ScenegraphNode(Meshes.at("Square").get(),
                                createShaderPrograms(Meshes.at("Square").get()),
                                Transforms.at("Square_Start"),
		                        glm::vec4(0.0f, 0.7f, 0.0f, 1.0f)); // Green
	pickagramRoot->addChild(square);

    ScenegraphNode* largeTriangle1 = new ScenegraphNode(Meshes.at("BigTriangle").get(),
                                createShaderPrograms(Meshes.at("BigTriangle").get()), 
                                Transforms.at("BigTriangle1_Start"),
		                        glm::vec4(0.85f, 0.0f, 0.85f, 1.0f)); // Magenta
	square->addChild(largeTriangle1);

    ScenegraphNode* tallSmallTriangle = new ScenegraphNode(Meshes.at("TallSmallTriangle").get(),
                                createShaderPrograms(Meshes.at("TallSmallTriangle").get()),
		                        Transforms.at("TallSmallTriangle_Start"),
                                glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)); // Cyan
	largeTriangle1->addChild(tallSmallTriangle);

    ScenegraphNode* largeTriangle2 = new ScenegraphNode(Meshes.at("BigTriangle").get(),
                                createShaderPrograms(Meshes.at("BigTriangle").get()),
                                Transforms.at("BigTriangle2_Start"),
                                glm::vec4(0.3f, 0.6f, 1.0f, 1.0f)); // Light Blue
	square->addChild(largeTriangle2);

    ScenegraphNode* mediumTriangle = new ScenegraphNode(Meshes.at("MediumTriangle").get(),
                                createShaderPrograms(Meshes.at("MediumTriangle").get()),
		                        Transforms.at("MediumTriangle_Start"),
                                glm::vec4(0.5f, 0.0f, 0.5f, 1.0f)); // Purple
	square->addChild(mediumTriangle);

    ScenegraphNode* shortSmallTriangle = new ScenegraphNode(Meshes.at("ShortSmallTriangle").get(),
                                createShaderPrograms(Meshes.at("ShortSmallTriangle").get()),
		                        Transforms.at("ShortSmallTriangle_Start"),
                                glm::vec4(0.5f, 0.0f, 0.0f, 1.0f)); // Red
	mediumTriangle->addChild(shortSmallTriangle);

	ScenegraphNode* parallelogram = new ScenegraphNode(Meshes.at("Parallelogram").get(),
                                createShaderPrograms(Meshes.at("Parallelogram").get()), 
		                        Transforms.at("Parallelogram_Start"),
		                        glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)); // Orange   
	largeTriangle1->addChild(parallelogram);
}


////////////////////////////////////////////////////////////////////////// SCENE

void MyApp::drawScene() {
    Root->draw();
}

////////////////////////////////////////////////////////////////////// CAMERA


// Eye(0,0,25) Center(0,0,0) Up(0,1,0)
const glm::mat4 ViewMatrix1 =
glm::lookAt(glm::vec3(0.0f, 0.0f, 25.0f), glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

// Eye(0,25,0) Center(0,0,0) Up(0,0,-1)
const glm::mat4 ViewMatrix2 =
glm::lookAt(glm::vec3(0.0f, 25.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, -1.0f));

// Orthographic LeftRight(-2,2) BottomTop(-2,2) NearFar(1,100)
const glm::mat4 ProjectionMatrix1 =
glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 1.0f, 100.0f);

// Perspective Fovy(30) Aspect(640/480) NearZ(1) FarZ(100)
const glm::mat4 ProjectionMatrix2 =
glm::perspective(glm::radians(30.0f), 4.0f / 3.0f, 1.0f, 200.0f);

void MyApp::createCamera() {
    CameraData camera;
    camera.ViewMatrix = ViewMatrix1;
    camera.PerspectiveMatrix = ProjectionMatrix2;
    camera.OrthoProjectionMatrix = ProjectionMatrix1;
    camera.currentRot = glm::quat(1, 0, 0, 0);
    camera.targetRot = glm::quat(1, 0, 0, 0);
    Cameras.push_back(camera);

    camera.ViewMatrix = ViewMatrix2;
    camera.currentRot = glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0));
    camera.targetRot = glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0));
    Cameras.push_back(camera);

    Camera = new mgl::Camera(UBO_BP);
    Camera->setViewMatrix(Cameras[currentCamera].ViewMatrix);
    Camera->setProjectionMatrix(Cameras[currentCamera].PerspectiveMatrix);
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

void MyApp::processInput() {
    if (keys[GLFW_KEY_RIGHT] && !keys[GLFW_KEY_LEFT])
        animationDirection = +1;
    else if (keys[GLFW_KEY_LEFT] && !keys[GLFW_KEY_RIGHT])
        animationDirection = -1;
    else
        animationDirection = 0;
}


////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow* win) {
    createMeshes();
    createCamera();
    transformations();
    createScenegraph();
}

void MyApp::windowSizeCallback(GLFWwindow* win, int winx, int winy) {
    glViewport(0, 0, winx, winy);
    float aspect = static_cast<float>(winx) / static_cast<float>(winy);
    Cameras[0].PerspectiveMatrix =
        glm::perspective(glm::radians(30.0f), aspect, 1.0f, 500.0f);
    Cameras[1].PerspectiveMatrix =
        glm::perspective(glm::radians(30.0f), aspect, 1.0f, 500.0f);
    updateCamera();
}

void MyApp::displayCallback(GLFWwindow* win, double elapsed) {
    animationT += animationDirection * animationSpeed * elapsed;

    animationT = glm::clamp(animationT, 0.0f, 1.0f);

    Root->updateAnimation(animationT);
	processInput();
    drawScene();
}

void MyApp::keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {

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

void MyApp::mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            rightMouseDown = true;
            glfwGetCursorPos(win, &lastMouseX, &lastMouseY);
        }
        else if (action == GLFW_RELEASE) {
            rightMouseDown = false;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            leftMouseDown = true;
            glfwGetCursorPos(win, &lastMouseX, &lastMouseY);
        }
        else if (action == GLFW_RELEASE) {
            leftMouseDown = false;
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
    if (leftMouseDown) {
        float dx = static_cast<float>(xpos - lastMouseX);
        float dy = static_cast<float>(lastMouseY - ypos);

		dx *= mouseSensitivity;
		dy *= mouseSensitivity;

        glm::vec3 camRight = Cameras[currentCamera].targetRot * glm::vec3(1, 0, 0);
        glm::vec3 camForward = Cameras[currentCamera].targetRot * glm::vec3(0, 0, -1);

        glm::vec3 movement =
            camRight * dx * 0.1f +
            camForward * dy * 0.1f;

		Root->setPosition((movement));

        lastMouseX = xpos;
        lastMouseY = ypos;
    }
}

void MyApp::scrollCallback(GLFWwindow* win, double xoffset, double yoffset) {
    Cameras[currentCamera].orbitRadius -= static_cast<float>(yoffset) * 2;
    if (Cameras[currentCamera].orbitRadius < 5.0f) {
        Cameras[currentCamera].orbitRadius = 5.0f;
    }
    if (Cameras[currentCamera].orbitRadius > 75.0f) {
        Cameras[currentCamera].orbitRadius = 75.0f;
    }
    updateCamera();
}

/////////////////////////////////////////////////////////////////////////// MAIN

int main(int argc, char* argv[]) {
    mgl::Engine& engine = mgl::Engine::getInstance();
    engine.setApp(new MyApp());
    engine.setOpenGL(4, 6);
    engine.setWindow(800, 600, "Hello Modern 3D World", 0, 1);
    engine.init();
    engine.run();
    exit(EXIT_SUCCESS);
}

////////////////////////////////////////////////////////////////////////////////