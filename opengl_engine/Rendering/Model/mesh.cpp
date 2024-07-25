#include "mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, int matId) : 
	vertexArray(vertices, indices), materialIndex(matId)
{
}

Mesh::Mesh() :
	vertexArray(), materialIndex(0)
{
}

Mesh::Mesh(const Mesh& other) :
	vertexArray(other.vertexArray), materialIndex(other.materialIndex)
{
}

Mesh::~Mesh()
{
}


void Mesh::draw(bool drawAsLines)
{
	//  assume the shader is already in use (the rendering process should have done it)

	vertexArray.setActive();

	GLuint draw_method = drawAsLines ? GL_LINE_STRIP : GL_TRIANGLES;

	if (vertexArray.getUseEBO())
	{
		glDrawElements(draw_method, vertexArray.getNBIndices(), GL_UNSIGNED_INT, 0);
	}
	else
	{
		glDrawArrays(draw_method, 0, vertexArray.getNBVertices());
	}
}

void Mesh::deleteObjects()
{
	vertexArray.deleteObjects();
}