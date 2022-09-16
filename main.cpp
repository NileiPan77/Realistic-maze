#include "glad/glad.h"  //Include order can matter here
#if defined(__APPLE__) || defined(__linux__)
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif
#include <cstdio>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#define GLM_FORCE_RADIANS
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include "scene.h"
#include "camera.h"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <iterator>

using namespace std;

int screenWidth = 1920;
int screenHeight = 1080;
float timePast = 0;

bool fullscreen = false;
#pragma comment (lib, "assimp-vc142-mt.lib")


int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)

	//Ask SDL to get a recent version of OpenGL (3.2 or greater)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	//Create a window (offsetx, offsety, width, height, flags)
	SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, screenWidth, screenHeight, SDL_WINDOW_OPENGL);

	//Create a context to draw in
	SDL_GLContext context = SDL_GL_CreateContext(window);

	//Load OpenGL extentions with GLAD
	if (gladLoadGLLoader(SDL_GL_GetProcAddress)) {
		printf("\nOpenGL loaded\n");
		printf("Vendor:   %s\n", glGetString(GL_VENDOR));
		printf("Renderer: %s\n", glGetString(GL_RENDERER));
		printf("Version:  %s\n\n", glGetString(GL_VERSION));
	}
	else {
		printf("ERROR: Failed to initialize OpenGL context.\n");
		return -1;
	}

	scene newScene;

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


	vector<std::string> faces
	{
			"Textures/skybox/right.bmp",
			"Textures/skybox/left.bmp",
			"Textures/skybox/top.bmp",
			"Textures/skybox/bottom.bmp",
			"Textures/skybox/front.bmp",
			"Textures/skybox/back.bmp"
	};

	GLuint skyboxTexture = newScene.loadSkyBox(faces);

	newScene.PBR_load();

	glEnable(GL_DEPTH_TEST);

	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	newScene.shaderProgram = newScene.InitShader("textured-Vertex.glsl", "physic_based.glsl");
	newScene.depthShader = newScene.InitShader("depth-vertex.glsl", "depth-fragment.glsl");
	newScene.skyboxShader = newScene.InitShader("skybox-vertex.glsl", "skybox-fragment.glsl");

	// Get a handle for our "MVP" uniform
	GLint uniView = glGetUniformLocation(newScene.shaderProgram, "view");
	GLint uniProj = glGetUniformLocation(newScene.shaderProgram, "proj");

	
	glm::vec3 spherePos = glm::vec3(3.3, 1.4, 3.3);
	mesh sphere(spherePos, 3.f, 3.f, 3.f, 1, 1, 1, Texture::Metal, primitives::sphere);

	newScene.loadMap("map.txt");

	glUseProgram(newScene.shaderProgram);

	GLuint Framebuffer = 0;
	glGenFramebuffers(1, &Framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer);

	GLuint depthTexture;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1920, 1080, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

	glDrawBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;


	//Event Loop (Loop forever processing each event as fast as possible)
	SDL_Event windowEvent;
	bool quit = false;
	SDL_SetRelativeMouseMode(SDL_TRUE);

	newScene.loadMap("map.txt");


	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	GLuint depthMatrixID = glGetUniformLocation(newScene.shaderProgram, "depthMVP");
	// Get a handle for our "MVP" uniform
	GLuint DepthBiasID = glGetUniformLocation(newScene.shaderProgram, "DepthBiasVP");
	GLuint ShadowMapID = glGetUniformLocation(newScene.shaderProgram, "shadowMap");
	GLuint skyboxID = glGetUniformLocation(newScene.skyboxShader, "skybox");
	glUniform1i(skyboxID, 0);

	while (!quit) {
		float currentTime = SDL_GetTicks() / 1000.0f;
		deltaTime = currentTime - lastFrame;
		lastFrame = currentTime;

		glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer);
		glViewport(0, 0, 1920, 1080);


		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(newScene.shaderProgram);

		// Use our shader

		while (SDL_PollEvent(&windowEvent)) { 

			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
				quit = true; //Exit event loop
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f) {
				fullscreen = !fullscreen;
				SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
			}

			newScene.p.position->cameraMovement(windowEvent, deltaTime, windowEvent.motion.xrel, windowEvent.motion.yrel, newScene.p.isJumping);
			newScene.stopCam();
			
		}
		newScene.p.jump(deltaTime);
		newScene.stopCam();


		glUniform3fv(glGetUniformLocation(newScene.shaderProgram, "viewPos"), 1, glm::value_ptr(newScene.p.position->cam));
		glUniform1f(glGetUniformLocation(newScene.shaderProgram, "phong"), 32.0f);

		glUniform3f(glGetUniformLocation(newScene.shaderProgram, "points[0].position"), -10.f, 30.f, -50.f);
		glUniform3f(glGetUniformLocation(newScene.shaderProgram, "points[0].color"), 50000.f, 50000.f, 50000.f);
		glUniform3f(glGetUniformLocation(newScene.shaderProgram, "points[0].direction"), -1.f, 1.f, -1.f);

		
		/*
		glUniform3fv(glGetUniformLocation(newScene.shaderProgram, "spots.position"), 1, glm::value_ptr(newScene.p.position->cam));
		glUniform3fv(glGetUniformLocation(newScene.shaderProgram, "spots.direction"), 1, glm::value_ptr(newScene.p.position->lookat));
		glUniform3f(glGetUniformLocation(newScene.shaderProgram, "spots.color"), 150.f, 150.f, 150.f);
		glUniform1f(glGetUniformLocation(newScene.shaderProgram, "spots.inner_falloff"), glm::cos(glm::radians(12.5f)));
		glUniform1f(glGetUniformLocation(newScene.shaderProgram, "spots.outter_falloff"), glm::cos(glm::radians(17.0f)));
		*/

		glm::vec3 pos = glm::vec3(-10, 30, -50.f);
		glm::vec3 dir = glm::vec3(-1, -1, -1);
		glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10, 10, -10, 10, 0.1, 100);
		glm::mat4 depthViewMatrix = glm::lookAt(pos, glm::vec3(0.38,-0.35,0.85), glm::vec3(0.15, 0.91, 0.37));

		glm::mat4 depthVP = depthProjectionMatrix * depthViewMatrix;

		//render depth to texture
		glUseProgram(newScene.depthShader);
		glBindVertexArray(VertexArrayID);

		glUniformMatrix4fv(glGetUniformLocation(newScene.depthShader, "DepthBiasVP"), 1, GL_FALSE, &depthVP[0][0]);
		glViewport(0, 0, screenWidth, screenHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, depthTexture);
		glClear(GL_DEPTH_BUFFER_BIT);

		sphere.Draw(newScene.depthShader);
		newScene.drawObjs(newScene.depthShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindVertexArray(0);

		glUniformMatrix4fv(depthMatrixID, 1, GL_FALSE, &depthVP[0][0]);


		//reset viewport
		glViewport(0, 0, screenWidth, screenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		
		// normal rendering
		glViewport(0, 0, screenWidth, screenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(newScene.shaderProgram);
		glBindVertexArray(VertexArrayID);

		glm::mat4 view = newScene.p.position->getView();
		//FOV, aspect, near, far	
		glm::mat4 proj = glm::perspective(3.14f / 4, screenWidth / (float)screenHeight, 0.1f, 100.0f); //FOV, aspect, near, far

		glUniformMatrix4fv(glGetUniformLocation(newScene.shaderProgram, "proj"), 1, GL_FALSE, &proj[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(newScene.shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
		
		timePast = SDL_GetTicks() / 1000.f;

		glm::mat4 biasMatrix(
			0.5, 0.0, 0.0, 0.0,
			0.0, 0.5, 0.0, 0.0,
			0.0, 0.0, 0.5, 0.0,
			0.5, 0.5, 0.5, 1.0
		);

		glm::mat4 depthBiasVP = biasMatrix * depthVP;
		glUniformMatrix4fv(DepthBiasID, 1, GL_FALSE, &depthBiasVP[0][0]);

		//bind depth map and normal textures
		newScene.PBR_bind();
		glActiveTexture(GL_TEXTURE15);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glUniform1i(ShadowMapID, 15);
		//render part
		sphere.Draw(newScene.shaderProgram);
		newScene.drawObjs(newScene.shaderProgram);

		glBindVertexArray(0);

		//draw skybox at last 
		glDepthFunc(GL_LEQUAL); 
		glUseProgram(newScene.skyboxShader);
		view = glm::mat4(glm::mat3(newScene.p.position->getView()));
		glUniformMatrix4fv(glGetUniformLocation(newScene.skyboxShader, "projection"), 1, GL_FALSE, &proj[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(newScene.skyboxShader, "view"), 1, GL_FALSE, &view[0][0]);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default


		SDL_GL_SwapWindow(window); //Double buffering

		if (newScene.goal) {
			quit = true;
		} 
	}

	//Clean Up

	glDeleteProgram(newScene.shaderProgram);
	glDeleteVertexArrays(1, &VertexArrayID);

	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}
