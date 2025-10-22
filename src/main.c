#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>

#define GRAVITY_CONSTANT 80.0

typedef struct CelestialObject {
  Vector2 pos;
  Vector2 vel;
  Color color;
  float radius;
  float mass;
} CelestialObject;

void doPairInteraction(CelestialObject **celestialObjectList, int lenList);
void interactGravity(CelestialObject *obj1, CelestialObject *obj2);
void updatePosition(CelestialObject **obj, int lenList);

int main() {
  const int screenWidth = 2000;
  const int screenHeight = 1000;

  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(screenWidth, screenHeight, "Celestial Mechanics Simulator");
  SetTargetFPS(60);

  float middleX = GetScreenWidth() / 2.0f;
  float middleY = GetScreenHeight() / 2.0f;

  float starSize = 10.0;

  CelestialObject alpha = {.pos = {.x = middleX, .y = middleY},
                           .vel = {.x = 0, .y = 0},
                           .color = YELLOW,
                           .radius = starSize,
                           .mass = 1000};
  CelestialObject beta = {.pos = {.x = middleX + 50, .y = middleY + 50},
                          .vel = {.x = 50, .y = -50},
                          .color = RED,
                          .radius = starSize,
                          .mass = 1000};
  CelestialObject gamma = {.pos = {.x = middleX - 50, .y = middleY - 50},
                           .vel = {.x = -30, .y = 20},
                           .color = GREEN,
                           .radius = starSize,
                           .mass = 1000};

  CelestialObject *objCollection[] = {&alpha, &beta, &gamma};

  while (!WindowShouldClose()) {
    // Update
    doPairInteraction(objCollection,
                      sizeof objCollection / sizeof objCollection[0]);
    updatePosition(objCollection,
                   sizeof objCollection / sizeof objCollection[0]);
    // Render
    BeginDrawing();
    ClearBackground(BLACK);

    DrawCircleV(alpha.pos, alpha.radius, alpha.color);
    DrawCircleV(beta.pos, beta.radius, beta.color);
    DrawCircleV(gamma.pos, gamma.radius, gamma.color);

    EndDrawing();
  }

  CloseWindow();

  return EXIT_SUCCESS;
}

void doPairInteraction(CelestialObject **listCelestialObj, int listLen) {
  for (int i = 0; i <= listLen - 2; i++) {
    for (int j = i + 1; j <= listLen - 1; j++) {
      interactGravity(listCelestialObj[i], listCelestialObj[j]);
    }
  }
}

void updatePosition(CelestialObject **objList, int listLen) {
  float deltaTime = GetFrameTime();
  for (int i = 0; i <= listLen - 1; i++) {
    CelestialObject *obj = objList[i];
    obj->pos = Vector2Add(obj->pos, Vector2Scale(obj->vel, deltaTime));
  }
}

void interactGravity(CelestialObject *obj1, CelestialObject *obj2) {
  float deltaTime = GetFrameTime();
  // Change object1 state by object2 influence
  Vector2 distanceVec = Vector2Subtract(obj2->pos, obj1->pos);
  float invSqrDist = 1.0 / Vector2LengthSqr(distanceVec);
  Vector2 gravityForce1 = Vector2Scale(
      distanceVec, (GRAVITY_CONSTANT * obj1->mass * obj2->mass * invSqrDist));

  Vector2 accel1 = Vector2Scale(gravityForce1, 1.0 / obj1->mass);
  Vector2 deltaVel1 = Vector2Scale(accel1, deltaTime);
  obj1->vel = Vector2Add(obj1->vel, deltaVel1);
}
