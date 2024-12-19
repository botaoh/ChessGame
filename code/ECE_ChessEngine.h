/*
Author: Botao Huang
Class: ECE6122 (A)
Last Date Modified: 12/3/2024
Description:
Header files for ECE chess engine
Interact with Komodo engine, and get response from it
*/

#ifndef ECE_CHESSENGINE_H
#define ECE_CHESSENGINE_H

#include <windows.h>
#include <string>
#include <iostream>
#include <stdexcept>

class ECE_ChessEngine {
private:
    HANDLE hInputWrite, hInputRead;
    HANDLE hOutputWrite, hOutputRead;
    PROCESS_INFORMATION engineProcess;

public:
    ECE_ChessEngine();
    ~ECE_ChessEngine();

    // Basic functions
    // Initialize the communication with engine
    void InitializeEngine();
    // Send move to the engine
    bool sendMove(const std::string& strMove);
    // Get response from the engine
    bool getResponseMove(std::string& strMove);

    // Helper functions
    // Read response with buffer to ensure data consistancy
    std::string readResponseWithBuffer(HANDLE hOutputRead, std::string& residualBuffer);
    // Response data cleaning
    std::string cleanEngineOutput(const std::string& output);
    // Find the best move recommended by the engine
    std::string extractBestMove(const std::string& response);

private:
    // Send command to the engine
    void sendCommand(const std::string& command);
    // Receive response from the engine
    std::string readResponse();
};

#endif