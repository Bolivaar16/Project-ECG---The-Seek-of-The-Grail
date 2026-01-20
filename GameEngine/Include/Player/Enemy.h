#pragma once

#include <../glm/glm.hpp>
#include <Physics/BoundingBox.h>

// Define types of enemies
enum class EnemyType {
    ZOMBIE,
    BOSS
};

class Enemy {
public:
    glm::vec3 position;
    float health;
    bool isDead;
    EnemyType type;
    BoundingBox box;
    glm::vec3 size;

    //Constructor
    Enemy(glm::vec3 startPos, EnemyType enemyType);

    // Updates enemy logic
    void update(float deltaTime, glm::vec3 playerPos);

    // Handles damage received
    void takeDamage(float amount);
};