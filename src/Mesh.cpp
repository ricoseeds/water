#include "../include/stb_image.h"
#include "../include/Mesh.h"
#include <iostream>
#include <sstream>
#include <fstream>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

//-----------------------------------------------------------------------------
// split
//
// Params:  s - string to split
//		    t - string to split (ie. delimiter)
//
//Result:  Splits string according to some substring and returns it as a vector.
//-----------------------------------------------------------------------------
std::vector<std::string> split(std::string s, std::string t)
{
	std::vector<std::string> res;
	while (1)
	{
		int pos = s.find(t);
		if (pos == -1)
		{
			res.push_back(s);
			break;
		}
		res.push_back(s.substr(0, pos));
		s = s.substr(pos + 1, s.size() - pos - 1);
	}
	return res;
}

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
Mesh::Mesh()
	: mLoaded(false)
{
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
}

//-----------------------------------------------------------------------------
// Loads a Wavefront OBJ model
//
// NOTE: This is not a complete, full featured OBJ loader.  It is greatly
// simplified.
// Assumptions!
//  - OBJ file must contain only triangles
//  - We ignore materials
//  - We ignore normals
//  - only commands "v", "vt" and "f" are supported
//-----------------------------------------------------------------------------
bool Mesh::loadOBJother(const std::string &filename, bool liv_flag)
{
	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> tempVertices;
	std::vector<glm::vec2> tempUVs;
	std::vector<glm::vec3> tempNormals;
	std::vector<glm::vec3> v_normal;
	// glm::vec3 *v_normal = new glm::vec3[10](); //TODO FIX

	if (filename.find(".obj") != std::string::npos)
	{
		std::ifstream fin(filename, std::ios::in);
		if (!fin)
		{
			std::cerr << "Cannot open " << filename << std::endl;
			return false;
		}

		std::cout << "Loading OBJ file " << filename << " ..." << std::endl;

		std::string lineBuffer;
		while (std::getline(fin, lineBuffer))
		{
			std::stringstream ss(lineBuffer);
			std::string cmd;
			ss >> cmd;

			if (cmd == "v")
			{
				glm::vec3 vertex;
				int dim = 0;
				while (dim < 3 && ss >> vertex[dim])
					dim++;

				tempVertices.push_back(vertex);
			}
			else if (cmd == "vt")
			{
				glm::vec2 uv;
				int dim = 0;
				while (dim < 2 && ss >> uv[dim])
					dim++;

				tempUVs.push_back(uv);
			}
			else if (cmd == "vn")
			{
				glm::vec3 normal;
				int dim = 0;
				while (dim < 3 && ss >> normal[dim])
					dim++;
				normal = glm::normalize(normal);
				tempNormals.push_back(normal);
			}
			else if (cmd == "f")
			{
				std::string faceData;
				int vertexIndex, uvIndex, normalIndex;

				while (ss >> faceData)
				{
					std::vector<std::string> data = split(faceData, "/");

					if (data[0].size() > 0)
					{
						sscanf(data[0].c_str(), "%d", &vertexIndex);
						if (liv_flag)
						{
							vertexIndices.push_back(vertexIndex + 1);
						}
						else
						{
							vertexIndices.push_back(vertexIndex);
						}
					}

					if (data.size() >= 1)
					{
						// Is face format v//vn?  If data[1] is empty string then
						// this vertex has no texture coordinate
						if (data[1].size() > 0)
						{
							sscanf(data[1].c_str(), "%d", &uvIndex);
							uvIndices.push_back(uvIndex);
						}
					}

					if (data.size() >= 2)
					{
						// Does this vertex have a normal?
						if (data[2].size() > 0)
						{
							sscanf(data[2].c_str(), "%d", &normalIndex);
							normalIndices.push_back(normalIndex);
						}
					}
				}
				//here
				// unsigned int last_index = vertexIndices[vertexIndices.size() - 1];
				// unsigned int second_last_index = vertexIndices[vertexIndices.size() - 2];
				// unsigned int third_last_index = vertexIndices[vertexIndices.size() - 3];
				// for (size_t i = 0; i < v_normal->length(); i++)
				// {
				// 	std::cout << v_normal->length() << "\n";
				// }

				// std::cout << last_index << " " << second_last_index << " " << third_last_index << "\n";
			}
		}

		// Close the file
		fin.close();
		for (int i = 0; i != tempVertices.size(); i++)
		{
			v_normal.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
		}

		std::cout << "\nNO OF VERT : " << v_normal.size() << "\n";

		//here TODO
		std::cout << "\nNO OF FACES : " << vertexIndices.size() / 3 << "\n";

		for (unsigned int i = 0; i < vertexIndices.size(); i += 3)
		{
			//i (i+1) (i+2) are the three faces of a triangle
			glm::vec3 A = tempVertices[vertexIndices[i] - 1];
			glm::vec3 B = tempVertices[vertexIndices[i + 1] - 1];
			glm::vec3 C = tempVertices[vertexIndices[i + 2] - 1];
			glm::vec3 AB = B - A;
			glm::vec3 AC = C - A;
			// glm::vec3 ABxAC = glm::normalize(glm::cross(AB, AC));
			glm::vec3 ABxAC = glm::cross(AB, AC);
			// std::cout << glm::to_string(ABxAC) << std::endl;
			v_normal[vertexIndices[i] - 1] += ABxAC;
			v_normal[vertexIndices[i + 1] - 1] += ABxAC;
			v_normal[vertexIndices[i + 2] - 1] += ABxAC;
		}

		// For each vertex of each triangle
		for (unsigned int i = 0; i < vertexIndices.size(); i++)
		{
			Vertex meshVertex;

			// Get the attributes using the indices

			if (tempVertices.size() > 0)
			{
				glm::vec3 vertex = tempVertices[vertexIndices[i] - 1];
				meshVertex.position = vertex;
			}

			// if (tempNormals.size() > 0)
			// {
			glm::vec3 normal = v_normal[vertexIndices[i] - 1];
			meshVertex.normal = glm::normalize(normal);
			// }

			if (tempUVs.size() > 0)
			{
				glm::vec2 uv = tempUVs[uvIndices[i] - 1];
				meshVertex.texCoords = uv;
			}

			mVertices.push_back(meshVertex);
		}

		// Create and initialize the buffers
		// initBuffers();

		for (size_t i = 0; i < mVertices.size(); i += 3)
		{
			glm::vec3 A = mVertices[i].position;
			glm::vec3 B = mVertices[i + 1].position;
			glm::vec3 C = mVertices[i + 2].position;
			glm::vec3 AB = B - A;
			glm::vec3 AC = C - A;
			glm::vec3 ABxAC = glm::normalize(glm::cross(AB, AC));
			mVertices[i].normal = ABxAC;
			mVertices[i + 1].normal = ABxAC;
			mVertices[i + 2].normal = ABxAC;
		}
		initBuffers();

		return (mLoaded = true);
	}

	// We shouldn't get here so return failure
	return false;
}

