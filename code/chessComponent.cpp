/*
Author: Botao Huang
Class: ECE6122 (A)
Last Date Modified: 12/3/2024
Description:
Chess component class definition file
*/

#include "chessComponent.h"


// Compute the Geometric center
// Inputs: None
// Output: None
void chessComponent::getGeometricCenter()
{
    // Reset the geometric center
    cGeometricCener = glm::vec3(0.0f);
    
    //Sum all vertices
    for (const auto& vertex : vertices)
    {
        cGeometricCener.x += vertex.x;
        cGeometricCener.y += vertex.y;
        cGeometricCener.z += vertex.z;
    }
    // Compute Average
    if (vertices.size() > 0)
    { // Divide-by-zero protection
        cGeometricCener = cGeometricCener / static_cast<float> (vertices.size());
    }
    else
    { // No Vertices (Weird case)
        cGeometricCener = glm::vec3(0.0f);
    }
}

// Compute the Bounding box
// Inputs: None
// Output: None
void chessComponent::getBoundingBox()
{
    // Initialize the min and max
    cBoundingLimitsMin = vertices.front();
    cBoundingLimitsMax = vertices.front();

    // Finding min and max iterating over
    // all vertices
    for (const auto& vertex : vertices)
    {
        cBoundingLimitsMin = glm::min(cBoundingLimitsMin, vertex);
        cBoundingLimitsMax = glm::max(cBoundingLimitsMax, vertex);
    }
}

// Constructor function
chessComponent::chessComponent()
{
    // Clear mesh properties
    meshProps.hasBones = false;
    meshProps.hasFaces = false;
    meshProps.hasNormals = false;
    meshProps.hasPositions = false;
    meshProps.hasTangentsAndBitangents = false;
    meshProps.hasTextureCoords = false;
    meshProps.hasVertexColors = false;
    meshProps.numOfUVChannels = 0;

    // Clear the vertice, normals, uv buffer vectors
    indices.clear();
    vertices.clear();
    uvs.clear();
    normals.clear();

    // OpenGL Buffers management
    vertexbuffer = 0;
    uvbuffer = 0;
    normalbuffer = 0;
    elementbuffer = 0;

    // Component ID
    cName = "";
    cTextureFile = "";

    // Reset the geometric center
    cGeometricCener = glm::vec3(0.0f);
    cBoundingLimitsMin = glm::vec3(0.0f);
    cBoundingLimitsMin = glm::vec3(0.0f);

    // Reset the Texture handle
    Texture = 0;
}

// Destructor function
chessComponent::~chessComponent()
{
    // Delete all the buffers
    deleteGLBuffers();
}

// Reserve storage
// Inputs: memory limits for performance
// Output: None
void chessComponent::reserveStorage(const unsigned int& vCapacity, const unsigned int& fCapacity)
{
    // Reserve Vertex storage capcity
    vertices.reserve(vCapacity);
    // Reserve Texture coordinates capacity
    uvs.reserve(vCapacity);
    // Reserve normal capacity
    normals.reserve(vCapacity);
    // Reserved face capacity
    indices.reserve(3U*fCapacity);
}

// Add vertices
// Inputs: Vertices read from OBJ file
// Output: None
void chessComponent::addVertices(glm::vec3& objVertice)
{
    // Add a vertice
    vertices.push_back(objVertice);
}

// Add Texture Coordinates
// Inputs: Texture coordinates read from OBJ file
// Output: None
void chessComponent::addTextureCor(glm::vec3& objUVW)
{
    // Add the texture coordinate
    uvs.push_back(glm::vec2(objUVW.x, objUVW.y));
}

// Add Vertices Normals
// Inputs: Vertices read from OBJ file
// Output: None
void chessComponent::addVerNormals(glm::vec3& objVerNormal)
{
    // Fill vertices normals
    normals.push_back(objVerNormal);
}

// Add Face indices
// Inputs: Face vertices read from OBJ file
// Output: None
void chessComponent::addFaceIndices(unsigned int *objFaceIndice)
{
    // Fill face indices
    // Assume the model has only triangles.
    indices.push_back(objFaceIndice[0]);
    indices.push_back(objFaceIndice[1]);
    indices.push_back(objFaceIndice[2]);
}

// Setup rendering buffers
// Inputs: None
// Output: None
void chessComponent::setupGLBuffers()
{
    // Load it into a VBO
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    
    // Load it into UV Buffer
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
    
    // Load it into the normals buffer
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

    // Compute the Geometric center
    getGeometricCenter();

}

// Setup Texture buffers
// Inputs: None
// Output: None
void chessComponent::setupTexture(GLuint& TextureID)
{
    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture);
    // Set our "myTextureSampler" sampler to use Texture Unit 0
    glUniform1i(TextureID, 0);
}

