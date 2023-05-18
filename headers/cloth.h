#pragma once
#include <vector>
#include <stdio.h>
#include <iostream>
#include <random>
#include "node.h"
#include "spring.h"
#include "constraint.h"
#include "method.h"

enum VelocityUpdate
{
	VEL_FRONT,
	VEL_BACK,
	VEL_UP,
	VEL_DOWN,
	VEL_LEFT_AND_UP,
	VEL_RIGHT_AND_UP
};

class Cloth
{
private:
	const GLdouble DEFAULT_INVMASS = 1.0;
	const GLdouble DISTANCE_COMPLIANCE = 0.0;
	const GLdouble BENDING_COMPLIANCE = 1.0;
	const glm::vec<3, GLdouble> gravity = glm::vec<3, GLdouble>(0.0, -10.0, 0.0);
	const double STRUCTURE_COEF = 1000.0;
	const double SHEAR_COEF = 50.0;
	const double BENDING_COEF = 400.0;
public:
	int Iteration;
	glm::vec<3, GLdouble> ClothPosition;
	int Width, Height;
	int NodesInWidth, NodesInHeight;
	MethodClass Method;
	const GLdouble DEFAULT_FORCE = 5.0; // used in velocity update with keyboard
	int ConstraintLevel;

	enum DrawModeEnum
	{
		DRAW_NODES = 0,
		DRAW_LINES = 1,
		DRAW_FACES = 2
	};
	DrawModeEnum drawMode = DRAW_FACES;

	std::vector<Node*> Nodes;
	std::vector<Constraint> Constraints; // for PBD & PPBD
	std::vector<Spring*> Springs; // for mass-spring system
	std::vector<Node*> Faces; // for rendering

	Cloth() {}
	Cloth(glm::vec3 position, glm::vec2 size, MethodClass method)
	{
		ClothPosition = position;
		Width = size.x;
		Height = size.y;
		NodesInWidth = method.MethodClothNodesNumber.x;
		NodesInHeight = method.MethodClothNodesNumber.y;
		Method = method;
		Iteration = method.MethodIteration;
		ConstraintLevel = method.ConstraintLevel;
		init();
	}
	// just a dummy version of copy constructor
	void set(glm::vec3 position, glm::vec2 size, MethodClass method)
	{
		ClothPosition = position;
		Width = size.x;
		Height = size.y;
		NodesInWidth = method.MethodClothNodesNumber.x;
		NodesInHeight = method.MethodClothNodesNumber.y;
		Method = method;
		Iteration = method.MethodIteration;
		ConstraintLevel = method.ConstraintLevel;
		init();
	}
	~Cloth()
	{
		Destroy();
	}

	Node* getNode(int w, int h) { return Nodes[h * NodesInWidth + w]; }
	glm::vec3 computeFaceNormal(Node* n1, Node* n2, Node* n3)
	{
		return glm::cross(n2->Position - n1->Position, n3->Position - n1->Position);
	}

	void computeNormal()
	{
		/** Reset nodes' normal **/
		glm::vec3 normal(0.0, 0.0, 0.0);
		for (int i = 0; i < Nodes.size(); i++)
			Nodes[i]->Normal = normal;
		/** Compute normal of each face **/
		for (int i = 0; i < Faces.size() / 3; i++)
		{
			// 3 nodes in each face
			Node* n1 = Faces[3 * i + 0];
			Node* n2 = Faces[3 * i + 1];
			Node* n3 = Faces[3 * i + 2];

			// Face normal
			normal = computeFaceNormal(n1, n2, n3);
			// Add all face normal
			n1->Normal += normal;
			n2->Normal += normal;
			n3->Normal += normal;
		}
		for (int i = 0; i < Nodes.size(); i++)
			Nodes[i]->Normal = glm::normalize(Nodes[i]->Normal);
	}

