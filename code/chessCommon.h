/*
Author: Botao Huang
Class: ECE6122 (A)
Last Date Modified: 12/3/2024
Description:
Game of chess common file (referenced: lab 3 src)
*/


#ifndef COMMON_H
#define COMMON_H

#include <unordered_map>
// Include GLM
#include <glm/glm.hpp>

// Mesh properties has table
typedef struct 
{
    bool hasBones;
    bool hasFaces;
    bool hasNormals;
    bool hasPositions;
    bool hasTangentsAndBitangents;
    bool hasTextureCoords;
    bool hasVertexColors;
    unsigned int numOfUVChannels;
} meshPropsT;

// Structure to hold target
// model matrix generation
typedef struct
{
    unsigned int rCnt;
    unsigned int rDis;
    float rAngle;
    glm::vec3 rAxis;
    glm::vec3 cScale;
    glm::vec3 tPos;
} tPosition;

struct ModelData {
    std::string id;        // The piece's ID
    tPosition position;    // The piece's tPosition data
    std::string location;  // The 2D board location (e.g., "e7")
};

// Chess board scaling
const float CBSCALE = 0.6f;
// Chess board square box size (per side)
//const float CHESS_BOX_SIZE = 3.f;
const float CHESS_BOX_SIZE = (float)(CBSCALE * 5.4);
// Chess pieces scaling
const float CPSCALE = 0.015f;
// Platform height
const float PHEIGHT = -3.0f;
// Hash to hold the target Model matrix spec for each Chess component
// A vector of ModelData
typedef std::vector<ModelData> tModelMap;

#endif
