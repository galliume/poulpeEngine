#pragma once
#include <volk.h>

namespace Poulpe
{
    class Texture
    {
    public:
        inline std::string getName() const { return m_Name; }
        inline std::string getPath() const { return m_Path; }
        inline VkImage getImage() const { return m_Image; }
        inline VkImageView getImageView() const { return m_ImageView; }
        inline VkSampler getSampler() const { return m_Sampler; }
        inline uint32_t getMipLevels() const { return m_MipLevels; }
        inline uint32_t getWidth() const { return m_Width; }
        inline uint32_t getHeight() const { return m_Height; }
        inline uint32_t getChannels() const { return m_Channels; }
        inline bool isPublic() const { return m_IsPublic; }
        inline std::vector<std::array<float, 3>> getNormalMap() const { return m_NormalMapTexture; }

        void setName(const std::string& name) { m_Name = name; }
        void setImage(const VkImage& image) { m_Image = image; }
        void setImageView(const VkImageView& imageView) { m_ImageView = imageView; }
        void setSampler(const VkSampler& sampler) { m_Sampler = sampler; }
        void setMipLevels(const uint32_t mipLevels) { m_MipLevels = mipLevels; }
        void setWidth(const uint32_t width) { m_Width = width; }
        void setHeight(const uint32_t height) { m_Height = height; }
        void setChannels(const uint32_t channels) { m_Channels = channels; }
        void setIsPublic(bool isPublic) { m_IsPublic = isPublic; }
        void setPath(std::string_view path) { m_Path = path; }
        void setNormalMap(std::vector<std::array<float, 3>> normalMapTexture) { m_NormalMapTexture = normalMapTexture; }

    private:
        std::string m_Name;
        std::string m_Path;
        VkImage m_Image;
        VkImageView m_ImageView;
        VkSampler m_Sampler;
        uint32_t m_MipLevels;
        uint32_t m_Width;
        uint32_t m_Height;
        uint32_t m_Channels;
        bool m_IsPublic;
        std::vector<std::array<float, 3>> m_NormalMapTexture;
    };
}