bool Mesh::loadOBJ(const std::string &filename)
{
	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> tempVertices;
	std::vector<glm::vec2> tempUVs;
	std::vector<glm::vec3> tempNormals;

	if (filename.find(".obj") != std::string::npos)
	{
		std::ifstream fin(filename, std::ios::in);
		if (!fin)
		{
			std::cerr << "Cannot open " << filename << std::endl;
			return false;
		}

		std::cout << "Loading OBJ file " << filename << " ..." << std::endl;

		std::string lineBuffer;
		while (std::getline(fin, lineBuffer))
		{
			std::stringstream ss(lineBuffer);
			std::string cmd;
			ss >> cmd;

			if (cmd == "v")
			{
				glm::vec3 vertex;
				int dim = 0;
				while (dim < 3 && ss >> vertex[dim])
					dim++;

				tempVertices.push_back(vertex);
			}
			else if (cmd == "vt")
			{
				glm::vec2 uv;
				int dim = 0;
				while (dim < 2 && ss >> uv[dim])
					dim++;

				tempUVs.push_back(uv);
			}
			else if (cmd == "vn")
			{
				glm::vec3 normal;
				int dim = 0;
				while (dim < 3 && ss >> normal[dim])
					dim++;
				normal = glm::normalize(normal);
				tempNormals.push_back(normal);
			}
			else if (cmd == "f")
			{
				std::string faceData;
				int vertexIndex, uvIndex, normalIndex;

				while (ss >> faceData)
				{
					std::vector<std::string> data = split(faceData, "/");

					if (data[0].size() > 0)
					{
						sscanf(data[0].c_str(), "%d", &vertexIndex);
						vertexIndices.push_back(vertexIndex);
					}

					if (data.size() >= 1)
					{
						// Is face format v//vn?  If data[1] is empty string then
						// this vertex has no texture coordinate
						if (data[1].size() > 0)
						{
							sscanf(data[1].c_str(), "%d", &uvIndex);
							uvIndices.push_back(uvIndex);
						}
					}

					if (data.size() >= 2)
					{
						// Does this vertex have a normal?
						if (data[2].size() > 0)
						{
							sscanf(data[2].c_str(), "%d", &normalIndex);
							normalIndices.push_back(normalIndex);
						}
					}
				}
			}
		}

		// Close the file
		fin.close();
		// int width, height, components;

		// // Use stbi image library to load our image
		// // unsigned char *imageData = stbi_load("/Users/arghachakraborty/Projects/wrath_of_khan/textures/iit.jpeg", &width, &height, &components, STBI_rgb_alpha);
		// unsigned char *imageData = stbi_load("/Users/arghachakraborty/Projects/wrath_of_khan/textures/perlin2.jpg", &width, &height, &components, STBI_rgb_alpha);
		// // unsigned char *imageData = stbi_load("/Users/arghachakraborty/Projects/wrath_of_khan/textures/craters.png", &width, &height, &components, STBI_rgb_alpha);
		// // unsigned char *imageData = stbi_load("/Users/arghachakraborty/Projects/wrath_of_khan/textures/tux.jpg", &width, &height, &components, STBI_rgb_alpha);
		// int widthInBytes = width * 4;
		// double grayscale, grayscale_h = 0.0, grayscale_l = 255;
		// double texel[height][width];
		// //changeable
		// double deform_low = -0.01;
		// double deform_high = 0.08;
		// for (int row = 0; row < height; row++)
		// {
		// 	for (int col = 0; col < width; col++)
		// 	{
		// 		unsigned char *pixelOffset = imageData + (row + height * col) * 4;

		// 		unsigned int r = pixelOffset[0];
		// 		unsigned int g = pixelOffset[1];
		// 		unsigned int b = pixelOffset[2];
		// 		grayscale = (0.2126f * r) + (0.7152f * g) + (0.0722f * b);
		// 		if (grayscale_h <= grayscale)
		// 		{
		// 			grayscale_h = grayscale;
		// 		}
		// 		if (grayscale_l >= grayscale)
		// 		{
		// 			grayscale_l = grayscale;
		// 		}
		// 		texel[row][col] = grayscale;
		// 	}
		// }

		// for (size_t i = 0; i < tempVertices.size(); i++)
		// {
		// 	glm::vec3 perturbed = glm::normalize(tempVertices[i]);
		// 	float u = atan2(perturbed.x, perturbed.z) / (2.0 * M_PI) + 0.5;
		// 	float v = 0.5 - asin(perturbed.y) / M_PI;
		// 	double this_grayscale = texel[(int)floor(u * (double)width)][(int)floor(v * (double)height)];
		// 	double t = (this_grayscale - grayscale_l) / (grayscale_h - grayscale_l);
		// 	double deform_param = (deform_low * (double)(1 - t)) + (double)(t * deform_high);
		// 	tempVertices[i] += glm::vec3(deform_param, deform_param, deform_param);
		// }

		// For each vertex of each triangle
		for (unsigned int i = 0; i < vertexIndices.size(); i++)
		{
			Vertex meshVertex;

			// Get the attributes using the indices

			if (tempVertices.size() > 0)
			{
				glm::vec3 vertex = tempVertices[vertexIndices[i] - 1];
				meshVertex.position = vertex;
			}

			if (tempNormals.size() > 0)
			{
				glm::vec3 normal = tempNormals[normalIndices[i] - 1];
				meshVertex.normal = normal;
			}

			if (tempUVs.size() > 0)
			{
				glm::vec2 uv = tempUVs[uvIndices[i] - 1];
				meshVertex.texCoords = uv;
			}

			mVertices.push_back(meshVertex);
		}

		// Create and initialize the buffers
		// make_displacement();
		// for (size_t i = 0; i < mVertices.size(); i += 3)
		// {
		// 	glm::vec3 A = mVertices[i].position;
		// 	glm::vec3 B = mVertices[i + 1].position;
		// 	glm::vec3 C = mVertices[i + 2].position;
		// 	glm::vec3 AB = B - A;
		// 	glm::vec3 AC = C - A;
		// 	glm::vec3 ABxAC = glm::normalize(glm::cross(AB, AC));
		// 	mVertices[i].normal = ABxAC;
		// 	mVertices[i + 1].normal = ABxAC;
		// 	mVertices[i + 2].normal = ABxAC;
		// }
		initBuffers();
		// make_displacement();

		return (mLoaded = true);
	}

	// We shouldn't get here so return failure
	return false;
}

