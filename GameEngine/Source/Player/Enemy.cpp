#include <../Include/Player/Enemy.h>
#include <iostream>

Enemy::Enemy(glm::vec3 startPos, EnemyType enemyType) {
    position = startPos;
    type = enemyType;
    isDead = false;
    

    if (type == EnemyType::BOSS) {
        health = 500.0f;
        size = glm::vec3(40.0f, 60.0f, 40.0f);
    }
    else {
        health = 100.0f;
        size = glm::vec3(10.0f, 20.0f, 10.0f);
    }

    box = BoundingBox(position, size);
}

void Enemy::update(float deltaTime, glm::vec3 playerPos) {
    if (isDead) return;

    //Movement towards the player
    glm::vec3 direction = glm::normalize(playerPos - position);
    float speed = (type == EnemyType::BOSS) ? 20.0f : 30.0f;
    position += direction * speed * deltaTime;
    box.update(position, size);
}

void Enemy::takeDamage(float amount) {
    if (isDead) return;

    health -= amount;

    if (health <= 0.0f) {
        health = 0.0f;
        isDead = true;

        
        std::cout << "Enemy died!" << std::endl;
    }
}