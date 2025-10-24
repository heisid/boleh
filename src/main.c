#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <time.h>

const float GRAVITY_CONSTANT = 500.0;
const float MASS_RADIUS_RATIO = 200.0;
const float MIN_DISTANCE = 10.0f;

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 800;
const int FIELD_RESOLUTION = 20;

typedef struct CelestialObject {
  Vector2 pos;
  Vector2 vel;
  Color color;
  float radius;
  float mass;
} CelestialObject;

void doPairInteraction(CelestialObject **celestialObjectList, int lenList);
void interactGravity(CelestialObject *obj1, CelestialObject *obj2);
void checkCollision(CelestialObject *obj1, CelestialObject *obj2);
void updatePosition(CelestialObject **obj, int lenList);
void setRadius(CelestialObject *obj);
CelestialObject **generateRandomObjects(int count);
void drawGravityField(CelestialObject **objList, int objCount);
Color getPotentialColor(float potential);

int main() {
  srand(time(NULL));
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Celestial Mechanics Simulator");
  SetTargetFPS(60);

  float middleX = GetScreenWidth() / 2.0f;
  float middleY = GetScreenHeight() / 2.0f;

  int objCount = 20;
  CelestialObject **objCollection = generateRandomObjects(objCount);

  while (!WindowShouldClose()) {
    // Update
    doPairInteraction(objCollection, objCount);
    updatePosition(objCollection, objCount);

    // Render
    BeginDrawing();
    ClearBackground(BLACK);

    drawGravityField(objCollection, objCount);

    for (int i = 0; i < objCount; i++) {
      DrawCircleV(objCollection[i]->pos, objCollection[i]->radius,
                  objCollection[i]->color);
    }

    EndDrawing();
  }

  for (int i = 0; i < objCount; i++) {
    free(objCollection[i]);
  }
  free(objCollection);

  CloseWindow();

  return EXIT_SUCCESS;
}

void drawGravityField(CelestialObject **objList, int objCount) {
  for (int x = 0; x < SCREEN_WIDTH; x += FIELD_RESOLUTION) {
    for (int y = 0; y < SCREEN_HEIGHT; y += FIELD_RESOLUTION) {
      Vector2 testPos = {x + FIELD_RESOLUTION / 2.0f,
                         y + FIELD_RESOLUTION / 2.0f};
      float totalPotential = 0.0f;

      for (int i = 0; i < objCount; i++) {
        Vector2 diff = Vector2Subtract(objList[i]->pos, testPos);
        float dist = Vector2Length(diff);

        if (dist < 1.0f)
          dist = 1.0f;

        // Gravitational potential: U = -G*M/r
        totalPotential += (GRAVITY_CONSTANT * objList[i]->mass) / dist;
      }

      Color fieldColor = getPotentialColor(totalPotential);
      DrawRectangle(x, y, FIELD_RESOLUTION, FIELD_RESOLUTION, fieldColor);
    }
  }
}

Color getPotentialColor(float potential) {
  // Normalize potential to a reasonable range (0-1)
  float normalized = potential / 150000.0f;
  if (normalized > 1.0f)
    normalized = 1.0f;

  // Create a color gradient: black -> blue -> purple -> red -> yellow
  if (normalized < 0.25f) {
    // Black to blue
    float t = normalized / 0.25f;
    return (Color){0, 0, (unsigned char)(t * 100), 255};
  } else if (normalized < 0.5f) {
    // Blue to purple
    float t = (normalized - 0.25f) / 0.25f;
    return (Color){(unsigned char)(t * 100), 0, (unsigned char)(100 + t * 55),
                   255};
  } else if (normalized < 0.75f) {
    // Purple to red
    float t = (normalized - 0.5f) / 0.25f;
    return (Color){(unsigned char)(100 + t * 155), 0,
                   (unsigned char)(155 - t * 155), 255};
  } else {
    // Red to yellow
    float t = (normalized - 0.75f) / 0.25f;
    return (Color){255, (unsigned char)(t * 200), 0, 255};
  }
}

void setRadius(CelestialObject *obj) {
  obj->radius = obj->mass / MASS_RADIUS_RATIO;
}

