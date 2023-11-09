#pragma once
#include <nlohmann/json.hpp>
#include <miniaudio.h>

namespace Poulpe
{
    class IAudioManager
    {

    public:
        IAudioManager() = default;
        ~IAudioManager() = default;

        virtual void load(nlohmann::json config) = 0;
        virtual void startSplash(int index = 0) = 0;
        virtual void stopSplash() = 0;
        virtual void startAmbient(int index = 0) = 0;
        virtual void stopAmbient() = 0;
        virtual std::vector<std::string> const getAmbientSound() = 0;
        virtual std::string const getState() = 0;
        virtual int getAmbientSoundIndex() = 0;
        virtual std::string getCurrentAmbientSound() = 0;
        virtual void toggleLooping() = 0;
    };
}
