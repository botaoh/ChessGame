/*
Author: Botao Huang
Class: ECE6122 (A)
Last Date Modified: 12/3/2024
Description:
Main program of the project, accept commands, do operations and renderings
*/

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <map>

// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
// User supporting files
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
// Specific chess class
#include "chessComponent.h"
#include "chessCommon.h"
// Chess Engine Class
#include "ECE_ChessEngine.h"

// Sets up the chess board
void setupChessGame(tModelMap& cTModelMap, std::map<std::string, std::string>& boardState);
// Process the command user input
void processCommand(const std::string& command, ECE_ChessEngine& engine);

// Validate whether a move command is ok
bool validatemove(const std::string& move, const std::map<std::string, std::string>& boardState, bool isPlayerTurn);
// Validate whether a move command is ok, but with no cout
bool validatemoveNoPrint(const std::string& move, const std::map<std::string, std::string>& boardState, bool isPlayerTurn);
// Execute the move
void executemove(const std::string& move, std::map<std::string, std::string>& boardState,
    tModelMap& cTModelMap, std::vector<chessComponent>& gchessComponents);

// Global variables
// Camera locations
float cTheta = 1.0f;
float cPhi = 0.0f;
float cRadius = 40.0f;
// Light locations
float lTheta = 0.0f;
float lPhi = 0.0f;
float lRadius = 15.0f;
// Light Power
float LightPower = 200.0f;
// My eaten piece index
int myEatenIndex = 0;
// Enemy's eaten piece index
int enemyEatenIndex = 0;

std::vector<chessComponent> gchessComponents;
// Setup the Chess board locations
tModelMap cTModelMap;
// Setup the Chess board state
std::map<std::string, std::string> boardState;
std::vector<std::string> moveHistory; // Stores all moves made in the game

// Define structs
struct ChessPiece {
    chessComponent component;
    tPosition positionData; // Contains position, rotation, etc.
};

// Main Entry Point
int main(void)
{
    // Initialize GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make macOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Game Of Chess 3D", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

    GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

    GLuint LightSwitchID = glGetUniformLocation(programID, "lightSwitch");

    GLuint LightPowerID = glGetUniformLocation(programID, "LightPower");

    // Load the OBJ files
    bool cBoard = loadAssImpLab3("objFiles/Stone_Chess_Board/12951_Stone_Chess_Board_v1_L3.obj", gchessComponents);
    bool cComps = loadAssImpLab3("objFiles/Chess/chess-mod.obj", gchessComponents);

    // Proceed iff OBJ loading is successful
    if (!cBoard || !cComps)
    {
        std::cout << "Program failed due to OBJ loading failure, please CHECK!" << std::endl;
        return -1;
    }

    setupChessGame(cTModelMap, boardState);

    for (auto cit = gchessComponents.begin(); cit != gchessComponents.end(); cit++)
    {
        // Setup VBO buffers
        cit->setupGLBuffers();
        // Setup Texture
        cit->setupTextureBuffers();
    }

    glUseProgram(programID);

    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    double lastTime = glfwGetTime();
    int nbFrames = 0;

    std::string command;
    bool firstTime = true;

    ECE_ChessEngine engine;
    engine.InitializeEngine();

    do {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Compute the VP matrix from keyboard and mouse input
        computeMatricesFromInputsFinalProject(cTheta, cPhi, cRadius);
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();

        float lightPosX = lRadius * sin(glm::radians(lTheta)) * cos(glm::radians(lPhi));
        float lightPosY = lRadius * sin(glm::radians(lTheta)) * sin(glm::radians(lPhi));
        float lightPosZ = lRadius * cos(glm::radians(lTheta));

        // Render loop
        for (tModelMap::const_iterator it = cTModelMap.begin(); it != cTModelMap.end(); ++it) {
            const std::string& id = it->id;
            const tPosition& cTPosition = it->position;

            auto componentIt = std::find_if(gchessComponents.begin(), gchessComponents.end(),
                [&id](const chessComponent& comp) { return comp.getComponentID() == id; });

            if (componentIt != gchessComponents.end()) {
                for (unsigned int pit = 0; pit < cTPosition.rCnt; ++pit) {
                    tPosition cTPositionMorph = cTPosition;
                    cTPositionMorph.tPos.x += pit * cTPosition.rDis * CHESS_BOX_SIZE;

                    glm::mat4 ModelMatrix = componentIt->genModelMatrix(cTPositionMorph);
                    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

                    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
                    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
                    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

                    glUniform3f(LightID, lightPosX, lightPosY, lightPosZ);

                    glUniform1f(LightPowerID, static_cast<float>(LightPower));
                    glUniform1i(LightSwitchID, static_cast<int>(true));

                    componentIt->setupTexture(TextureID);
                    componentIt->renderMesh();
                }
            }
        }

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        std::getline(std::cin, command);
        processCommand(command, engine);

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);

    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    glfwTerminate();
    return 0;
}

