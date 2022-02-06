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
				uint32_t i = 3 * index.vertex_index;
				uint32_t j = 2 * index.texcoord_index;

				vertex.pos = {
					attrib.vertices[i + 0],
					attrib.vertices[i + 1],
					attrib.vertices[i + 2]
				};
					
				if (j < attrib.texcoords.size()) {
					vertex.texCoord = {
						attrib.texcoords[j + 0],
						1.0f - attrib.texcoords[j + 1]
					};
				}

				if (i < attrib.normals.size()) {
					vertex.color = {
						attrib.normals[i + 0],
						attrib.normals[i + 1],
						attrib.normals[i + 2]
					};
				}

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