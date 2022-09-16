#pragma once
#ifndef CAMERA_H_
#define CAMERA_H_

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
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

class camera {

public:
	glm::vec3 cam;
	glm::vec3 lookat;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldup;
	glm::vec3 moved;
	glm::vec3 dir_move;
	float yaw;
	float pitch;
	float cameraSpeed;
	float mouseSensitivity;
	float height;

	bool startMouse;

	camera() {
		cam = glm::vec3(0.f, 0.f, 0.f);
		lookat = glm::vec3(0.f, 0.f, -1.f);
		up = glm::vec3(0.f, 1.f, 0.f);
		worldup = glm::vec3(0.f, 1.f, 0.f);
		
		yaw = -90.0f;
		pitch = 0.f;
		cameraSpeed = 5.0f;
		mouseSensitivity = 0.1f;
		startMouse = true;
		height = 1.7;

		update();
	}

	camera(float camX, float camY, float camZ, float lookX, float lookY, float lookZ, float upX, float upY, float upZ,
		float leftright, float updown, float speed, float sensitivity) {
		cam = glm::vec3(camX, camY, camZ);
		lookat = glm::vec3(lookX, lookY, lookZ);
		up = glm::vec3(upX, upY, upZ);	
		worldup = glm::vec3(0.f, 1.f, 0.f);

		yaw = leftright;
		pitch = updown;
		cameraSpeed = speed;
		mouseSensitivity = sensitivity;
		startMouse = true;
		height = 1.7;
		update();
	}

	glm::mat4 getView() {
		return glm::lookAt(cam, cam + lookat, up);
	}

	void cameraMovement(SDL_Event& e, float deltaTime, float xrel, float yrel, bool &jump) {
		float speedMultiplier = 1.0;
		if (e.key.keysym.mod & KMOD_SHIFT) {
			speedMultiplier = 2.0;
		}
		const Uint8* keystates = SDL_GetKeyboardState(NULL);
		float move = cameraSpeed * deltaTime * speedMultiplier;
		moved = cam;

		if (e.type == SDL_KEYDOWN) {
			if (keystates[SDL_SCANCODE_W] && keystates[SDL_SCANCODE_D]) {
				glm::vec3 forward = lookat * move;
				glm::vec3 right = glm::normalize(glm::cross(lookat, up)) * move;
				cam += ((forward + right) / 2.0f);
			}
			else if (keystates[SDL_SCANCODE_W] && keystates[SDL_SCANCODE_A]) {
				glm::vec3 forward = lookat * move;
				glm::vec3 left =  glm::normalize(glm::cross(lookat, up)) * move * -1.f;
				cam += ((forward + left) / 2.0f);
			}
			else if (keystates[SDL_SCANCODE_S] && keystates[SDL_SCANCODE_A]) {
				glm::vec3 forward = lookat * move * -1.f;
				glm::vec3 left = glm::normalize(glm::cross(lookat, up)) * move * -1.f;
				cam += ((forward + left) / 2.0f);
			}
			else if (keystates[SDL_SCANCODE_S] && keystates[SDL_SCANCODE_D]) {
				glm::vec3 forward = lookat * move * -1.f;
				glm::vec3 left = glm::normalize(glm::cross(lookat, up)) * move;
				cam += ((forward + left) / 2.0f);
			}
			else if (keystates[SDL_SCANCODE_W] && keystates[SDL_SCANCODE_SPACE]) {
				jump = true;
				dir_move = lookat * move;
			}
			else if (keystates[SDL_SCANCODE_S] && keystates[SDL_SCANCODE_SPACE]) {
				jump = true;
				dir_move = lookat * move * -1.f;
			}
			else if (keystates[SDL_SCANCODE_D] && keystates[SDL_SCANCODE_SPACE]) {
				jump = true;
				dir_move = glm::normalize(glm::cross(lookat, up)) * move;
			}
			else if (keystates[SDL_SCANCODE_A] && keystates[SDL_SCANCODE_SPACE]) {
				jump = true;
				dir_move = glm::normalize(glm::cross(lookat, up)) * move * -1.f;
			}
			else if (keystates[SDL_SCANCODE_W]) {
				cam += lookat * move;
			}
			else if (keystates[SDL_SCANCODE_S]) {
				cam -= lookat * move;
			}
			else if (keystates[SDL_SCANCODE_A]) {
				cam -= glm::normalize(glm::cross(lookat, up)) * move;
			}
			else if (keystates[SDL_SCANCODE_D]) {
				cam += glm::normalize(glm::cross(lookat, up)) * move;
			}
			else if (e.key.keysym.sym == SDLK_SPACE) {
				//cam += up * move;
				jump = true;
				dir_move = glm::vec3(0, 0, 0);
			}
			moved = cam - moved;
			
		}
		if (e.type == SDL_MOUSEMOTION) {
			if (startMouse) {
				startMouse = false;
				xrel = yrel = 0.f;
			}

			yaw += xrel * mouseSensitivity;
			pitch += yrel * -mouseSensitivity;

			if (pitch > 89.0f) {
				pitch = 90.0f - 0.0001f;
			}
			if (pitch < -89.0f) {
				pitch = -90.0f + 0.0001f;
			}


		}
		update();
		cam.y = height;
		//printf("look at: %f, %f, %f\n", lookat.x, lookat.y, lookat.z);
		//printf("up: %f, %f, %f\n", up.x, up.y, up.z);

	}

private:


	void update() {

		lookat.x = cos(glm::radians(yaw) * cos(glm::radians(pitch)));
		lookat.y = sin(glm::radians(pitch));
		lookat.z = sin(glm::radians(yaw) * cos(glm::radians(pitch)));
		lookat = glm::normalize(lookat);

		right = glm::normalize(glm::cross(lookat, worldup));
		up = glm::normalize(glm::cross(right, lookat));
	}
};


#endif //CAMERA_H_