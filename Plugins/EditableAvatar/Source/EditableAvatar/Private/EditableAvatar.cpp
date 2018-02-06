// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "EditableAvatarPrivatePCH.h"



class FEditableAvatar : public IEditableAvatar
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FEditableAvatar, EditableAvatar )



void FEditableAvatar::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
}


void FEditableAvatar::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}