// Validate whether camera input is reasonable
bool validateCameraInput(float theta, float phi) {
    if (theta >= 10 && theta <= 80)
    {
        if (phi >= 0 && phi <= 360)
        {
            return true;
        }
    }
    return false;
}

// If camera input is reasonable, then update the angles
void adjustCamera(float theta, float phi, float r)
{
    cTheta = theta;
    cPhi = phi;
    cRadius = r;
}

// Validate whether light input is reasonable
bool validateLightInput(float theta, float phi) {
    if (theta >= 10 && theta <= 80)
    {
        if (phi >= 0 && phi <= 360)
        {
            return true;
        }
    }
    return false;
}

// If light input is reasonable, then update the lights
void adjustLight(float theta, float phi, float r)
{
    lTheta = theta;
    lPhi = phi;
    lRadius = r;
}

// Helper function to determine whether a sliding piece can move
bool checkPathClear(char fileSource, char rankSource, char fileDest, char rankDest,
    const std::map<std::string, std::string>& boardState) {
    int fileStep = (fileDest > fileSource) ? 1 : (fileDest < fileSource ? -1 : 0);
    int rankStep = (rankDest > rankSource) ? 1 : (rankDest < rankSource ? -1 : 0);

    char currentFile = fileSource + fileStep;
    char currentRank = rankSource + rankStep;

    while (currentFile != fileDest || currentRank != rankDest) {
        std::string position = std::string(1, currentFile) + std::string(1, currentRank);

        if (boardState.find(position) != boardState.end()) {
            return false; // Path is blocked
        }

        currentFile += fileStep;
        currentRank += rankStep;
    }

    return true; // Path is clear
}

// Helper function to determine whether a king is under attack
bool isKingUnderAttack(const std::string& kingPosition, const std::map<std::string, std::string>& boardState, bool isPlayerTurn) {
    for (const auto& piece : boardState) {
        std::string pieceID = piece.second;
        std::string source = piece.first;

        // Skip if it's the current player's piece
        if (isPlayerTurn &&
            (pieceID == "PEDONE13" || pieceID.find("3") != std::string::npos || pieceID == "REGINA2" || pieceID == "RE2"))
            continue;

        if (!isPlayerTurn &&
            (pieceID == "PEDONE12" || pieceID.find("02") != std::string::npos || pieceID == "REGINA01" || pieceID == "RE01"))
            continue;

        // Check if this piece can attack the king's position
        if (validatemoveNoPrint(source + kingPosition, boardState, !isPlayerTurn)) {
            return true; // King is under attack
        }
    }

    return false; // King is safe
}

