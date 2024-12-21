#include "raylib.h"
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdio> // For sprintf_s

// Function prototypes
void StartGame(bool vsAI, const std::string& player1Name, const std::string& player2Name);
bool ShowMenu(std::string& player1Name, std::string& player2Name);
void CapturePlayerName(std::string& playerName, const std::string& prompt);
void ShowGameOverScreen(bool playerWon, bool vsAI, const std::string& player1Name, const std::string& player2Name);
void LogGameResult(const std::string& mode, const std::string& winner);

// Button Struct
struct Button {
    Rectangle rect;          // Button rectangle (position and size)
    std::string text;        // Button text
    Texture2D image;         // Button image
    bool hovered;            // Is the button hovered?
};

// Function to log game results to a file
void LogGameResult(const std::string& mode, const std::string& winner) {
    std::ofstream outFile("game_results.txt", std::ios::app); // Append to the file
    if (outFile.is_open()) {
        // Get the current time
        time_t now = time(nullptr);
        struct tm timeInfo;
        localtime_s(&timeInfo, &now);  // Safe time handling

        char timeBuffer[100];
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &timeInfo);

        // Write to the file
        outFile << "Mode: " << mode << " | Date & Time: " << timeBuffer << " | Winner: " << winner << "\n";
        outFile.close();
    }
    else {
        std::cerr << "Error: Could not open file to log results.\n";
    }
}

// Function to Draw Button (with image and text)
void DrawButton(Button button, Color textColor) {
    DrawTexturePro(
        button.image,
        { 0.0f, 0.0f, (float)button.image.width, (float)button.image.height }, // Source rectangle
        button.rect,                                                        // Destination rectangle
        { 0.0f, 0.0f },                                                     // Origin
        0.0f,                                                               // Rotation
        WHITE                                                               // Tint color
    );

    // Draw button text
    int fontSize = 20;
    Vector2 textSize = MeasureTextEx(GetFontDefault(), button.text.c_str(), (float)fontSize, 1.0f);
    DrawText(button.text.c_str(),
        static_cast<int>(button.rect.x + (button.rect.width / 2 - textSize.x / 2)),
        static_cast<int>(button.rect.y + (button.rect.height / 2 - textSize.y / 2)),
        fontSize, textColor);
}

