#pragma once

#ifndef MODEL_H_
#define MODEL_H_

#include "glad/glad.h"  //Include order can matter here



#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <SDL.h>
#include <SDL_opengl.h>

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include "Mesh.h"

using namespace std;

GLint loadTexture(const char* path);

class Model
{
public:
  /*  Functions   */
  // Constructor, expects a filepath to a 3D model.
  Model(GLchar* path)
  {
    this->loadModel(path);
  }

  // Draws the model, and thus all its meshes
  void Draw(GLuint shader)
  {
    for (GLuint i = 0; i < this->meshes.size(); i++)
      this->meshes[i].Draw(shader);
  }

private:
  /*  Model Data  */
  vector<mesh> meshes;
  string directory;
  vector<texture> textures_loaded;

  /*  Functions   */
  // Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
  void loadModel(string path)
  {
    // Read file via ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    // Check for errors
    if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
      cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
      return;
    }
    // Retrieve the directory path of the filepath
    this->directory = path.substr(0, path.find_last_of('/'));

    // Process ASSIMP's root node recursively
    this->processNode(scene->mRootNode, scene);
  }

  // Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
  void processNode(aiNode* node, const aiScene* scene)
  {
    // Process each mesh located at the current node
    for (GLuint i = 0; i < node->mNumMeshes; i++)
    {
      // The node object only contains indices to index the actual objects in the scene. 
      // The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
      aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
      this->meshes.push_back(this->processMesh(mesh, scene));
    }
    // After we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (GLuint i = 0; i < node->mNumChildren; i++)
    {
      this->processNode(node->mChildren[i], scene);
    }

  }

  mesh processMesh(aiMesh* imesh, const aiScene* scene)
  {
    // Data to fill
    vector<vertex> vertices;
    vector<GLuint> indices;
    vector<texture> textures;

    // Walk through each of the mesh's vertices
    for (GLuint i = 0; i < imesh->mNumVertices; i++)
    {
      vertex vertex;
      glm::vec3 vector; // We declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
      // Positions
      vector.x = imesh->mVertices[i].x;
      vector.y = imesh->mVertices[i].y;
      vector.z = imesh->mVertices[i].z;
      vertex.position = vector;
      // Normals
      vector.x = imesh->mNormals[i].x;
      vector.y = imesh->mNormals[i].y;
      vector.z = imesh->mNormals[i].z;
      vertex.normal = vector;
      // Texture Coordinates
      if (imesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
      {
        glm::vec2 vec;
        // A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
        // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
        vec.x = imesh->mTextureCoords[0][i].x;
        vec.y = imesh->mTextureCoords[0][i].y;
        vertex.texCoord = vec;
      }
      else
        vertex.texCoord = glm::vec2(0.0f, 0.0f);
      vertices.push_back(vertex);
    }
    // Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (GLuint i = 0; i < imesh->mNumFaces; i++)
    {
      aiFace face = imesh->mFaces[i];
      // Retrieve all indices of the face and store them in the indices vector
      for (GLuint j = 0; j < face.mNumIndices; j++)
        indices.push_back(face.mIndices[j]);
    }
    // Process materials
    if (imesh->mMaterialIndex >= 0)
    {
      aiMaterial* material = scene->mMaterials[imesh->mMaterialIndex];

      // 1. Diffuse maps
      vector<texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
      textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
      // 2. Specular maps
      vector<texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
      textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
      // 3. normal maps
      vector<texture> NormalMaps = this->loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
      textures.insert(textures.end(), NormalMaps.begin(), NormalMaps.end());
    }

    // Return a mesh object created from the extracted mesh data
    return mesh(vertices,indices,textures);
  }

  // Checks all material textures of a given type and loads the textures if they're not loaded yet.
  // The required info is returned as a Texture struct.
  vector<texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
  {
    vector<texture> textures;
    for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
    {
      aiString str;
      mat->GetTexture(type, i, &str);
      // Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
      GLboolean skip = false;
      for (GLuint j = 0; j < textures_loaded.size(); j++)
      {
        if (std::strcmp(textures_loaded[j].path.C_Str(), str.C_Str()) == 0)
        {
          textures.push_back(textures_loaded[j]);
          skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
          break;
        }
      }
      if (!skip)
      {   // If texture hasn't been loaded already, load it
        texture texture;
        texture.id = loadTexture(this->directory.c_str());
        texture.typeName = typeName;
        texture.path = str;
        textures.push_back(texture);
        this->textures_loaded.push_back(texture);  // Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
      }
    }
    return textures;
  }
};





GLint loadTexture(const char* file) {
	//// Allocate Texture ///////
	SDL_Surface* surface = SDL_LoadBMP(file);
	if (surface == NULL) { //If it failed, print the error
		printf("Error: \"%s\"\n", SDL_GetError()); exit(1);
	}
	GLuint textureID;

	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);
	//Load the texture into memory
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture


	//What to do outside 0-1 range
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
	SDL_FreeSurface(surface);
	//// End Allocate Texture ///////
	return textureID;
}


#endif //MODEL_H_
