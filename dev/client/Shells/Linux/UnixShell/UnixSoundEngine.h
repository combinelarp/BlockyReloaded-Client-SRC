#ifndef  __UNIX_SOUND_ENGINE_H__
#define  __UNIX_SOUND_ENGINE_H__

#include "Audio/SoundEngine.h"

namespace LORD
{
	class UnixSoundEngine : public SoundEngine
	{
	public:
		UnixSoundEngine();
		~UnixSoundEngine();

		virtual void end() override;

		// Background music
		void preloadBackgroundMusic(String strFilePath) override;
		void playBackgroundMusic(String strFilePath, bool bLoop, float fMinInterval, float fMaxInterval, unsigned int nParam = 0) override;
		void stopBackgroundMusic(bool bReleaseData = false, unsigned int nParam = 0) override;
		void pauseBackgroundMusic() override;
		void resumeBackgroundMusic() override;
		void rewindBackgroundMusic() override;
		bool willPlayBackgroundMusic();
		bool isBackgroundMusicPlaying();

		float getBackgroundMusicVolume();
		void setBackgroundMusicVolume(float volume);

		float getEffectsVolume();
		void setEffectsVolume(float volume);

		// Sound effects
		unsigned int playEffect(String strFilePath, bool bLoop, float fMinInterval, float fMaxInterval, float fVolume = 1.0F, const LORD::Vector3 &pos = LORD::Vector3::INVALID, unsigned int nParam = 0) override;
		void pauseEffect(unsigned int nSoundId);
		void pauseAllEffects();
		void resumeEffect(unsigned int nSoundId);
		void resumeAllEffects();
		void stopEffect(unsigned int nSoundId);
		void stopAllEffects();
		void preloadEffect(String strFilePath);
		void unloadEffect(String strFilePath);
	};
}

#endif