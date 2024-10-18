#include "audioSound.h"
#include <ServiceLocator/locator.h>
#include <ServiceLocator/audio.h>
#include <FMod/fmod.hpp>
#include <FMod/fmod_errors.h>
#include <iostream>

AudioSound::AudioSound(FMOD::Sound* fmodSound_, SoundSettings soundSettings) : FModSound(fmodSound_), Settings(soundSettings)
{
}

AudioSound::~AudioSound()
{
	releaseFMod();
}



bool AudioSound::isValid() const
{
	return FModSound != nullptr;
}

FMOD::Sound* AudioSound::getFMod() const
{
	return FModSound;
}

void AudioSound::releaseFMod()
{
	if (!isValid()) return;

	Audio& audio = Locator::getAudio();
	if (!audio.IsAudioSystemValid()) return;

	FModSound->release();
}

bool AudioSound::hasSetting(SoundSettings setting) const
{
	return Settings & setting;
}

void AudioSound::setMinMaxDistance(float min, float max) const
{
	if (!isValid())
	{
		std::cout << "Audio Sound Error: Tried to set a min and max distance on an invalid sound.\n";
		return;
	}

	if (!hasSetting(ACTIVATE_3D))
	{
		std::cout << "Audio Sound Error: Tried to set a min and max distance on a 2D sound.\n";
		return;
	}

	FMOD_RESULT result;
	result = FModSound->set3DMinMaxDistance(min, max);
	if (result != FMOD_OK)
	{
		std::cout << "Audio Sound Error: Failed to set min and max distance. | Associated FMOD Error: " << FMOD_ErrorString(result) << "\n";
	}
}