#pragma once
#include "MeshRendering.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>


//load a model with assimp lib and translate it into multiple meshes
class Model
{
public:
	std::vector<Mesh> meshes;

	Model(std::string const &path)
	{
		loadModel(path);
	}

	void Draw(unsigned int shader) {
		for (Mesh curMesh : meshes)
		{
			curMesh.Draw(shader);
		}
	}
	void DrawModelViewProjection(unsigned int shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
		for (Mesh curMesh : meshes)
		{
			curMesh.DrawModelViewProjection(shader, model, view, projection);
		}
	}

private:
	// model data
	std::string directory; //dir to hold the model data

	void loadModel(std::string const &path) {
		//loading
		Assimp::Importer importer;
		const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs); //post processing options: generate triangles if not present, flips text coords if neccesary

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) //check for scene and root node not null
		{
			std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));

		//convert scene into Meshes
		processNode(scene->mRootNode, scene);

		std::cout << "Loaded Mesh: " << path << " From Dir: " << directory << std::endl;
	}

	void processNode(aiNode *node, const aiScene *scene) { //recursively process all nodes in scene into Meshes
		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh processMesh(aiMesh *mesh, const aiScene *scene) { //create Meshes from aiMesh
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			// process vertex positions, normals and texture coordinates
			glm::vec3 pos;
			pos.x = mesh->mVertices[i].x;
			pos.y = mesh->mVertices[i].y;
			pos.z = mesh->mVertices[i].z;
			vertex.Position = pos;

			if (mesh->HasVertexColors(0)) {
				glm::vec3 color;
				color.x = mesh->mColors[0][i].r;
				color.y = mesh->mColors[0][i].g;
				color.z = mesh->mColors[0][i].b;
				vertex.Color = color;
			}
			else
				vertex.Color = glm::vec3(0.0f, 71.8f, 92.2f); //cyan

			if (mesh->HasNormals()) {
				glm::vec3 normal;
				normal.x = mesh->mNormals[i].x;
				normal.y = mesh->mNormals[i].y;
				normal.z = mesh->mNormals[i].z;
				vertex.Normal = normal;
			}
			else
				vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f);

			if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);


			vertices.push_back(vertex);
		}

		// process indices
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i]; //go over each face and store indices of vertices for drawing
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		
		// TODO process material

		return Mesh(vertices, indices, textures);
	}

	//TODO
	std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};