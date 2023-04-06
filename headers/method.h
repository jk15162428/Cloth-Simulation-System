#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum MethodEnum
{
    PPBD = 1,
    PBD = 2,
    PPBD_SS = 3 // PPBD with small step
};

class MethodClass
{
private:
    MethodEnum MethodId;
    std::string MethodName;
public:
    int MethodIteration;
    glm::vec2 MethodClothNodesNumber;

    MethodClass(MethodEnum methodId, std::string methodName, int methodIteration, glm::vec2 methodClothNodesNumber) :
    MethodId(methodId), MethodName(methodName), MethodIteration(methodIteration), MethodClothNodesNumber(methodClothNodesNumber)
    {}
    MethodClass() {}

    MethodEnum getId() { return MethodId; }
    std::string getName() { return MethodName; }
};

MethodClass M_PPBD(PPBD, "PPBD", 40, glm::vec2(64, 64));
MethodClass M_PBD(PBD, "PBD", 40, glm::vec2(64, 64));
MethodClass M_PPBD_SS(PPBD_SS, "PPBD_SS", 10, glm::vec2(100, 100));