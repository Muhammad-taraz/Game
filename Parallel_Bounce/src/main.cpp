#include "raylib.h"
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdio> // For sprintf_s

using namespace std;

// Function prototypes
void StartGame(bool vsAI, const string& player1Name, const string& player2Name, Font customFont);
bool ShowMenu(string& player1Name, string& player2Name, Font customFont);
void CapturePlayerName(string& playerName, const string& prompt, Font customFont);
bool ShowGameOverScreen(bool playerWon, bool vsAI, const string& player1Name, const string& player2Name, Font customFont);
void LogGameResult(const string& mode, const string& winner);

// Button Struct
struct Button {
    Rectangle rect;          // Button rectangle (position and size)
    string text;        // Button text
    Texture2D image;         // Button image
    bool hovered;            // Is the button hovered?
};

// Function to log game results to a file
void LogGameResult(const string& mode, const string& winner) {
    if (winner.empty()) return; // Don't log if there's no winner

    ofstream outFile("game_results.txt", ios::app); // Append to the file
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
        cerr << "Error: Could not open file to log results.\n";
    }
}

// Function to Draw Button (with image and text)
void DrawButton(Button button, Color textColor, Font customFont) {
    DrawTexturePro(
        button.image,
        { 0.0f, 0.0f, (float)button.image.width, (float)button.image.height }, // Source rectangle
        button.rect,                                                        // Destination rectangle
        { 0.0f, 0.0f },                                                     // Origin
        0.0f,                                                               // Rotation
        WHITE                                                               // Tint color
    );

    // Draw button text with a shadow for a more beautiful look
    int fontSize = 30;
    Vector2 textSize = MeasureTextEx(customFont, button.text.c_str(), (float)fontSize, 1.0f);
    DrawTextEx(customFont, button.text.c_str(),
        { button.rect.x + (button.rect.width / 2 - textSize.x / 2) + 2,
          button.rect.y + (button.rect.height / 2 - textSize.y / 2) + 2 }, 
        fontSize, 1.0f, DARKGRAY);  // Text shadow

    DrawTextEx(customFont, button.text.c_str(),
        { button.rect.x + (button.rect.width / 2 - textSize.x / 2),
          button.rect.y + (button.rect.height / 2 - textSize.y / 2) },
        fontSize, 1.0f, textColor); // Main text
}

// Function to load the menu and handle transitions
bool ShowMenu(string& player1Name, string& player2Name, Font customFont) {
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

        DrawTextEx(customFont, "Welcome to the Pong Game!",
            { (float)(screenWidth / 2 - MeasureText("Welcome to the Pong Game!", 30) / 2),
            50 }, 30, 1.0f, RAYWHITE);

        for (int i = 0; i < 2; i++) {
            DrawButton(buttons[i], buttons[i].hovered ? GOLD : WHITE, customFont);
        }

        EndDrawing();
    }

    UnloadTexture(background);
    UnloadTexture(buttonImage);

    return false;
}

// Function to show the Game Over screen with "Main Menu" option
bool ShowGameOverScreen(bool playerWon, bool vsAI, const string& player1Name, const string& player2Name, Font customFont) {
    const int screenWidth = 792;
    const int screenHeight = 534;

    string winner = playerWon ? player1Name : (vsAI ? "AI" : player2Name);
    string mode = vsAI ? "AI vs Player" : "Multiplayer";

    // Log the result only once and with a valid winner
    LogGameResult(mode, winner);  // Log only if a valid winner exists

    Button mainMenuButton = { {screenWidth / 2 - 100, screenHeight / 2 + 40, 200, 60}, "Main Menu", {}, false };

    while (!WindowShouldClose()) {
        Vector2 mousePoint = GetMousePosition();
        mainMenuButton.hovered = CheckCollisionPointRec(mousePoint, mainMenuButton.rect);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mainMenuButton.hovered) {
            return true; // Return to main menu
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // Draw winner text with shadow and better font style
        DrawTextEx(customFont, (winner + " Wins!").c_str(),
            { (float)(screenWidth / 2 - MeasureText((winner + " Wins!").c_str(), 40) / 2) + 2,
              (float)(screenHeight / 2 - 30) + 2 }, 40, 1.0f, DARKGRAY); // Shadow
        DrawTextEx(customFont, (winner + " Wins!").c_str(),
            { (float)(screenWidth / 2 - MeasureText((winner + " Wins!").c_str(), 40) / 2),
              (float)(screenHeight / 2 - 30) }, 40, 1.0f, GREEN); // Main text

        DrawButton(mainMenuButton, mainMenuButton.hovered ? GOLD : WHITE, customFont);

        EndDrawing();
    }

    return false; // Default to exit
}

