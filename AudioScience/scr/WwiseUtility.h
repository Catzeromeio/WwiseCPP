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
#include "../../WwiseProject/GeneratedSoundBanks/Wwise_IDs.h"

using namespace AK;
using namespace AK::SoundEngine;

#define BANKNAME_INIT L"Init.bnk"
#define BANKNAME_FIRE_USERDEFINE L"FireBank_UserDefine.bnk"

CAkFilePackageLowLevelIODeferred g_lowLevelIO;

AkGameObjectID TargetGameObj = 3;

void LoadBanks()
{
	g_lowLevelIO.SetBasePath(AKTEXT("D:/Projects/WwiseCPP/WwiseProject/GeneratedSoundBanks/Windows/"));
	g_lowLevelIO.SetBasePath(AKTEXT("D:/Projects/WwiseCPP/WwiseProject/GeneratedSoundBanks/Windows/Event/"));
	g_lowLevelIO.SetBasePath(AKTEXT("D:/Projects/WwiseCPP/WwiseProject/GeneratedSoundBanks/Windows/Media/"));
	StreamMgr::SetCurrentLanguage(AKTEXT("English(US)"));

	//设置听者
	AkGameObjectID MY_DEFAULT_LISTENER = 0;
	// 注册主要听者。
	AK::SoundEngine::RegisterGameObj(MY_DEFAULT_LISTENER, "My Default Listener");
	// 将一个听者设置为默认。
	AK::SoundEngine::SetDefaultListeners(&MY_DEFAULT_LISTENER, 1);


	AkBankID bankID; // Not used. These banks can be unloaded with their file name.
	AKRESULT eResult = LoadBank(BANKNAME_INIT, bankID);
	eResult = LoadBank(BANKNAME_FIRE_USERDEFINE, bankID);

	eResult = LoadBank(L"Fire.bnk", bankID,AkBankTypeEnum::AkBankType_Event);
	SDL_Log("load fire bank : %d", eResult);

	AkUniqueID fireID = EVENTS::FIRE;
	eResult = PrepareEvent(PreparationType::Preparation_Load, &fireID, 1);
	SDL_Log("prepare fire bank : %d", eResult);

	RegisterGameObj(TargetGameObj);
}

void Unload()
{
	AkUniqueID fireID = EVENTS::FIRE;
	PrepareEvent(PreparationType::Preparation_Unload, &fireID, 1);
	PrepareBank(PreparationType::Preparation_Unload, fireID);
}

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
	initSettings.bEnableGameSyncPreparation = true;
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

	LoadBanks();

	return true;
}

void Fire()
{
	AkPlayingID playingID = PostEvent(AK::EVENTS::FIRE2, TargetGameObj);
	//AkPlayingID playingID = PostEvent(AK::EVENTS::FIRE, TargetGameObj);
	SDL_Log("playing id is : %d", playingID);
}

void ProcessAudio()
{
	// Process bank requests, events, positions, RTPC, etc.
	AK::SoundEngine::RenderAudio();
}

void TermSoundEngine()
{
	Unload();

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