// Helper function to determine whether a game is going to over
bool isCheckmate(const std::map<std::string, std::string>& boardState, bool isPlayerTurn) {
    // Find the king's position
    std::string kingID = isPlayerTurn ? "RE2" : "RE01";
    std::string kingPosition;

    for (const auto& piece : boardState) {
        if (piece.second == kingID) {
            kingPosition = piece.first;
            break;
        }
    }

    if (kingPosition.empty()) {
        std::cout << "Error: King not found on the board!\n";
        return false;
    }

    // Check if the king is under attack
    if (!isKingUnderAttack(kingPosition, boardState, isPlayerTurn)) {
        return false; // Not checkmated, as the king is not in check
    }

    // Try all possible moves for all pieces
    for (const auto& piece : boardState) {
        std::string pieceID = piece.second;

        // Ensure the piece belongs to the current player
        if (isPlayerTurn &&
            !(pieceID == "PEDONE13" || pieceID.find("3") != std::string::npos || pieceID == "REGINA2" || pieceID == "RE2"))
            continue;

        if (!isPlayerTurn &&
            !(pieceID == "PEDONE12" || pieceID.find("02") != std::string::npos || pieceID == "REGINA01" || pieceID == "RE01"))
            continue;

        std::string source = piece.first;

        // Iterate over all possible destinations on the board
        for (char file = 'a'; file <= 'h'; ++file) {
            for (char rank = '1'; rank <= '8'; ++rank) {
                std::string destination = std::string(1, file) + std::string(1, rank);

                // Skip if the destination is the same as the source
                if (source == destination) continue;

                // Copy the board state to simulate the move
                std::map<std::string, std::string> simulatedBoard = boardState;

                // Check if the move is valid and simulate it
                if (validatemoveNoPrint(source + destination, simulatedBoard, isPlayerTurn)) {
                    // Simulate the move
                    executemove(source + destination, simulatedBoard, cTModelMap, gchessComponents);

                    // Check if the king is still under attack
                    if (!isKingUnderAttack(kingPosition, simulatedBoard, isPlayerTurn)) {
                        return false; // Found a valid move to escape check, not checkmate
                    }
                }
            }
        }
    }

    return true; // No valid moves found to escape check, checkmate
}


