#include <../Include/Logic/GameManager.h>
#include <../Include/Player/Projectile.h>
#include <../Include/Player/Enemy.h>

GameManager::GameManager() {
    currentTaskIndex = 0;
    gameFinished = false;
    grailSpawned = false;
    grailPosition = glm::vec3(0.0f, 0.0f, -20.0f);
    isGameOver = false;

    // 1. Search among the rocks
    Task t1;
    t1.description = "Mission 1: Find the staff hidden among the rocks.";
    t1.requiredKills = 0;
    t1.requiresPickup = false; // Assuming it is zone-based
    t1.isCompleted = false;
    tasks.push_back(t1);

    // 2. Kill 5 zombies
    Task t2;
    t2.description = "Mission 2: Eliminate 5 zombies to clear the area.";
    t2.requiredKills = 5;
    t2.currentKills = 0;
    t2.requiresPickup = false;
    t2.isCompleted = false;
    tasks.push_back(t2);

    // 3. Kill the 

    Task t3;
    t3.description = "Mission 3: THE BOSS HAS AWAKENED! Finish him.";
    t3.requiredKills = 1; // The boss counts as 1 special kill
    t3.currentKills = 0;
    t3.requiresPickup = false;
    t3.isCompleted = false;
    tasks.push_back(t3);

    // 4. Pick up the Holy Grail
    Task t4;
    t4.description = "Mission 4: The Grail has descended. Pick it up.";
    t4.requiredKills = 0;
    t4.requiresPickup = true;
    t4.isCompleted = false;
    tasks.push_back(t4);

    // 5. Take to the tower
    Task t5;
    t5.description = "Mission 5: Take the Grail to the King's Tower.";
    t5.requiredKills = 0;
    t5.requiresPickup = false;
    t5.isCompleted = false;
    tasks.push_back(t5);
}

void GameManager::update(glm::vec3 playerPos, std::vector<Enemy>& enemies) {
    if (gameFinished) return;

    Task& current = tasks[currentTaskIndex];

    // MISSION 1 LOGIC: Go to the rocks
    if (currentTaskIndex == 0) {
        // Let's say the rocks are at (10, 0, 10)
        if (glm::distance(playerPos, glm::vec3(10.0f, 0.0f, 100.0f)) < 5.0f) {
            current.isCompleted = true;
        }
    }
    // MISSION 2 LOGIC: Count kills
    else if (currentTaskIndex == 1) {
        int deadCount = 0;
        for (const auto& enemy : enemies) {
            if (enemy.type == EnemyType::ZOMBIE && enemy.isDead) {
                deadCount++;
            }
        }
        current.currentKills = deadCount;
        if (deadCount >= current.requiredKills) {
            current.isCompleted = true;
            std::cout << "Mission 2 Completed: Zombies purged." << std::endl;
        }
    }
    // MISSION 3 LOGIC: The Boss
    else if (currentTaskIndex == 2) {
        bool bossIsDead = false;
        for (const auto& enemy : enemies) {
            if (enemy.type == EnemyType::BOSS && enemy.isDead) {
                bossIsDead = true;
                break;
            }
        }

        if (bossIsDead) {
            current.isCompleted = true;
            grailSpawned = true; // Trigger Grail spawn
            std::cout << "Mission 3 Completed: Boss defeated." << std::endl;
        }
    }
    // MISSION 4 LOGIC: Pick up Grail
    else if (currentTaskIndex == 3) {
        if (grailSpawned && glm::distance(playerPos, grailPosition) < 2.0f) {
            current.isCompleted = true;
            grailSpawned = false; // We picked it up (remove from world)
        }
    }
    // MISSION 5 LOGIC: Go to the Tower
    else if (currentTaskIndex == 4) {
        // The tower is at (0, 0, 30) for example
        if (glm::distance(playerPos, glm::vec3(0.0f, 0.0f, 30.0f)) < 5.0f) {
            current.isCompleted = true;
            gameFinished = true;
        }
    }

    // Advance mission
    if (current.isCompleted) {
        currentTaskIndex++;
        if (currentTaskIndex >= tasks.size()) {
            // Game Finished
        }
    }
}

std::string GameManager::getCurrentTaskInfo() {
    if (gameFinished) return "YOU WIN!";
    return tasks[currentTaskIndex].description;
}

// Function to handle collisions between projectiles and enemies
void checkProjectileCollisions(std::vector<Projectile>& projectiles, std::vector<Enemy>& enemies) {

    for (auto& proj : projectiles) {

        if (!proj.isActive) continue;

        for (auto& enemy : enemies) {

            // Skip this enemy if it is already dead
            if (enemy.isDead) continue;

            // Use the checkCollision method from your BoundingBox class
            if (proj.box.checkCollision(enemy.box)) {

                //Apply damage
                enemy.takeDamage(50.0f);

                //Deactivate the projectile immediately
                proj.isActive = false;

                // Since the projectile is destroyed, it cannot hit other enemies
                break;
            }
        }
    }
}

