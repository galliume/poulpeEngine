#include "rebulkpch.h"
#include "TextureManager.h"

namespace Rbk
{
    TextureManager::TextureManager(std::shared_ptr<VulkanRenderer> renderer) : m_Renderer(renderer)
    {

    }

    void TextureManager::Load()
    {
        std::future textureFuture = std::async(std::launch::async, [this]() {
            AddTexture("minecraft_water", "assets/mesh/minecraft/water.jpg");
            AddTexture("campfire_tex", "assets/mesh/campfire/Campfire_MAT_BaseColor_01.jpg");
            AddTexture("tree_tex", "assets/mesh/tree/tree.jpg");
            AddTexture("tree_top_tex", "assets/mesh/tree/tree.png");
            AddTexture("crosshair", "assets/texture/crosshair/simple_crosshair.png");
            AddTexture("crosshair2", "assets/texture/crosshair/crosshair2.png");
            AddTexture("moon", "assets/mesh/moon/diffuse.jpg");
            AddTexture("trunk_tree_cartoon", "assets/mesh/tree/cartoon/Trunk_4_Cartoon.jpg");
            AddTexture("grass", "assets/mesh/grass/grass.png");
            AddTexture("rooftiles", "assets/mesh/house/textures/rooftiles/T_darkwood_basecolor.png");
            AddTexture("dark_wood", "assets/mesh/house/textures/wood/T_darkwood_basecolor.png");
            AddTexture("bright_wood", "assets/mesh/house/textures/wood/T_brightwood_basecolor.png");
            AddTexture("rocks", "assets/mesh/house/textures/rocks/rock_bc.jpg");
            AddTexture("dog_base_color", "assets/mesh/doghouse/doghouse0908_PBR_BaseColor.png");
            AddTexture("dog", "assets/mesh/dog/Texture_albedo.jpg");
            AddTexture("fence", "assets/mesh/fence/cit_1001_Diffuse.png");
            AddTexture("minecraft_grass", "assets/mesh/minecraft/Grass_Block_TEX.png");
        });

        std::future skyboxFuture = std::async(std::launch::async, [this]() {
            std::vector<std::string>skyboxImages;
            skyboxImages.emplace_back("assets/texture/skybox/bluesky/right.jpg");
            skyboxImages.emplace_back("assets/texture/skybox/bluesky/left.jpg");
            skyboxImages.emplace_back("assets/texture/skybox/bluesky/top.jpg");
            skyboxImages.emplace_back("assets/texture/skybox/bluesky/bottom.jpg");
            skyboxImages.emplace_back("assets/texture/skybox/bluesky/front.jpg");
            skyboxImages.emplace_back("assets/texture/skybox/bluesky/back.jpg");
            AddSkyBox(skyboxImages);
        });
    }

    void TextureManager::AddSkyBox(std::vector<std::string> skyboxImages)
    {
        int texWidth = 0, texHeight = 0, texChannels = 0;
        std::vector<stbi_uc*>skyboxPixels;

        for (std::string path : skyboxImages) {
            if (!std::filesystem::exists(path)) {
                Rbk::Log::GetLogger()->critical("texture file {} does not exits.", path);
                return;
            }

            stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

            if (!pixels) {
                Rbk::Log::GetLogger()->warn("failed to load skybox texture image %s", path);
                return;
            }

            skyboxPixels.emplace_back(pixels);
        }

        VkImage skyboxImage;
        VkDeviceMemory textureImageMemory;
        uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
        VkCommandPool commandPool = m_Renderer->CreateCommandPool();

        texWidth = static_cast<uint32_t>(texWidth);
        texHeight = static_cast<uint32_t>(texHeight);
        texChannels = static_cast<uint32_t>(texChannels);

        VkCommandBuffer commandBuffer = m_Renderer->AllocateCommandBuffers(commandPool)[0];
        m_Renderer->BeginCommandBuffer(commandBuffer);
        m_Renderer->CreateSkyboxTextureImage(commandBuffer, skyboxPixels, texWidth, texHeight, mipLevels, skyboxImage, textureImageMemory, VK_FORMAT_R8G8B8A8_SRGB);

        VkImageView textureImageView = m_Renderer->CreateSkyboxImageView(skyboxImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels);
        VkSampler textureSampler = m_Renderer->CreateSkyboxTextureSampler(mipLevels);

        m_Skybox.image = skyboxImage;
        m_Skybox.imageMemory = textureImageMemory;
        m_Skybox.imageView = textureImageView;
        m_Skybox.sampler = textureSampler;
        m_Skybox.mipLevels = mipLevels;
        m_Skybox.width = texWidth;
        m_Skybox.height = texHeight;
        m_Skybox.channels = texChannels;

        vkFreeCommandBuffers(m_Renderer->GetDevice(), commandPool, 1, &commandBuffer);
        vkDestroyCommandPool(m_Renderer->GetDevice(), commandPool, nullptr);
    }

    void TextureManager::AddTexture(std::string name, std::string path)
    {
        if (!std::filesystem::exists(path.c_str())) {
            Rbk::Log::GetLogger()->critical("texture file {} does not exits.", path);
            return;
        }

        if (0 != m_Textures.count(name.c_str())) {
            std::cout << "Texture " << name << " already imported" << std::endl;
            return;
        }

        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (!pixels) {
            Rbk::Log::GetLogger()->warn("failed to load texture image %s", name);
            return;
        }

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
        VkCommandPool commandPool = m_Renderer->CreateCommandPool();
        VkCommandBuffer commandBuffer = m_Renderer->AllocateCommandBuffers(commandPool)[0];

        m_Renderer->BeginCommandBuffer(commandBuffer);
        m_Renderer->CreateTextureImage(commandBuffer, pixels, texWidth, texHeight, mipLevels, textureImage, textureImageMemory, VK_FORMAT_R8G8B8A8_SRGB);

        VkImageView textureImageView = m_Renderer->CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels);
        VkSampler textureSampler = m_Renderer->CreateTextureSampler(mipLevels);

        Rbk::Texture texture;
        texture.name = name;
        texture.image = textureImage;
        texture.imageMemory = textureImageMemory;
        texture.imageView = textureImageView;
        texture.sampler = textureSampler;
        texture.mipLevels = mipLevels;
        texture.width = texWidth;
        texture.height = texHeight;
        texture.channels = texChannels;

        m_Textures.emplace(name, texture);
        vkFreeCommandBuffers(m_Renderer->GetDevice(), commandPool, 1, &commandBuffer);
        vkDestroyCommandPool(m_Renderer->GetDevice(), commandPool, nullptr);
    }
}
