#pragma once
#include <../glm/glm.hpp>
#include <Physics/BoundingBox.h>

class QuestItem {
public:
    glm::vec3 position;
    BoundingBox box;

    bool isActive;
    bool isCollected;

    QuestItem(glm::vec3 startPosition, glm::vec3 size);
    void update();
};