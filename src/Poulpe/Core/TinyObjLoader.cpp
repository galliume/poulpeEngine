#include "TinyObjLoader.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT
//#include "mapbox/earcut.hpp"
#include "tiny_obj_loader.h"

#include <filesystem>

namespace Poulpe
{
    std::vector<Poulpe::material_t> TinyObjLoader::m_TinyObjMaterials{};

    std::vector<TinyObjData> TinyObjLoader::loadData(std::string const & path, bool shouldInverseTextureY)
    {
        std::vector<TinyObjData> dataList = {};

        tinyobj::ObjReader reader;
        tinyobj::ObjReaderConfig reader_config;

        std::filesystem::path p = path;
        reader_config.mtl_search_path = p.remove_filename().string();

        if (!reader.ParseFromFile(path, reader_config)) {
            if (!reader.Error().empty()) {
                PLP_ERROR("TinyObjReader: {}", reader.Error());
            }
            exit(1);
        }

        if (!reader.Warning().empty()) {
            PLP_WARN("TinyObjReader: {}", reader.Warning());
        }

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();
        auto& materials = reader.GetMaterials();


        for (auto & material : materials) {
          
          Poulpe::material_t mat{};
          mat.name = material.name;
          mat.ambient = { material.ambient[0], material.ambient[1], material.ambient[2] };
          mat.diffuse = { material.diffuse[0], material.diffuse[1], material.diffuse[2] };
          mat.specular = { material.specular[0], material.specular[1], material.specular[2] };
          mat.transmittance = { material.transmittance[0], material.transmittance[1], material.transmittance[2] };
          mat.emission = { material.emission[0], material.emission[1], material.emission[2] };
          mat.shininess = material.shininess;
          mat.ior = material.ior;
          mat.dissolve = material.dissolve;
          mat.ambientTexname = material.ambient_texname;

          std::string n;
          if (material.diffuse_texname.size() > 0) {
            n = material.diffuse_texname;
            n.resize(n.size() - 4); //remove extension

            std::replace(n.begin(), n.end(), '\\', '_');
          }

          mat.diffuseTexname = n;
          mat.specularTexname = material.specular_texname;
          mat.specularHighlightTexname = material.specular_highlight_texname;
          mat.bumpTexname = material.bump_texname;

          m_TinyObjMaterials.emplace_back(mat);
        }

        std::unordered_map<Poulpe::Vertex, uint32_t> uniqueVertices{};
        uniqueVertices.clear();

        //glm::vec3 pos = glm::vec3(0.0f);

        for (uint32_t s = 0; s < shapes.size(); s++) {

            TinyObjData data;

            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {

                    Poulpe::Vertex vertex{};
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                    tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                    tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                    vertex.pos = { vx, vy, vz };

                    // Check if `normal_index` is zero or positive. negative = no normal data
                    if (idx.normal_index >= 0) {
                        tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                        tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                        tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

                        vertex.normal = { nx, ny, nz };
                    }

                    // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                    if (idx.texcoord_index >= 0) {
                        tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                        tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

                        vertex.texCoord = { tx, ty };
                        if (shouldInverseTextureY) vertex.texCoord.y *= -1.0f;
                    }

                    // Optional: vertex colors
                     //tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                     //tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                     //tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
                    // vertex.color = { red, green, blue };

                    if (uniqueVertices.count(vertex) == 0) {
                        uniqueVertices[vertex] = static_cast<uint32_t>(data.vertices.size());
                        data.vertices.push_back(vertex);
                    }

                    data.indices.push_back(uniqueVertices[vertex]);
                }

                index_offset += fv;

                // per-face material
                data.materialId = (-1 != shapes[s].mesh.material_ids[f]) ? shapes[s].mesh.material_ids[f] : 0;
                data.facesMaterialId.emplace_back(shapes[s].mesh.material_ids[f]);
            }
            dataList.emplace_back(data);
        }

        return dataList;
    }
}
