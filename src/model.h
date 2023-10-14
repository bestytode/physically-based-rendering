#pragma once

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif 

#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GL/glew.h>

#include "mesh.h"
#include "shader.h"

unsigned int TextureFromFile(const char* path, const std::string& directory);

class Model
{
public:
	Model() = delete;

	Model(const std::string& _filePath) {
		LoadModel(_filePath);
	}

    // Draws the model using the provided shader.
    //
    // Usage:
    //   - To draw the model using only specific types of textures:
    //       model.Draw(shader, {"texture_diffuse", "texture_specular"});
    //   - To draw the model using all available textures:
    //       model.Draw(shader);
    //
	void Render(Shader& _shader, const std::vector<std::string>& textureTypeToUse = {}) {
		for (size_t i = 0; i < meshes.size(); i++)
			meshes[i].Render(_shader, textureTypeToUse);
	}

	const std::vector<Mesh>& GetMesh() const{
		return meshes;
	}

	// CalculateAABB computes the Axis-Aligned Bounding Box (AABB) for the model.
    // It returns a pair of glm::vec3 representing the minimum and maximum vertex positions that define the AABB.
	std::pair<glm::vec3, glm::vec3> CalculateAABB();

private:
	void LoadModel(const std::string& _filePath);

	void ProcessNode(aiNode* node, const aiScene* scene);

	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type,
		std::string typeName);

private:
	std::vector<Mesh>meshes; // Meshes where actually hold the data
	std::vector<Texture>textures_loaded; // To prevent double texture loading
	std::string directory;

	bool firstTime = true; // the first time to load mesh
};

std::pair<glm::vec3, glm::vec3> Model::CalculateAABB()
{
	glm::vec3 minVertexPos = glm::vec3(FLT_MAX);
	glm::vec3 maxVertexPos = glm::vec3(-FLT_MAX);

	for (const auto& mesh : this->meshes) {
		for (const auto& vertex : mesh.vertices) {
			minVertexPos.x = std::min(minVertexPos.x, vertex.position.x);
			minVertexPos.y = std::min(minVertexPos.y, vertex.position.y);
			minVertexPos.z = std::min(minVertexPos.z, vertex.position.z);

			maxVertexPos.x = std::max(maxVertexPos.x, vertex.position.x);
			maxVertexPos.y = std::max(maxVertexPos.y, vertex.position.y);
			maxVertexPos.z = std::max(maxVertexPos.z, vertex.position.z);
		}
	}

	return { minVertexPos, maxVertexPos };
}

// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
void Model::LoadModel(const std::string& _filePath)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(_filePath, 
		aiProcess_Triangulate | aiProcess_FlipUVs );

#ifdef _DEBUG
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
#endif 

	directory = _filePath.substr(0, _filePath.find_last_of('/'));

	ProcessNode(scene->mRootNode, scene);
}

// Iterate through all Node, from scene->mRootNode
inline void Model::ProcessNode(aiNode* currentNode, const aiScene* scene)
{
	for (size_t i = 0; i < currentNode->mNumMeshes; i++) {
		// mMeshes in node store the index,
		// where mMeshes in scene hold the actual objects
		aiMesh* mesh = scene->mMeshes[currentNode->mMeshes[i]];

		// Using emplace_back to construct a Mesh object in-place at the end of the meshes vector.
		// The ProcessMesh function returns a Mesh object, and its ownership is transferred
		// to the vector using move semantics (via the Mesh object's move constructor).
		meshes.emplace_back(ProcessMesh(mesh, scene));
	}

	for (size_t i = 0; i < currentNode->mNumChildren; i++) {
		ProcessNode(currentNode->mChildren[i], scene);
	}
}

// Retriving information from aiMesh and aiScene, converting all to our own Mesh
inline Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex>vertices;
	std::vector<unsigned int>indices;
	std::vector<Texture>textures;
	bool hasTangentsAndBitangents = mesh->HasTangentsAndBitangents();

	// Process vertices
	for (size_t i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;

		// Positions
		vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

		// Normals
		if (mesh->HasNormals())
			vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

		// Texture Coordinates
		if (mesh->mTextureCoords[0])
			vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		else
			vertex.texCoords = glm::vec2(0.0f, 0.0f);

		// Tangents and Bitangents
		if (hasTangentsAndBitangents) {
			vertex.Tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			vertex.Bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
		}

		vertices.push_back(vertex);
	}

	// Process indices
	for (size_t i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	// Process textures based on shader naming conventions
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// 1. diffuse maps
		std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// 2. specular maps
		std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		// 3. normal maps
		std::vector<Texture> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		// 4. height maps
		std::vector<Texture> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
	}

#ifdef  _DEBUG
	if (this->firstTime) {
		std::cout << "Mesh " << (hasTangentsAndBitangents ? "has" : "does not have") << " tangents and bitangents.\n";
		this->firstTime = false;
	}
#endif 

	return Mesh(vertices, indices, textures, hasTangentsAndBitangents);
}

// Return a vector contains Texture, retriving texture information from aiMaterial to our own textures and textures_loaded
inline std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	std::vector<Texture> textures;

	// Iterate all textures, i stands for the number of texture counts of current texture type
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {

		// Get the (i)th texture of current type
		aiString str;
		mat->GetTexture(type, i, &str);

		bool skip = false;
		for (size_t j = 0; j < textures_loaded.size(); j++) {
			// Iterate through textures_loaded vector, to check if it already exisits.
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
				// Use the located texture directly since we've already loaded
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}

		// If the texture has not been loaded yet, add it
		if (!skip) {
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), directory);
			texture.type = typeName;
			texture.path = str.C_Str();

			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}
	}
	return textures;
}

// Load a texture and return the actual id.
unsigned int TextureFromFile(const char* path, const std::string& directory)
{
	// Directory + filepath 
	std::string filename = std::string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format = GL_RED;
		if (nrComponents == 1) format = GL_RED;
		if (nrComponents == 3) format = GL_RGB;
		if (nrComponents == 4) format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}