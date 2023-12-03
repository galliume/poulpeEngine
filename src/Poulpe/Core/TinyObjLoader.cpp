#include "TinyObjLoader.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT
//#include "mapbox/earcut.hpp"
#include "tiny_obj_loader.h"

#include <filesystem>

namespace Poulpe
{
    std::vector<Poulpe::material_t> TinyObjLoader::m_TinyObjMaterials{};

    std::vector<TinyObjData> TinyObjLoader::loadData(std::string const & path, [[maybe_unused]] bool shouldInverseTextureY)
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

          auto cleanName = [](std::string const & name) -> std::string {
              std::string cleaned{ name };
              if (name.size() > 0) {
                  cleaned.resize(cleaned.size() - 4); //remove extension
                  std::replace(cleaned.begin(), cleaned.end(), '\\', '_');
              }
              return cleaned;
          };

          mat.ambientTexname = cleanName(material.ambient_texname);
          mat.diffuseTexname = cleanName(material.diffuse_texname);
          mat.specularTexname = cleanName(material.specular_texname);
          mat.specularHighlightTexname = cleanName(material.specular_highlight_texname);
          mat.bumpTexname = cleanName(material.bump_texname);

          m_TinyObjMaterials.emplace_back(mat);
        }

        std::unordered_map<Poulpe::Vertex, uint32_t> uniqueVertices{};
        uniqueVertices.clear();

        //glm::vec3 pos = glm::vec3(0.0f);
        for (uint32_t s = 0; s < shapes.size(); s++) {

            TinyObjData data;
            std::vector<uint32_t> indices;

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
                    } else {
                        vertex.normal = { 0, 0, 0 };
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

                    vertex.tangent = glm::vec4(0.0f);

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

            //tangeant
            std::vector<glm::vec3> tangents{};
            std::vector<glm::vec3> bitangents{};
            auto def = glm::vec3(0.0f);

            for (size_t i = 0; i < data.indices.size(); ++i) {
                tangents.emplace_back(def);
                bitangents.emplace_back(def);
            }

            for (size_t i = 0; i < data.indices.size(); i += 3)
            {
                uint32_t i0 = data.indices[i + 0];
                uint32_t i1 = data.indices[i + 1];
                uint32_t i2 = data.indices[i + 2];
 
                glm::vec3 & p0 = data.vertices[i0].pos;
                glm::vec3 & p1 = data.vertices[i1].pos;
                glm::vec3 & p2 = data.vertices[i2].pos;
 
                glm::vec2 & w0 = data.vertices[i0].texCoord;
                glm::vec2 & w1 = data.vertices[i1].texCoord;
                glm::vec2 & w2 = data.vertices[i2].texCoord;

                glm::vec3 e1 = p1 - p0;
                glm::vec3 e2 = p2 - p0;

                float x1 = w1.x - w0.x;
                float y1 = w1.y - w0.y;
                float x2 = w2.x - w0.x;
                float y2 = w2.y - w0.y;

                float r = 1.0f / (x1 * y2 - x2 * y1);
                glm::vec3 t = (e1 * y2 - e2 * y1) * r;
                glm::vec3 b = (e2 * x1 - e1 * x2) * r;

                tangents[i0] += t;
                tangents[i1] += t;
                tangents[i2] += t;

                bitangents[i0] += b;
                bitangents[i1] += b;
                bitangents[i2] += b;
            }

            for (size_t i = 0; i < data.vertices.size(); ++i) {
                auto t = tangents[i];
                auto b = bitangents[i];
                auto n = data.vertices.at(i).normal;

                auto a =  glm::normalize((t - n * (glm::dot(t, n) / glm::dot(n, n))));
                auto w = (glm::dot(glm::cross(t, b), n) > 0.0f) ? 1.0f : -1.0f;

                //data.vertices.at(i).texCoord.y *= w;
                data.vertices.at(i).tangent = glm::vec4{a.x, a.y, a.z, w};
            }

            dataList.emplace_back(data);
        }

        return dataList;
    }
}
