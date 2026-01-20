#pragma once
#include <../glm/glm.hpp>
#include "../Physics/BoundingBox.h"
class Shader;
class Mesh;

class Projectile
{
public:
	glm::vec3 position;
	glm::vec3 direction;
	float speed;
	float lifeTime;
	bool isActive;

	BoundingBox box;

	// Constructor
	Projectile(glm::vec3 startPos, glm::vec3 dir);

	// Update position and physics
	void update(float deltaTime);

	// Draw the projectile using a specific mesh
	void draw(Shader& shader, Mesh& mesh);
};