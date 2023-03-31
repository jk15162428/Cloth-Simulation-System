#pragma once
#include <iostream>
#include <stdio.h>
#include "cloth.h"
#include "shader.h"
#include "camera.h"

// public domain image loader
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/** constant variable **/
const int WIDTH = 1600;
const int HEIGHT = 900;
const std::string TEXTURE_PATH = "textures/cloth1.jpg";
const std::string TEXTURE2_PATH = "textures/tex1.jpg";
const std::string CLOTH_VERTEX_PATH = "shaders/cloth.vs";
const std::string CLOTH_FRAGMENT_PATH = "shaders/cloth.fs";
const std::string TEXT_VERTEX_PATH = "shaders/text.vs";
const std::string TEXT_FRAGMENT_PATH = "shaders/text.fs";
const std::string FONT_PATH = "fonts/arial.ttf";
/** end of constant variable**/

int loadTexture(const std::string texturePath);

class Light
{
public:
	glm::vec3 Position = glm::vec3(-5.0f, 7.0f, -5.0f);
	glm::vec3 Color = glm::vec3(0.7f, 0.7f, 1.0f);
};
Light sun;

class ClothRenderer
{
private:
	const unsigned int aPtrPosition = 0, aPtrTexture = 1, aPtrNormal = 2;

public:
	Cloth* ClothObject;
	int NodeCount;

	glm::vec3* VertexBufferObjectsPosition;
	glm::vec2* VertexBufferObjectsTexture;
	glm::vec3* VertexBufferObjectsNormal;

	unsigned int ShaderProgramID;
	unsigned int VertexArrayObjectsID; // VAO
	unsigned int VertexBufferObjectsIDs[3]; // VBO
	int Texture1, Texture2;

	ClothRenderer() {}
	void init(Cloth* cloth)
	{
		NodeCount = (int)(cloth->Faces.size());
		if (NodeCount <= 0) {
			std::cout << "ERROR::ClothRender : No node exists." << std::endl;
			exit(-1);
		}
		this->ClothObject = cloth;

		VertexBufferObjectsPosition = new glm::vec3[NodeCount];
		VertexBufferObjectsTexture = new glm::vec2[NodeCount];
		VertexBufferObjectsNormal = new glm::vec3[NodeCount];

		for (int i = 0; i < NodeCount; i++) {
			Node* n = cloth->Faces[i];
			VertexBufferObjectsPosition[i] = glm::vec3(n->Position.x, n->Position.y, n->Position.z);
			VertexBufferObjectsTexture[i] = glm::vec2(n->TextureCoord.x, n->TextureCoord.y); // Texture coord will only be set here
			VertexBufferObjectsNormal[i] = glm::vec3(n->Normal.x, n->Normal.y, n->Normal.z);
		}

		// Build render program
		Shader clothShader(CLOTH_VERTEX_PATH.c_str(), CLOTH_FRAGMENT_PATH.c_str());
		ShaderProgramID = clothShader.ID;
		std::cout << "Cloth Shader Program ID: " << ShaderProgramID << std::endl;

		/** binding and setting VAO and VBO **/
		// 1. Bind VAO
		// 2. Copy our vertices array in a buffer for OpenGL to use
		// 3. Set the vertex attributes pointers
		glGenVertexArrays(1, &VertexArrayObjectsID);
		glGenBuffers(3, VertexBufferObjectsIDs);
		glBindVertexArray(VertexArrayObjectsID);

		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObjectsIDs[0]);
		glBufferData(GL_ARRAY_BUFFER, NodeCount * sizeof(glm::vec3), VertexBufferObjectsPosition, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(aPtrPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObjectsIDs[1]);
		glBufferData(GL_ARRAY_BUFFER, NodeCount * sizeof(glm::vec2), VertexBufferObjectsTexture, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(aPtrTexture, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObjectsIDs[2]);
		glBufferData(GL_ARRAY_BUFFER, NodeCount * sizeof(glm::vec3), VertexBufferObjectsNormal, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(aPtrNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(aPtrPosition);
		glEnableVertexAttribArray(aPtrTexture);
		glEnableVertexAttribArray(aPtrNormal);
		/** end of binding and setting VAO and VBO **/

		Texture1 = loadTexture(TEXTURE_PATH);
		Texture2 = loadTexture(TEXTURE2_PATH);

		// activate/use the shader before setting uniforms
		clothShader.use();
		clothShader.setInt("Texture1", 0);
		clothShader.setInt("Texture2", 1);

		// Model Matrix : Put cloth into the world
		// Model matrix won't change, so we set it here.
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(cloth->ClothPosition.x, cloth->ClothPosition.y, cloth->ClothPosition.z));
		clothShader.setMat4("model", modelMatrix);

		// Light
		clothShader.setVec3("lightPosition", sun.Position);
		clothShader.setVec3("lightColor", sun.Color);

		// Clean
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void render()
	{
		// Update all the positions of nodes
		for (int i = 0; i < NodeCount; i++) { // Tex coordinate dose not change
			Node* n = ClothObject->Faces[i];
			VertexBufferObjectsPosition[i] = glm::vec3(n->Position.x, n->Position.y, n->Position.z);
			VertexBufferObjectsNormal[i] = glm::vec3(n->Normal.x, n->Normal.y, n->Normal.z);
		}

		glUseProgram(ShaderProgramID);

		glBindVertexArray(VertexArrayObjectsID);

		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObjectsIDs[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, NodeCount * sizeof(glm::vec3), VertexBufferObjectsPosition);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObjectsIDs[1]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, NodeCount * sizeof(glm::vec2), VertexBufferObjectsTexture);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObjectsIDs[2]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, NodeCount * sizeof(glm::vec3), VertexBufferObjectsNormal);

		// binding texture 
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);

		// projection matrix
		glUniformMatrix4fv(glGetUniformLocation(ShaderProgramID, "projection"), 1, GL_FALSE, &camera.GetProjectionMatrix()[0][0]);

		// View Matrix : The camera
		glUniformMatrix4fv(glGetUniformLocation(ShaderProgramID, "view"), 1, GL_FALSE, &camera.GetViewMatrix()[0][0]);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// setting drawmode
		switch (ClothObject->drawMode)
		{
		case Cloth::DRAW_NODES:
			glDrawArrays(GL_POINTS, 0, NodeCount);
			break;
		case Cloth::DRAW_LINES:
			glDrawArrays(GL_LINES, 0, NodeCount);
			break;
		default:
			glDrawArrays(GL_TRIANGLES, 0, NodeCount);
			break;
		}

		// End of rendering
		// glDisable(GL_BLEND);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);
	}
};

