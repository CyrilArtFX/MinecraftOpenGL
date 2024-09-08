#pragma once
#include <ServiceLocator/audio.h>
#include <Audio/audioSound.h>

/**
* The audio null service provider class.
*/
class NullAudio : public Audio
{
public:
	AudioSound LoadSound(std::string soundFile, SoundSettings settings = 0) override { return AudioSound(nullptr, 0); }


	void SetGlobalVolume(const float volume) override {}
	float GetGlobalVolume() override { return 0.0f; }


	std::uint32_t CreateAudioSourceGroup(ChannelSpatialization spatialization, const std::string name = "") override { return 0; }
	void ReleaseAudioSourceGroup(const std::uint32_t index) override {}

	void PlaySoundOnAudioSource(const std::uint32_t index, const AudioSound& sound, const int loop = 0) override {}
	void StopAudioSource(const std::uint32_t index) override {}

	void PauseAudioSource(const std::uint32_t index, const bool pause) override {}
	bool GetAudioSourcePaused(const std::uint32_t index) override { return false; }

	void SetAudioSourceGroupVolume(const std::uint32_t index, const float volume) override {}
	float GetAudioSourceGroupVolume(const std::uint32_t index) override { return 0.0f; }

	void SetAudioSourceGroupPos(const std::uint32_t index, const Vector3 position) override {}
	Vector3 GetAudioSourceGroupPos(const std::uint32_t index) override { return Vector3::zero; }


	void InstantPlaySound2D(const AudioSound& sound, const float volume = 1.0f, const int loop = 0) override {}
	void InstantPlaySound3D(const AudioSound& sound, const Vector3 playPosition, const float volume = 1.0f, const int loop = 0) override {}
};