// Function to load the menu and handle transitions
bool ShowMenu(std::string& player1Name, std::string& player2Name) {
    const int screenWidth = 792;
    const int screenHeight = 534;

    // Load resources
    Texture2D background = LoadTexture("background.png");
    Texture2D buttonImage = LoadTexture("button_image.png");

    if (background.id == 0 || buttonImage.id == 0) {
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(RED);
            DrawText(
                background.id == 0 ? "ERROR: 'background.png' not found!"
                : "ERROR: 'button_image.png' not found!",
                100, screenHeight / 2 - 20, 20, WHITE);
            DrawText("Place the file in the same directory as the .exe!",
                100, screenHeight / 2 + 20, 20, WHITE);
            EndDrawing();
        }
        CloseWindow();
        return false; // Exit if resources are missing
    }

    // Buttons
    Button buttons[2] = {
        {{(float)(screenWidth / 2 - 100), 160.0f, 200.0f, 60.0f}, "Play with AI", buttonImage, false},
        {{(float)(screenWidth / 2 - 100), 240.0f, 200.0f, 60.0f}, "Multiplayer", buttonImage, false}
    };

    bool exitGame = false;

    while (!WindowShouldClose() && !exitGame) {
        Vector2 mousePoint = GetMousePosition();
        for (int i = 0; i < 2; i++) {
            buttons[i].hovered = CheckCollisionPointRec(mousePoint, buttons[i].rect);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (buttons[0].hovered) {
                return true;  // Play with AI
            }
            if (buttons[1].hovered) {
                player1Name = "";
                player2Name = "";
                return false; // Multiplayer
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(
            background,
            { 0.0f, 0.0f, (float)background.width, (float)background.height },
            { 0.0f, 0.0f, (float)screenWidth, (float)screenHeight },
            { 0.0f, 0.0f },
            0.0f,
            WHITE
        );

        DrawText("Welcome to the Pong Game!",
            screenWidth / 2 - MeasureText("Welcome to the Pong Game!", 30) / 2,
            50, 30, RAYWHITE);

        for (int i = 0; i < 2; i++) {
            DrawButton(buttons[i], buttons[i].hovered ? GOLD : WHITE);
        }

        EndDrawing();
    }

    UnloadTexture(background);
    UnloadTexture(buttonImage);

    return false;
}

// Function to show the Game Over screen
void ShowGameOverScreen(bool playerWon, bool vsAI, const std::string& player1Name, const std::string& player2Name) {
    const int screenWidth = 792;
    const int screenHeight = 534;

    std::string winner = playerWon ? player1Name : (vsAI ? "AI" : player2Name);
    std::string mode = vsAI ? "AI vs Player" : "Multiplayer";

    LogGameResult(mode, winner);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        DrawText((winner + " Wins!").c_str(),
            screenWidth / 2 - MeasureText((winner + " Wins!").c_str(), 30) / 2,
            screenHeight / 2 - 30, 30, GREEN);

        DrawText("Press 'Esc' to Exit",
            screenWidth / 2 - MeasureText("Press 'Esc' to Exit", 20) / 2,
            screenHeight / 2 + 60, 20, WHITE);

        EndDrawing();

        if (IsKeyPressed(KEY_ESCAPE)) {
            break;
        }
    }
}

// Function to capture a single player's name
void CapturePlayerName(std::string& playerName, const std::string& prompt) {
    const int screenWidth = 792;
    const int screenHeight = 534;

    // Input buffer for player name
    playerName = "";

    while (!WindowShouldClose()) {
        // Handle text input
        if (IsKeyPressed(KEY_BACKSPACE) && playerName.length() > 0) {
            playerName.pop_back();  // Remove last character on backspace
        }

        // Allow characters to be typed (A-Z, a-z, space, etc.)
        for (int i = 32; i < 123; ++i) {
            if (IsKeyPressed(i)) {
                if (playerName.length() < 49) {  // Limit name to 50 characters
                    playerName += static_cast<char>(i);
                }
            }
        }

        // Start Drawing
        BeginDrawing();
        ClearBackground(BLACK);

        // Draw prompt
        DrawText(prompt.c_str(), screenWidth / 2 - MeasureText(prompt.c_str(), 20) / 2, 100, 20, WHITE);

        // Draw current player name input
        DrawText(playerName.c_str(), screenWidth / 2 - MeasureText(playerName.c_str(), 20) / 2, 130, 20, WHITE);

        // Prompt to press Enter when finished
        DrawText("Press Enter to confirm name", screenWidth / 2 - MeasureText("Press Enter to confirm name", 20) / 2, 180, 20, WHITE);

        EndDrawing();

        // Break if Enter is pressed
        if (IsKeyPressed(KEY_ENTER) && playerName.length() > 0) {
            break;
        }
    }
}

