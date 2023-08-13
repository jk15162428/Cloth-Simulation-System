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
	glm::vec<3, GLdouble> Normal;         // for shading

	/** for XPBD **/
	GLdouble InvMass;			          // inverse mass, i.e. w = 1 / mass
	glm::vec<3, GLdouble> OldPosition;
	/** end of for XPBD **/

	/** for mass-spring system **/
	glm::vec<3, double> Force;
	void addForce(glm::vec<3, double> f)
	{
		Force += f;
	}
	/** end of for mass-spring system **/

	Node() {}
	Node(GLdouble invMass, glm::vec<3, GLdouble> position, glm::vec<3, GLdouble> acceleration = glm::vec<3, GLdouble>(0.0f, 0.0f, 0.0f))
	{
		Position = position;
		Velocity = glm::vec<3, GLdouble>(0.0f, 0.0f, 0.0f);
		Acceleration = acceleration;
		TextureCoord = glm::vec2(0.0f, 0.0f);
		Normal = glm::vec<3, GLdouble>(0.0f, 0.0f, 0.0f);
		InvMass = invMass;
		OldPosition = Position;
		Force = glm::vec<3, double>(0, 0, 0);
	}
	~Node() {}
};