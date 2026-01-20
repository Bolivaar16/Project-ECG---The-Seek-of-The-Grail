#include <Player/QuestItem.h>

QuestItem::QuestItem(glm::vec3 startPosition, glm::vec3 size) {
    position = startPosition;
    isActive = true;
    isCollected = false;
    box = BoundingBox(position, size);
}

void QuestItem::update() {
    box.update(position, box.size);
}