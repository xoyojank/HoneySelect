// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
//
// Copyright 2015 Heiko Fink, All Rights Reserved.
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
//
// Description:
// This header defines the class to compose textures.
// See also:
// https://wiki.unrealengine.com/Texture_Merging_With_UCanvasRenderTarget2D
//
#pragma once
#include "EngineMinimal.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "CompositeTexture.generated.h"

///@brief Class for merging multiple textures into one to reduce texture samples and simplify shaders.
///Layers can be changed at any time, however after changing them UpdateResource must be called to finalize the changes.
UCLASS(Blueprintable)
class UCompositeTexture : public UCanvasRenderTarget2D
{
	GENERATED_BODY()

	UCompositeTexture();

	UFUNCTION()
	void PerformMerge(UCanvas* Canvas, int32 Width, int32 Height);

public:

	///@brief Texture layers
	UPROPERTY(EditAnywhere, Category = "Layers")
	TArray<UTexture2D*> Layers;
	///@brief Texture layer offsets
	UPROPERTY(EditAnywhere, Category = "Layers")
	TArray<FVector2D> Offsets;
	///@brief Tint applied to texture layers
	UPROPERTY(EditAnywhere, Category = "Layers")
	TArray<FColor> LayerTints;

	///@brief Creates a layered texture and updates it based on the passed in layers.
	static UCompositeTexture* Create(UObject* WorldContextObject, const TArray<UTexture2D*>& Layers, const TArray<FVector2D> Offsets);
	///@brief Creates a layered texture and updates it like the other version. Also applies tint to layers.
	static UCompositeTexture* Create(UObject* WorldContextObject, const TArray<UTexture2D*>& Layers, const TArray<FVector2D> Offsets, const TArray<FColor>& LayerTints);

	///@brief Update single layer.
	static UCompositeTexture* UpdateLayer(UCompositeTexture* RenderTarget, UTexture2D* Layer, FVector2D Offset, const int32 Index);
	///@brief Update all layers.
	static UCompositeTexture* UpdateAllLayers(UCompositeTexture* RenderTarget, TArray<UTexture2D*>& Layers, const TArray<FVector2D> Offsets);

	///@brief Update single tint layer.
	static UCompositeTexture* UpdateTint(UCompositeTexture* RenderTarget, const FColor& Tint, const int32 Index);
	///@brief Update all tint layers.
	static UCompositeTexture* UpdateAllTints(UCompositeTexture* RenderTarget, const TArray<FColor>& LayerTints);
};