int genRandomInt(int min, int max) { return rand() % (max - min + 1) + min; }

float genRandomFloat(float min, float max) {
  float scale = (float)rand() / (float)RAND_MAX;
  return min + scale * (max - min);
}

CelestialObject **generateRandomObjects(int count) {
  CelestialObject **list = malloc(count * sizeof(CelestialObject *));
  for (int i = 0; i < count; i++) {
    CelestialObject *obj = malloc(sizeof(CelestialObject));
    obj->pos.x = genRandomInt(0, SCREEN_WIDTH);
    obj->pos.y = genRandomInt(0, SCREEN_HEIGHT);

    obj->vel.x = genRandomFloat(-200.0, 200.0);
    obj->vel.y = genRandomFloat(-200.0, 200.0);

    obj->mass = genRandomFloat(1000, 5000);
    setRadius(obj);

    obj->color = WHITE;
    list[i] = obj;
  }

  return list;
}

void doPairInteraction(CelestialObject **listCelestialObj, int listLen) {
  for (int i = 0; i < listLen; i++) {
    for (int j = i + 1; j < listLen; j++) {
      checkCollision(listCelestialObj[i], listCelestialObj[j]);

      interactGravity(listCelestialObj[i], listCelestialObj[j]);
      interactGravity(listCelestialObj[j], listCelestialObj[i]);
    }
  }
}

void checkCollision(CelestialObject *obj1, CelestialObject *obj2) {
  float distance = Vector2Distance(obj1->pos, obj2->pos);
  float minDist = obj1->radius + obj2->radius;

  if (distance < minDist && distance > 0.001f) {
    float overlap = minDist - distance;

    Vector2 direction = Vector2Normalize(Vector2Subtract(obj2->pos, obj1->pos));

    float totalMass = obj1->mass + obj2->mass;
    float obj1Ratio = obj2->mass / totalMass;
    float obj2Ratio = obj1->mass / totalMass;

    obj1->pos = Vector2Subtract(obj1->pos,
                                Vector2Scale(direction, overlap * obj1Ratio));
    obj2->pos =
        Vector2Add(obj2->pos, Vector2Scale(direction, overlap * obj2Ratio));

    Vector2 relativeVel = Vector2Subtract(obj2->vel, obj1->vel);
    float velAlongNormal = Vector2DotProduct(relativeVel, direction);

    if (velAlongNormal < 0) {
      float restitution = 0.5f;

      float impulseScalar = -(1 + restitution) * velAlongNormal;
      impulseScalar /= (1.0f / obj1->mass + 1.0f / obj2->mass);

      Vector2 impulse = Vector2Scale(direction, impulseScalar);
      obj1->vel =
          Vector2Subtract(obj1->vel, Vector2Scale(impulse, 1.0f / obj1->mass));
      obj2->vel =
          Vector2Add(obj2->vel, Vector2Scale(impulse, 1.0f / obj2->mass));
    }
  }
}

void updatePosition(CelestialObject **objList, int listLen) {
  float deltaTime = GetFrameTime();
  for (int i = 0; i < listLen; i++) {
    CelestialObject *obj = objList[i];
    obj->pos = Vector2Add(obj->pos, Vector2Scale(obj->vel, deltaTime));
  }
}

void interactGravity(CelestialObject *obj1, CelestialObject *obj2) {
  float deltaTime = GetFrameTime();

  Vector2 distanceVec = Vector2Subtract(obj2->pos, obj1->pos);
  float distSqr = Vector2LengthSqr(distanceVec);

  float minDistSqr = MIN_DISTANCE * MIN_DISTANCE;
  if (distSqr < minDistSqr) {
    distSqr = minDistSqr;
  }

  Vector2 direction = Vector2Normalize(distanceVec);
  float invSqrDist = 1.0 / distSqr;

  Vector2 gravityForce = Vector2Scale(
      direction, (GRAVITY_CONSTANT * obj1->mass * obj2->mass * invSqrDist));

  Vector2 accel = Vector2Scale(gravityForce, 1.0 / obj1->mass);
  Vector2 deltaVel = Vector2Scale(accel, deltaTime);
  obj1->vel = Vector2Add(obj1->vel, deltaVel);
}
