#pragma once

#include <memory>
#include "../mgl/mgl.hpp"

/**
 * @brief Simple TRS (Translation, Rotation, Scale) container for node transforms.
 */
typedef struct TransformTRS {
	glm::vec3 position = glm::vec3(0.0f);
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	TransformTRS(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
		: position(position), rotation(rotation), scale(scale) {}
	TransformTRS (const glm::vec3& position)
		: position(position) {}
	TransformTRS (const glm::quat& rotation)
		: rotation(rotation) {}
	TransformTRS() = default;
} TransformTRS;

/**
 * @brief Interpolates between two TRS transforms and returns a composed matrix.
 * @param a Start transform.
 * @param b End transform.
 * @param t Blend factor [0,1].
 * @return Interpolated model matrix.
 */
static glm::mat4 interpolateTRS(TransformTRS& a, TransformTRS& b, float t);

/**
 * @brief Scene graph node that can render a mesh and manage hierarchical transforms.
 *
 * Supports local transforms, color, child nodes, and optional animation between
 * two TRS states driven by a parameter t in [0,1].
 */
class ScenegraphNode {
	public:
		/**
		 * @brief Constructs a renderable node with mesh, shaders, base transform and color.
		 */
		ScenegraphNode(mgl::Mesh* mesh, mgl::ShaderProgram* shaders, TransformTRS transformTRS, glm::vec4 color);
		ScenegraphNode() = default;
		/** @brief Adds a child node to this node. */
		void addChild(ScenegraphNode* child);
		/** @brief Draws this node and recursively draws children. */
		void draw();
		/** @brief Sets local position component of the transform. */
		void setPosition(const glm::vec3& position);
		/** @brief Sets local rotation from axis-angle. */
		void setRotation(float angle, const glm::vec3& axis);
		/** @brief Sets local scale component of the transform. */
		void setScale(const glm::vec3& scale);
		/** @brief Defines start/end TRS states for animation. */
		void setAnimation(TransformTRS start, TransformTRS end);
		/** @brief Updates node animation using blend factor t in [0,1]. */
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
		bool isAnimated = false;
};

