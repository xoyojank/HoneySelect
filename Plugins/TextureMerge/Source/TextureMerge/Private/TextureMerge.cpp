// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
// 
// Copyright 2015 Heiko Fink, All Rights Reserved.
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
// 
// Description:
// This code implements the TextureMerge module startup and shutdown functions.
// 

#include "TextureMergePrivatePCH.h"

#define LOCTEXT_NAMESPACE "FTextureMergeModule"

void FTextureMergeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	UE_LOG(LogInit, Log, TEXT("TextureMerge Plugin started."));
}

void FTextureMergeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
	UE_LOG(LogExit, Log, TEXT("TextureMerge Plugin unloaded."));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTextureMergeModule, TextureMerge)