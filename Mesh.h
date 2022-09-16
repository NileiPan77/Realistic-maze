#pragma once
/*
	for assimp loaded objects only
*/

#ifndef MESH_H_
#define MESH_H_

#include "glad/glad.h"  //Include order can matter here

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags


#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

using namespace std;
enum class Texture {
  Wood,
  Brick,
  Metal,
  Metal2,
  NotExist5,
  None
};
enum class primitives {
  cube,
  sphere,
  gate,
  man,
  teapot
};
struct object_tan{
  glm::vec3 position;

  float xScale;
  float yScale;
  float zScale;

  int xNum;
  int yNum;
  int zNum;
  bool draw;
  Texture text;
  primitives shape;

  object_tan() {
    draw = false;
  }

  object_tan(glm::vec3 pos, float xS, float yS, float zS, int xN, int yN, int zN, Texture t, primitives p) 
  :position(pos), xScale(xS), yScale(yS), zScale(zS), xNum(xN), yNum(yN), zNum(zN), text(t), shape(p){
    draw = true;

  }

  void inside(glm::vec3& p, glm::vec3 move, float radius) {
    if (draw) {
      glm::vec3 moved = p + move;
      float x = moved.x + radius;
      float y = moved.y + radius;
      float z1 = moved.z + radius;

      float dist1 = distance(p, glm::vec3(position.x + 0.5, position.y - 0.5, position.z - 0.5));
      float dist2 = distance(p, glm::vec3(position.x - 0.5, position.y - 0.5, position.z - 0.5));
      glm::vec3 minPoint = glm::vec3(position.x - 0.5 - radius, position.y - 0.5 - radius, position.z - 0.5 - radius);
      float height = yScale * yNum;
      float length = xScale * xNum;
      float width = zScale * zNum;

      glm::vec3 maxPoint = glm::vec3(minPoint.x + length + radius, minPoint.y + height + radius,
        minPoint.z + width + radius);


      //printf("minY: %f, maxY: %f, curY: %f\n", minPoint.y, maxPoint.y, y);

      /*
      if (y > minPoint.y && y < maxPoint.y) {
        cout << "2 in ..." << endl;
      }
      if (x > minPoint.x && x < maxPoint.x) {
        cout << "1 in ..." << endl;
      }
      if (z1 > minPoint.z && z1 < maxPoint.z) {
        cout << " 3 in ..." << endl;
      }
      */

      float xMove = (move.x < 0) ? -0.01 : 0.01;
      float yMove = (move.y < 0) ? -0.01 : 0.01;
      float zMove = (move.z < 0) ? -0.01 : 0.01;

      float offset = radius * 2.0f;

      if (x > minPoint.x && x < maxPoint.x + offset &&
        y > minPoint.y - radius && y < maxPoint.y + offset &&
        z1 > minPoint.z && z1 < maxPoint.z + offset) {
        if (move.x != 0) {
          p.x -= move.x + xMove;
        }
        if (move.z != 0) {
          p.z -= move.z + zMove;
        }
        if (move.y != 0) {
          p.y -= move.y + yMove;
        }



      }
    }


  }
};


class mesh {
private:
  GLuint vertexbuffer, uvbuffer, normalbuffer, 
    tangentbuffer, bitangentbuffer, elementbuffer;

	void setUp() {
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

    glGenBuffers(1, &tangentbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer);
    glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), &tangents[0], GL_STATIC_DRAW);

    glGenBuffers(1, &bitangentbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer);
    glBufferData(GL_ARRAY_BUFFER, bitangents.size() * sizeof(glm::vec3), &bitangents[0], GL_STATIC_DRAW);
	  
    // Generate a buffer for the indices as well
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
  
  }