	void Integrate(GLdouble dt)
	{
		switch (Method.getId())
		{
		case PPBD:
		case PBD:
			// n iterations
			for (int i = 0; i < Nodes.size(); i++)
			{
				if (Nodes[i]->InvMass == 0.0)
					continue;
				Nodes[i]->Velocity += Nodes[i]->Acceleration * dt;
				Nodes[i]->OldPosition = Nodes[i]->Position;
				Nodes[i]->Position += Nodes[i]->Velocity * dt;
			}
			for (int i = 0; i < Constraints.size(); i++)
				Constraints[i].SetLambda(0.0f);
			for (int n = 0; n < Iteration; n++)
			{
				for (int i = 0; i < Constraints.size(); i++)
				{
					Constraints[i].Solve(dt, Method.getId());
				}
			}
			for (int i = 0; i < Nodes.size(); i++)
			{
				if (Nodes[i]->InvMass == 0.0f)
					continue;
				Nodes[i]->Velocity = (Nodes[i]->Position - Nodes[i]->OldPosition) * 1.0 / dt;
			}
			break;
		case PPBD_SS:
			// only one iteration
			for (int i = 0; i < Nodes.size(); i++)
			{
				if (Nodes[i]->InvMass == 0.0)
					continue;
				Nodes[i]->Velocity += Nodes[i]->Acceleration * dt;
				Nodes[i]->OldPosition = Nodes[i]->Position;
				Nodes[i]->Position += Nodes[i]->Velocity * dt;
			}
			for (int i = 0; i < Constraints.size(); i++)
			{
				Constraints[i].Solve(dt, Method.getId());
			}
			for (int i = 0; i < Nodes.size(); i++)
			{
				if (Nodes[i]->InvMass == 0.0f)
					continue;
				Nodes[i]->Velocity = (Nodes[i]->Position - Nodes[i]->OldPosition) * 1.0 / dt;
			}
			break;
			// wait a minute.. it looks like Explicit Euler
		case Verlet_Integration:
		case Explicit_Euler:
		case Semi_Implicit_Euler:
			// n iterations
			for (int iter = 0; iter < Iteration; iter++)
			{
				// compute force first
				for (int i = 0; i < Nodes.size(); i++)
				{
					if (Nodes[i]->InvMass == 0.0) continue;
					Nodes[i]->addForce(gravity * 1.0 / Nodes[i]->InvMass / (double)Iteration);
				}
				for (int i = 0; i < Springs.size(); i++)
				{
					Springs[i]->applyInternalForce(dt);
				}

				// update the position using integration
				switch (Method.getId())
				{
				// Note: dt = 1/60 won't work with Explicit_Euler, will explode; but 1/600 works
				case Explicit_Euler:
					for (int i = 0; i < Nodes.size(); i++)
					{
						if (Nodes[i]->InvMass == 0.0) continue;
						Nodes[i]->Acceleration = Nodes[i]->Force * Nodes[i]->InvMass;
						glm::vec<3, double> temp = Nodes[i]->Velocity;
						Nodes[i]->Velocity += Nodes[i]->Acceleration * dt;
						Nodes[i]->Position += temp * dt;
					}
					break;
				case Semi_Implicit_Euler:
					for (int i = 0; i < Nodes.size(); i++)
					{
						if (Nodes[i]->InvMass == 0.0) continue;
						Nodes[i]->Acceleration = Nodes[i]->Force * Nodes[i]->InvMass;
						Nodes[i]->Velocity += Nodes[i]->Acceleration * dt;
						Nodes[i]->Position += Nodes[i]->Velocity * dt;
					}
					break;
				case Verlet_Integration:
					for (int i = 0; i < Nodes.size(); i++)
					{
						if (Nodes[i]->InvMass == 0.0) continue;
						glm::vec<3, double> temp = Nodes[i]->Position;
						Nodes[i]->Acceleration = Nodes[i]->Force * Nodes[i]->InvMass;
						Nodes[i]->Position += (Nodes[i]->Position - Nodes[i]->OldPosition) + Nodes[i]->Acceleration * dt * dt;
						Nodes[i]->OldPosition = temp;
					}
					break;
				}
				// clear the force
				for (int i = 0; i < Nodes.size(); i++)
				{
					Nodes[i]->Force = glm::vec<3, double>(0, 0, 0);
				}
			}
			break;
		}
	}

