#include "TinyObjLoader.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
  #define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "mapbox/earcut.hpp"
#include <tiny_obj_loader.h>

#include <filesystem>

namespace Poulpe
{
  void TinyObjLoader::loadData(
    std::string const& path,
    bool const flip_Y,
    std::function<void(
      TinyObjData const& _data,
      std::vector<material_t> const& materials,
      bool const exists)> callback)
  {
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig reader_config;

    std::filesystem::path p = path;
    reader_config.mtl_search_path = p.remove_filename().string();
    reader_config.vertex_color = true;
    reader_config.triangulate = true;

    PLP_WARN("{}", path);

    {
      SCOPED_TIMER();
      if (!reader.ParseFromFile(path, reader_config)) {
        if (!reader.Error().empty()) {
          PLP_ERROR("TinyObjReader: {}", reader.Error());
        }
        exit(1);
      }
    }
    if (!reader.Warning().empty()) {
      PLP_WARN("TinyObjReader: {}", reader.Warning());
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    //@todo refacto...
    std::unordered_map<uint32_t, std::array<unsigned int, 3>> triangles;
    std::unordered_map<uint32_t, Vertex*> listVertex;
    std::unordered_map<uint32_t, Vertex*> iToVertex;

    //uint32_t i = 0, k = 0;

    //glm::vec3 pos = glm::vec3(0.0f);
    size_t const size = shapes.size();
    size_t count{ 0 };

    for (uint32_t s = 0; s < shapes.size(); s++) {
      auto const & shape = shapes[s];

      std::jthread loading("LoadingOBJ", [=]() {

        std::vector<material_t> meshMaterials;
        meshMaterials.reserve(materials.size());

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

          mat.name_texture_ambient = TinyObjLoader::cleanName(material.ambient_texname);
          mat.name_texture_diffuse = TinyObjLoader::cleanName(material.diffuse_texname);
          mat.name_texture_specular = TinyObjLoader::cleanName(material.specular_texname);
          mat.name_texture_specular_highlight = TinyObjLoader::cleanName(material.specular_highlight_texname);
          mat.name_texture_bump = TinyObjLoader::cleanName(material.bump_texname);
          mat.name_texture_alpha = TinyObjLoader::cleanName(material.alpha_texname);
          mat.illum = material.illum;

          meshMaterials.emplace_back(mat);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        TinyObjData data;
        data.name = shape.name + "_" + std::to_string(s);
        std::vector<uint32_t> indices;
        std::vector<uint32_t> materials_ID;

        // Loop over faces(polygon)
        size_t index_offset{ 0 };
        std::vector<int> ids{};
        ids.resize(4);
        int samplerId{ 0 };
        std::unordered_map<int, int> texidsmap;

        texidsmap.reserve(shape.mesh.nu_face_vertices.size());

        for (size_t f = 0; f < shape.mesh.nu_face_vertices.size(); f++) {
            size_t fv = size_t(shape.mesh.nu_face_vertices[f]);
                 
            int id = (-1 != shape.mesh.material_ids[f]) ? shape.mesh.material_ids[f] : 0;

            if (!texidsmap.contains(id)) {
                ids.emplace_back(id);
                materials_ID.emplace_back(id);
                texidsmap[id] = samplerId;
                samplerId += 1;
            }
            //@todo work for only 10 textures per face, but well...
            float t =(texidsmap[id] == 0) ? 0.0f : static_cast<float>(texidsmap[id]) / 10.0f;

            // Loop over vertices in the face.
            data.vertices.reserve(fv);

            for (size_t v = 0; v < fv; v++) {

                Vertex vertex{};
                vertex.fidtidBB = glm::vec4(static_cast<float>(f), t, 0.0f, 0.0f);

                // access to vertex
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
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
                    if (flip_Y) vertex.texCoord.y *= -1.0f;
                }

              // Optional: vertex colors
                tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];

                vertex.color = { red, green, blue };

                vertex.tangent = glm::vec4(0.0f);

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(data.vertices.size());
                    data.vertices.push_back(vertex);
                }
                data.indices.push_back(uniqueVertices[vertex]);
            }

            index_offset += fv;

            // per-face material
            data.material_ID = (-1 != shape.mesh.material_ids[f]) ? static_cast<uint32_t>(shape.mesh.material_ids[f]) : 0;
            data.face_material_ID.emplace_back(shape.mesh.material_ids[f]);
            data.materials_ID = materials_ID;
        }
        callback(data, meshMaterials, false);
      });
    }
  }

  std::string const TinyObjLoader::cleanName(std::string const & name)
  {
    std::string cleaned{};

    if (name.size() > 0) {
      size_t lastindex = name.find_last_of(".");
      cleaned = name.substr(0, lastindex);

      std::replace(cleaned.begin(), cleaned.end(), '\\', '_');
      std::replace(cleaned.begin(), cleaned.end(), '/', '_');
    }

    return cleaned;
  };
}
