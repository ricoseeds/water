#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>

#define GLEW_STATIC
#include "GL/glew.h" // Important - this header must come before glfw3 header
#include "glm/glm.hpp"

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
};

class Mesh
{
  public:
	Mesh();
	~Mesh();

	bool loadOBJ(const std::string &filename);
	bool loadOBJother(const std::string &filename, bool liv_flag);

	void draw();
	void make_displacement();

  private:
	void initBuffers();

	bool mLoaded;
	std::vector<Vertex> mVertices;
	GLuint mVBO, mVAO;
};
#endif //MESH_H
