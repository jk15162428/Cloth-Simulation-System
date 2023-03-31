#pragma once
#include "node.h"

// for PBD and XPBD
class Constraint
{
private:
	GLdouble    RestLength;
	Node* Node1;
	Node* Node2;
	GLdouble    Stiffness;   // for PBD (0.0f - 1.0f)
	GLdouble    Compliance;  // for XPBD
	GLdouble    Lambda;      // for XPBD

public:
	Constraint(Node* n1, Node* n2) :
		RestLength(0.0f), Node1(n1), Node2(n2),
		Stiffness(0.1f), Compliance(0.0f), Lambda(0.0f)
	{
		glm::vec3 n1_to_n2 = Node2->Position - Node1->Position;
		RestLength = glm::length(n1_to_n2);
	}
	Constraint(Node* n1, Node* n2, GLdouble compliance) :
		RestLength(0.0f), Node1(n1), Node2(n2),
		Stiffness(0.1f), Lambda(0.0f)
	{
		glm::vec3 n1_to_n2 = Node2->Position - Node1->Position;
		RestLength = glm::length(n1_to_n2);
		Compliance = compliance;
	}

	void SetLambda(GLdouble val) { Lambda = val; }

	GLdouble GetStiffness() { return Stiffness; }
	GLdouble SetStiffness(GLdouble s) { Stiffness = s; }

	void Solve(GLdouble dt)
	{
		GLdouble invMass1 = Node1->InvMass, invMass2 = Node2->InvMass;
		if (invMass1 + invMass2 == 0.0f) return;
		glm::vec<3, double> p2_to_p1 = Node1->Position - Node2->Position;
		GLdouble dist = glm::length(p2_to_p1);
		if (dist == 0.0f) return;
		GLdouble constraint = dist - RestLength; // C_j(x)
		glm::vec<3, double> deltaPosition;
		GLdouble alpha = Compliance / dt / dt; // \tilde{alpha}
		// Note: zero compliance for cloth
		GLdouble deltaLambda = (-constraint - alpha * Lambda) / ((invMass1 + invMass2) + alpha); // equation (18)
		deltaPosition = deltaLambda * p2_to_p1 / dist; // equation (17), M^{-1} is provided later.
		Lambda += deltaLambda;
		// for PBD, TODO: finish this
		// else { p2_to_p1.normalize();  deltaPosition = stiffness * p2_to_p1 * -constraint / (invMass1 + invMass2); }
		Node1->Position += (invMass1 * deltaPosition);
		Node2->Position += (-invMass2 * deltaPosition);
	}
};