	glm::vec<3, GLdouble> getWorldPos(Node* n) { return ClothPosition + n->Position; }
	void setWorldPos(Node* n, glm::vec<3, GLdouble> position) { n->Position = position - ClothPosition; }
	void reset() { Destroy();  init(); }
	void UpdateVelocity(VelocityUpdate update, GLdouble force = -1.0)
	{
		if (force < 0) force = DEFAULT_FORCE;
		for (int i = 0; i < Nodes.size(); i++)
		{
			if (Nodes[i]->InvMass == 0) continue;
			if (Method.getId() <= 3)
			{
				switch (update)
				{
				case VEL_UP:
					Nodes[i]->Velocity.y += force * Nodes[i]->InvMass;
					break;
				case VEL_DOWN:
					Nodes[i]->Velocity.y -= force * Nodes[i]->InvMass;
					break;
				case VEL_FRONT:
					Nodes[i]->Velocity.z += force * Nodes[i]->InvMass;
					break;
				case VEL_BACK:
					Nodes[i]->Velocity.z -= force * Nodes[i]->InvMass;
					break;
				case VEL_LEFT_AND_UP:
					Nodes[i]->Velocity.x -= force * Nodes[i]->InvMass;
					Nodes[i]->Velocity.z -= force * Nodes[i]->InvMass / 50;
					break;
				case VEL_RIGHT_AND_UP:
					Nodes[i]->Velocity.x += force * Nodes[i]->InvMass;
					Nodes[i]->Velocity.z -= force * Nodes[i]->InvMass / 50;
					break;
				}
			}
			else
			{
				switch (update)
				{
				case VEL_UP:
					Nodes[i]->Force.y += force * Nodes[i]->InvMass * 10;
					break;
				case VEL_DOWN:
					Nodes[i]->Force.y -= force * Nodes[i]->InvMass * 10;
					break;
				case VEL_FRONT:
					Nodes[i]->Force.z += force * Nodes[i]->InvMass * 10;
					break;
				case VEL_BACK:
					Nodes[i]->Force.z -= force * Nodes[i]->InvMass * 10;
					break;
				case VEL_LEFT_AND_UP:
					Nodes[i]->Force.x -= force * Nodes[i]->InvMass * 10;
					Nodes[i]->Force.z -= force * Nodes[i]->InvMass / 2.0;
					break;
				case VEL_RIGHT_AND_UP:
					Nodes[i]->Force.x += force * Nodes[i]->InvMass * 10;
					Nodes[i]->Force.z -= force * Nodes[i]->InvMass / 2.0;
					break;
				}
			}
		}
	}
private:
	void MakeConstraint(Node* n1, Node* n2, GLdouble compliance = 0.0f) { Constraints.push_back(Constraint(n1, n2, compliance)); }

	void init()
	{
		initNodes();
		initFaces();
		initConstraints();
	}

	void initNodes()
	{
		Nodes.resize(NodesInWidth * NodesInHeight);
		printf("Init cloth with %d nodes, %d in width and %d in height.\n", NodesInWidth * NodesInHeight, NodesInWidth, NodesInHeight);
		for (int w = 0; w < NodesInWidth; w++) {
			for (int h = 0; h < NodesInHeight; h++) {
				/** Create node by position **/
				glm::vec3 position = glm::vec3(Width * (GLdouble)w / (GLdouble)NodesInWidth, -(Height * (GLdouble)h / (GLdouble)NodesInHeight), 0.0f);
				GLdouble invMass = DEFAULT_INVMASS;
				if ((h == 0) && (w == 0) || (h == 0) && (w == NodesInWidth - 1)) { invMass = 0.0f; }
				Node* node = new Node(invMass, position, gravity);
				/** Set texture coordinates **/
				node->TextureCoord.y = (double)h / (NodesInHeight - 1);
				node->TextureCoord.x = (double)w / (1 - NodesInWidth);
				/** Add node to cloth **/
				Nodes[h * NodesInWidth + w] = node;
				// std::cout << node << std::endl;
				// printf("\t%d: [%d, %d] (%f, %f, %f) - (%f, %f)\n", h * NodesInWidth + w, w, h, node->Position.x, node->Position.y, node->Position.z, node->TextureCoord.x, node->TextureCoord.y);
			}
			//std::cout << std::endl;
		}
		// printf("Actual cloth has %i nodes.\n", Nodes.size());

		if (Method.getId() > 3) // mass-spring system
		{
			for (int i = 0; i < NodesInHeight; i++) {
				for (int j = 0; j < NodesInWidth; j++) {
					// Structural
					if (i < NodesInHeight - 1) Springs.push_back(new Spring(getNode(i, j), getNode(i + 1, j), STRUCTURE_COEF));
					if (j < NodesInWidth - 1) Springs.push_back(new Spring(getNode(i, j), getNode(i, j + 1), STRUCTURE_COEF));
					// Shear 
					if (i < NodesInHeight - 1 && j < NodesInWidth - 1)
					{
						Springs.push_back(new Spring(getNode(i, j), getNode(i + 1, j + 1), SHEAR_COEF));
						Springs.push_back(new Spring(getNode(i + 1, j), getNode(i, j + 1), SHEAR_COEF));
					}
					// Bending
					if (i < NodesInHeight - 2) Springs.push_back(new Spring(getNode(i, j), getNode(i + 2, j), BENDING_COEF));
					if (j < NodesInWidth - 2) Springs.push_back(new Spring(getNode(i, j), getNode(i, j + 2), BENDING_COEF));
				}
			}
			//for (int i = 0; i < NodesInWidth; i++) {
			//	for (int j = 0; j < NodesInHeight; j++) {
			//		// Structural
			//		if (i < NodesInWidth - 1) Springs.push_back(new Spring(getNode(i, j), getNode(i + 1, j), STRUCTURE_COEF));
			//		if (j < NodesInHeight - 1) Springs.push_back(new Spring(getNode(i, j), getNode(i, j + 1), STRUCTURE_COEF));
			//		// Shear 
			//		if (i < NodesInWidth - 1 && j < NodesInHeight - 1)
			//		{
			//			Springs.push_back(new Spring(getNode(i, j), getNode(i + 1, j + 1), SHEAR_COEF));
			//			Springs.push_back(new Spring(getNode(i + 1, j), getNode(i, j + 1), SHEAR_COEF));
			//		}
			//		// Bending
			//		if (i < NodesInWidth - 2) Springs.push_back(new Spring(getNode(i, j), getNode(i + 2, j), BENDING_COEF));
			//		if (j < NodesInHeight - 2) Springs.push_back(new Spring(getNode(i, j), getNode(i, j + 2), BENDING_COEF));
			//	}
			//}
			printf("Cloth has %i springs.\n", Springs.size());
		}
	}

