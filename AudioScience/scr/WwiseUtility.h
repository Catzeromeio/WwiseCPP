#pragma once

#include <AK/SoundEngine/Common/AkMemoryMgr.h>                  // Memory Manager interface
#include <AK/SoundEngine/Common/AkModule.h>                     // Default memory manager
#include <AK/SoundEngine/Common/AkSoundEngine.h>                // Sound Engine
#include <AK/MusicEngine/Common/AkMusicEngine.h>     
#include <AK/SpatialAudio/Common/AkSpatialAudio.h>   
#include <AK/SoundEngine/Common/IAkStreamMgr.h>                 // Streaming Manager
#include <AK/Tools/Common/AkPlatformFuncs.h>                    // Thread defines

#include "AkFilePackageLowLevelIODeferred.h"

#ifndef AK_OPTIMIZED
#include <AK/Comm/AkCommunication.h>
#endif // AK_OPTIMIZED

#include <cassert>

CAkFilePackageLowLevelIODeferred g_lowLevelIO;

bool InitSoundEngine()
{
	AkMemSettings memSettings;
	AK::MemoryMgr::GetDefaultSettings(memSettings);

	if (AK::MemoryMgr::Init(&memSettings) != AK_Success)
	{
		assert(!"Could not create the memory manager.");
		return false;
	}

	//
	// Create and initialize an instance of the default streaming manager. Note
	// that you can override the default streaming manager with your own. 
	//

	AkStreamMgrSettings stmSettings;
	AK::StreamMgr::GetDefaultSettings(stmSettings);

	// Customize the Stream Manager settings here.

	if (!AK::StreamMgr::Create(stmSettings))
	{
		assert(!"Could not create the Streaming Manager");
		return false;
	}

	//
	// Create a streaming device.
	// Note that you can override the default low-level I/O module with your own. 
	//
	AkDeviceSettings deviceSettings;
	AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);

	// Customize the streaming device settings here.

	// CAkFilePackageLowLevelIODeferred::Init() creates a streaming device
	// in the Stream Manager, and registers itself as the File Location Resolver.
	if (g_lowLevelIO.Init(deviceSettings) != AK_Success)
	{
		assert(!"Could not create the streaming device and Low-Level I/O system");
		return false;
	}


	AkInitSettings initSettings;
	AkPlatformInitSettings platformInitSettings;
	AK::SoundEngine::GetDefaultInitSettings(initSettings);
	AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);

	if (AK::SoundEngine::Init(&initSettings, &platformInitSettings) != AK_Success)
	{
		assert(!"Could not initialize the Sound Engine.");
		return false;
	}


	AkMusicSettings musicInit;
	AK::MusicEngine::GetDefaultInitSettings(musicInit);

	if (AK::MusicEngine::Init(&musicInit) != AK_Success)
	{
		assert(!"Could not initialize the Music Engine.");
		return false;
	}

	AkSpatialAudioInitSettings settings; // The constructor fills AkSpatialAudioInitSettings with the recommended default settings. 
	if (AK::SpatialAudio::Init(settings) != AK_Success)
	{
		assert(!"Could not initialize the Spatial Audio.");
		return false;
	}

#ifndef AK_OPTIMIZED
	//
	// Initialize communications (not in release build!)
	//
	AkCommSettings commSettings;
	AK::Comm::GetDefaultInitSettings(commSettings);
	if (AK::Comm::Init(commSettings) != AK_Success)
	{
		assert(!"Could not initialize communication.");
		return false;
	}
#endif // AK_OPTIMIZED

	return true;
}

void ProcessAudio()
{
	// Process bank requests, events, positions, RTPC, etc.
	AK::SoundEngine::RenderAudio();
}

void TermSoundEngine()
{
#ifndef AK_OPTIMIZED
	//
	// Terminate Communication Services
	//
	AK::Comm::Term();
#endif // AK_OPTIMIZED


	AK::MusicEngine::Term();

	if (AK::SoundEngine::IsInitialized())
		AK::SoundEngine::Term();


	if (AK::IAkStreamMgr::Get())
	{
		g_lowLevelIO.Term();
		AK::IAkStreamMgr::Get()->Destroy();
	}

	if (AK::MemoryMgr::IsInitialized())
	{
		AK::MemoryMgr::Term();
	}
}
