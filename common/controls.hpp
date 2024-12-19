/*

Objective:
Use the common.cpp file from the Git Tutorial and add a custom function
for ECE6122 Labs. It does NOT touch the existing functions.

*/

#ifndef CONTROLS_HPP
#define CONTROLS_HPP

#include <iostream>

void computeMatricesFromInputs();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
bool getLightSwitch();


void computeMatricesFromInputsFinalProject(float _theta, float _phi, float _radius);

#endif