	void initFaces()
	{
		for (int w = 0; w < NodesInWidth - 1; w++)
		{
			for (int h = 0; h < NodesInHeight - 1; h++)
			{
				Node* node0 = getNode(w, h);
				Node* node1 = getNode(w + 1, h);
				Node* node2 = getNode(w, h + 1);
				Node* node3 = getNode(w + 1, h + 1);
				// Left upper triangle
				Faces.push_back(node0);
				Faces.push_back(node1);
				Faces.push_back(node2);
				// Right bottom triangle
				Faces.push_back(node2);
				Faces.push_back(node1);
				Faces.push_back(node3);
			}
		}
	}

	void initConstraints()
	{
		// Distance constraints
		for (int w = 0; w < NodesInWidth; w++)
		{
			for (int h = 0; h < NodesInHeight; h++)
			{
				// Each edges have a distance constraint
				if (w < NodesInWidth - 1) { MakeConstraint(getNode(w, h), getNode(w + 1, h), DISTANCE_COMPLIANCE); }
				if (h < NodesInHeight - 1) { MakeConstraint(getNode(w, h), getNode(w, h + 1), DISTANCE_COMPLIANCE); }
				if (w + 1 < NodesInWidth && h + 1 < NodesInHeight)
				{
					MakeConstraint(getNode(w + 1, h), getNode(w, h + 1), DISTANCE_COMPLIANCE);
					MakeConstraint(getNode(w, h), getNode(w + 1, h + 1), DISTANCE_COMPLIANCE);
				}
			}
		}
		// Bending constraints
		if (ConstraintLevel > 0)
		{
			for (int w = 0; w < NodesInWidth; w++)
			{
				for (int h = 0; h < NodesInHeight; h++)
				{
					if (ConstraintLevel != 2)
					{
						if (w < NodesInWidth - 2) { MakeConstraint(getNode(w, h), getNode(w + 2, h), BENDING_COMPLIANCE); }
						if (h < NodesInHeight - 2) { MakeConstraint(getNode(w, h), getNode(w, h + 2), BENDING_COMPLIANCE); }
					}

					if (ConstraintLevel != 1)
					{
						if (w < NodesInWidth - 2 && h < NodesInHeight - 2)
						{
							MakeConstraint(getNode(w, h), getNode(w + 2, h + 2), BENDING_COMPLIANCE);
							MakeConstraint(getNode(w + 2, h), getNode(w, h + 2), BENDING_COMPLIANCE);
						}
					}
				}
			}
		}
		// The constraints solve order will affect the simulation result,
		// so use shuffle with determined seed to avoid this situation.
		auto rng = std::default_random_engine{ 15162428 };
		std::shuffle(std::begin(Constraints), std::end(Constraints), rng);
		printf("Total constraints number: %d\n", Constraints.size());
	}

	void Destroy()
	{
		for (int i = 0; i < Nodes.size(); i++) { delete Nodes[i]; }
		for (int i = 0; i < Springs.size(); i++) { delete Springs[i]; }
		Nodes.clear();
		Faces.clear();
		Springs.clear();
		Constraints.clear();
	}
};