struct Character
{
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
};

class TextRenderer
{
public: 
	std::map<GLchar, Character> Characters; 
	Shader shader;
	unsigned int VAO, VBO;
	TextRenderer(){};
	void init(int fontSize)
	{
		shader = Shader(TEXT_VERTEX_PATH.c_str(), TEXT_FRAGMENT_PATH.c_str());
		glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(WIDTH), 0.0f, static_cast<float>(HEIGHT));
		shader.use();
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		FT_Library FreeType;
		if (FT_Init_FreeType(&FreeType))
		{
			std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
			exit(-1);
		}

		FT_Face FTface;
		if (FT_New_Face(FreeType, FONT_PATH.c_str(), 0, &FTface))
		{
			std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
			exit(-1);
		}

		// The function sets the font's width and height parameters. 
		// Setting the width to 0 lets the face dynamically calculate the width based on the given height.
		FT_Set_Pixel_Sizes(FTface, 0, fontSize);

		// disable byte-alignment restriction
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		for (unsigned char c = 0; c < 128; c++)
		{
			// load character glyph 
			if (FT_Load_Char(FTface, c, FT_LOAD_RENDER))
			{
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
				continue;
			}
			// generate texture
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				FTface->glyph->bitmap.width,
				FTface->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				FTface->glyph->bitmap.buffer
			);
			// set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// now store character for later use
			Character character = {
				texture,
				glm::ivec2(FTface->glyph->bitmap.width, FTface->glyph->bitmap.rows),
				glm::ivec2(FTface->glyph->bitmap_left, FTface->glyph->bitmap_top),
				static_cast<unsigned int>(FTface->glyph->advance.x)
			};
			Characters.insert(std::pair<char, Character>(c, character));
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		FT_Done_Face(FTface);
		FT_Done_FreeType(FreeType);

		// configure VAO/VBO
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void RenderText(std::string text, float x, float y, float scale, glm::vec3 color)
	{
		// activate corresponding render state	
		shader.use();
		glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(VAO);

		// iterate through all characters
		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++)
		{
			Character ch = Characters[*c];

			float xpos = x + ch.Bearing.x * scale;
			float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

			float w = ch.Size.x * scale;
			float h = ch.Size.y * scale;
			// update VBO for each character
			float vertices[6][4] = {
				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos,     ypos,       0.0f, 1.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },

				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },
				{ xpos + w, ypos + h,   1.0f, 0.0f }
			};
			// render glyph texture over quad
			glBindTexture(GL_TEXTURE_2D, ch.TextureID);
			// update content of VBO memory
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			// render quad
			glDrawArrays(GL_TRIANGLES, 0, 6);
			// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
		}
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
};

int loadTexture(const std::string texturePath)
{
	unsigned int texture;
	// Assign texture ID and gengeration
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// Set the texture wrapping parameters (for 2D tex: S, T)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering parameters (Minify, Magnify)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/** Load image and configure texture **/
	// stbi_set_flip_vertically_on_load(true); // upside down
	int texWidth, texHeight, colorChannels; // After loading the image, stb_image will fill them
	std::cout << "Try to load texture: " << texturePath.c_str() << std::endl;
	unsigned char* data = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &colorChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		// Automatically generate all the required mipmaps for the currently bound texture.
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	// Always free image memory
	stbi_image_free(data);
	std::cout << "Texture loaded successfully, Texture ID: " << texture << std::endl;
	return texture;
}