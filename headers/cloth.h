#pragma once
#include <vector>
#include <stdio.h>
#include <iostream>
#include <random>
#include "node.h"
#include "constraint.h"

enum VelocityUpdate {
	VEL_UP,
	VEL_DOWN,
	VEL_LEFT,
	VEL_RIGHT
};

class Cloth
{
private:
	const GLdouble DEFAULT_INVMASS = 5.0;
	const GLdouble BENDING_CONSTRAINT = 1.0;
	const GLdouble FORCE = 1.0; // used in velocity update using keyboard
	const glm::vec<3, GLdouble> gravity = glm::vec<3, GLdouble>(0.0, -10.0, 0.0);

public:
	int Iteration; // substep in XPBD
	glm::vec<3, GLdouble> ClothPosition;
	int Width, Height;
	int NodesInWidth, NodesInHeight;

	enum DrawModeEnum {
		DRAW_NODES = 0,
		DRAW_LINES = 1,
		DRAW_FACES = 2
	};
	DrawModeEnum drawMode = DRAW_FACES;

	std::vector<Node*> Nodes;
	std::vector<Constraint> Constraints; // for PBD & XPBD
	//std::vector<Spring*> springs; // for mass-spring system
	std::vector<Node*> Faces; // for rendering

	Cloth(glm::vec3 position, glm::vec2 size, glm::vec2 nodesNumber, int iteration = 5)
	{
		ClothPosition = position;
		Width = size.x;
		Height = size.y;
		NodesInWidth = nodesNumber.x;
		NodesInHeight = nodesNumber.y;
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
		for (int i = 0; i < Nodes.size(); i++) {
			Nodes[i]->Normal = normal;
		}
		/** Compute normal of each face **/
		for (int i = 0; i < Faces.size() / 3; i++) { // 3 nodes in each face
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

		for (int i = 0; i < Nodes.size(); i++) {
			Nodes[i]->Normal = glm::normalize(Nodes[i]->Normal);
		}
	}

	void Integrate(GLdouble dt)
	{
		/** Node **/
		for (int i = 0; i < Nodes.size(); i++)
		{
			if (Nodes[i]->InvMass == 0.0)
				continue;
			Nodes[i]->Integrate(dt);
		}
		for (int i = 0; i < Constraints.size(); i++)
		{
			Constraints[i].SetLambda(0.0f);
			Constraints[i].Solve(dt);
		}
		for (int i = 0; i < Nodes.size(); i++)
		{
			if (Nodes[i]->InvMass == 0.0f)
				continue;
			Nodes[i]->Velocity = (Nodes[i]->Position - Nodes[i]->OldPosition) * 1.0 / dt;
		}
	}

	glm::vec3 getWorldPos(Node* n) { return ClothPosition + n->Position; }
	void setWorldPos(Node* n, glm::vec<3, GLdouble> position) { n->Position = position - ClothPosition; }
	void reset() { Destroy();  init(); }
	void UpdateVelocity(VelocityUpdate update)
	{
		for (int i = 0; i < Nodes.size(); i++)
		{
			if (Nodes[i]->InvMass == 0) continue;
			switch (update)
			{
				case VEL_UP:
					Nodes[i]->Velocity.z += FORCE * Nodes[i]->InvMass;
					break;
				case VEL_DOWN:
					Nodes[i]->Velocity.z -= FORCE * Nodes[i]->InvMass;
					break;
				case VEL_LEFT:
					Nodes[i]->Velocity.x -= FORCE * Nodes[i]->InvMass;
					Nodes[i]->Velocity.z -= FORCE * Nodes[i]->InvMass / 100;
					break;
				case VEL_RIGHT:
					Nodes[i]->Velocity.x += FORCE * Nodes[i]->InvMass;
					Nodes[i]->Velocity.z -= FORCE * Nodes[i]->InvMass / 100;
					break;
			}
		}
	}
private:
	void MakeConstraint(Node* n1, Node* n2, GLdouble compliance = 0.0f) { Constraints.push_back(Constraint(n1, n2, compliance)); }

	void init()
	{
		/** Add nodes **/
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

		/*
		for (int i = 0; i < nodesPerRow; i++) {
			for (int j = 0; j < nodesPerCol; j++) {

				if (i < nodesPerRow - 1) springs.push_back(new Spring(getNode(i, j), getNode(i + 1, j), structuralCoef));
				if (j < nodesPerCol - 1) springs.push_back(new Spring(getNode(i, j), getNode(i, j + 1), structuralCoef));

				if (i < nodesPerRow - 1 && j < nodesPerCol - 1) {
					springs.push_back(new Spring(getNode(i, j), getNode(i + 1, j + 1), shearCoef));
					springs.push_back(new Spring(getNode(i + 1, j), getNode(i, j + 1), shearCoef));
				}
				if (i < nodesPerRow - 2) springs.push_back(new Spring(getNode(i, j), getNode(i + 2, j), bendingCoef));
				if (j < nodesPerCol - 2) springs.push_back(new Spring(getNode(i, j), getNode(i, j + 2), bendingCoef));
			}
		}
		*/

		/** Triangle faces **/
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
				/*
				std::cout << node0 << " " << node1 << " " << node2 << " " << node3 << " " << std::endl;
				printf("\t%d: [%d, %d] (%f, %f, %f)\n", h * NodesInWidth + w, w, h, node0->Position.x, node0->Position.y, node0->Position.z);
				printf("\t%d: [%d, %d] (%f, %f, %f)\n", h * NodesInWidth + (w + 1), w+1, h, node1->Position.x, node1->Position.y, node1->Position.z);
				printf("\t%d: [%d, %d] (%f, %f, %f)\n", (h + 1) * NodesInWidth + w, w, h+1, node2->Position.x, node2->Position.y, node2->Position.z);
				printf("\t%d: [%d, %d] (%f, %f, %f)\n", (h + 1) * NodesInWidth + w + 1, w+1, h + 1,node3->Position.x, node3->Position.y, node3->Position.z);
				std::cout << std::endl;
				*/
			}
		}
		// Note: initConstraints 3 times may help, but I don't know why, need to fix
		initConstraints();
		auto rng = std::default_random_engine{};
		std::shuffle(std::begin(Constraints), std::end(Constraints), rng);
		printf("Constraints number: %d\n", Constraints.size());
	}

	void initConstraints()
	{
		// Distance constraints
		for (int w = 0; w < NodesInWidth; w++)
		{
			for (int h = 0; h < NodesInHeight; h++)
			{
				// Each edges have a distance constraint
				if (w < NodesInWidth - 1) { MakeConstraint(getNode(w, h), getNode(w + 1, h)); }
				if (h < NodesInHeight - 1) { MakeConstraint(getNode(w, h), getNode(w, h + 1)); }

				if (w + 1 < NodesInWidth && h + 1< NodesInHeight)
				{
					MakeConstraint(getNode(w + 1, h), getNode(w, h + 1));
				}

				if (w + 1 < NodesInWidth && h + 1 < NodesInHeight)
				{
					MakeConstraint(getNode(w, h), getNode(w + 1, h + 1));
				}
			}
		}
		// Bending constraints
		for (int w = 0; w < NodesInWidth; w++)
		{
			for (int h = 0; h < NodesInHeight; h++)
			{
				// Copied from github xpbd, but not correct for me.
				
				if (w < NodesInWidth - 2) { MakeConstraint(getNode(w, h), getNode(w + 2, h), BENDING_CONSTRAINT); }
				if (h < NodesInHeight - 2) { MakeConstraint(getNode(w, h), getNode(w, h + 2), BENDING_CONSTRAINT); }
				if (w < NodesInWidth - 2 && h < NodesInHeight - 2)
				{
					MakeConstraint(getNode(w, h), getNode(w + 2, h + 2), BENDING_CONSTRAINT);
					MakeConstraint(getNode(w + 2, h), getNode(w, h + 2), BENDING_CONSTRAINT);
				}
				
				// why adding bending_constraint will make the whole things crap
				// Correct bending constraints
				/*
				if (w + 1 < NodesInWidth && h + 2 < NodesInHeight)
				{
					MakeConstraint(getNode(w + 1, h), getNode(w, h + 2), BENDING_CONSTRAINT);
				}

				if (w + 2 < NodesInWidth && h + 1 < NodesInHeight)
				{
					MakeConstraint(getNode(w, h + 1), getNode(w + 2, h), BENDING_CONSTRAINT);
				}
				*/
			}
		}
	}

	// This is one correct implementation !before , but without the ball and ground and also the wind, etc.
	// now the constraints looks more good, I add a shuffle.......???haha
	void CORRECT_initConstraints()
	{
		// Distance constraints
		for (int w = 0; w < NodesInWidth; w++)
		{
			for (int h = 0; h < NodesInHeight; h++)
			{
				// Each edges have a distance constraint
				if (w < NodesInWidth - 1) { MakeConstraint(getNode(w, h), getNode(w + 1, h)); }
				if (h < NodesInHeight - 1) { MakeConstraint(getNode(w, h), getNode(w, h + 1)); }

				if (w < NodesInWidth - 1 && h < NodesInHeight - 1)
				{
					MakeConstraint(getNode(w + 1, h), getNode(w, h + 1));
				}
			}
		}
		// Bending constraints
		for (int w = 0; w < NodesInWidth; w++)
		{
			for (int h = 0; h < NodesInHeight; h++)
			{
				// Copied from github xpbd, but not correct for me.
				/*
				if (w < NodesInWidth - 2) { MakeConstraint(getNode(w, h), getNode(w + 2, h), BENDING_CONSTRAINT); }
				if (h < NodesInHeight - 2) { MakeConstraint(getNode(w, h), getNode(w, h + 2), BENDING_CONSTRAINT); }
				if (w < NodesInWidth - 2 && h < NodesInHeight - 2)
				{
					MakeConstraint(getNode(w, h), getNode(w + 2, h + 2), BENDING_CONSTRAINT);
					MakeConstraint(getNode(w + 2, h), getNode(w, h + 2), BENDING_CONSTRAINT);
				}
				*/


				// Correct bending constraints
				if (w + 1 < NodesInWidth && h + 1 < NodesInHeight)
				{
					MakeConstraint(getNode(w, h), getNode(w + 1, h + 1));
				}
				if (w + 1 < NodesInWidth && h + 2 < NodesInHeight)
				{
					MakeConstraint(getNode(w + 1, h), getNode(w, h + 2));
				}

				if (w + 2 < NodesInWidth && h + 1 < NodesInHeight)
				{
					MakeConstraint(getNode(w, h + 1), getNode(w + 2, h), BENDING_CONSTRAINT);
				}


			}
		}
	}
	void Destroy()
	{
		for (int i = 0; i < Nodes.size(); i++) { delete Nodes[i]; }
		//for (int i = 0; i < springs.size(); i++) { delete springs[i]; }
		Nodes.clear();
		//springs.clear();
		Faces.clear();
		Constraints.clear();
	}
};