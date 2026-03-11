export module Engine.Component.Texture;

import std;

import Engine.Core.Volk;

namespace Poulpe
{
  export class Texture
  {
  public:
      inline std::uint32_t getChannels() const { return _channels; }
      inline std::uint32_t getHeight() const { return _height; }
      inline VkImage getImage() const { return _image; }
      inline VkImageView getImageView() const { return _imageview; }
      inline std::uint32_t getMipLevels() const { return _mip_lvls; }
      inline std::vector<std::array<float, 3>> getNormalMap() const { return _normal_map_texture; }
      inline std::string getPath() const { return _path; }
      inline std::string getName() const { return _name; }
      inline VkSampler getSampler() const { return _sampler; }
      inline std::uint32_t getWidth() const { return _width; }

      inline bool isPublic() const { return _is_public; }

      void setChannels(std::uint32_t const channels) { _channels = channels; }
      void setHeight(std::uint32_t const height) { _height = height; }
      void setImage(VkImage const  image) { _image = image; }
      void setImageView(VkImageView const  imageview) { _imageview = imageview; }
      void setIsPublic(bool isPublic) { _is_public = isPublic; }
      void setMipLevels(std::uint32_t const mip_lvls) { _mip_lvls = mip_lvls; }
      void setName(std::string const& name) { _name = name; }
      void setNormalMap(std::vector<std::array<float, 3>> normal_map_texture) { _normal_map_texture = normal_map_texture; }
      void setPath(std::string_view path) { _path = path; }
      void setSampler(VkSampler sampler) { _sampler = sampler; }
      void setWidth(std::uint32_t const width) { _width = width; }

  private:
      std::uint32_t _channels;
      std::uint32_t _height;
      std::uint32_t _mip_lvls;
      std::uint32_t _width;

      bool _is_public;

      std::string _name;
      std::string _path;

      VkImage _image;
      VkImageView _imageview;
      VkSampler _sampler;

      std::vector<std::array<float, 3>> _normal_map_texture;
  };
}
