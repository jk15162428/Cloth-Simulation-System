#pragma once
#include <vector>
#include <stdio.h>
#include <iostream>
#include <random>
#include "node.h"
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
	const GLdouble DEFAULT_INVMASS = 5.0;
	const GLdouble DISTANCE_COMPLIANCE = 0.0;
	const GLdouble BENDING_COMPLIANCE = 1.0;
	const glm::vec<3, GLdouble> gravity = glm::vec<3, GLdouble>(0.0, -10.0, 0.0);

public:
	int Iteration; // substep in PPBD
	glm::vec<3, GLdouble> ClothPosition;
	int Width, Height;
	int NodesInWidth, NodesInHeight;
	MethodEnum Method = PPBD;
	const GLdouble DEFAULT_FORCE = 1.0; // used in velocity update with keyboard

	enum DrawModeEnum 
	{
		DRAW_NODES = 0,
		DRAW_LINES = 1,
		DRAW_FACES = 2
	};
	DrawModeEnum drawMode = DRAW_FACES;

	std::vector<Node*> Nodes;
	std::vector<Constraint> Constraints; // for PBD & PPBD
	std::vector<Node*> Faces; // for rendering

	Cloth() {}
	Cloth(glm::vec3 position, glm::vec2 size, glm::vec2 nodesNumber, MethodEnum method, int iteration = 5)
	{
		ClothPosition = position;
		Width = size.x;
		Height = size.y;
		NodesInWidth = nodesNumber.x;
		NodesInHeight = nodesNumber.y;
		Method = method;
		Iteration = iteration;
		init();
	}
	// just a dummy version of copy constructor
	void set(glm::vec3 position, glm::vec2 size, glm::vec2 nodesNumber, MethodEnum method, int iteration = 5)
	{
		ClothPosition = position;
		Width = size.x;
		Height = size.y;
		NodesInWidth = nodesNumber.x;
		NodesInHeight = nodesNumber.y;
		Method = method;
		Iteration = iteration;
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
		/** Node **/
		for (int i = 0; i < Nodes.size(); i++)
		{
			if (Nodes[i]->InvMass == 0.0)
				continue;
			Nodes[i]->Velocity += Nodes[i]->Acceleration * dt;
			Nodes[i]->OldPosition = Nodes[i]->Position;
			Nodes[i]->Position += Nodes[i]->Velocity * dt;
		}
		
		if (Method == PPBD || Method == PBD) 
		{
			for (int i = 0; i < Constraints.size(); i++)
				Constraints[i].SetLambda(0.0f);
			for (int n = 0; n < Iteration; n++)
			{
				for (int i = 0; i < Constraints.size(); i++)
				{
					Constraints[i].Solve(dt, Method);
				}
			}
		}

		if (Method == PPBD_SS)
		{
			for (int i = 0; i < Constraints.size(); i++)
			{
				Constraints[i].Solve(dt, Method);
			}
		}

		for (int i = 0; i < Nodes.size(); i++)
		{
			if (Nodes[i]->InvMass == 0.0f)
				continue;	
			Nodes[i]->Velocity = (Nodes[i]->Position - Nodes[i]->OldPosition) * 1.0 / dt;
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
		printf("Actual cloth has %i nodes\n", Nodes.size());
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
				if (w + 1 < NodesInWidth && h + 1< NodesInHeight)
				{
					MakeConstraint(getNode(w + 1, h), getNode(w, h + 1), DISTANCE_COMPLIANCE);
					MakeConstraint(getNode(w, h), getNode(w + 1, h + 1), DISTANCE_COMPLIANCE);
				}
			}
		}
		// Bending constraints
		for (int w = 0; w < NodesInWidth; w++)
		{
			for (int h = 0; h < NodesInHeight; h++)
			{
				if (w < NodesInWidth - 2) { MakeConstraint(getNode(w, h), getNode(w + 2, h), BENDING_COMPLIANCE); }
				if (h < NodesInHeight - 2) { MakeConstraint(getNode(w, h), getNode(w, h + 2), BENDING_COMPLIANCE); }
				
				if (w < NodesInWidth - 2 && h < NodesInHeight - 2)
				{
					MakeConstraint(getNode(w, h), getNode(w + 2, h + 2), BENDING_COMPLIANCE);
					MakeConstraint(getNode(w + 2, h), getNode(w, h + 2), BENDING_COMPLIANCE);
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
		Nodes.clear();
		Faces.clear();
		Constraints.clear();
	}
};