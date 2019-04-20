#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#define GLEW_STATIC
#include "GL/glew.h" // Important - this header must come before glfw3 header
#include "GLFW/glfw3.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/norm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "../include/ShaderProgram.h"
#include "../include/Texture2D.h"
#include "../include/Camera.h"
#include "../include/Mesh.h"
#include "../include/shader.hpp"
#include "../include/texture.hpp"
#include "../include/controls.hpp"

struct Particle
{
	glm::vec3 pos, speed;
	unsigned char r, g, b, a; // Color
	float size, angle, weight;
	float life;			  // Remaining life of the particle. if <0 : dead and unused.
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const Particle &that) const
	{
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};

// Global Variables
const char *APP_TITLE = "Wrath of khan";
int gWindowWidth = 1024;
int gWindowHeight = 768;
GLFWwindow *gWindow = NULL;
bool gWireframe = false;
bool gFlashlightOn = true;
glm::vec4 gClearColor(0.5f, 0.5f, 1.0f, 1.0f);
static bool mac_moved = false;
const int MaxParticles = 10000;
Particle ParticlesContainer[MaxParticles];
int LastUsedParticle = 0;
static GLfloat *g_particule_position_size_data = new GLfloat[MaxParticles * 4];
static GLubyte *g_particule_color_data = new GLubyte[MaxParticles * 4];
static const GLfloat g_vertex_buffer_data[] = {
	-0.5f,
	-0.5f,
	0.0f,
	0.5f,
	-0.5f,
	0.0f,
	-0.5f,
	0.5f,
	0.0f,
	0.5f,
	0.5f,
	0.0f,
};
GLuint billboard_vertex_buffer;
GLuint particles_position_buffer;
GLuint particles_color_buffer;
GLuint TextureID;
glm::vec3 campos(0.0f, 0.0f, 5.0f);
FPSCamera fpsCamera(campos, glm::vec3(0.0, 0.0, 0.0));
double bezier_camera_param = 0.0f;
double bezier_camera_param2 = 0.0f;
const double ZOOM_SENSITIVITY = -3.0;
const float MOVE_SPEED = 5.0; // units per second
const float MOUSE_SENSITIVITY = 0.1f;
GLuint CameraRight_worldspace_ID; //= glGetUniformLocation(programID, "CameraRight_worldspace");
GLuint CameraUp_worldspace_ID;	//= glGetUniformLocation(programID, "CameraUp_worldspace");
GLuint ViewProjMatrixID;		  //= glGetUniformLocation(programID, "VP");
GLuint Texture;
GLuint VertexArrayID;
GLuint programID;
float Blend[16] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	-3.0f, 3.0f, 0.0f, 0.0f,
	3.0f, -6.0f, 3.0f, 0.0f,
	-1.0f, 3.0f, -3.0f, 1.0f};
glm::mat4 blend_mat = glm::make_mat4(Blend);

std::vector<glm::vec3> dynamic_camera_points;
std::vector<glm::vec3> dynamic_camera_explore;

// Function prototypes
void glfw_onKey(GLFWwindow *window, int key, int scancode, int action, int mode);
void glfw_onFramebufferSize(GLFWwindow *window, int width, int height);
void glfw_onMouseScroll(GLFWwindow *window, double deltaX, double deltaY);
void update(double elapsedTime);
void showFPS(GLFWwindow *window);
bool initOpenGL();
void mac_patch(GLFWwindow *window);
glm::vec3 get_bezier_points(double t, float *point_array);
float RandomFloat(float a, float b);

// Finds a Particle in ParticlesContainer which isn't used yet.
// (i.e. life < 0);
int FindUnusedParticle()
{

	for (int i = LastUsedParticle; i < MaxParticles; i++)
	{
		if (ParticlesContainer[i].life < 0)
		{
			LastUsedParticle = i;
			return i;
		}
	}

	for (int i = 0; i < LastUsedParticle; i++)
	{
		if (ParticlesContainer[i].life < 0)
		{
			LastUsedParticle = i;
			return i;
		}
	}

	return 0; // All particles are taken, override the first one
}

