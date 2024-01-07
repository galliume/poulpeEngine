#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-identifier"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wformat"
#pragma clang diagnostic ignored "-Wunused-function"
#include <miniaudio.h>
#pragma clang diagnostic pop

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