// Setup rendering buffers
// Inputs: None
// Output: None
void chessComponent::setupTextureBuffers()
{
    // Matching pattern and rule creation
    // Any combination of 0-9, space in the beginning or end is allowed!
    // Discarded while converting to integer
    std::string regexPattern = "\\s*([\\w0-9]+)\\.\\s*";  // Pattern       
    std::regex  regexRule(regexPattern);                  // Create a custom search rule
    // Match string
    std::smatch matches;

    // Perform search on the Texture file name search
    if (std::regex_search(cTextureFile, matches, regexRule))
    {
        // Update the Texture file name (since we only support bmps)
        cTextureFile = matches[1].str();
        // Process directory path, it's a short cut for now!
        if (cTextureFile == "12951_Stone_Chess_Board_diff")
        { // Chess board directory path
            cTextureFile = "objFiles/Stone_Chess_Board/" + cTextureFile;
        }
        else
        { // Chess pieces directory path
            cTextureFile = "objFiles/Chess/" + cTextureFile;
        }
        // Add the bmp extension
        cTextureFile += ".bmp";
        // Test print
        // std::cout << "Texture file is " << cTextureFile << std::endl;
    }
    else
    {
        std::cout << "Texture file not found for chess compoent!" << cName << std::endl;
    }

    // Load the texture
    Texture = loadBMP_custom(&cTextureFile[0]);
}

// Render a mesh
// Inputs: None
// Output: None
void chessComponent::renderMesh()
{
    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        0,                  // attribute
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
        1,                                // attribute
        2,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );

    // 3rd attribute buffer : normals
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glVertexAttribPointer(
        2,                                // attribute
        3,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

    // Draw the triangles !
    glDrawElements(
        GL_TRIANGLES,      // mode
        indices.size(),    // count
        GL_UNSIGNED_SHORT,   // type
        (void*)0           // element array buffer offset
    );

    // Disable the arrays
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

// Render a mesh
// Inputs: None
// Output: None
void chessComponent::deleteGLBuffers()
{
    // Cleanup VBO
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteBuffers(1, &elementbuffer);
    // Cleanup Texture buffer
    glDeleteTextures(1, &Texture);
}

// Stores a component ID
// Inputs: None
// Output: None
void chessComponent::storeComponentID(std::string cName)
{
    // Capture the component name
    this->cName = cName;
    // Testing
    // std::cout << "The child name is " << this->cName << std::endl;
}

// Stores a Texture file name
// Inputs: None
// Output: None
void chessComponent::storeTextureID(std::string cTextureFile)
{
    // Capture the component name
    this->cTextureFile = cTextureFile;
    // Testing
    // std::cout << "The texture file is " << this->cTextureFile << std::endl;
}

// Store Mesh properties (mainly for debug and bound checks)
// Inputs: None
// Output: None
void chessComponent::storeMeshProps(meshPropsT meshProps)
{
    this->meshProps = meshProps;
}

// Generate model matrix
// Inputs: None
// Output: None
glm::mat4 chessComponent::genModelMatrix(tPosition& cTPosition)
{
    // Start with the Identity matrix
    glm::mat4 tModel = glm::mat4(1.0f);
    // Target World Coordinates
    tModel = glm::translate(tModel, cTPosition.tPos);
    // Apply target rotation
    if (cTPosition.rAngle != 0.f)   
    {
        // Rotate Knight/Bishop by another 180 degree aroudn Z
        if (cName == "Object3" || cName == "ALFIERE3")
        {
            tModel = glm::rotate(tModel, glm::radians(180.f), {0, 0, 1});
        }
        tModel = glm::rotate(tModel, glm::radians(cTPosition.rAngle), cTPosition.rAxis);
    }
    // Apply scaling
    tModel = glm::scale(tModel, cTPosition.cScale);
    // Pull it to origin first (with height adjusted to X/Z plane)!
    // tModel = glm::translate(tModel, -cGeometricCener);
    // We want the board surface to be in the X/Z plane. Need to move in -y direction
    // equal to board's height.
    if (cName == "12951_Stone_Chess_Board")
    { // For Chess board eliminate the height by pushing it down by the height
        // Apply the adjustment (Z is compensated to push the board down by depth)
        tModel = glm::translate(tModel, {-cGeometricCener.x, -cGeometricCener.y, -cGeometricCener.z/2});
    }
    else
    { // For all others get to X/Z plane with Y=0
        tModel = glm::translate(tModel, {-cGeometricCener.x, 0.f, -cGeometricCener.z});
    }
    // Return the matrix
    return tModel;
}

// Get ID
// Inputs: None
// Output: ID
std::string chessComponent::getComponentID()
{
    return cName;
}

// Definition
std::string chessComponent::getComponentID() const {
    return cName;
}

void chessComponent::storePosition(const tPosition& newPosition) {
    currentPosition = newPosition; // Update the stored position
}

const tPosition& chessComponent::getPosition() const {
    return currentPosition; // Return the current position
}
