#include <raylib.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>

using namespace std;

const int screenWidth = 1920;
const int screenHeight = 1080;
const int fishCount = 100;
const float speed = 3.0f;
const float fishSize = 32.0f;

float sepWeight = 2.0f;
float alignWeight = 0.1f;
float cohesionWeight = 0.03f;

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

        if (IsKeyPressed(KEY_Q)) sepWeight -= 0.2f;
        if (IsKeyPressed(KEY_W)) sepWeight += 0.2f;
        if (IsKeyPressed(KEY_A)) alignWeight -= 0.02f;
        if (IsKeyPressed(KEY_S)) alignWeight += 0.02f;
        if (IsKeyPressed(KEY_Z)) cohesionWeight -= 0.02f;
        if (IsKeyPressed(KEY_X)) cohesionWeight += 0.02f;

        for (int i = 0; i < fish.size(); i++){
            float accelerationChanger;
            bool currentlyStraight = true;
            if (fish[i].position.x < 100){
                accelerationChanger = (100 - fish[i].position.x) / 150;
                fish[i].acceleration.x += accelerationChanger;
                currentlyStraight = false;
            }
            if (fish[i].position.x > screenWidth - 100){
                accelerationChanger = (screenWidth - 100 - fish[i].position.x) / 150;
                fish[i].acceleration.x += accelerationChanger;
                currentlyStraight = false;
            }
            if (fish[i].position.y < 100){
                accelerationChanger = (100 - fish[i].position.y) / 150;
                fish[i].acceleration.y += accelerationChanger;
                currentlyStraight = false;
            }
            if (fish[i].position.y > screenHeight - 100){
                accelerationChanger = (screenHeight - 100 - fish[i].position.y) / 150;
                fish[i].acceleration.y += accelerationChanger;
                currentlyStraight = false;
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

                if (distance < 45 && i != j){
                    float urgencyX = (45 - distance) / 45;
                    float urgencyY = (45 - distance) / 45;
                    fish[i].acceleration.x -= urgencyX * (directionX - perpX) * sepWeight;
                    fish[i].acceleration.y -= urgencyY * (directionY - perpY) * sepWeight;
                    currentlyStraight = false;
                }
            }

            float averageAngle = 0;
            int angleAmount = 0;
            for (int j = 0; j < fish.size(); j++){
                float differenceX = fish[j].position.x - fish[i].position.x;
                float differenceY = fish[j].position.y - fish[i].position.y;
                float distance = sqrt(differenceX * differenceX + differenceY * differenceY);

                if (distance < 120 && i != j){
                    float currentAngle = atan2f(fish[j].velocity.y, fish[j].velocity.x);
                    averageAngle += currentAngle;
                    angleAmount++;
                    currentlyStraight = false;
                }
            }

            if (angleAmount > 0){
                averageAngle /= angleAmount;

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

                if (distance < 120 && i != j){
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

            if (fish[i].velocity.x > 3){
                fish[i].velocity.x = 3;
            }

            if (fish[i].velocity.x < -3){
                fish[i].velocity.x = -3;
            }

            if (fish[i].velocity.y > 3){
                fish[i].velocity.y = 3;
            }

            if (fish[i].velocity.y < -3){
                fish[i].velocity.y = -3;
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