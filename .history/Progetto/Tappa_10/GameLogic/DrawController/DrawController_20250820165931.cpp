#include "DrawController.h"

DrawController::DrawController(std::vector<DrawAnimation> animations)
    : animations(std::move(animations)) {}