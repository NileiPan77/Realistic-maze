#pragma once
#ifndef SCENE_H_
#define SCENE_H_

#include "glad/glad.h"  //Include order can matter here
#if defined(__APPLE__) || defined(__linux__)
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif
#include <cstdio>

#define GLM_FORCE_RADIANS
#include "glm.hpp"
#include "camera.h"
#include "player.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "Mesh.h"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <algorithm>

using namespace std;
float colR = 1, colG = 1, colB = 1;


struct key_door {
	mesh* key;
	mesh* door;
	bool held;
	bool matched;
	key_door() {
		key = nullptr;
		door = nullptr;
		matched = true;
		held = false;
	}
	key_door(mesh* k) {
		key = k;
		door = nullptr;
		matched = false;
		held = false;
	}

};

constexpr auto MAX_TEXTURES = 5;

class scene {
private:
	

	vector<mesh> holdings;
	vector<key_door*> keyDoors;

	vector<mesh> obj_collection;
	float* modelData;
	int modelSize;
	int textureCount;
	glm::vec3 goalPosition;
	glm::vec3 transportPosition;

	
public:
	int textCount;
	GLuint diffuseMappings[MAX_TEXTURES];
	GLuint normalMappings[MAX_TEXTURES];
	GLuint albedoMappings[MAX_TEXTURES];
	GLuint PBRnormalMappings[MAX_TEXTURES];
	GLuint aoMappings[MAX_TEXTURES];
	GLuint roughnessMappings[MAX_TEXTURES];
	GLuint metallicMappings[MAX_TEXTURES];


	bool goal;

	player p;
	camera *newCam;
	GLuint shaderProgram;
	GLuint depthShader;
	GLuint skyboxShader;
	int pointLight;	
	int lightingSphere;	
	GLuint textures[MAX_TEXTURES];

	scene() {
		modelData = nullptr;
		modelSize = 0;
		pointLight = 0;
		textCount = 0;
		p.position = new camera(20.f, 0.f, 20.f,
			0.f, 0.f, -1.f,
			0.f, 1.f, 0.f,
			-90.0f, 0.0f,
			6.f, 0.05f);
		transportPosition = glm::vec3(-10, 1.7, -10);
	}

	void PBR_load() {
		PBR_surface("Textures/hardwood/albedo.bmp", "Textures/hardwood/normal.bmp",	
			"Textures/hardwood/metallic.bmp", "Textures/hardwood/roughness.bmp",
			"Textures/hardwood/ao.bmp", (int)Texture::Wood);

		PBR_surface("Textures/rock-slab-wall1-bl/albedo.bmp", "Textures/rock-slab-wall1-bl/normal.bmp",
			"Textures/rock-slab-wall1-bl/metallic.bmp", "Textures/rock-slab-wall1-bl/roughness.bmp",
			"Textures/rock-slab-wall1-bl/ao.bmp", (int)Texture::Brick);

		PBR_surface("Textures/metal/albedo.bmp", "Textures/metal/normal.bmp",
			"Textures/metal/metallic.bmp", "Textures/metal/roughness.bmp",
			"Textures/metal/ao.bmp", (int)Texture::Metal);
	}

	void loadTextures() {
		
		
		//// Allocate Texture 0 (Wood) ///////
		this->surfaceAllocation("Textures/wood_diffused.bmp", "Textures/wood_normal.bmp", 
					(int)Texture::Wood);

		//// Allocate Texture 1 (Brick) ///////
		this->surfaceAllocation("Textures/Stylized_Bricks_001_basecolor.bmp", "Textures/Stylized_Bricks_001_normal.bmp",
			(int)Texture::Brick);

		//// Allocate Texture 2 (Door) ///////
		this->surfaceAllocation("Textures/metal2_diffused.bmp", "Textures/metal2_normal.bmp",
			(int)Texture::Metal);

		this->surfaceAllocation("Textures/metal_diffused.bmp", "Textures/metal_normal.bmp",
			(int)Texture::Metal2);
		
	}

