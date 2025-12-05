#pragma once

#include <memory>
#include "../mgl/mgl.hpp"

class ScenegraphNode {
	public:
		ScenegraphNode(mgl::Mesh* mesh, mgl::ShaderProgram* shaders, glm::mat4 transform, glm::vec4 color) :
			Mesh(mesh), Shaders(shaders), localTransform(transform), color(color) {}
		ScenegraphNode() = default;
		void addChild(ScenegraphNode* child);
		void draw();
		void setPosition(const glm::vec3& position);
		void setRotation(float angle, const glm::vec3& axis);
		void setScale(const glm::vec3& scale);
		
	private:
		const GLuint UBO_BP = 0;
		mgl::ShaderProgram* Shaders = nullptr;
		mgl::Mesh* Mesh = nullptr;
		std::vector<std::unique_ptr<ScenegraphNode>> children;
		ScenegraphNode* parent = nullptr;
		glm::mat4 localTransform = glm::mat4(1.0f);
		glm::vec4 color = glm::vec4(1.0f);
};