//-----------------------------------------------------------------------------
// Create and initialize the vertex buffer and vertex array object
// Must have valid, non-empty std::vector of Vertex objects.
//-----------------------------------------------------------------------------
void Mesh::initBuffers()
{
	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);

	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(Vertex), &mVertices[0], GL_STATIC_DRAW);

	// Vertex Positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	// Normals attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Vertex Texture Coords
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// unbind to make sure other code does not change it somewhere else
	glBindVertexArray(0);
}

//-----------------------------------------------------------------------------
// Render the mesh
//-----------------------------------------------------------------------------
void Mesh::draw()
{
	if (!mLoaded)
		return;

	glBindVertexArray(mVAO);
	glDrawArrays(GL_TRIANGLES, 0, mVertices.size());
	glBindVertexArray(0);
}
void Mesh::make_displacement()
{
	// make use of mVertices to displace
	// int width, height, components;

	// // Use stbi image library to load our image
	// // unsigned char *imageData = stbi_load("/Users/arghachakraborty/Projects/wrath_of_khan/textures/iit.jpeg", &width, &height, &components, STBI_rgb_alpha);
	// unsigned char *imageData = stbi_load("/Users/arghachakraborty/Projects/wrath_of_khan/textures/tux.jpg", &width, &height, &components, STBI_rgb_alpha);
	// // unsigned char *imageData = stbi_load("/Users/arghachakraborty/Projects/wrath_of_khan/textures/craters.png", &width, &height, &components, STBI_rgb_alpha);
	// // unsigned char *imageData = stbi_load("/Users/arghachakraborty/Projects/wrath_of_khan/textures/tux.jpg", &width, &height, &components, STBI_rgb_alpha);
	// int widthInBytes = width * 4;
	// double grayscale, grayscale_h = 0.0, grayscale_l = 255;
	// double texel[height][width];
	// //changeable
	// double deform_low = -0.3;
	// double deform_high = 0.8;
	// for (int row = 0; row < height; row++)
	// {
	// 	for (int col = 0; col < width; col++)
	// 	{
	// 		unsigned char *pixelOffset = imageData + (row + height * col) * 4;

	// 		unsigned int r = pixelOffset[0];
	// 		unsigned int g = pixelOffset[1];
	// 		unsigned int b = pixelOffset[2];
	// 		grayscale = (0.2126f * r) + (0.7152f * g) + (0.0722f * b);
	// 		if (grayscale_h <= grayscale)
	// 		{
	// 			grayscale_h = grayscale;
	// 		}
	// 		if (grayscale_l >= grayscale)
	// 		{
	// 			grayscale_l = grayscale;
	// 		}
	// 		texel[row][col] = grayscale;
	// 	}
	// }
	// std::cout << " Lowest " << grayscale_l << " highest " << grayscale_h << "\n";
	// double min_u = 10.0, max_u = -10.0, min_v = 10.0, max_v = -10.0;
	// for (size_t i = 0; i < mVertices.size(); i++)
	// {
	// 	glm::vec3 perturbed = glm::normalize(mVertices[i].position);
	// 	float u = atan2(perturbed.x, perturbed.z) / (2.0 * M_PI) + 0.5;
	// 	float v = 0.5 - asin(perturbed.y) / M_PI;
	// 	double this_grayscale = texel[(int)floor(u * (double)width)][(int)floor(v * (double)height)];
	// 	double t = (this_grayscale - grayscale_l) / (grayscale_h - grayscale_l);
	// 	double deform_param = (deform_low * (double)(1 - t)) + (double)(t * deform_high);
	// 	mVertices[i].position += glm::vec3(deform_param, deform_param, deform_param);
	// }
	// std::cout << " MIN U : " << min_u << " MAX U : " << max_u << " MIN V : " << min_v << " MAX V : " << max_v << "\n";
	// unsigned char r = pixelOffset[0];
	// unsigned char g = pixelOffset[1];
	// unsigned char b = pixelOffset[2];
}
