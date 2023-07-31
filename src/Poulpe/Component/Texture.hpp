#pragma once
#include <volk.h>

namespace Poulpe
{
    class Texture
    {
    public:
        inline std::string GetName() const { return m_Name; }
        inline VkImage GetImage() const { return m_Image; }
        inline VkImageView GetImageView() const { return m_ImageView; }
        inline VkSampler GetSampler() const { return m_Sampler; }
        inline uint32_t GetMipLevels() const { return m_MipLevels; }
        inline uint32_t GetWidth() const { return m_Width; }
        inline uint32_t GetHeight() const { return m_Height; }
        inline uint32_t GetChannels() const { return m_Channels; }
        inline bool IsPublic() const { return m_IsPublic; }

        void SetName(const std::string& name) { m_Name = name; }
        void SetImage(const VkImage& image) { m_Image = image; }
        void SetImageView(const VkImageView& imageView) { m_ImageView = imageView; }
        void SetSampler(const VkSampler& sampler) { m_Sampler = sampler; }
        void SetMipLevels(const uint32_t mipLevels) { m_MipLevels = mipLevels; }
        void SetWidth(const uint32_t width) { m_Width = width; }
        void SetHeight(const uint32_t height) { m_Height = height; }
        void SetChannels(const uint32_t channels) { m_Channels = channels; }
        void SetIsPublic(bool isPublic) { m_IsPublic = isPublic; }

    private:
        std::string m_Name;
        VkImage m_Image;
        VkImageView m_ImageView;
        VkSampler m_Sampler;
        uint32_t m_MipLevels;
        uint32_t m_Width;
        uint32_t m_Height;
        uint32_t m_Channels;
        bool m_IsPublic;
    };
}
