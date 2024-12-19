/*
Objective:
Use the common.cpp file from the Git Tutorial and add a custom function
for ECE6122 Labs. It does NOT touch the existing functions.

*/

#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <iostream>

bool loadOBJ(
	const char * path, 
	std::vector<glm::vec3> & out_vertices, 
	std::vector<glm::vec2> & out_uvs, 
	std::vector<glm::vec3> & out_normals
);



bool loadAssImp(
	const char * path, 
	std::vector<unsigned short> & indices,
	std::vector<glm::vec3> & vertices,
	std::vector<glm::vec2> & uvs,
	std::vector<glm::vec3> & normals
);

#ifdef USE_LAB3_ASSIMP // don't use this #define, it's only for Lab3
// Include Chess Component Class    
#include "code/chessComponent.h"
#include "code/chessCommon.h"

// Reads the multiple meshes OBJ file
bool loadAssImpLab3(
	const char* path,
	std::vector<chessComponent>& gchessComponents
);

#endif

#endif