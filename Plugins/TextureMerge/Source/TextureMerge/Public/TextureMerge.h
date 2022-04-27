// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
// 
// Copyright 2015 Heiko Fink, All Rights Reserved.
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
// 
// Description:
// This header defines the classes that are needed to startup and shutdown the module.
// 

#pragma once

#include "Modules/ModuleManager.h"



class FTextureMergeModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};