void SortParticles()
{
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}
//-----------------------------------------------------------------------------
// Main Application Entry Point
//-----------------------------------------------------------------------------
int main()
{
	if (!initOpenGL())
	{
		// An error occured
		std::cerr << "GLFW initialization failed" << std::endl;
		return -1;
	}

	ShaderProgram lightingShader;
	lightingShader.loadShaders("shaders/lighting_dir_point_spot.vert", "shaders/lighting_dir_point_spot.frag");

	// Load meshes and textures
	const int numModels = 3;
	Mesh mesh[numModels];
	Texture2D texture[numModels];

	mesh[0].loadOBJ("models/watergun.obj");
	mesh[1].loadOBJ("models/watergun.obj");
	mesh[2].loadOBJ("models/watergun.obj");

	texture[0].loadTexture("textures/gray.png", true);
	texture[1].loadTexture("textures/gray.png", true);
	texture[2].loadTexture("textures/gray.png", true);

	// Model positions
	glm::vec3 modelPos[] = {
		glm::vec3(-2.0f, -1.0f, 0.0f), // planet
		glm::vec3(2.0f, 0.0f, 0.0f),   // planet
		glm::vec3(-2.0f, 0.0f, 0.0f)   // planet
									   // bomb

	};

	// Model scale
	glm::vec3 modelScale[] = {
		glm::vec3(0.4068f, 0.4068f, 0.4068), // barrel
		glm::vec3(0.2068f, 0.2068f, 0.2068), // barrel
		glm::vec3(0.3068f, 0.3068f, 0.3068)  // barrel

	};

	// Point Light positions
	glm::vec3 pointLightPos[3] = {
		glm::vec3(-5.0f, 3.8f, 0.0f),
		glm::vec3(0.5f, 3.8f, 0.0f),
		glm::vec3(5.0f, 3.8, 0.0f)};

	double lastTime = glfwGetTime();

	// Rendering loop
	// glDepthFunc(GL_LESS);
	//shadervarss
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	programID = LoadShaders("shaders/Particle.vertexshader", "shaders/Particle.fragmentshader");
	CameraRight_worldspace_ID = glGetUniformLocation(programID, "CameraRight_worldspace");
	CameraUp_worldspace_ID = glGetUniformLocation(programID, "CameraUp_worldspace");
	ViewProjMatrixID = glGetUniformLocation(programID, "VP");
	TextureID = glGetUniformLocation(programID, "myTextureSampler");
	for (int i = 0; i < MaxParticles; i++)
	{
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
	}
	Texture = loadDDS("textures/particle.DDS");
	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
	double circle_radii = 0.1;
	bool cam_positioned = false;
	bool hit = false;
	double save_time;
	while (!glfwWindowShouldClose(gWindow))
	{
		showFPS(gWindow);
		glUseProgram(programID);

		double currentTime = glfwGetTime();
		double deltaTime = currentTime - lastTime;

		// Poll for and process events
		glfwPollEvents();
		update(deltaTime);

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model(1.0), view(1.0), projection(1.0);

		// Create the View matrix
		view = fpsCamera.getViewMatrix();
		glm::mat4 ViewMatrix = view;
		// Create the projection matrix
		projection = glm::perspective(glm::radians(35.0f), (float)gWindowWidth / (float)gWindowHeight, 0.1f, 200.0f);

		// update the view (camera) position
		glm::vec3 viewPos;
		// std::cout << glm::to_string(fpsCamera.getPosition()) << std::endl;
		viewPos.x = fpsCamera.getPosition().x;
		viewPos.y = fpsCamera.getPosition().y;
		viewPos.z = fpsCamera.getPosition().z;
		glm::vec3 CameraPosition(glm::inverse(view)[3]);
		glm::mat4 ViewProjectionMatrix = projection * view;
		lightingShader.use();

		//BEGIN PARTICLES
		int newparticles = (int)(deltaTime * 10000.0);
		// std::cout << glm::distance(modelPos[2], glm::vec3(0.0f, 0.0f, 0.0f)) << "\n";
		// if (glm::distance(modelPos[2], glm::vec3(0.0f, 0.0f, 0.0f)) < 1.3)
		// {
		// 	hit = true;
		// 	save_time = glfwGetTime();
		// }
		//TODO

		newparticles = (int)(deltaTime * 10000.0);
		if (newparticles > (int)(0.016f * 10000.0))
			newparticles = (int)(0.016f * 10000.0);
		lightingShader.use();
		lightingShader.setUniform("model", glm::mat4(1.0)); // do not need to translate the models so just send the identity matrix
		lightingShader.setUniform("view", view);
		lightingShader.setUniform("projection", projection);
		lightingShader.setUniform("viewPos", viewPos);

		// // Directional light
		lightingShader.setUniform("sunLight.direction", glm::vec3(0.0f, -0.9f, -0.17f));
		lightingShader.setUniform("sunLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
		lightingShader.setUniform("sunLight.diffuse", glm::vec3(0.1f, 0.1f, 0.1f)); // dark
		lightingShader.setUniform("sunLight.specular", glm::vec3(0.1f, 0.1f, 0.1f));

		lightingShader.setUniform("spotLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
		lightingShader.setUniform("spotLight.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		lightingShader.setUniform("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setUniform("spotLight.position", glm::vec3(0.982347, 3.500000, 10.248156));
		lightingShader.setUniform("spotLight.direction", glm::vec3(-0.202902, -0.470038, -0.859008));
		lightingShader.setUniform("spotLight.cosInnerCone", glm::cos(glm::radians(15.0f)));
		lightingShader.setUniform("spotLight.cosOuterCone", glm::cos(glm::radians(20.0f)));
		lightingShader.setUniform("spotLight.constant", 1.0f);
		lightingShader.setUniform("spotLight.linear", 0.007f);
		lightingShader.setUniform("spotLight.exponent", 0.0017f);
		lightingShader.setUniform("spotLight.on", gFlashlightOn);

		for (int i = 0; i < newparticles / 4; i++)
		{
			int particleIndex = FindUnusedParticle();
			ParticlesContainer[particleIndex].life = 1.5f; // This particle will live 5 seconds.
			// ParticlesContainer[particleIndex].pos = glm::vec3(0, 0, -11.0f);
			ParticlesContainer[particleIndex].pos = glm::vec3(-1.3, 0, 0);

			float spread = 1.5f;
			glm::vec3 maindir = glm::vec3(5.0f, 10.0f, 0.0f); // main direction of thw particles
			// Very bad way to generate a random direction;
			// See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
			// combined with some user-controlled parameters (main direction, spread, etc)
			glm::vec3 randomdir = glm::vec3(
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f);

			ParticlesContainer[particleIndex].speed = maindir + randomdir * spread;

			ParticlesContainer[particleIndex].r = 90;
			ParticlesContainer[particleIndex].g = 90;
			if (RandomFloat(0.0f, 10.0f) < 8.0)
			{
				ParticlesContainer[particleIndex].b = 255;
			}
			else
			{
				ParticlesContainer[particleIndex].b = 200;
			}

			// ParticlesContainer[particleIndex].b = RandomFloat(200.0f, 255.0f);
			;
			ParticlesContainer[particleIndex].a = RandomFloat(10.0f, 255.0f);

			// ParticlesContainer[particleIndex].size = 0.06f;
			ParticlesContainer[particleIndex].size = RandomFloat(0.01, 0.08);
			// ParticlesContainer[particleIndex].size = (rand() % 1000) / 2000.0f + 0.1f;
		}

		for (int i = newparticles / 4; i < newparticles / 2; i++)
		{
			int particleIndex = FindUnusedParticle();
			ParticlesContainer[particleIndex].life = 1.0f; // This particle will live 5 seconds.
			// ParticlesContainer[particleIndex].pos = glm::vec3(0, 0, -11.0f);
			ParticlesContainer[particleIndex].pos = glm::vec3(1.6, 0.4, 0);

			float spread = 1.5f;
			glm::vec3 maindir = glm::vec3(-10.0f, 10.0f, 0.0f); // main direction of thw particles
			// Very bad way to generate a random direction;
			// See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
			// combined with some user-controlled parameters (main direction, spread, etc)
			glm::vec3 randomdir = glm::vec3(
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f);

			ParticlesContainer[particleIndex].speed = maindir + randomdir * spread;

			ParticlesContainer[particleIndex].r = 90;
			ParticlesContainer[particleIndex].g = 90;
			if (RandomFloat(0.0f, 10.0f) < 8.0)
			{
				ParticlesContainer[particleIndex].b = 255;
			}
			else
			{
				ParticlesContainer[particleIndex].b = 200;
			}

			// ParticlesContainer[particleIndex].b = RandomFloat(200.0f, 255.0f);
			;
			ParticlesContainer[particleIndex].a = RandomFloat(10.0f, 255.0f);

			// ParticlesContainer[particleIndex].size = 0.06f;
			ParticlesContainer[particleIndex].size = RandomFloat(0.01, 0.08);
			// ParticlesContainer[particleIndex].size = (rand() % 1000) / 2000.0f + 0.1f;
		}

		for (int i = newparticles / 2; i < newparticles; i++)
		{
			int particleIndex = FindUnusedParticle();
			ParticlesContainer[particleIndex].life = 0.5f; // This particle will live 5 seconds.
			// ParticlesContainer[particleIndex].pos = glm::vec3(0, 0, -11.0f);
			ParticlesContainer[particleIndex].pos = glm::vec3(-1.3, 0.79, 0);

			float spread = 1.5f;
			glm::vec3 maindir = glm::vec3(10.0f, 10.0f, 0.0f); // main direction of thw particles
			// Very bad way to generate a random direction;
			// See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
			// combined with some user-controlled parameters (main direction, spread, etc)
			glm::vec3 randomdir = glm::vec3(
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f);

			ParticlesContainer[particleIndex].speed = maindir + randomdir * spread;

			ParticlesContainer[particleIndex].r = 150;
			ParticlesContainer[particleIndex].g = 90;
			if (RandomFloat(0.0f, 10.0f) < 8.0)
			{
				ParticlesContainer[particleIndex].b = 255;
			}
			else
			{
				ParticlesContainer[particleIndex].b = 200;
			}

			// ParticlesContainer[particleIndex].b = RandomFloat(200.0f, 255.0f);
			;
			ParticlesContainer[particleIndex].a = RandomFloat(10.0f, 255.0f);

			// ParticlesContainer[particleIndex].size = 0.06f;
			ParticlesContainer[particleIndex].size = RandomFloat(0.01, 0.08);
			// ParticlesContainer[particleIndex].size = (rand() % 1000) / 2000.0f + 0.1f;
		}

		int ParticlesCount = 0;
		for (int i = 0; i < MaxParticles; i++)
		{

			Particle &p = ParticlesContainer[i]; // shortcut

			if (p.life > 0.0f)
			{

				// Decrease life
				p.life -= deltaTime;
				if (p.life > 0.0f)
				{

					// Simulate simple physics : gravity only, no collisions
					p.speed += glm::vec3(0.0f, -9.81f, 0.0f) * (float)deltaTime * 0.5f;
					p.pos += p.speed * (float)deltaTime;

					// std::cout << glm::to_string(p.pos) << std::endl;
					p.cameradistance = glm::length2(p.pos - CameraPosition);
					//ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

					// Fill the GPU buffer
					g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
					g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
					g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos.z;

					g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

					g_particule_color_data[4 * ParticlesCount + 0] = p.r;
					g_particule_color_data[4 * ParticlesCount + 1] = p.g;
					g_particule_color_data[4 * ParticlesCount + 2] = p.b;
					g_particule_color_data[4 * ParticlesCount + 3] = p.a;
				}
				else
				{
					// Particles that just died will be put at the end of the buffer in SortParticles();
					p.cameradistance = -1.0f;
				}

				ParticlesCount++;
			}
		}
		SortParticles();
		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Use our shader
		glUseProgram(programID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);
		glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
		glUniform3f(CameraUp_worldspace_ID, ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

		glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

		// 1rst attribute buffer : vertices
		glBindVertexArray(VertexArrayID);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
		glVertexAttribPointer(
			0,		  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void *)0 // array buffer offset
		);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glVertexAttribPointer(
			1,		  // attribute. No particular reason for 1, but must match the layout in the shader.
			4,		  // size : x + y + z + size => 4
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void *)0 // array buffer offset
		);

		// 3rd attribute buffer : particles' colors
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
		glVertexAttribPointer(
			2,				  // attribute. No particular reason for 1, but must match the layout in the shader.
			4,				  // size : r + g + b + a => 4
			GL_UNSIGNED_BYTE, // type
			GL_TRUE,		  // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
			0,				  // stride
			(void *)0		  // array buffer offset
		);

		// These functions are specific to glDrawArrays*Instanced*.
		// The first parameter is the attribute buffer we're talking about.
		// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
		// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
		glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
		glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
		glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		//END OF PARTICLES
		// Must be called BEFORE setting uniforms because setting uniforms is done
		// on the currently active shader program.

		lightingShader.use();

		// Render the scene
		// Point Light 1
		lightingShader.setUniform("pointLights[0].ambient", glm::vec3(0.2f, 0.2f, 0.2f));
		lightingShader.setUniform("pointLights[0].diffuse", glm::vec3(1.0f, 1.0f, 1.0f)); // green-ish light
		lightingShader.setUniform("pointLights[0].specular", glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setUniform("pointLights[0].position", pointLightPos[0]);
		lightingShader.setUniform("pointLights[0].constant", 1.0f);
		lightingShader.setUniform("pointLights[0].linear", 0.022f);
		lightingShader.setUniform("pointLights[0].exponent", 0.020f);

		// Point Light 2
		lightingShader.setUniform("pointLights[1].ambient", glm::vec3(0.2f, 0.2f, 0.2f));
		lightingShader.setUniform("pointLights[1].diffuse", glm::vec3(1.0f, 1.0f, 1.0f)); // red-ish light
		lightingShader.setUniform("pointLights[1].specular", glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setUniform("pointLights[1].position", pointLightPos[1]);
		lightingShader.setUniform("pointLights[1].constant", 1.0f);
		lightingShader.setUniform("pointLights[1].linear", 0.022f);
		lightingShader.setUniform("pointLights[1].exponent", 0.020f);

		// Point Light 3
		lightingShader.setUniform("pointLights[2].ambient", glm::vec3(0.2f, 0.2f, 0.2f));
		lightingShader.setUniform("pointLights[2].diffuse", glm::vec3(1.0f, 1.0f, 1.0f)); // blue-ish light
		lightingShader.setUniform("pointLights[2].specular", glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setUniform("pointLights[2].position", pointLightPos[2]);
		lightingShader.setUniform("pointLights[2].constant", 1.0f);
		lightingShader.setUniform("pointLights[2].linear", 0.22f);
		lightingShader.setUniform("pointLights[2].exponent", 0.20f);
		for (int i = 0; i < 3; i++)
		{
			if (i == 0)
			{
				model = glm::translate(glm::mat4(1.0), modelPos[i]) * glm::scale(glm::mat4(1.0), modelScale[i]) * glm::rotate(glm::mat4(1.0), glm::radians((float)(-90)), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0), glm::radians((float)(34)), glm::vec3(0.0f, 1.0f, 0.0f));
			}
			else if (i == 1)
			{
				model = glm::translate(glm::mat4(1.0), modelPos[i]) * glm::scale(glm::mat4(1.0), modelScale[i] * 0.8f) * glm::rotate(glm::mat4(1.0), glm::radians((float)(-90)), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0), glm::radians((float)(-44)), glm::vec3(0.0f, 1.0f, 0.0f));
			}
			else if (i == 2)
			{
				model = glm::translate(glm::mat4(1.0), modelPos[i]) * glm::scale(glm::mat4(1.0), modelScale[i]) * glm::rotate(glm::mat4(1.0), glm::radians((float)(-90)), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0), glm::radians((float)(39)), glm::vec3(0.0f, 1.0f, 0.0f));
			}
			// model = glm::translate(glm::mat4(1.0), modelPos[i]) * glm::scale(glm::mat4(1.0), modelScale[i]) * glm::rotate(glm::mat4(1.0), glm::radians((float)(-90)), glm::vec3(1.0f, 0.0f, 0.0f));

			// model = glm::translate(glm::mat4(1.0), modelPos[i]) * glm::scale(glm::mat4(1.0), modelScale[i]); // * glm::rotate(glm::mat4(1.0), glm::radians((float)(glfwGetTime() * 100.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
			lightingShader.setUniform("model", model);

			// 	// Set material properties
			lightingShader.setUniform("material.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
			lightingShader.setUniformSampler("material.diffuseMap", 0);
			lightingShader.setUniform("material.specular", glm::vec3(0.2f, 0.2f, 0.2f));
			lightingShader.setUniform("material.shininess", 10.0f);

			texture[i].bind(0); // set the texture before drawing.  Our simple OBJ mesh loader does not do materials yet.
			mesh[i].draw();		// Render the OBJ mesh
			texture[i].unbind(0);
		}

		// Swap front and back buffers
		glfwSwapBuffers(gWindow);
		mac_patch(gWindow);
		lastTime = currentTime;
	}

	glfwTerminate();

	return 0;
}

//-----------------------------------------------------------------------------
// Initialize GLFW and OpenGL
//-----------------------------------------------------------------------------
bool initOpenGL()
{
	// Intialize GLFW
	// GLFW is configured.  Must be called before calling any GLFW functions
	if (!glfwInit())
	{
		// An error occured
		std::cerr << "GLFW initialization failed" << std::endl;
		return false;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // forward compatible with newer versions of OpenGL as they become available but not backward compatible (it will not run on devices that do not support OpenGL 3.3

	// Create an OpenGL 3.3 core, forward compatible context window
	gWindow = glfwCreateWindow(gWindowWidth, gWindowHeight, APP_TITLE, NULL, NULL);
	if (gWindow == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	// Make the window's context the current one
	glfwMakeContextCurrent(gWindow);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return false;
	}

	// Set the required callback functions
	glfwSetKeyCallback(gWindow, glfw_onKey);
	glfwSetFramebufferSizeCallback(gWindow, glfw_onFramebufferSize);
	glfwSetScrollCallback(gWindow, glfw_onMouseScroll);
	glClearColor(gClearColor.r, gClearColor.g, gClearColor.b, gClearColor.a);
	dynamic_camera_points.push_back(campos);
	dynamic_camera_points.push_back(glm::vec3(0.0f, 10.0f, 10.0f));
	dynamic_camera_points.push_back(glm::vec3(0.0f, 4.0f, 0.0f));
	dynamic_camera_points.push_back(glm::vec3(0.0f, 8.0f, -10.0f));
	glViewport(0, 0, gWindowWidth, gWindowHeight);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	return true;
}

//-----------------------------------------------------------------------------
// Is called whenever a key is pressed/released via GLFW
//-----------------------------------------------------------------------------
void glfw_onKey(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		gWireframe = !gWireframe;
		if (gWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		// toggle the flashlight
		gFlashlightOn = !gFlashlightOn;
	}
}

//-----------------------------------------------------------------------------
// Is called when the window is resized
//-----------------------------------------------------------------------------
void glfw_onFramebufferSize(GLFWwindow *window, int width, int height)
{
	gWindowWidth = width;
	gWindowHeight = height;
	glViewport(0, 0, gWindowWidth, gWindowHeight);
}

//-----------------------------------------------------------------------------
// Called by GLFW when the mouse wheel is rotated
//-----------------------------------------------------------------------------
void glfw_onMouseScroll(GLFWwindow *window, double deltaX, double deltaY)
{
	double fov = fpsCamera.getFOV() + deltaY * ZOOM_SENSITIVITY;

	fov = glm::clamp(fov, 1.0, 120.0);

	fpsCamera.setFOV((float)fov);
}

//-----------------------------------------------------------------------------
// Update stuff every frame
//-----------------------------------------------------------------------------
void update(double elapsedTime)
{
	// Camera orientation
	double mouseX, mouseY;

	// Get the current mouse cursor position delta
	// glfwGetCursorPos(gWindow, &mouseX, &mouseY);

	// // Rotate the camera the difference in mouse distance from the center screen.  Multiply this delta by a speed scaler
	// fpsCamera.rotate((float)(gWindowWidth / 2.0 - mouseX) * MOUSE_SENSITIVITY, (float)(gWindowHeight / 2.0 - mouseY) * MOUSE_SENSITIVITY);

	// // Clamp mouse cursor to center of screen
	// glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);

	// Camera FPS movement

	// Forward/backward
	if (glfwGetKey(gWindow, GLFW_KEY_W) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getLook());
	else if (glfwGetKey(gWindow, GLFW_KEY_S) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getLook());

	// Strafe left/right
	if (glfwGetKey(gWindow, GLFW_KEY_A) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getRight());
	else if (glfwGetKey(gWindow, GLFW_KEY_D) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getRight());

	// Up/down
	if (glfwGetKey(gWindow, GLFW_KEY_Z) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * glm::vec3(0.0f, 1.0f, 0.0f));
	else if (glfwGetKey(gWindow, GLFW_KEY_X) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -glm::vec3(0.0f, 1.0f, 0.0f));
}

//-----------------------------------------------------------------------------
// Code computes the average frames per second, and also the average time it takes
// to render one frame.  These stats are appended to the window caption bar.
//-----------------------------------------------------------------------------
void showFPS(GLFWwindow *window)
{
	static double previousSeconds = 0.0;
	static int frameCount = 0;
	double elapsedSeconds;
	double currentSeconds = glfwGetTime(); // returns number of seconds since GLFW started, as double float

	elapsedSeconds = currentSeconds - previousSeconds;

	// Limit text updates to 4 times per second
	if (elapsedSeconds > 0.25)
	{
		previousSeconds = currentSeconds;
		double fps = (double)frameCount / elapsedSeconds;
		double msPerFrame = 1000.0 / fps;

		// The C++ way of setting the window title
		std::ostringstream outs;
		outs.precision(3); // decimal places
		outs << std::fixed
			 << APP_TITLE << "    "
			 << "FPS: " << fps << "    "
			 << "Frame Time: " << msPerFrame << " (ms)";
		glfwSetWindowTitle(window, outs.str().c_str());

		// Reset for next average.
		frameCount = 0;
	}

	frameCount++;
}
void mac_patch(GLFWwindow *window)
{
	if (glfwGetTime() > 2.0)
	{
		mac_moved = true;
	}
	// glfwGetTim

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE && !mac_moved)
	{
		int x, y;
		glfwGetWindowPos(window, &x, &y);
		glfwSetWindowPos(window, ++x, y);
	}
	else
	{
		mac_moved = true;
	}
}

glm::vec3 get_bezier_points(double t, float *point_array)
{
	// float *point_array = &dynamic_points[0].x;
	glm::mat4x3 control_p = glm::make_mat4x3(point_array);
	return control_p * blend_mat * glm::vec4(1.0f, t, t * t, t * t * t);
}
float RandomFloat(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}