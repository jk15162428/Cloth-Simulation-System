#pragma once
#include "node.h"
#include "method.h"

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
		Stiffness(0.2f), Lambda(0.0f)
	{
		glm::vec3 n1_to_n2 = Node2->Position - Node1->Position;
		RestLength = glm::length(n1_to_n2);
		Compliance = compliance;
	}

	void SetLambda(GLdouble val) { Lambda = val; }

	GLdouble GetStiffness() { return Stiffness; }
	GLdouble SetStiffness(GLdouble s) { Stiffness = s; }

	void Solve(GLdouble dt, MethodEnum method)
	{
		GLdouble invMass1 = Node1->InvMass, invMass2 = Node2->InvMass;
		if (invMass1 + invMass2 == 0.0f) return;
		glm::vec<3, double> p2_to_p1 = Node1->Position - Node2->Position;
		GLdouble dist = glm::length(p2_to_p1);
		if (dist == 0.0f) return;
		GLdouble constraint = dist - RestLength; // C_j(x)
		glm::vec<3, double> deltaPosition;
		GLdouble deltaLambda, alpha;
		switch (method)
		{
			case XPBD: // trivial XPBD
				alpha = Compliance / (dt * dt); // \tilde{alpha}
				// Note: zero compliance for cloth
				deltaLambda = (-constraint - alpha * Lambda) / ((invMass1 + invMass2) + alpha); // equation (18)
				deltaPosition = deltaLambda * p2_to_p1 / (dist + FLT_EPSILON); // equation (17)
				Lambda += deltaLambda;
				break;
			case PBD:
				p2_to_p1 = glm::normalize(p2_to_p1);  deltaPosition = Stiffness * p2_to_p1 * -constraint / (invMass1 + invMass2);
				break;
			case XPBD_SS: // XPBD with small step, lambda is set to 0.0 every step, so no lambda at all
				alpha = Compliance / (dt * dt); // \tilde{alpha}
				deltaLambda = -constraint / ((invMass1 + invMass2) + alpha);
				deltaPosition = deltaLambda * p2_to_p1 / (dist + FLT_EPSILON);
		}
		Node1->Position += (invMass1 * deltaPosition);
		Node2->Position += (-invMass2 * deltaPosition);
	}
};