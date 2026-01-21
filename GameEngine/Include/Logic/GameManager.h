#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <../glm/glm.hpp>
#include <Physics/BoundingBox.h>
#include <Player/Enemy.h>

// Structure to define a game task
struct Task {
    std::string description; // Text shown to player
    glm::vec3 targetPosition; // Where the task happens
    float interactionRadius; // How close you need to be
    bool isCompleted;
    bool requiresCombat; // If true, task completes when enemies are dead
    int requiredKills;
    int currentKills;
    bool requiresPickup;
};

class GameManager {
public:
    std::vector<Task> tasks;
    int currentTaskIndex;
    bool gameFinished;
    bool grailSpawned;
    glm::vec3 grailPosition;
    bool isGameOver;

    // Constructor
    GameManager();

    // Check if the player has completed the current task
    // We pass player position and a flag checking if enemies are dead
    void update(glm::vec3 playerPos, std::vector<Enemy>& enemies);

    // Get current task description for display
    std::string getCurrentTaskInfo();
};