#pragma once
#include <../glm/glm.hpp>

class BoundingBox
{
public:
	glm::vec3 min; // Back inferior corner
	glm::vec3 max; // Front top corner
	glm::vec3 center;
	glm::vec3 size;

	// Constructors
	BoundingBox();
	BoundingBox(glm::vec3 _center, glm::vec3 _size);

	// Actualiza la posición y recalcula los bordes Update the position and recalculate the bounds
	void update(glm::vec3 _center, glm::vec3 _size);

	// Verifies collision with another box
	bool checkCollision(const BoundingBox& other);
};