// Validate whether a move is reasonable without cout
bool validatemoveNoPrint(const std::string& move, const std::map<std::string, std::string>& boardState, bool isPlayerTurn) {
    if (move.size() != 4) {
        return false;
    }

    std::string source = move.substr(0, 2);
    std::string destination = move.substr(2, 2);

    if (boardState.find(source) == boardState.end()) {
        return false;
    }

    std::string pieceID = boardState.at(source);

    // Check if player controls the piece
    if (isPlayerTurn && pieceID.find("PEDONE13") == std::string::npos &&
        pieceID.find("TORRE3") == std::string::npos &&
        pieceID.find("Object3") == std::string::npos &&
        pieceID.find("ALFIERE3") == std::string::npos &&
        pieceID.find("REGINA2") == std::string::npos &&
        pieceID.find("RE2") == std::string::npos) {
        return false;
    }

    // Validate destination bounds
    char fileSource = source[0], rankSource = source[1];
    char fileDest = destination[0], rankDest = destination[1];

    if (fileSource < 'a' || fileSource > 'h' || rankSource < '1' || rankSource > '8' ||
        fileDest < 'a' || fileDest > 'h' || rankDest < '1' || rankDest > '8') {
        return false;
    }

    // Allow capturing enemy pieces
    if (boardState.find(destination) != boardState.end()) {
        std::string targetID = boardState.at(destination);

        // Determine if the target piece is an enemy
        bool isEnemyPiece = (isPlayerTurn &&
            (targetID.find("TORRE02") != std::string::npos ||
                targetID.find("Object02") != std::string::npos ||
                targetID.find("ALFIERE02") != std::string::npos ||
                targetID.find("REGINA01") != std::string::npos ||
                targetID.find("RE01") != std::string::npos ||
                targetID == "PEDONE12")) ||
            (!isPlayerTurn &&
                (targetID.find("TORRE3") != std::string::npos ||
                    targetID.find("Object3") != std::string::npos ||
                    targetID.find("ALFIERE3") != std::string::npos ||
                    targetID.find("REGINA2") != std::string::npos ||
                    targetID.find("RE2") != std::string::npos ||
                    targetID == "PEDONE13"));

        if (!isEnemyPiece) {
            return false;
        }
    }

    // Validate move based on piece type
    char fileDiff = std::abs(fileDest - fileSource);
    char rankDiff = std::abs(rankDest - rankSource);

    if (pieceID.find("TORRE") != std::string::npos) { // Rook
        if (fileDiff != 0 && rankDiff != 0) {
            return false;
        }
        if (!checkPathClear(fileSource, rankSource, fileDest, rankDest, boardState)) {
            return false;
        }
    }
    else if (pieceID.find("ALFIERE") != std::string::npos) { // Bishop
        if (fileDiff != rankDiff) {
            return false;
        }
        if (!checkPathClear(fileSource, rankSource, fileDest, rankDest, boardState)) {
            return false;
        }
    }
    else if (pieceID.find("REGINA") != std::string::npos) { // Queen
        if (fileDiff != rankDiff && fileDiff != 0 && rankDiff != 0) {
            return false;
        }
        if (!checkPathClear(fileSource, rankSource, fileDest, rankDest, boardState)) {
            return false;
        }
    }
    else if (pieceID.find("Object") != std::string::npos) { // Knight
        if (!(fileDiff == 2 && rankDiff == 1) && !(fileDiff == 1 && rankDiff == 2)) {
            return false;
        }
    }
    else if (pieceID.find("RE") != std::string::npos) { // King
        if (fileDiff > 1 || rankDiff > 1) {
            return false;
        }
    }
    else if (pieceID == "PEDONE13" || pieceID == "PEDONE12") { // Pawn
        if (fileDiff > 1 || rankDiff == 0 || rankDiff > 2) {
            return false;
        }
        if (fileDiff == 1 && boardState.find(destination) == boardState.end()) {
            return false;
        }
        if (fileDiff == 0 && rankDiff == 2 && rankSource != '2' && rankSource != '7') {
            return false;
        }
    }

    return true; // Move is valid
}

