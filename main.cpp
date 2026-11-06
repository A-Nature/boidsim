#include <raylib.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>

using namespace std;

const int screenWidth = 1920;
const int screenHeight = 1080;
const int fishCount = 100;
const float speed = 2.7f;
const float fishSize = 32.0f;

float sepWeight = 0.80f;
float alignWeight = 0.15f;
float cohesionWeight = 0.06f;

class Fish{
    public:
        Vector2 position;
        Vector2 velocity;
        Vector2 acceleration;
        float angle;

        Fish(){
            position = {(float)(rand() % screenWidth), (float)(rand() % screenHeight)};
            angle = (rand() % 360) * DEG2RAD;
            velocity = {cosf(angle) * speed, sinf(angle) * speed};
            acceleration = {0, 0};
        }
};

int main(){
    srand(time(NULL));

    InitWindow(screenWidth, screenHeight, "Boid Sim");
    ToggleFullscreen();
    SetTargetFPS(60);

    Texture2D fishTexture = LoadTexture("fish.png");

    vector<Fish> fish;
    for (int i = 0; i < fishCount; i++){
        fish.push_back(Fish());
    }

    while (!WindowShouldClose()){

        if (IsKeyPressed(KEY_Q)) sepWeight -= 0.05f;
        if (IsKeyPressed(KEY_W)) sepWeight += 0.05f;
        if (IsKeyPressed(KEY_A)) alignWeight -= 0.01f;
        if (IsKeyPressed(KEY_S)) alignWeight += 0.01f;
        if (IsKeyPressed(KEY_Z)) cohesionWeight -= 0.01f;
        if (IsKeyPressed(KEY_X)) cohesionWeight += 0.01f;

        for (int i = 0; i < fish.size(); i++){
            float accelerationChanger;
            float wallX = 0;
            float wallY = 0;
            bool currentlyStraight = true;
            if (fish[i].position.x < 100){
                accelerationChanger = (100 - fish[i].position.x) / 100;
                wallX += accelerationChanger;
                currentlyStraight = false;
            }
            if (fish[i].position.x > screenWidth - 100){
                accelerationChanger = (screenWidth - 100 - fish[i].position.x) / 100;
                wallX += accelerationChanger;
                currentlyStraight = false;
            }
            if (fish[i].position.y < 100){
                accelerationChanger = (100 - fish[i].position.y) / 100;
                wallY += accelerationChanger;
                currentlyStraight = false;
            }
            if (fish[i].position.y > screenHeight - 100){
                accelerationChanger = (screenHeight - 100 - fish[i].position.y) / 100;
                wallY += accelerationChanger;
                currentlyStraight = false;
            }

            float wallStrength = sqrt(wallX * wallX + wallY * wallY);
            float fishSpeed = sqrt(fish[i].velocity.x * fish[i].velocity.x + fish[i].velocity.y * fish[i].velocity.y);
            if (wallStrength > 0 && fishSpeed > 0.01f){
                fish[i].acceleration.x += (wallX > 0 ? 1 : -1) * fmaxf(0, 2 * fabsf(wallX) - 1);
                fish[i].acceleration.y += (wallY > 0 ? 1 : -1) * fmaxf(0, 2 * fabsf(wallY) - 1);

                float intoWall = -(fish[i].velocity.x * wallX + fish[i].velocity.y * wallY) / (fishSpeed * wallStrength);
                if (intoWall > 0){
                    float turn = wallStrength * 0.6f * intoWall;
                    if (fish[i].velocity.x * wallY - fish[i].velocity.y * wallX < 0){
                        turn = -turn;
                    }
                    float oldVelocityX = fish[i].velocity.x;
                    fish[i].velocity.x = oldVelocityX * cosf(turn) - fish[i].velocity.y * sinf(turn);
                    fish[i].velocity.y = oldVelocityX * sinf(turn) + fish[i].velocity.y * cosf(turn);
                }
            }

            for (int j = 0; j < fish.size(); j++){
                float differenceX = fish[j].position.x - fish[i].position.x;
                float differenceY = fish[j].position.y - fish[i].position.y;
                float distance = sqrt(differenceX * differenceX + differenceY * differenceY);
                if (distance < 0.01f) distance = 0.01f;

                float directionX = differenceX / distance;
                float directionY = differenceY / distance;

                float perpX = -directionY;
                float perpY = directionX;

                if (j % 2 == 0){
                    perpX = -perpX;
                    perpY = -perpY;
                }

                if (distance < 80 && i != j){
                    float urgencyX = (80 - distance) / 80;
                    float urgencyY = (80 - distance) / 80;
                    fish[i].acceleration.x -= urgencyX * directionX * sepWeight;
                    fish[i].acceleration.y -= urgencyY * directionY * sepWeight;
                    currentlyStraight = false;
                }
            }

            float alignSumX = 0;
            float alignSumY = 0;
            int angleAmount = 0;
            for (int j = 0; j < fish.size(); j++){
                float differenceX = fish[j].position.x - fish[i].position.x;
                float differenceY = fish[j].position.y - fish[i].position.y;
                float distance = sqrt(differenceX * differenceX + differenceY * differenceY);

                if (distance < 175 && i != j){
                    alignSumX += fish[j].velocity.x;
                    alignSumY += fish[j].velocity.y;
                    angleAmount++;
                    currentlyStraight = false;
                }
            }

            if (angleAmount > 0){
                float averageAngle = atan2f(alignSumY, alignSumX);

                fish[i].acceleration.x += cosf(averageAngle) * alignWeight;
                fish[i].acceleration.y += sinf(averageAngle) * alignWeight;
            }

            float averageX = 0;
            float averageY = 0;
            int positionAmount = 0;
            for (int j = 0; j < fish.size(); j++){
                float differenceX = fish[j].position.x - fish[i].position.x;
                float differenceY = fish[j].position.y - fish[i].position.y;
                float distance = sqrt(differenceX * differenceX + differenceY * differenceY);

                if (distance < 175 && i != j){
                    averageX += fish[j].position.x;
                    averageY += fish[j].position.y;
                    positionAmount++;
                    currentlyStraight = false;
                }
            }

            if (positionAmount > 0){
                averageX /= positionAmount;
                averageY /= positionAmount;

                float differenceToAverageX = averageX - fish[i].position.x;
                float differenceToAverageY = averageY - fish[i].position.y;

                fish[i].acceleration.x += differenceToAverageX * cohesionWeight * 0.01f;
                fish[i].acceleration.y += differenceToAverageY * cohesionWeight * 0.01f;
            }

            if (currentlyStraight){
                fish[i].velocity.x *= 1.05;
                fish[i].velocity.y *= 1.05;
            }

            fish[i].velocity.x += fish[i].acceleration.x;
            fish[i].velocity.y += fish[i].acceleration.y;

            float currentSpeed = sqrt(fish[i].velocity.x * fish[i].velocity.x + fish[i].velocity.y * fish[i].velocity.y);
            if (currentSpeed > 2.7f){
                fish[i].velocity.x = (fish[i].velocity.x / currentSpeed) * 2.7f;
                fish[i].velocity.y = (fish[i].velocity.y / currentSpeed) * 2.7f;
            }

            fish[i].position.x += fish[i].velocity.x;
            fish[i].position.y += fish[i].velocity.y;

            fish[i].acceleration.x = 0;
            fish[i].acceleration.y = 0;
        }

        BeginDrawing();
        ClearBackground(BLUE);

        for (int i = 0; i < fish.size(); i++){
            float angle = atan2f(fish[i].velocity.y, fish[i].velocity.x) * RAD2DEG;

            Rectangle source = {0, 0, (float)fishTexture.width, (float)fishTexture.height};
            Rectangle dest = {fish[i].position.x, fish[i].position.y, fishSize, fishSize};
            Vector2 origin = {fishSize / 2, fishSize / 2};

            DrawTexturePro(fishTexture, source, dest, origin, angle, WHITE);
        }

        const char* text = TextFormat("SEP: %.2f  ALIGN: %.2f  COH: %.2f", sepWeight, alignWeight, cohesionWeight);
        int textWidth = MeasureText(text, 20);
        DrawText(text, screenWidth - textWidth - 20, 20, 20, WHITE);

        EndDrawing();
    }

    UnloadTexture(fishTexture);
    CloseWindow();
    return 0;
}