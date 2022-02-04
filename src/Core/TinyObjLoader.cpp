#include "TinyObjLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace Rbk
{
	bool TinyObjLoader::LoadMesh(Rbk::Mesh& mesh, const char* path)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path)) {
			throw std::runtime_error(warn + err);
		}

		std::unordered_map<Rbk::Vertex, uint32_t> uniqueVertices{};

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {

				Rbk::Vertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};


				/*			vertex.texCoord = {
								attrib.texcoords[2 * index.texcoord_index + 0],
								1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
							};*/


				vertex.color = {
					attrib.normals[3 * index.vertex_index + 0],
					attrib.normals[3 * index.vertex_index + 1],
					attrib.normals[3 * index.vertex_index + 2]
				};

				mesh.vertices.push_back(vertex);

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
					mesh.vertices.push_back(vertex);
				}

				mesh.indices.push_back(uniqueVertices[vertex]);
			}
		}

		return true;
	}
}