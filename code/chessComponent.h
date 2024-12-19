/*
Author: Botao Huang
Class: ECE6122 (A)
Last Date Modified: 12/3/2024
Description:
Chess component class header file
*/
#ifndef CHESS_COMPONENTS_H
#define CHESS_COMPONENTS_H

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include "chessCommon.h"

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// Include GLEW
#include <GL/glew.h>

// Load BMP function support
#include <common/texture.hpp>

class chessComponent
{
private:
    // Existing private members...
    std::vector<unsigned short> indices;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    GLuint vertexbuffer = 0;
    GLuint uvbuffer = 0;
    GLuint normalbuffer = 0;
    GLuint elementbuffer = 0;

    std::string cName;
    std::string cTextureFile;

    meshPropsT meshProps;
    glm::vec3 cGeometricCener = { 0, 0, 0 };
    glm::vec3 cBoundingLimitsMin = { 0, 0, 0 };
    glm::vec3 cBoundingLimitsMax = { 0, 0, 0 };

    GLuint Texture;

    // Add this member to track the piece's position and orientation
    tPosition currentPosition;

    void getGeometricCenter();
    void getBoundingBox();

public:
    chessComponent();
    ~chessComponent();

    // Existing public methods...
    void reserveStorage(const unsigned int& vCapacity, const unsigned int& fCapacity);
    void addVertices(glm::vec3& objVertice);
    void addTextureCor(glm::vec3& objUVW);
    void addVerNormals(glm::vec3& objVerNormal);
    void addFaceIndices(unsigned int* objFaceIndice);
    void setupGLBuffers();
    void setupTextureBuffers();
    void setupTexture(GLuint& TextureID);
    void renderMesh();
    void deleteGLBuffers();
    void storeComponentID(std::string cName);
    void storeTextureID(std::string cTextureFile);
    void storeMeshProps(meshPropsT meshProps);
    glm::mat4 genModelMatrix(tPosition& cTPosition);

    std::string getComponentID();
    std::string getComponentID() const;

    // New methods to handle position updates
    void storePosition(const tPosition& newPosition); // Update position
    const tPosition& getPosition() const;            // Retrieve position
};

#endif
