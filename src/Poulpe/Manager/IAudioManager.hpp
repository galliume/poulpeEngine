#pragma once

#include <miniaudio.h>

#include <nlohmann/json.hpp>

namespace Poulpe
{
    class IAudioManager
    {

    public:
        IAudioManager() = default;
        virtual ~IAudioManager() = default;

        virtual std::vector<std::string> const getAmbientSound() = 0;
        virtual unsigned int getAmbientSoundIndex() = 0;
        virtual std::string getCurrentAmbientSound() = 0;
        virtual std::string const getState() = 0;
        virtual void load(nlohmann::json config) = 0;
        virtual void startAmbient(unsigned int const index = 0) = 0;
        virtual void stopAmbient() = 0;
        virtual void toggleLooping() = 0;
    };
}