	void PBR_bind() {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, albedoMappings[(int)Texture::Wood]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_albedo[0]"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, PBRnormalMappings[(int)Texture::Wood]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_normal[0]"), 1);


		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, metallicMappings[(int)Texture::Wood]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_metallic[0]"), 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, roughnessMappings[(int)Texture::Wood]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_roughness[0]"), 3);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, aoMappings[(int)Texture::Wood]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_ao[0]"), 4);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, albedoMappings[(int)Texture::Brick]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_albedo[1]"), 5);

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, PBRnormalMappings[(int)Texture::Brick]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_normal[1]"), 6);


		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, metallicMappings[(int)Texture::Brick]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_metallic[1]"), 7);

		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, roughnessMappings[(int)Texture::Brick]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_roughness[1]"), 8);

		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, aoMappings[(int)Texture::Brick]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_ao[1]"), 9);

		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, albedoMappings[(int)Texture::Metal]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_albedo[2]"), 10);

		glActiveTexture(GL_TEXTURE11);
		glBindTexture(GL_TEXTURE_2D, PBRnormalMappings[(int)Texture::Metal]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_normal[2]"), 11);


		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_2D, metallicMappings[(int)Texture::Metal]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_metallic[2]"), 12);

		glActiveTexture(GL_TEXTURE13);
		glBindTexture(GL_TEXTURE_2D, roughnessMappings[(int)Texture::Metal]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_roughness[2]"), 13);

		glActiveTexture(GL_TEXTURE14);
		glBindTexture(GL_TEXTURE_2D, aoMappings[(int)Texture::Metal]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_ao[2]"), 14);
	}

	void bindTextures() {
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMappings[(int)Texture::Wood]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_diffuse[0]"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMappings[(int)Texture::Wood]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_normal[0]"), 1);


		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, diffuseMappings[(int)Texture::Brick]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_diffuse[1]"), 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, normalMappings[(int)Texture::Brick]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_normal[1]"), 3);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, diffuseMappings[(int)Texture::Metal]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_diffuse[2]"), 4);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, normalMappings[(int)Texture::Metal]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_normal[2]"), 5);

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, diffuseMappings[(int)Texture::Metal2]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_diffuse[3]"), 6);

		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, normalMappings[(int)Texture::Metal2]);
		glUniform1i(glGetUniformLocation(this->shaderProgram, "texture_normal[3]"), 7);
		
	}

	GLuint surfaceAllocation(const char* file, const char* file2, int texID) {
		//// Allocate Texture ///////
		SDL_Surface* surface = SDL_LoadBMP(file);
		if (surface == NULL) { //If it failed, print the error
			printf("Error: \"%s\"\n", SDL_GetError()); exit(1);
		}
		glGenTextures(1, &diffuseMappings[texID]);

		
		glBindTexture(GL_TEXTURE_2D, diffuseMappings[texID]);
		textCount++;
		//What to do outside 0-1 range
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		//Load the texture into memory
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels);
		glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture

		SDL_FreeSurface(surface);


		/// allocate normal mapping ////
		surface = SDL_LoadBMP(file2);
		if (surface == NULL) { //If it failed, print the error
			printf("Error: \"%s\"\n", SDL_GetError()); exit(1);
		}
		glGenTextures(1, &normalMappings[texID]);

		
		glBindTexture(GL_TEXTURE_2D, normalMappings[texID]);
		textCount++;
		//What to do outside 0-1 range
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		//Load the texture into memory
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels);
		glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture

		SDL_FreeSurface(surface);


		//// End Allocate Texture ///////
	}


	GLuint PBR_surface(const char* albedo, const char* normal, const char* metallic, const char* roughness, const char* ao, int texID) {
		//// Allocate Texture ///////
		SDL_Surface* surface = SDL_LoadBMP(albedo);
		if (surface == NULL) { //If it failed, print the error
			printf("Error: \"%s\"\n", SDL_GetError()); exit(1);
		}
		glGenTextures(1, &albedoMappings[texID]);


		glBindTexture(GL_TEXTURE_2D, albedoMappings[texID]);
		textCount++;
		//What to do outside 0-1 range
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		//Load the texture into memory
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels);
		glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture

		SDL_FreeSurface(surface);


		/// allocate normal mapping ////
		surface = SDL_LoadBMP(normal);
		if (surface == NULL) { //If it failed, print the error
			printf("Error: \"%s\"\n", SDL_GetError()); exit(1);
		}
		glGenTextures(1, &PBRnormalMappings[texID]);


		glBindTexture(GL_TEXTURE_2D, PBRnormalMappings[texID]);
		textCount++;
		//What to do outside 0-1 range
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		//Load the texture into memory
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels);
		glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture

		SDL_FreeSurface(surface);


		/// allocate metallic mapping ////
		surface = SDL_LoadBMP(metallic);
		if (surface == NULL) { //If it failed, print the error
			printf("Error: \"%s\"\n", SDL_GetError()); exit(1);
		}
		glGenTextures(1, &metallicMappings[texID]);


		glBindTexture(GL_TEXTURE_2D, metallicMappings[texID]);
		textCount++;
		//What to do outside 0-1 range
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		//Load the texture into memory
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels);
		glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture

		SDL_FreeSurface(surface);

		/// allocate roughness mapping ////
		surface = SDL_LoadBMP(roughness);
		if (surface == NULL) { //If it failed, print the error
			printf("Error: \"%s\"\n", SDL_GetError()); exit(1);
		}
		glGenTextures(1, &roughnessMappings[texID]);


		glBindTexture(GL_TEXTURE_2D, roughnessMappings[texID]);
		textCount++;
		//What to do outside 0-1 range
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		//Load the texture into memory
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels);
		glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture

		SDL_FreeSurface(surface);

		/// allocate metallic mapping ////
		surface = SDL_LoadBMP(ao);
		if (surface == NULL) { //If it failed, print the error
			printf("Error: \"%s\"\n", SDL_GetError()); exit(1);
		}
		glGenTextures(1, &aoMappings[texID]);


		glBindTexture(GL_TEXTURE_2D, aoMappings[texID]);
		textCount++;
		//What to do outside 0-1 range
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		//Load the texture into memory
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels);
		glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture

		SDL_FreeSurface(surface);
		//// End Allocate Texture ///////


	}
	// Create a NULL-terminated string by reading the provided file
	static char* readShaderSource(const char* shaderFile) {
		FILE* fp;
		long length;
		char* buffer;

		// open the file containing the text of the shader code
		fp = fopen(shaderFile, "r");

		// check for errors in opening the file
		if (fp == NULL) {
			printf("can't open shader source file %s\n", shaderFile);
			return NULL;
		}

		// determine the file size
		fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
		length = ftell(fp);  // return the value of the current position

		// allocate a buffer with the indicated number of bytes, plus one
		buffer = new char[length + 1];

		// read the appropriate number of bytes from the file
		fseek(fp, 0, SEEK_SET);  // move position indicator to the start of the file
		fread(buffer, 1, length, fp); // read all of the bytes

		// append a NULL character to indicate the end of the string
		buffer[length] = '\0';

		// close the file
		fclose(fp);

		// return the string
		return buffer;
	}

	GLuint loadSkyBox(vector<string> faces) {
		GLuint skyboxText;
		glGenTextures(1, &skyboxText);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxText);

		for (size_t i = 0; i < faces.size(); i++) {
			SDL_Surface* surface = SDL_LoadBMP(faces[i].c_str());
			if (surface == NULL) { //If it failed, print the error
				printf("Error: \"%s\"\n", SDL_GetError()); exit(1);
			}
			else {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels);
				SDL_FreeSurface(surface);
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		
		return skyboxText;

	}

	// Create a GLSL program object from vertex and fragment shader files
	GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName) {
		GLuint vertex_shader, fragment_shader;
		GLchar* vs_text, * fs_text;
		GLuint program;

		// check GLSL version
		printf("GLSL version: %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

		// Create shader handlers
		vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

		// Read source code from shader files
		vs_text = readShaderSource(vShaderFileName);
		fs_text = readShaderSource(fShaderFileName);

		// error check
		if (vs_text == NULL) {
			printf("Failed to read from vertex shader file %s\n", vShaderFileName);
			exit(1);
		}
		else {
			printf("Vertex Shader:\n=====================\n");
			printf("%s\n", vs_text);
			printf("=====================\n\n");
		}
		if (fs_text == NULL) {
			printf("Failed to read from fragent shader file %s\n", fShaderFileName);
			exit(1);
		}
		else {
			printf("\nFragment Shader:\n=====================\n");
			printf("%s\n", fs_text);
			printf("=====================\n\n");
		}

		// Load Vertex Shader
		const char* vv = vs_text;
		glShaderSource(vertex_shader, 1, &vv, NULL);  //Read source
		glCompileShader(vertex_shader); // Compile shaders

		// Check for errors
		GLint  compiled;
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
			printf("Vertex shader failed to compile:\n");

			GLint logMaxSize, logLength;
			glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
			printf("printing error message of %d bytes\n", logMaxSize);
			char* logMsg = new char[logMaxSize];
			glGetShaderInfoLog(vertex_shader, logMaxSize, &logLength, logMsg);
			printf("%d bytes retrieved\n", logLength);
			printf("error message: %s\n", logMsg);
			delete[] logMsg;

			exit(1);
		}

		// Load Fragment Shader
		const char* ff = fs_text;
		glShaderSource(fragment_shader, 1, &ff, NULL);
		glCompileShader(fragment_shader);
		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);

		//Check for Errors
		if (!compiled) {
			printf("Fragment shader failed to compile\n");

			GLint logMaxSize, logLength;
			glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
			printf("printing error message of %d bytes\n", logMaxSize);
			char* logMsg = new char[logMaxSize];
			glGetShaderInfoLog(fragment_shader, logMaxSize, &logLength, logMsg);
			printf("%d bytes retrieved\n", logLength);
			printf("error message: %s\n", logMsg);
			delete[] logMsg;

			exit(1);
		}

		// Create the program
		program = glCreateProgram();

		// Attach shaders to program
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);

		// Link and set program to use
		glLinkProgram(program);

		return program;
	}

	/*
	bool loadObj(string objFiles) {
		vector<glm::vec3> vertices;
		vector<glm::vec2> uvs;
		vector<glm::vec3> normals;


		vector<unsigned short> vIndices;
		vector<unsigned short> uvIndices;
		vector<unsigned short> nIndices;
		ifstream fd;
		fd.open(objFiles);

		string lineRead;

		while (getline(fd, lineRead)) {
			std::size_t pos = lineRead.find_first_of(" ");
			std::string type = lineRead.substr(0, pos);
			std::stringstream ss;

			if (type == "v") {
				glm::vec3 vertex;
				std::string rest = lineRead.substr(pos + 1, lineRead.size());
				ss << rest;
				
				ss >> vertex.x;
				ss >> vertex.y;
				ss >> vertex.z;

				vertices.push_back(vertex);

			}
			else if (type == "vt") {
				glm::vec2 vtexture;
				std::string rest = lineRead.substr(pos + 1, lineRead.size());
				ss << rest;

				ss >> vtexture.x;
				ss >> vtexture.y;

				uvs.push_back(vtexture);
			}
			else if (type == "vn") {
				glm::vec3 norms;
				std::string rest = lineRead.substr(pos + 1, lineRead.size());
				ss << rest;

				ss >> norms.x;
				ss >> norms.y;
				ss >> norms.z;

				normals.push_back(norms);
			}
			else if (type == "f") {
				std::string rest = lineRead.substr(pos + 1, lineRead.size());
				std::replace(rest.begin(), rest.end(), '/', ' ');
				//cout << rest << endl;
				ss << rest;
				unsigned short temp;
				ss >> temp;
				vIndices.push_back(temp);
				ss >> temp;
				uvIndices.push_back(temp);
				ss >> temp;
				nIndices.push_back(temp);

				ss >> temp;
				vIndices.push_back(temp);
				ss >> temp;
				uvIndices.push_back(temp);
				ss >> temp;
				nIndices.push_back(temp);

				ss >> temp;
				vIndices.push_back(temp);
				ss >> temp;
				uvIndices.push_back(temp);
				ss >> temp;
				nIndices.push_back(temp);
				
			}
		}
		string outFile = objFiles;
		outFile[objFiles.size() - 1] = 't';
		outFile[objFiles.size() - 2] = 'x';
		outFile[objFiles.size() - 3] = 't';

		ofstream od;
		od.open(outFile);
		od << nIndices.size() * 8 << "\n";

		cout << vertices.size() << " total of vertices" << endl;
		cout << normals.size() << " total of normals" << endl;
		cout << uvs.size() << " total of uvs" << endl;
		for (int i = 0; i < vIndices.size(); i++) {
			//cout << "i: " << i << endl;
			int vertexIndex = vIndices.at(i)-1;
			//cout << "Vindex: " << vertexIndex << endl;
			od << vertices.at(vertexIndex).x << "\n";
			od << vertices.at(vertexIndex).y << "\n";
			od << vertices.at(vertexIndex).z << "\n";

			int normalIndex = nIndices.at(i)-1;
			//cout << "normalIndex: " << normalIndex << endl;
			od << normals.at(normalIndex).x << "\n";
			od << normals.at(normalIndex).y << "\n";
			od << normals.at(normalIndex).z << "\n";

			int uvIndex = uvIndices.at(i)-1;
			//cout << "uvIndex: " << uvIndex << endl;

			od << uvs.at(uvIndex).x << "\n";
			od << uvs.at(uvIndex).y << "\n";
		}
		addModelData(outFile);
		return true;
	}
	
	void gpu_malloc(GLuint& vao, GLuint vbo[]) {
		//Build a Vertex Array Object (VAO) to store mapping of shader attributse to VBO
		glGenVertexArrays(1, &vao); //Create a VAO
		glBindVertexArray(vao); //Bind the above created VAO to the current context

		//Allocate memory on the graphics card to store geometry (vertex buffer object)
		glGenBuffers(1, vbo);  //Create 1 buffer called vbo
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
		glBufferData(GL_ARRAY_BUFFER, modelSize * 8 * sizeof(float), modelData, GL_STATIC_DRAW); //upload vertices to vbo
		//GL_STATIC_DRAW means we won't change the geometry, GL_DYNAMIC_DRAW = geometry changes infrequently
		//GL_STREAM_DRAW = geom. changes frequently.  This effects which types of GPU memory is used
	}
	
	void shader_input() {
		//Tell OpenGL how to set fragment shader input 
		GLint posAttrib = glGetAttribLocation(pointLight, "position");
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
		//Attribute, vals/attrib., type, isNormalized, stride, offset
		glEnableVertexAttribArray(posAttrib);

		// GLint colAttrib = glGetAttribLocation(phongShader, "inColor");
		// glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
		// glEnableVertexAttribArray(colAttrib);

		GLint normAttrib = glGetAttribLocation(pointLight, "inNormal");
		glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(normAttrib);

		GLint texAttrib = glGetAttribLocation(pointLight, "inTexcoord");
		glEnableVertexAttribArray(texAttrib);
		glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		
	}
	*/
	/*
	void addObjs(glm::vec3 pos, int r, int c, int z, float ls, float hs, float ws, Texture ts, primitives s) {
		
		obj_collection.push_back(mesh(pos, r, c, z, ls, hs, ws ,ts, s));

		if (s == primitives::key) {
			keyDoors.push_back(new key_door(objs.at(objs.size()-1)));
		}
		
		else if (ts == Texture::Door) {
			for (auto pairs : keyDoors) {
				if (!pairs->matched) {
					pairs->door = objs.at(objs.size() - 1);
					pairs->matched = true;
				}
			}
		}
		
	}
	*/
	/*
	void addModelInfo(int start, int end) {
		modelInfos.push_back(new modelInfo(start, end));
	}

	void addModelData(string file) {
		int size = 0;
		ifstream modelFile;
		modelFile.open(file);
		int numLines = 0;
		modelFile >> numLines;
		float* mod = new float[numLines];
		for (int i = 0; i < numLines; i++) {
			modelFile >> mod[i];
		}
		printf("%d\n", numLines);
		size = numLines / 8;
		modelFile.close();

		float* temp = new float[(size + modelSize) * 8];
		copy(modelData, modelData + modelSize * 8, temp);
		copy(mod, mod + size * 8, temp + modelSize * 8);
		delete modelData;
		modelData = nullptr;
		modelData = temp;
		addModelInfo(modelSize, modelSize + size);
		modelSize += size;
	}
	*/
	void addMesh(glm::vec3 pos, float xS, float yS, float zS, int xN, int yN, int zN, Texture t, primitives p) {
		obj_collection.push_back(mesh(pos, xS, yS, zS, xN, yN, zN, t, p));
	}

	void play() {
		/*
		for (int i = 0; i < keyDoors.size(); i++) {
			if (!keyDoors.at(i)->held && distance( keyDoors.at(i)->key->pos, p.position->cam) < 1) {
				keyDoors.at(i)->key->draw = false;
				holdings.push_back(keyDoors.at(i)->key);
				keyDoors.at(i)->held = true;
				printf("one Key picked up\n");
			}
		}
		*/
		if (distance(goalPosition, p.position->cam) < 1) {
			p.position->cam = transportPosition;
			goal = true;
		}
		/*
		for (int i = 0; i < holdings.size(); i++) {
			if (distance(holdings.at(i)->pos, p.position->cam) < 3) {
				holdings.at(i)->draw = false;
				holdings.push_back(keyDoors.at(i)->door);
				holdings.erase(holdings.begin() + i);
			}
		}
		*/
	}
	
	void drawObjs(GLuint shader) {
		for (int i = 0; i < obj_collection.size(); i++) {
			obj_collection[i].Draw(shader);
		}
			
	}

	void stopCam() {
		for (int obj = 0; obj < obj_collection.size(); obj++) {

			obj_collection[obj].obj.inside(p.position->cam, p.position->moved, p.radius);
			
		}
	}

	void loadMap(const char* map) {
		int width = 0;
		int height = 0;
		string in;
		ifstream mapFile;
		mapFile.open(map);
		mapFile >> width >> height;
		float startX = 3.f;
		float startY = 0.f;
		float startZ = 3.f;
		
		obj_collection.push_back(mesh(glm::vec3(startX, startY, startZ), 1, 1, 1, width, 1, height, Texture::Wood, primitives::cube));
		
		obj_collection.push_back(mesh(glm::vec3(startX -1, startY + 1.f, startZ - 1.f),  1, 1, 1, width + 1, 2, 1, Texture::Brick, primitives::cube));
		obj_collection.push_back(mesh(glm::vec3(startX -1, startY + 1.f, startZ - 1.f),  1, 1, 1, 1, 2, height + 1, Texture::Brick, primitives::cube));
		obj_collection.push_back(mesh(glm::vec3(startX -1 , startY + 1.f, startZ + height),  1, 1, 1, width + 2, 2, 1, Texture::Brick, primitives::cube));
		obj_collection.push_back(mesh(glm::vec3(startX + width, startY + 1.f, startZ -1.f), 1, 1, 1, 1, 2, height + 1, Texture::Brick, primitives::cube));
		
		string lineRead;
		startY = 1.0f;
		int x = 0, y = 0;
		startX--;
		while (getline(mapFile,lineRead)) {
			std::stringstream ss;
			ss << lineRead;
			string word;
			while (!ss.eof()) {
				ss >> word;
				if (word == "0") {
					x++;
				}
				else if (word == "W") {
					x++;
					mesh brick(glm::vec3(startX + x, startY, startZ + y),  1, 1, 1, 1, 2, 1, Texture::Brick, primitives::cube);
					obj_collection.push_back(brick);
				}
				else if (word == "A") {
					x++;
					//mesh metal(glm::vec3(startX + x, startY, startZ + y),  1, 1, 1, 1, 3, 1, Texture::Metal, primitives::cube);
					//obj_collection.push_back(metal);
				}
				else if (word == "a") {
					x++;
					
					//obj_collection.push_back(mesh(glm::vec3(startX + x, startY , startZ + y), 1, 1, 1, 1, 1, 1, Texture::Metal, primitives::key));
					printf("key position: %f, %f, %f\n", startX + x, startY, startZ + y);
				}
				else if (word == "S") {
					x++;
					p.position->cam = glm::vec3(startX + x, startY, startZ + y);
				}
				else if (word == "G") {
					x++;
					goalPosition = glm::vec3(startX + x, p.position->height + 1, startZ + y);
					printf("goal position: %f, %f, %f\n", goalPosition.x, goalPosition.y, goalPosition.z);

				}
				else if (word == "m") {
					x++;
					//obj_collection.push_back(mesh(glm::vec3(startX + x, startY - 2.5, startZ + y), 1, 1, 1, 1, 1, 1, Texture::Metal2, primitives::man));
				}
				if (x >= width) {
					x -= (width);
					y++;
				}
			}
			
		}
		printf("%d number of keys\n", keyDoors.size());

	}

};

#endif //SCENE_H_