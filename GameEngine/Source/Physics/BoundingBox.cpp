#include "Physics/BoundingBox.h"

BoundingBox::BoundingBox()
{
	min = glm::vec3(0.0f);
	max = glm::vec3(0.0f);
	center = glm::vec3(0.0f);
	size = glm::vec3(0.0f);
}

BoundingBox::BoundingBox(glm::vec3 _center, glm::vec3 _size)
{
	update(_center, _size);
}

void BoundingBox::update(glm::vec3 _center, glm::vec3 _size)
{
	center = _center;
	size = _size;

	// We calculate the bounds from the center
	min = center - (size / 2.0f);
	max = center + (size / 2.0f);
}

bool BoundingBox::checkCollision(const BoundingBox& other)
{
	// Colissionn AABB (Axis-Aligned Bounding Box)
	bool collisionX = min.x <= other.max.x && max.x >= other.min.x;
	bool collisionY = min.y <= other.max.y && max.y >= other.min.y;
	bool collisionZ = min.z <= other.max.z && max.z >= other.min.z;

	return collisionX && collisionY && collisionZ;
}