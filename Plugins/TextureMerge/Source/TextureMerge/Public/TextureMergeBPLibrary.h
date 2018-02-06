// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
//
// Copyright 2015 Heiko Fink, All Rights Reserved.
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
//
// Description:
// This header defines the classes that are provided by the TextureMerge Blueprint Function Library.
//
#pragma once
#include "Engine.h"
#include "TextureMergeBPLibrary.generated.h"

/*
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu.
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/


UCLASS()
class UTextureMergeBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, Category = "Texture Merge", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", DisplayName = "Create", Keywords = "Create composed texture"))
	static UCompositeTexture* Create(UObject* WorldContextObject, const TArray<UTexture2D*>& Layers, const TArray<FVector2D> Offsets);
	UFUNCTION(BlueprintCallable, Category = "Texture Merge", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", DisplayName = "CreateWithTints", Keywords = "Create composed texture with tints"))
	static UCompositeTexture* CreateWithTints(UObject* WorldContextObject, const TArray<UTexture2D*>& Layers, const TArray<FVector2D> Offsets, const TArray<FColor>& LayerTints);

	UFUNCTION(BlueprintCallable, Category = "Texture Merge", meta = (DisplayName = "UpdateLayer", Keywords = "Update single layer"))
	static UCompositeTexture* UpdateLayer(UCompositeTexture* RenderTarget, UTexture2D* Layer, FVector2D Offset, const int32 Index);
	UFUNCTION(BlueprintCallable, Category = "Texture Merge", meta = (DisplayName = "UpdateAllLayers", Keywords = "Update all layers"))
	static UCompositeTexture* UpdateAllLayers(UCompositeTexture* RenderTarget, TArray<UTexture2D*> Layers, const TArray<FVector2D> Offsets);

	UFUNCTION(BlueprintCallable, Category = "Texture Merge", meta = (DisplayName = "UpdateTint", Keywords = "Update single tint layer"))
	static UCompositeTexture* UpdateTint(UCompositeTexture* RenderTarget, const FColor& Tint, int32 Index);
	UFUNCTION(BlueprintCallable, Category = "Texture Merge", meta = (DisplayName = "UpdateAllTints", Keywords = "Update all tint layers"))
	static UCompositeTexture* UpdateAllTints(UCompositeTexture* RenderTarget, const TArray<FColor>& LayerTints);

};