// Full StartGame function implementation
void StartGame(bool vsAI, const std::string& player1Name, const std::string& player2Name) {
    const int screenWidth = 792;
    const int screenHeight = 534;
    const float paddleSpeed = 400.0f;
    const float ballSpeed = 350.0f;
    const int maxHearts = 5;

    Rectangle playerPaddle = { screenWidth - 70, screenHeight / 2 - 60, 20, 120 };
    Rectangle opponentPaddle = { 50, screenHeight / 2 - 60, 20, 120 };
    Vector2 ballPosition = { screenWidth / 2, screenHeight / 2 };
    Vector2 ballSpeedVector = { -ballSpeed, ballSpeed };

    int playerScore = 0;
    int opponentScore = 0;
    int playerHearts = maxHearts;
    int opponentHearts = maxHearts;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_W) && playerPaddle.y > 0) playerPaddle.y -= paddleSpeed * GetFrameTime();
        if (IsKeyDown(KEY_S) && playerPaddle.y + playerPaddle.height < screenHeight) playerPaddle.y += paddleSpeed * GetFrameTime();

        if (!vsAI) {
            if (IsKeyDown(KEY_UP) && opponentPaddle.y > 0) opponentPaddle.y -= paddleSpeed * GetFrameTime();
            if (IsKeyDown(KEY_DOWN) && opponentPaddle.y + opponentPaddle.height < screenHeight) opponentPaddle.y += paddleSpeed * GetFrameTime();
        }
        else {
            if (ballPosition.y < opponentPaddle.y && opponentPaddle.y > 0) opponentPaddle.y -= paddleSpeed * GetFrameTime();
            if (ballPosition.y > opponentPaddle.y + opponentPaddle.height && opponentPaddle.y + opponentPaddle.height < screenHeight) opponentPaddle.y += paddleSpeed * GetFrameTime();
        }

        ballPosition.x += ballSpeedVector.x * GetFrameTime();
        ballPosition.y += ballSpeedVector.y * GetFrameTime();

        if (ballPosition.y <= 0 || ballPosition.y >= screenHeight) ballSpeedVector.y *= -1;
        if (CheckCollisionCircleRec(ballPosition, 10, playerPaddle)) ballSpeedVector.x *= -1;
        if (CheckCollisionCircleRec(ballPosition, 10, opponentPaddle)) ballSpeedVector.x *= -1;

        if (ballPosition.x <= 0) {
            playerScore++;
            opponentHearts--;
            ballPosition = { screenWidth / 2, screenHeight / 2 };
            ballSpeedVector.x *= -1;
        }

        if (ballPosition.x >= screenWidth) {
            opponentScore++;
            playerHearts--;
            ballPosition = { screenWidth / 2, screenHeight / 2 };
            ballSpeedVector.x *= -1;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("PONG GAME", screenWidth / 2 - MeasureText("PONG GAME", 20) / 2, 10, 20, RAYWHITE);

        // Display hearts only (swap positions)
        char playerHeartsText[50];
        char opponentHeartsText[50];
        sprintf_s(playerHeartsText, sizeof(playerHeartsText), "Hearts: %d", playerHearts);
        sprintf_s(opponentHeartsText, sizeof(opponentHeartsText), "Hearts: %d", opponentHearts);

        // Player hearts on the right, opponent hearts on the left
        DrawText(playerHeartsText, screenWidth - 200, 10, 20, GREEN); // Player hearts on the right
        DrawText(opponentHeartsText, 10, 10, 20, RED); // Opponent hearts on the left

        DrawRectangleRec(playerPaddle, GREEN);
        DrawRectangleRec(opponentPaddle, RED);
        DrawCircleV(ballPosition, 10, RAYWHITE);

        EndDrawing();

        if (playerHearts <= 0 || opponentHearts <= 0) {
            ShowGameOverScreen(playerHearts > 0, vsAI, player1Name, player2Name);
            break;
        }
    }
}

// Main function
int main() {
    const int screenWidth = 792;
    const int screenHeight = 534;

    InitWindow(screenWidth, screenHeight, "Pong Game");
    SetTargetFPS(60);

    std::string player1Name, player2Name;
    bool vsAI = ShowMenu(player1Name, player2Name);

    if (vsAI) {
        StartGame(true, player1Name.empty() ? "Player 1" : player1Name, "AI");
    }
    else {
        CapturePlayerName(player1Name, "Enter Player 1 Name: ");
        CapturePlayerName(player2Name, "Enter Player 2 Name: ");
        StartGame(false, player1Name, player2Name);
    }

    CloseWindow();
    return 0;
}
