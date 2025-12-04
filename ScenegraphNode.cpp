#include "ScenegraphNode.h"

void ScenegraphNode::addChild(ScenegraphNode* child) {
	this->children.push_back(std::unique_ptr<ScenegraphNode>(child));
	child->parent = this;
}

void ScenegraphNode::draw() {
	if (!(Shaders == nullptr || Mesh == nullptr)) {
		// Compute global transform
		glm::mat4 globalTransform = localTransform;
		ScenegraphNode* p = parent;
		while (p != nullptr) {
			globalTransform = p->localTransform * globalTransform;
			p = p->parent;
		}

		Shaders->bind();
		GLint ModelMatrixId = Shaders->Uniforms[mgl::MODEL_MATRIX].index;
		GLint ColorId = Shaders->Uniforms[mgl::COLOR_ATTRIBUTE].index;
		glUniformMatrix4fv(ModelMatrixId, 1, GL_FALSE, glm::value_ptr(globalTransform));
		glUniform4fv(ColorId, 1, glm::value_ptr(color));
		if (Mesh != nullptr) Mesh->draw();
		Shaders->unbind();
	}

	// Draw children
	for (auto& child : children) {
		child->draw();
	}
}

void ScenegraphNode::setPosition(const glm::vec3& position) {
	localTransform = glm::translate(glm::mat4(1.0f), position) * localTransform;
}

void ScenegraphNode::setRotation(float angle, const glm::vec3& axis) {
	localTransform = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis) * localTransform;
}

void ScenegraphNode::setScale(const glm::vec3& scale) {
	localTransform = glm::scale(glm::mat4(1.0f), scale) * localTransform;
}