// Function to capture a single player's name
void CapturePlayerName(string& playerName, const string& prompt, Font customFont) {
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

        BeginDrawing();
        ClearBackground(BLACK);

        // Draw text with a better font style
        DrawTextEx(customFont, prompt.c_str(), 
            { (float)(screenWidth / 2 - MeasureText(prompt.c_str(), 20) / 2), 100 }, 20, 1.0f, WHITE);
        DrawTextEx(customFont, playerName.c_str(),
            { (float)(screenWidth / 2 - MeasureText(playerName.c_str(), 20) / 2), 130 }, 20, 1.0f, WHITE);
        DrawTextEx(customFont, "Press Enter to confirm name", 
            { (float)(screenWidth / 2 - MeasureText("Press Enter to confirm name", 20) / 2), 180 }, 20, 1.0f, WHITE);

        EndDrawing();

        if (IsKeyPressed(KEY_ENTER) && playerName.length() > 0) {
            break;
        }
    }
}

// Full StartGame function implementation
void StartGame(bool vsAI, const string& player1Name, const string& player2Name, Font customFont) {
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

        if (playerHearts <= 0 || opponentHearts <= 0) {
            break;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // Draw player 1 info on the left with red text
        DrawTextEx(customFont, (player1Name + "'s Hearts: " + to_string(playerHearts)).c_str(),
            { 20, 20 }, 20, 1.0f, RED);

        // Draw player 2 info on the right with green text
        DrawTextEx(customFont, (player2Name + "'s Hearts: " + to_string(opponentHearts)).c_str(),
            { (float)(screenWidth - 200), 20 }, 20, 1.0f, GREEN);

        // Draw game name in the center with a beautiful font style
        DrawTextEx(customFont, "Pong Game", 
            { (float)(screenWidth / 2 - MeasureText("Pong Game", 30) / 2), 20 }, 30, 1.0f, YELLOW);

        // Draw paddles and ball
        DrawRectangleRec(playerPaddle, RED);  // Left paddle red
        DrawRectangleRec(opponentPaddle, GREEN);  // Right paddle green
        DrawCircleV(ballPosition, 10, WHITE);

        EndDrawing();
    }

    ShowGameOverScreen(playerHearts > 0, vsAI, player1Name, player2Name, customFont);
}

int main() {
    const int screenWidth = 792;
    const int screenHeight = 534;

    InitWindow(screenWidth, screenHeight, "Pong Game");
    SetTargetFPS(60);

    // Load a custom font (Make sure the font file is in the assets/fonts/ directory)
    Font customFont = LoadFont("assets/fonts/Roboto-Regular.ttf"); // Ensure you have this font file

    while (!WindowShouldClose()) {
        string player1Name, player2Name;
        bool vsAI = ShowMenu(player1Name, player2Name, customFont);

        if (vsAI) {
            StartGame(true, player1Name.empty() ? "Player 1" : player1Name, "AI", customFont);
        }
        else {
            CapturePlayerName(player1Name, "Enter Player 1 Name: ", customFont);
            CapturePlayerName(player2Name, "Enter Player 2 Name: ", customFont);
            StartGame(false, player1Name, player2Name, customFont);
        }
    }

    UnloadFont(customFont);  // Don't forget to unload the font when done
    CloseWindow();
    return 0;
}
