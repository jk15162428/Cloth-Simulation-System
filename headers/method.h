#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum MethodEnum
{
    PPBD = 1,
    PBD = 2,
    PPBD_SS = 3, // PPBD with small step
    // all for mass-spring system
    Verlet_Integration = 4, 
    Explicit_Euler = 5,
    Semi_Implicit_Euler = 6
};

class MethodClass
{
private:
    MethodEnum MethodId;
    std::string MethodName;
public:
    int MethodIteration;
    glm::vec2 MethodClothNodesNumber;
    int ConstraintLevel; // 0: no bending constraint, 1: only diagonal bending constraint, 2: only edge bending constraint, 3: all bending constraint

    MethodClass(MethodEnum methodId, std::string methodName, int methodIteration, glm::vec2 methodClothNodesNumber, int constraintLevel = 3) :
    MethodId(methodId), MethodName(methodName), MethodIteration(methodIteration), MethodClothNodesNumber(methodClothNodesNumber), ConstraintLevel(constraintLevel)
    {}
    MethodClass() {}

    MethodEnum getId() { return MethodId; }
    std::string getName() { return MethodName; }
};

MethodClass M_PPBD(PPBD, "PPBD", 40, glm::vec2(64, 64));
MethodClass M_PBD(PBD, "PBD", 40, glm::vec2(64, 64));
MethodClass M_PPBD_SS(PPBD_SS, "PPBD_SS", 10, glm::vec2(64, 64));
MethodClass M_Verlet_Integration(Verlet_Integration, "Verlet_Integration", 40, glm::vec2(64, 64));
MethodClass M_Explicit_Euler(Explicit_Euler, "Explicit_Euler", 40, glm::vec2(64, 64));
MethodClass M_Semi_Implicit_Euler(Semi_Implicit_Euler, "Semi_Implicit_Euler", 40, glm::vec2(64, 64));