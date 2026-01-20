#include "../../Include/Player/Projectile.h"
#include "../../Shaders/shader.h"
#include "../../Model Loading/mesh.h"
#include <../glm/gtc/matrix_transform.hpp>

Projectile::Projectile(glm::vec3 startPos, glm::vec3 dir)
{
	position = startPos;
	direction = glm::normalize(dir);
	speed = 100.0f; 
	lifeTime = 5.0f;
	isActive = true;

	// Initialize the bounding box
	box = BoundingBox(position, glm::vec3(0.5f, 0.5f, 0.5f));
}

void Projectile::update(float deltaTime)
{
	if (!isActive) return;

	// Move the projectile forward
	position += direction * speed * deltaTime;
	lifeTime -= deltaTime;

	// Deactivate if time runs out
	if (lifeTime <= 0.0f) {
		isActive = false;
	}

	// Update the collision box to follow the mesh
	box.update(position, glm::vec3(0.5f, 0.5f, 0.5f));
}

void Projectile::draw(Shader& shader, Mesh& mesh)
{
	if (!isActive) return;

	// Create model matrix for the fireball
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::scale(model, glm::vec3(0.2f)); // Scale down the sphere to look like a projectile

	// Send uniforms to shader (assuming standard shader names from your main.cpp)
	glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "model"), 1, GL_FALSE, &model[0][0]);

	// Draw the mesh
	mesh.draw(shader);
}