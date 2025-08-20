#include "DrawController.h"

void DrawController::queueDraw(int n) {
    cardsToDraw += n;
}

int DrawController::pending() const {
    return cardsToDraw;
}