// Check whether a move command is reasonable
bool validatemove(const std::string& move, const std::map<std::string, std::string>& boardState, bool isPlayerTurn) {
    if (move.size() != 4) {
        return false;
    }

    std::string source = move.substr(0, 2);
    std::string destination = move.substr(2, 2);

    if (boardState.find(source) == boardState.end()) {
        std::cout << "Invalid move: Source position does not exist or is empty.\n";
        return false;
    }

    std::string pieceID = boardState.at(source);

    // Check if player controls the piece
    if (isPlayerTurn && pieceID.find("PEDONE13") == std::string::npos &&
        pieceID.find("TORRE3") == std::string::npos &&
        pieceID.find("Object3") == std::string::npos &&
        pieceID.find("ALFIERE3") == std::string::npos &&
        pieceID.find("REGINA2") == std::string::npos &&
        pieceID.find("RE2") == std::string::npos) {
        std::cout << "Invalid move: Player cannot move this piece.\n";
        return false;
    }

    // Validate destination bounds
    char fileSource = source[0], rankSource = source[1];
    char fileDest = destination[0], rankDest = destination[1];

    if (fileSource < 'a' || fileSource > 'h' || rankSource < '1' || rankSource > '8' ||
        fileDest < 'a' || fileDest > 'h' || rankDest < '1' || rankDest > '8') {
        std::cout << "Invalid move: Destination is out of bounds.\n";
        return false;
    }

    // Allow capturing enemy pieces
    if (boardState.find(destination) != boardState.end()) {
        std::string targetID = boardState.at(destination);

        // Determine if the target piece is an enemy
        bool isEnemyPiece = (isPlayerTurn &&
            (targetID.find("TORRE02") != std::string::npos ||
                targetID.find("Object02") != std::string::npos ||
                targetID.find("ALFIERE02") != std::string::npos ||
                targetID.find("REGINA01") != std::string::npos ||
                targetID.find("RE01") != std::string::npos ||
                targetID == "PEDONE12")) ||
            (!isPlayerTurn &&
                (targetID.find("TORRE3") != std::string::npos ||
                    targetID.find("Object3") != std::string::npos ||
                    targetID.find("ALFIERE3") != std::string::npos ||
                    targetID.find("REGINA2") != std::string::npos ||
                    targetID.find("RE2") != std::string::npos ||
                    targetID == "PEDONE13"));

        if (!isEnemyPiece) {
            std::cout << "Invalid move: Destination is occupied by a friendly piece.\n";
            return false;
        }
    }

    // Validate move based on piece type
    char fileDiff = std::abs(fileDest - fileSource);
    char rankDiff = std::abs(rankDest - rankSource);

    if (pieceID.find("TORRE") != std::string::npos) { // Rook
        if (fileDiff != 0 && rankDiff != 0) {
            std::cout << "Invalid move: Rook can only move horizontally or vertically.\n";
            return false;
        }
        if (!checkPathClear(fileSource, rankSource, fileDest, rankDest, boardState)) {
            std::cout << "Invalid move: Path is blocked for the Rook.\n";
            return false;
        }
    }
    else if (pieceID.find("ALFIERE") != std::string::npos) { // Bishop
        if (fileDiff != rankDiff) {
            std::cout << "Invalid move: Bishop can only move diagonally.\n";
            return false;
        }
        if (!checkPathClear(fileSource, rankSource, fileDest, rankDest, boardState)) {
            std::cout << "Invalid move: Path is blocked for the Bishop.\n";
            return false;
        }
    }
    else if (pieceID.find("REGINA") != std::string::npos) { // Queen
        if (fileDiff != rankDiff && fileDiff != 0 && rankDiff != 0) {
            std::cout << "Invalid move: Queen must move like a rook or bishop.\n";
            return false;
        }
        if (!checkPathClear(fileSource, rankSource, fileDest, rankDest, boardState)) {
            std::cout << "Invalid move: Path is blocked for the Queen.\n";
            return false;
        }
    }
    else if (pieceID.find("Object") != std::string::npos) { // Knight
        if (!(fileDiff == 2 && rankDiff == 1) && !(fileDiff == 1 && rankDiff == 2)) {
            std::cout << "Invalid move: Knight must move in an L-shape.\n";
            return false;
        }
    }
    else if (pieceID.find("RE") != std::string::npos) { // King
        if (fileDiff > 1 || rankDiff > 1) {
            std::cout << "Invalid move: King can only move one square in any direction.\n";
            return false;
        }
    }
    else if (pieceID == "PEDONE13" || pieceID == "PEDONE12") { // Pawn
        if (fileDiff > 1 || rankDiff == 0 || rankDiff > 2) {
            std::cout << "Invalid move: Pawn can only move forward or capture diagonally.\n";
            return false;
        }
        if (fileDiff == 1 && boardState.find(destination) == boardState.end()) {
            std::cout << "Invalid move: Pawn can only capture diagonally.\n";
            return false;
        }
        if (fileDiff == 0 && rankDiff == 2 && rankSource != '2' && rankSource != '7') {
            std::cout << "Invalid move: Pawn can only move two squares forward from its starting rank.\n";
            return false;
        }
    }

    return true; // Move is valid
}

