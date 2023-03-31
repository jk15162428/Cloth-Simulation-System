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
		GLdouble deltaLambda = -constraint / ((invMass1 + invMass2) + alpha); // 10 minutes physics
		// GLdouble deltaLambda = (-constraint - alpha * Lambda) / ((invMass1 + invMass2) + alpha); // equation (18)
		deltaPosition = deltaLambda * p2_to_p1 / dist; // equation (17), M^{-1} is provided later.
		// printf("dx: [%f, %f, %f]\n", deltaPosition.x, deltaPosition.y, deltaPosition.z);
		Lambda += deltaLambda;

		// for PBD
		// else { p2_to_p1.normalize();  deltaPosition = stiffness * p2_to_p1 * -constraint / (invMass1 + invMass2); }
		Node1->Position += (invMass1 * deltaPosition);
		Node2->Position += (-invMass2 * deltaPosition);
		/*
		if (deltaPosition.x > 0.0001 || deltaPosition.y > 0.0001 || deltaPosition.z > 0.0001 || deltaLambda > 0.0001)
		{
			printf("C: %f, alpha: %f, deltaLambda: %f\n", constraint, alpha, deltaLambda);
			printf("invMass1: %f, invMass2: %f, dx: [%f, %f, %f]\n", invMass1, invMass2, deltaPosition.x, deltaPosition.y, deltaPosition.z);
			printf("Node1.Position: [%f, %f, %f], Node2.Position: [%f, %f, %f]\n", Node1->Position.x, Node1->Position.y, Node1->Position.z, Node2->Position.x, Node2->Position.y, Node2->Position.z);
		}
		*/
		/*
		if (deltaPosition.x > 5 || deltaPosition.y > 5 || deltaPosition.z > 5 || deltaLambda > 5)
		{
			printf("C: %f, alpha: %f, deltaLambda: %f\n", constraint, alpha, deltaLambda);
			printf("invMass1: %f, invMass2: %f, dx: [%f, %f, %f]\n", invMass1, invMass2, deltaPosition.x, deltaPosition.y, deltaPosition.z);
			printf("Node1.Position: [%f, %f, %f], Node2.Position: [%f, %f, %f]\n", Node1->Position.x, Node1->Position.y, Node1->Position.z, Node2->Position.x, Node2->Position.y, Node2->Position.z);
		}
		*/
	}
};