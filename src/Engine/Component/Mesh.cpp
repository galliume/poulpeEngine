module Engine.Component.Mesh;

namespace Poulpe
{
  void Mesh::addUbos(std::vector<std::vector<UniformBufferObject>> const& ubos)
  {
    _data._ubos.insert(_data._ubos.end(), ubos.begin(), ubos.end());
  }

  void Mesh::addStorageBuffer(Buffer& buffer)
  {
    _storage_buffers.emplace_back(std::move(buffer));
  }
}
