#pragma once

#ifndef PLAYER_H_
#define PLAYER_H_

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
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;




class player {
public:
	float walk_velocity;
	float distance_to_peak;
	float peak;
	float time_to_peak;
	float time_since_jump;
	camera *position;
	bool isJumping;
	float gravity;
	float initial_velocity;
	float radius;

	bool isSmall;
	bool isBig;
	bool isNormal;

	glm::vec3 directional_move;
	player() {
		walk_velocity = 2.0;
		distance_to_peak = 2.0;
		peak = 1.5;
		time_since_jump = 0.0;
		isJumping = false;
		time_to_peak = distance_to_peak / walk_velocity;
		gravity = -9.8;
		initial_velocity = 2.0 * peak * time_to_peak;
		radius = 0.2;
	}

	player(float wv,  float dsp, camera &cm, float p) : 
			walk_velocity(wv),distance_to_peak(dsp), peak(p) {
		position = &cm;
		isJumping = false;
		time_since_jump = 0.0;
		time_to_peak = distance_to_peak / walk_velocity;

	}

	void small() {
		position->height = 1;
		walk_velocity /= 4.0;
		peak /= 4.0;
		radius /= 4.0;
		isSmall = true;
	}

	void big() {
		position->height = 4;
		walk_velocity /= 4.0;
		peak *= 4.0;
		radius *= 2.0;
		isBig = true;
	}
	void jump(float deltaTime) {
		if (isJumping) {
			directional_move = position->dir_move / (time_to_peak * 2.0f);
			float deltaheight = (0.5 * gravity * pow(time_since_jump, 2.0) + initial_velocity * time_since_jump + position->height);
			if (deltaheight > position->height) {
				if (directional_move.x == directional_move.z && directional_move.z == 0) {
					position->moved = glm::vec3(0, deltaheight - position->cam.y,0);
				}
				else {
					position->moved = directional_move;

				}
				position->cam.y = deltaheight;
				position->cam += directional_move;
				
			}
			else {
				isJumping = false;
				position->cam.y = position->height;
				time_since_jump = 0;
				directional_move = glm::vec3(0, 0, 0);
			}
			
			
			//cout << "height: " << deltaheight << endl;
			time_since_jump += deltaTime;
			//cout << "deltaTime : " << deltaTime << endl;
			if (time_since_jump >= 2 * time_to_peak) {
				isJumping = false;
				time_since_jump = 0;
				position->cam.y = position->height;
				directional_move = glm::vec3(0, 0, 0);

			}
		}
		
		
	}

	void calculateInterval(int samples) {

	}


};



#endif //PLAYER_H_