public:
  vector<unsigned short> indices;
  vector<glm::vec3> vertices;
  vector<glm::vec2> uvs;
  vector<glm::vec3> normals;
  vector<glm::vec3> tangents;
  vector<glm::vec3> bitangents;
  object_tan obj;

  mesh(glm::vec3 pos, float xS, float yS, float zS, int xN, int yN, int zN, Texture t, primitives p){
    obj = object_tan(pos, xS, yS, zS, xN, yN, zN, t, p);
    if (p == primitives::cube) {
      this->loadObj("models/square.obj");
    }
    else if (p == primitives::sphere) {
      this->loadObj("models/ball.obj");
    }
    else if (p == primitives::gate) {
      this->loadObj("models/gate.obj");
    }
  }

  mesh(vector<unsigned short> out_indices, 
    vector<glm::vec3> out_vertices, 
    vector<glm::vec2> out_uvs,
    vector<glm::vec3> out_normals,
    vector<glm::vec3> out_tangents,
    vector<glm::vec3> out_bitangents)
  {
    indices = out_indices;
    vertices = out_vertices;
    uvs = out_uvs;
    normals = out_normals;
    tangents = out_tangents;
    bitangents = out_bitangents;


    // Now that we have all the required data, set the vertex buffers and its attribute pointers.
    this->setUp();
  }


  void addTexture(const char* file) {
    SDL_Surface* surface = SDL_LoadBMP(file);
    if (surface == NULL) { //If it failed, print the error
      printf("Error: \"%s\"\n", SDL_GetError()); exit(1);
    }
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    //What to do outside 0-1 range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    //Load the texture into memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels);
    glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture

    SDL_FreeSurface(surface);
  }

  bool is_near(float v1, float v2) {
    return fabs(v1 - v2) < 0.01f;
  }

  bool getSimilarVertexIndex(glm::vec3& in_vertex,glm::vec2& in_uv,glm::vec3& in_normal,unsigned short& result) {
    // Lame linear search
    for (unsigned int i = 0; i < vertices.size(); i++) {
      if (
        is_near(in_vertex.x, vertices[i].x) &&
        is_near(in_vertex.y, vertices[i].y) &&
        is_near(in_vertex.z, vertices[i].z) &&
        is_near(in_uv.x, uvs[i].x) &&
        is_near(in_uv.y, uvs[i].y) &&
        is_near(in_normal.x, normals[i].x) &&
        is_near(in_normal.y, normals[i].y) &&
        is_near(in_normal.z, normals[i].z)
        ) {
        result = i;
        return true;
      }
    }
    // No other vertex could be used instead.
    // Looks like we'll have to add it to the VBO.
    return false;
  }

  void loadObj(string objFiles) {

    vector<glm::vec3> in_vertices;
    vector<glm::vec2> in_uvs;
    vector<glm::vec3> in_normals;
    vector<glm::vec3> in_tangents;
    vector<glm::vec3> in_bitangents;


    vector<glm::vec3> verticesTemp;
    vector<glm::vec2> uvsTemp;
    vector<glm::vec3> normalsTemp;

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

        verticesTemp.push_back(vertex);

      }
      else if (type == "vt") {
        glm::vec2 vtexture;
        std::string rest = lineRead.substr(pos + 1, lineRead.size());
        ss << rest;

        ss >> vtexture.x;
        ss >> vtexture.y;

        uvsTemp.push_back(vtexture * 12.f);
      }
      else if (type == "vn") {
        glm::vec3 norms;
        std::string rest = lineRead.substr(pos + 1, lineRead.size());
        ss << rest;

        ss >> norms.x;
        ss >> norms.y;
        ss >> norms.z;

        normalsTemp.push_back(norms);
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

    // For each vertex of each triangle
    for (unsigned int i = 0; i < vIndices.size(); i++) {

      // Get the indices of its attributes
      unsigned int vertexIndex = vIndices[i];
      unsigned int uvIndex = uvIndices[i];
      unsigned int normalIndex = nIndices[i];

      // Get the attributes thanks to the index
      glm::vec3 vertex = verticesTemp[vertexIndex - 1];
      glm::vec2 uv = uvsTemp[uvIndex - 1];
      glm::vec3 normal = normalsTemp[normalIndex - 1];

      // Put the attributes in buffers
      in_vertices.push_back(vertex);
      in_uvs.push_back(uv);
      in_normals.push_back(normal);

    }

    for (unsigned int i = 0; i < in_vertices.size(); i += 3) {

      // Shortcuts for vertices
      glm::vec3& v0 = in_vertices[i + 0];
      glm::vec3& v1 = in_vertices[i + 1];
      glm::vec3& v2 = in_vertices[i + 2];

      // Shortcuts for UVs
      glm::vec2& uv0 = in_uvs[i + 0];
      glm::vec2& uv1 = in_uvs[i + 1];
      glm::vec2& uv2 = in_uvs[i + 2];

      // Edges of the triangle : postion delta
      glm::vec3 deltaPos1 = v1 - v0;
      glm::vec3 deltaPos2 = v2 - v0;

      // UV delta
      glm::vec2 deltaUV1 = uv1 - uv0;
      glm::vec2 deltaUV2 = uv2 - uv0;

      float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
      glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
      glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

      // Set the same tangent for all three vertices of the triangle.
      // They will be merged later, in vboindexer.cpp
      in_tangents.push_back(tangent);
      in_tangents.push_back(tangent);
      in_tangents.push_back(tangent);

      // Same thing for binormals
      in_bitangents.push_back(bitangent);
      in_bitangents.push_back(bitangent);
      in_bitangents.push_back(bitangent);

    }



    // For each input vertex
    for (unsigned int i = 0; i < in_vertices.size(); i++) {

      // Try to find a similar vertex in out_XXXX
      unsigned short index;

      bool found = getSimilarVertexIndex(in_vertices[i], in_uvs[i], in_normals[i], index);

      if (found) { // A similar vertex is already in the VBO, use it instead !
        indices.push_back(index);

        // Average the tangents and the bitangents
        tangents[index] += in_tangents[i];
        bitangents[index] += in_bitangents[i];
      }
      else { // If not, it needs to be added in the output data.
        vertices.push_back(in_vertices[i]);
        uvs.push_back(in_uvs[i]);
        normals.push_back(in_normals[i]);
        tangents.push_back(in_tangents[i]);
        bitangents.push_back(in_bitangents[i]);
        indices.push_back((unsigned short)vertices.size() - 1);
      }
    }


    this->setUp();
  }

  // Render the mesh
  void Draw(GLuint program) {

    GLuint ModelMatrixID = glGetUniformLocation(program, "model");
    
    

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // 3rd attribute buffer : normals
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // 4th attribute buffer : tangents
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


    // 5th attribute buffer : bitangents
    glEnableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    GLint uniTexID = glGetUniformLocation(program, "texID");
    glm::mat4 ModelMatrix = glm::mat4(1.0);

    for (int x = 0; x < obj.xNum; x++) {
      for (int y = 0; y < obj.yNum; y++) {
        for (int z = 0; z < obj.zNum; z++) {
          ModelMatrix = glm::mat4(1.0);

          ModelMatrix = glm::scale(ModelMatrix, glm::vec3(obj.xScale, obj.yScale, obj.zScale));
          ModelMatrix = 
            glm::translate(ModelMatrix, 
              obj.position + 
              glm::vec3(x,
                y, 
                z));

            // Index buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
            glUniform1i(uniTexID, (int)obj.text);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, glm::value_ptr(ModelMatrix));
            // Draw the triangles !
            glDrawElements(
              GL_TRIANGLES,      // mode
              indices.size(),    // count
              GL_UNSIGNED_SHORT, // type
              (void*)0           // element array buffer offset
            );
        }
      }
    }

   

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
  }

};



#endif //MESH_H_