// Execute a move if it's reasonable
void executemove(const std::string& move, std::map<std::string, std::string>& boardState,
    tModelMap& cTModelMap, std::vector<chessComponent>& gchessComponents) {
    static int playerCapturedIndex = 0; // Tracks captured player pieces (a0, b0, c0, ...)
    static int enemyCapturedIndex = 0;  // Tracks captured enemy pieces (a9, b9, c9, ...)

    std::string source = move.substr(0, 2);
    std::string destination = move.substr(2, 2);
    char promotionPiece = (move.length() == 5) ? move[4] : '\0'; // Check for promotion character

    auto pieceIt = boardState.find(source);
    if (pieceIt == boardState.end()) {
        std::cerr << "Error: No piece found at source position " << source << "\n";
        return;
    }

    std::string pieceID = pieceIt->second;

    // Handle capturing an enemy piece
    if (boardState.find(destination) != boardState.end()) {
        std::string capturedPieceID = boardState[destination];
        boardState.erase(destination);

        // Determine if the captured piece is an enemy piece
        bool isEnemyPiece =
            capturedPieceID == "PEDONE12" ||
            capturedPieceID == "TORRE02" || capturedPieceID == "Object02" ||
            capturedPieceID == "ALFIERE02" || capturedPieceID == "REGINA01" ||
            capturedPieceID == "RE01";

        // If it's an enemy piece, relocate it
        if (isEnemyPiece) {
            // Find the captured piece in cTModelMap
            for (auto& entry : cTModelMap) {
                if (entry.id == capturedPieceID && entry.location == destination) {
                    char capturedFile = 'a' + (enemyCapturedIndex % 8); // Cycle through a-h
                    float capturedRank = 0.0f; // Rank 0 for enemy pieces
                    enemyCapturedIndex++;

                    std::string capturedPosition = std::string(1, capturedFile) + std::to_string(static_cast<int>(capturedRank));
                    entry.location = capturedPosition;

                    // Update the 3D position
                    entry.position.tPos.x = (capturedFile - 'a') * CHESS_BOX_SIZE - 3.5f * CHESS_BOX_SIZE;
                    entry.position.tPos.y = (capturedRank - 1) * CHESS_BOX_SIZE - 3.5f * CHESS_BOX_SIZE;

                    break;
                }
            }
        }
        else {
            // Relocate player's piece if captured
            char capturedFile = 'a' + (playerCapturedIndex % 8); // Cycle through a-h
            float capturedRank = 9.0f; // Rank 9 for player's pieces
            playerCapturedIndex++;

            // Find the player's captured piece in cTModelMap
            for (auto& entry : cTModelMap) {
                if (entry.id == capturedPieceID && entry.location == destination) {
                    std::string capturedPosition = std::string(1, capturedFile) + std::to_string(static_cast<int>(capturedRank));
                    entry.location = capturedPosition;

                    // Update the 3D position
                    entry.position.tPos.x = (capturedFile - 'a') * CHESS_BOX_SIZE - 3.5f * CHESS_BOX_SIZE;
                    entry.position.tPos.y = (capturedRank - 1) * CHESS_BOX_SIZE - 3.5f * CHESS_BOX_SIZE;

                    break;
                }
            }
        }

        // Debug: Piece captured
        std::cout << "Captured piece: " << capturedPieceID << " relocated to edge of the board.\n";
    }

    // Update board state
    if (promotionPiece != '\0') {
        // Handle promotion (update the piece ID)
        std::string promotedID;
        switch (promotionPiece) {
        case 'q': promotedID = "REGINA2"; break; // Queen
        case 'r': promotedID = "TORRE3"; break;  // Rook
        case 'b': promotedID = "ALFIERE3"; break; // Bishop
        case 'n': promotedID = "Object3"; break;  // Knight
        default: std::cerr << "Invalid promotion piece: " << promotionPiece << "\n"; return;
        }
        boardState[destination] = promotedID;
        std::cout << "Pawn promoted to " << promotedID << " at " << destination << "\n";
    }
    else {
        boardState[destination] = pieceID;
    }
    boardState.erase(source);


    // Update the piece's position in cTModelMap
    for (auto& entry : cTModelMap) {
        if (entry.location == source) {
            entry.location = destination;
            entry.position.tPos.x = (destination[0] - 'a') * CHESS_BOX_SIZE - 3.5f * CHESS_BOX_SIZE;
            entry.position.tPos.y = (destination[1] - '1') * CHESS_BOX_SIZE - 3.5f * CHESS_BOX_SIZE;
            break;
        }
    }

    // Update the position in gchessComponents
    for (auto& component : gchessComponents) {
        if (component.getComponentID() == pieceID) {
            tPosition newPos = {
                1, 0, 90.f, {1, 0, 0}, glm::vec3(CPSCALE),
                {(destination[0] - 'a') * CHESS_BOX_SIZE - 3.5f * CHESS_BOX_SIZE,
                 (destination[1] - '1') * CHESS_BOX_SIZE - 3.5f * CHESS_BOX_SIZE, PHEIGHT} };
            component.storePosition(newPos);
            break;
        }
    }
}

