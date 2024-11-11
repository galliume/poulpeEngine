#pragma once
#include <volk.h>

namespace Poulpe
{
    class Texture
    {
    public:
        inline uint32_t getChannels() const { return _channels; }
        inline uint32_t getHeight() const { return _height; }
        inline VkImage getImage() const { return _image; }
        inline VkImageView getImageView() const { return _imageview; }
        inline uint32_t getMipLevels() const { return _mip_lvls; }
        inline std::vector<std::array<float, 3>> getNormalMap() const { return _normal_map_texture; }
        inline std::string getPath() const { return _path; }
        inline std::string getName() const { return _name; }
        inline VkSampler getSampler() const { return _sampler; }
        inline uint32_t getWidth() const { return _width; }

        inline bool isPublic() const { return _is_public; }

        void setChannels(const uint32_t channels) { _channels = channels; }
        void setHeight(const uint32_t height) { _height = height; }
        void setImage(const VkImage& image) { _image = image; }
        void setImageView(const VkImageView& imageview) { _imageview = imageview; }
        void setIsPublic(bool isPublic) { _is_public = isPublic; }
        void setMipLevels(const uint32_t mip_lvls) { _mip_lvls = mip_lvls; }
        void setName(const std::string & name) { _name = name; }
        void setNormalMap(std::vector<std::array<float, 3>> normal_map_texture) { _normal_map_texture = normal_map_texture; }
        void setPath(std::string_view path) { _path = path; }
        void setSampler(const VkSampler & sampler) { _sampler = sampler; }
        void setWidth(const uint32_t width) { _width = width; }

    private:
        uint32_t _channels;
        uint32_t _height;
        uint32_t _mip_lvls;
        uint32_t _width;

        bool _is_public;

        std::string _name;
        std::string _path;

        VkImage _image;
        VkImageView _imageview;
        VkSampler _sampler;

        std::vector<std::array<float, 3>> _normal_map_texture;
    };
}
