#pragma once
#include <nlohmann/json.hpp>
#include <miniaudio.h>

namespace Rbk
{
    class IAudioManager
    {

    public:
        IAudioManager() = default;
        ~IAudioManager() = default;

        virtual void Load(nlohmann::json config) = 0;
        virtual void StartSplash(int index = 0) = 0;
        virtual void StopSplash() = 0;
        virtual void StartAmbient(int index = 0) = 0;
        virtual void StopAmbient() = 0;
        virtual std::vector<std::string> const GetAmbientSound() = 0;
        virtual std::string const GetState() = 0;
        virtual int GetAmbientSoundIndex() = 0;
        virtual std::string GetCurrentAmbientSound() = 0;
        virtual void ToggleLooping() = 0;
    };
}