// Match the input string with the operations needed to be done
void processCommand(const std::string& command, ECE_ChessEngine& engine) {
    std::istringstream iss(command);
    std::string action;
    iss >> action;

    if (action == "move") {
        std::string move;
        iss >> move;

        // Save current board state in case we need to revert
        auto previousBoardState = boardState;
        auto previousCTModelMap = cTModelMap;
        auto previousMoveHistory = moveHistory;

        // Validate and execute the user's move
        if (validatemove(move, boardState, true)) {
            executemove(move, boardState, cTModelMap, gchessComponents);

            // Add the user's move to the move history
            moveHistory.push_back(move);

            // Construct the full move history string
            std::string moveHistoryStr = "";
            for (const auto& pastMove : moveHistory) {
                if (moveHistoryStr == "")
                    moveHistoryStr += pastMove;
                else
                    moveHistoryStr += (" " + pastMove);
            }

            // Send the move to the chess engine
            engine.sendMove(moveHistoryStr);

            // Get the engine's response
            std::string engineMove;
            if (engine.getResponseMove(engineMove)) {

                // Validate the engine's move
                if (validatemove(engineMove, boardState, false)) {
                    moveHistory.push_back(engineMove);
                    executemove(engineMove, boardState, cTModelMap, gchessComponents);
                }
                else {
                    // Revert the board state and cTModelMap
                    std::cerr << "Invalid command or move!!\n";
                    boardState = previousBoardState;
                    cTModelMap = previousCTModelMap;
                    moveHistory = previousMoveHistory;
                }
            }
            else {
                // Revert the board state and cTModelMap
                std::cerr << "Invalid command or move!!\n";
                boardState = previousBoardState;
                cTModelMap = previousCTModelMap;
                moveHistory = previousMoveHistory;
            }
        }
        else {
            std::cout << "Invalid command or move!!" << std::endl;
        }
    }
    else if (action == "camera") {
        float theta, phi, r;
        iss >> theta >> phi >> r;
        if (r > 0.0f && validateCameraInput(theta, phi)) {
            adjustCamera(theta, phi, r);
        }
        else {
            std::cout << "Invalid command or move!!" << std::endl;
        }
    }
    else if (action == "light") {
        float theta, phi, r;
        iss >> theta >> phi >> r;
        if (r > 0.0f && validateLightInput(theta, phi)) {
            adjustLight(theta, phi, r);
        }
        else {
            std::cout << "Invalid command or move!!" << std::endl;
        }
    }
    else if (action == "power") {
        float power;
        iss >> power;
        if (power > 0.0f) {
            LightPower = power;
        }
        else {
            std::cout << "Invalid command or move!!" << std::endl;
        }
    }
    else if (action == "quit") {
        std::cout << "Thanks for playing!" << std::endl;
        exit(0);
    }
    else {
        std::cout << "Invalid command or move!!" << std::endl;
    }
}

