#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum MethodEnum
{
	XPBD = 1,
	PBD = 2,
	XPBD_SS = 3, // XPBD with small step
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

	MethodClass(MethodEnum methodId, std::string methodName, int methodIteration, glm::vec2 methodClothNodesNumber, int constraintLevel = 0) :
	MethodId(methodId), MethodName(methodName), MethodIteration(methodIteration), MethodClothNodesNumber(methodClothNodesNumber), ConstraintLevel(constraintLevel)
	{}
	MethodClass() {}

	MethodEnum getId() { return MethodId; }
	std::string getName() { return MethodName; }
};

MethodClass M_PPBD(XPBD, "XPBD", 10, glm::vec2(64, 64));
MethodClass M_PBD(PBD, "PBD", 20, glm::vec2(64, 64));
MethodClass M_PPBD_SS(XPBD_SS, "XPBD_SS", 10, glm::vec2(64, 64));
MethodClass M_Verlet_Integration(Verlet_Integration, "Verlet_Integration", 40, glm::vec2(64, 64));
// Note: Explicit_Euler will explode if timestep is too small, 1/200 will only be good for several secs. 1/1200 works for 40 iteration
//       1/600 works for 100 iteration, can't keep stable under 1/60 timestep
MethodClass M_Explicit_Euler(Explicit_Euler, "Explicit_Euler", 100, glm::vec2(64, 64));
MethodClass M_Semi_Implicit_Euler(Semi_Implicit_Euler, "Semi_Implicit_Euler", 40, glm::vec2(64, 64));