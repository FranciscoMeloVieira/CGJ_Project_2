#include "ScenegraphNode.h"


static glm::mat4 interpolateTRS(TransformTRS& a, TransformTRS& b, float t) {
	glm::vec3 pos = glm::mix(a.position, b.position, t);
	glm::vec3 scale = glm::mix(a.scale, b.scale, t);
	glm::quat rot = glm::slerp(a.rotation, b.rotation, t);

	return glm::translate(glm::mat4(1.0f), pos)
		* glm::mat4_cast(rot)
		* glm::scale(glm::mat4(1.0f), scale);
}

ScenegraphNode::ScenegraphNode(mgl::Mesh* mesh, mgl::ShaderProgram* shaders, TransformTRS transformTRS, glm::vec4 color) {
	Mesh = mesh;
	Shaders = shaders;
	localTransform = glm::translate(glm::mat4(1.0f), transformTRS.position)
					* glm::mat4_cast(transformTRS.rotation)
					* glm::scale(glm::mat4(1.0f), transformTRS.scale);
	this->color = color;
	start = transformTRS;
	end = transformTRS;
}

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

void ScenegraphNode::setAnimation(TransformTRS start, TransformTRS end) {
	this->start = start;
	this->end = end;
	isAnimated = true;
}

void ScenegraphNode::updateAnimation(float t) {
	if (isAnimated) localTransform = interpolateTRS(start, end, t);
	for (auto& child : children) {
		child->updateAnimation(t);
	}
}
