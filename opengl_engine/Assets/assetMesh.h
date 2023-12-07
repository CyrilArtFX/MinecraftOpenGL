#pragma once
#include <Rendering/Model/mesh.h>

#include <vector>
#include <string>


struct MeshCollection
{
	std::vector<Mesh> collection;
};


class AssetMesh
{
public:
	/**
	* Load a single mesh from handcoded vertices. 
	*/
	static Mesh LoadSingleMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices = {});

	/**
	* Load a single mesh from a 3D file using assimp.
	* WARNING : It will load only the root mesh.
	*/
	static Mesh LoadSingleMesh(std::string filepath);

	/**
	* Load an entire model from a 3D file using assimp.
	* Will return the meshes in a vector.
	*/
	static MeshCollection LoadMeshCollection(std::string filepath);
};