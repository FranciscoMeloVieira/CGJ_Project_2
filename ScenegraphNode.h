#pragma once

#include <memory>
#include "../mgl/mgl.hpp"

typedef struct TransformTRS {
	glm::vec3 position = glm::vec3(0.0f);
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	TransformTRS(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
		: position(position), rotation(rotation), scale(scale) {}
	TransformTRS (const glm::vec3& position)
		: position(position) {}
	TransformTRS (const glm::vec3& position, const glm::quat& rotation)
		: position(position), rotation(rotation) {}
	TransformTRS() = default;
} TransformTRS;

static glm::mat4 interpolateTRS(TransformTRS& a, TransformTRS& b, float t);

class ScenegraphNode {
	public:
		ScenegraphNode(mgl::Mesh* mesh, mgl::ShaderProgram* shaders, TransformTRS transformTRS, glm::vec4 color);
		ScenegraphNode() = default;
		void addChild(ScenegraphNode* child);
		void draw();
		void setPosition(const glm::vec3& position);
		void setRotation(float angle, const glm::vec3& axis);
		void setScale(const glm::vec3& scale);
		void setAnimation(TransformTRS start, TransformTRS end);
		void updateAnimation(float t);
		
	private:
		const GLuint UBO_BP = 0;
		mgl::ShaderProgram* Shaders = nullptr;
		mgl::Mesh* Mesh = nullptr;
		std::vector<std::unique_ptr<ScenegraphNode>> children;
		ScenegraphNode* parent = nullptr;
		glm::mat4 localTransform = glm::mat4(1.0f);
		glm::vec4 color = glm::vec4(1.0f);
		TransformTRS start;
		TransformTRS end;;
};

