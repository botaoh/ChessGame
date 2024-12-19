/*
Author: Botao Huang
Class: ECE6122 (A)
Last Date Modified: 12/3/2024
Description:
Code for communication with Komodo chess engine
*/

#include <windows.h>
#include <string>
#include <iostream>
#include <stdexcept>
#include "ECE_ChessEngine.h"

// Constructor function for Chess Engine
ECE_ChessEngine::ECE_ChessEngine() : hInputWrite(NULL), hInputRead(NULL), hOutputWrite(NULL), hOutputRead(NULL) {
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    CreatePipe(&hOutputRead, &hOutputWrite, &sa, 0);
    CreatePipe(&hInputRead, &hInputWrite, &sa, 0);
}

// Destructor function for Chess Engine
ECE_ChessEngine::~ECE_ChessEngine() {
    CloseHandle(hInputWrite);
    CloseHandle(hInputRead);
    CloseHandle(hOutputWrite);
    CloseHandle(hOutputRead);
    TerminateProcess(engineProcess.hProcess, 0);
    CloseHandle(engineProcess.hProcess);
    CloseHandle(engineProcess.hThread);
}

// Initialize Chess Engine
void ECE_ChessEngine::InitializeEngine() {
    // Create pipes for input and output
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    CreatePipe(&hOutputRead, &hOutputWrite, &sa, 0);
    CreatePipe(&hInputRead, &hInputWrite, &sa, 0);

    // Start the Komodo engine
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hInputRead;
    si.hStdOutput = hOutputWrite;
    si.hStdError = hOutputWrite;

    // Path to Komodo executable
    std::string enginePath = "komodo.exe";
    if (!CreateProcess(NULL, const_cast<char*>(enginePath.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        std::cerr << "Failed to start engine" << std::endl;
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

// Send move command to the engine
bool ECE_ChessEngine::sendMove(const std::string& strMove) {
    try {
        sendCommand("position startpos moves " + strMove);
        sendCommand("go depth 7"); // Ask the engine to calculate the best move
        return true;
    } catch (const std::exception& e) {
        std::cerr << "";
        return false;
    }
}

// Read response with buffer to ensure data consistancy
std::string ECE_ChessEngine::readResponseWithBuffer(HANDLE hOutputRead, std::string& residualBuffer) {
    char buffer[4096];
    DWORD bytesRead;
    std::string output;

    if (ReadFile(hOutputRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        output = residualBuffer + std::string(buffer); // Append residual buffer
        residualBuffer.clear();
    }
    else {
        output = residualBuffer;
        residualBuffer.clear();
    }

    // Handle incomplete lines by splitting at the last newline
    size_t lastNewline = output.find_last_of('\n');
    if (lastNewline != std::string::npos && lastNewline < output.size() - 1) {
        residualBuffer = output.substr(lastNewline + 1); // Store incomplete part
        output = output.substr(0, lastNewline + 1);
    }

    return output;
}

// Response data cleaning
std::string ECE_ChessEngine::cleanEngineOutput(const std::string& output) {
    std::string cleanOutput;
    for (char c : output) {
        if (!isspace(c) || c == '\n') {
            cleanOutput += c;
        }
    }
    return cleanOutput;
}

// Find the best move recommended by the engine
std::string ECE_ChessEngine::extractBestMove(const std::string& response) {
    size_t bestMovePos = response.find("bestmove");
    if (bestMovePos != std::string::npos) {
        size_t moveStart = bestMovePos + 9; // Skip "bestmove "
        size_t moveEnd = response.find(" ", moveStart); // Find space after move
        if (moveEnd != std::string::npos) {
            return response.substr(moveStart, moveEnd - moveStart);
        }
        else {
            return response.substr(moveStart); // Until the end if no space
        }
    }
    return "";
}

// Get response from the engine
bool ECE_ChessEngine::getResponseMove(std::string& strMove) {
    try {
        std::string response;
        std::string residualBuffer;
        bool bestmoveFound = false;

        while (true) {
            response = readResponseWithBuffer(hOutputRead, residualBuffer);

            // Check for "Cannot execute move" in the response
            if (response.find("Cannot execute move") != std::string::npos) {
                std::cerr << "";
                strMove = "a1g1"; // Return fallback move
                return false;     // Indicate that the move is invalid
            }

            if (response.find("bestmove") != std::string::npos) {
                size_t pos = response.find("bestmove");
                size_t moveStart = pos + 9; // Skip "bestmove "
                size_t moveEnd = response.find(" ", moveStart); // Find space after move

                if (moveEnd != std::string::npos) {
                    strMove = response.substr(moveStart, moveEnd - moveStart);
                }
                else {
                    strMove = response.substr(moveStart); // Until the end if no space
                }

                // Check if move length is valid (4 or 5 for promotion)
                if (strMove.length() != 4 && strMove.length() != 5) {
                    std::cerr << "";
                    return false;
                }

                std::cout << "Engine Response: " << strMove << std::endl;
                bestmoveFound = true;
                break;
            }

            if (response.empty() && residualBuffer.empty()) {
                std::cerr << "Error: Engine did not produce a valid response." << std::endl;
                break;
            }

            std::cout << response; // Log intermediate responses
        }

        return bestmoveFound;
    }
    catch (const std::exception& e) {
        std::cerr << "Error reading move from engine: " << e.what() << std::endl;
        return false;
    }
}

// Send command to the engine
void ECE_ChessEngine::sendCommand(const std::string& command) {
    DWORD written;
    WriteFile(hInputWrite, command.c_str(), command.length(), &written, NULL);
    WriteFile(hInputWrite, "\n", 1, &written, NULL);
}

// Receive response from the engine
std::string ECE_ChessEngine::readResponse() {
    std::cout << "Enter readResponse" << std::endl;
    char buffer[4096];
    DWORD read;
    std::string output;
    if (ReadFile(hOutputRead, buffer, sizeof(buffer) - 1, &read, NULL) && read > 0) {
        buffer[read] = '\0';
        output = buffer;
    }
    std::cout << "Leave readResponse" << std::endl;
    return output;
}
