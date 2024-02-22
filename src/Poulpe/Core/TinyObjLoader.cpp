#include "TinyObjLoader.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT
//#include "mapbox/earcut.hpp"
#include <tiny_obj_loader.h>

#include <filesystem>

namespace Poulpe
{

    static void computeTangentBasis(
      const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
      const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2,
      glm::vec3& tangent, glm::vec3& bitangent) {

      glm::vec3 e1 = v1 - v0;
      glm::vec3 e2 = v2 - v0;

      float x1 = uv1.x - uv0.x;
      float y1 = uv1.y - uv0.y;
      float x2 = uv2.x - uv0.x;
      float y2 = uv2.y - uv0.y;

      float r = 1.0f / (x1 * y2 - x2 * y1);
      tangent += (e1 * y2 - e2 * y1) * r;
      bitangent += (e2 * x1 - e1 * x2) * r;
    }

    [[clang::no_destroy]] std::vector<material_t> TinyObjLoader::m_TinyObjMaterials{};

    std::vector<TinyObjData> TinyObjLoader::loadData(std::string const & path, [[maybe_unused]] bool shouldInverseTextureY)
    {
        std::vector<TinyObjData> dataList = {};

        tinyobj::ObjReader reader;
        tinyobj::ObjReaderConfig reader_config;

        std::filesystem::path p = path;
        reader_config.mtl_search_path = p.remove_filename().string();
        reader_config.vertex_color = true;

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
          
          material_t mat{};
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
          mat.alphaTexname = cleanName(material.alpha_texname);
          mat.illum = material.illum;

          m_TinyObjMaterials.emplace_back(mat);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        
        size_t vIndex{ 0 };

        for (uint32_t s = 0; s < shapes.size(); s++) {

            TinyObjData data;
            data.name = shapes[s].name + "_" + std::to_string(s);
            std::vector<uint32_t> indices;
            std::vector<uint32_t> materialsID;

            // Loop over faces(polygon)
            size_t index_offset = 0;
            std::vector<int> ids{};
            ids.resize(4);
            int samplerId = 0;
            std::unordered_map<int, int> texidsmap;

            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
                 
                int id = (-1 != shapes[s].mesh.material_ids[f]) ? shapes[s].mesh.material_ids[f] : 0;

                if (!texidsmap.contains(id)) {
                    ids.emplace_back(id);
                    materialsID.emplace_back(id);
                    texidsmap[id] = samplerId;
                    samplerId += 1;
                }
                //@todo work for only 10 textures per face, but well...
                float t =(texidsmap[id] == 0) ? 0.0f : static_cast<float>(texidsmap[id]) / 10.0f;
                glm::vec3 tangent;
                glm::vec3 bitangent;

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {

                    Vertex vertex{};
                    vertex.fidtidBB = glm::vec4(static_cast<float>(f), t, 0.0f, 0.0f);

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
                     tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                     tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                     tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];

                     vertex.color = { red, green, blue };

                    //@todo works only for triangles
                    tinyobj::index_t idx1;
                    tinyobj::index_t idx2;
                    
                    if (static_cast<int>(v) == 0) {
                      idx1 = shapes[s].mesh.indices[index_offset + 1];
                      idx2 = shapes[s].mesh.indices[index_offset + 2];
                   
                      if (idx.texcoord_index >= 0 && idx1.texcoord_index >= 0 && idx2.texcoord_index >= 0) {

                        glm::vec3 pos1 = glm::vec3(attrib.vertices[3 * size_t(idx1.vertex_index) + 0],
                          attrib.vertices[3 * size_t(idx1.vertex_index) + 1],
                          attrib.vertices[3 * size_t(idx1.vertex_index) + 2]);

                        glm::vec3 pos2 = glm::vec3(attrib.vertices[3 * size_t(idx2.vertex_index) + 0],
                          attrib.vertices[3 * size_t(idx2.vertex_index) + 1],
                          attrib.vertices[3 * size_t(idx2.vertex_index) + 2]);

                        glm::vec2 texCoord1 = glm::vec2(attrib.texcoords[2 * size_t(idx1.texcoord_index) + 0],
                          attrib.texcoords[2 * size_t(idx1.texcoord_index) + 1]);
                        if (shouldInverseTextureY) texCoord1.y *= -1.0f;

                        glm::vec2 texCoord2 = glm::vec2(attrib.texcoords[2 * size_t(idx2.texcoord_index) + 0],
                          attrib.texcoords[2 * size_t(idx2.texcoord_index) + 1]);
                        if (shouldInverseTextureY) texCoord2.y *= -1.0f;

                        computeTangentBasis(vertex.pos, pos1, pos2, vertex.texCoord, texCoord1, texCoord2, tangent, bitangent);
                      }
                   }
                   if (uniqueVertices.count(vertex) == 0) {
                     uniqueVertices[vertex] = static_cast<uint32_t>(data.vertices.size());
                     data.vertices.push_back(vertex);
                     vIndex += 1;
                   }
                  data.indices.push_back(uniqueVertices[vertex]);
                }
                index_offset += fv;

                size_t start = data.vertices.size() - vIndex;

                for (size_t i = start; i < data.vertices.size(); i++) {
                  auto& vertex = data.vertices.at(i);

                  //orthonormalize tangent
                  glm::vec3 ot = glm::normalize((tangent - vertex.normal * (glm::dot(tangent, vertex.normal) / glm::dot(vertex.normal, vertex.normal))));

                  //handedness
                  auto w = (glm::dot(glm::cross(tangent, bitangent), vertex.normal) > 0.0f) ? 1.0f : -1.0f;

                  vertex.tangent = glm::vec4{ ot, w };
                  vertex.bitangent = bitangent;
                }
                vIndex = 0;

              // per-face material
              data.materialId = (-1 != shapes[s].mesh.material_ids[f]) ? static_cast<uint32_t>(shapes[s].mesh.material_ids[f]) : 0;
              data.facesMaterialId.emplace_back(shapes[s].mesh.material_ids[f]);
              data.materialsID = materialsID;
            }
            dataList.emplace_back(data);
          }
       return dataList;
    }
}

