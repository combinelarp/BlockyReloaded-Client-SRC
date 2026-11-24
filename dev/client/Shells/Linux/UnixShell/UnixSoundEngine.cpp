#include "UnixSoundEngine.h"

namespace LORD
{
	UnixSoundEngine::UnixSoundEngine()
	{
	}

	UnixSoundEngine::~UnixSoundEngine()
	{
	}

	void UnixSoundEngine::end()
	{
		// no-op
	}

	void UnixSoundEngine::preloadBackgroundMusic(String strFilePath)
	{
		// no-op
	}

	void UnixSoundEngine::playBackgroundMusic(String strFilePath, bool bLoop, float fMinInterval, float fMaxInterval, unsigned int nParam)
	{
		// no-op
	}

	void UnixSoundEngine::stopBackgroundMusic(bool bReleaseData, unsigned int nParam)
	{
		// no-op
	}

	void UnixSoundEngine::pauseBackgroundMusic()
	{
		// no-op
	}

	void UnixSoundEngine::resumeBackgroundMusic()
	{
		// no-op
	}

	void UnixSoundEngine::rewindBackgroundMusic()
	{
		// no-op
	}

	bool UnixSoundEngine::willPlayBackgroundMusic()
	{
		return false;
	}

	bool UnixSoundEngine::isBackgroundMusicPlaying()
	{
		return false;
	}

	float UnixSoundEngine::getBackgroundMusicVolume()
	{
		return 1.0f;
	}

	void UnixSoundEngine::setBackgroundMusicVolume(float volume)
	{
		// no-op
	}

	float UnixSoundEngine::getEffectsVolume()
	{
		return 1.0f;
	}

	void UnixSoundEngine::setEffectsVolume(float volume)
	{
		// no-op
	}

	unsigned int UnixSoundEngine::playEffect(String strFilePath, bool bLoop, float fMinInterval, float fMaxInterval, float fVolume, const LORD::Vector3 &pos, unsigned int nParam)
	{
		return 0;
	}

	void UnixSoundEngine::pauseEffect(unsigned int nSoundId) {}
	void UnixSoundEngine::pauseAllEffects() {}
	void UnixSoundEngine::resumeEffect(unsigned int nSoundId) {}
	void UnixSoundEngine::resumeAllEffects() {}
	void UnixSoundEngine::stopEffect(unsigned int nSoundId) {}
	void UnixSoundEngine::stopAllEffects() {}
	void UnixSoundEngine::preloadEffect(String strFilePath) {}
	void UnixSoundEngine::unloadEffect(String strFilePath) {}
}