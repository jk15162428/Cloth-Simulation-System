#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

// for cloth
class Node
{
public:
	glm::vec<3, GLdouble> Position;
	glm::vec<3, GLdouble> Velocity;
	glm::vec<3, GLdouble> Acceleration;
	glm::vec2 TextureCoord;
	glm::vec<3, GLdouble> Normal;       // for shading

	/** for XPBD **/
	GLdouble InvMass;			// inverse mass, i.e. w = 1 / mass
	glm::vec<3, GLdouble> OldPosition;
	/** end of for XPBD **/

	Node() {}
	Node(GLdouble invMass, glm::vec<3, GLdouble> position, glm::vec<3, GLdouble> acceleration = glm::vec<3, GLdouble>(0.0f, 0.0f, 0.0f))
	{
		Position = position;
		Velocity = glm::vec<3, GLdouble>(0.0f, 0.0f, 0.0f);
		Acceleration = acceleration;
		TextureCoord = glm::vec2(0.0f, 0.0f);
		Normal = glm::vec<3, GLdouble>(0.0f, 0.0f, 0.0f);
		InvMass = invMass; // for simplicity, all nodes' mass is 1 for default
		OldPosition = Position;
	}
	~Node() {}

	void Integrate(GLdouble dt)
	{
		Velocity += Acceleration * dt;
		OldPosition = Position;
		Position += Velocity * dt;
		// printf("Velocity: [%f, %f]\n", Velocity.x, Velocity.y);
		//printf("v: [%f, %f, %f], dx: [%f, %f, %f], x: [%f, %f, %f] \n", 
		//	Velocity.x, Velocity.y, Velocity.z, (Velocity * dt).x, (Velocity * dt).y, (Velocity * dt).z,
		//	Position.x, Position.y, Position.z);
	}
};