// Initialize chess pieces and chessboard
void setupChessGame(tModelMap& cTModelMap, std::map<std::string, std::string>& boardState) {
    // Add chessboard to the model map
    cTModelMap.push_back({ "12951_Stone_Chess_Board",
                          {1, 0, 0.f, {1, 0, 0}, glm::vec3(CBSCALE), {0.f, 0.f, PHEIGHT}} });

    // Initialize first player's back rank
    std::vector<std::pair<std::string, std::string>> backRankFirstPlayer = {
        {"a1", "TORRE3"}, {"b1", "Object3"}, {"c1", "ALFIERE3"}, {"d1", "REGINA2"},
        {"e1", "RE2"}, {"f1", "ALFIERE3"}, {"g1", "Object3"}, {"h1", "TORRE3"}
    };

    for (const auto& pair : backRankFirstPlayer) {
        const std::string& position = pair.first;
        const std::string& id = pair.second;

        boardState[position] = id;
        tPosition location = { 1, 0, 90.f, {1, 0, 0}, glm::vec3(CPSCALE),
                             {-3.5f * CHESS_BOX_SIZE + (position[0] - 'a') * CHESS_BOX_SIZE,
                              -3.5f * CHESS_BOX_SIZE,
                              PHEIGHT} };
        
        struct ModelData newEle;
        newEle.id = id;
        newEle.location = position;
        newEle.position = location;

        cTModelMap.push_back(newEle);
    }

    // Initialize first player's pawns
    for (char file = 'a'; file <= 'h'; ++file) {
        std::string position = std::string(1, file) + "2";
        std::string pawnID = "PEDONE13";
        boardState[position] = pawnID;

        tPosition location =   {1, 0, 90.f, {1, 0, 0}, glm::vec3(CPSCALE),
                               {-3.5f * CHESS_BOX_SIZE + (file - 'a') * CHESS_BOX_SIZE,
                                -2.5f * CHESS_BOX_SIZE,
                                PHEIGHT}};

        struct ModelData newEle;
        newEle.id = pawnID;
        newEle.location = position;
        newEle.position = location;

        cTModelMap.push_back(newEle);
    }

    // Initialize second player's back rank
    std::vector<std::pair<std::string, std::string>> backRankSecondPlayer = {
        {"a8", "TORRE02"}, {"b8", "Object02"}, {"c8", "ALFIERE02"}, {"d8", "REGINA01"},
        {"e8", "RE01"}, {"f8", "ALFIERE02"}, {"g8", "Object02"}, {"h8", "TORRE02"}
    };

    for (const auto& pair : backRankSecondPlayer) {
        const std::string& position = pair.first;
        const std::string& id = pair.second;

        boardState[position] = id;
        tPosition location = { 1, 0, 90.f, {1, 0, 0}, glm::vec3(CPSCALE),
                             {-3.5f * CHESS_BOX_SIZE + (position[0] - 'a') * CHESS_BOX_SIZE,
                              3.5f * CHESS_BOX_SIZE,
                             PHEIGHT} };

        struct ModelData newEle;
        newEle.id = id;
        newEle.location = position;
        newEle.position = location;

        cTModelMap.push_back(newEle);
    }

    // Initialize second player's pawns
    for (char file = 'a'; file <= 'h'; ++file) {
        std::string position = std::string(1, file) + "7";
        std::string pawnID = "PEDONE12";
        boardState[position] = pawnID;

        tPosition location = { 1, 0, 90.f, {1, 0, 0}, glm::vec3(CPSCALE),
                               {-3.5f * CHESS_BOX_SIZE + (file - 'a') * CHESS_BOX_SIZE,
                                2.5f * CHESS_BOX_SIZE,
                                PHEIGHT} };

        struct ModelData newEle;
        newEle.id = pawnID;
        newEle.location = position;
        newEle.position = location;

        cTModelMap.push_back(newEle);
    }

    // Debug output for initialized pieces
    std::cout << "Initialized pieces in tModelMap:\n";
    for (const auto& pair : cTModelMap) {
        const std::string& id = pair.id;
        const tPosition& data = pair.position;

        std::cout << "Piece ID: " << id
            << " | Position: (" << data.tPos.x << ", " << data.tPos.y << ", " << data.tPos.z << ")\n";
    }
}
