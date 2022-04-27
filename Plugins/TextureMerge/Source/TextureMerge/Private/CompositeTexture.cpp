// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
//
// Copyright 2015 Heiko Fink, All Rights Reserved.
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
//
// Description:
// This code implements the member functions that are provided by the UCompositeTexture class.
// See also:
// https://wiki.unrealengine.com/Texture_Merging_With_UCanvasRenderTarget2D
//
#include "CompositeTexture.h"
#include "TextureMergePrivatePCH.h"
#include "Engine.h"
#include "Runtime/Engine/Classes/Engine/Canvas.h"


UCompositeTexture::UCompositeTexture()
{
	OnCanvasRenderTargetUpdate.AddDynamic(this, &UCompositeTexture::PerformMerge);
}

void UCompositeTexture::PerformMerge(UCanvas* Canvas, int32 Width, int32 Height)
{
	for (int32 i = 0; i < Layers.Num(); ++i)
	{
		UTexture2D* LayerTex = Layers[i];
		if (LayerTex)
		{
			FColor TintColor = FColor::White;
			if (LayerTints.Num() > i)
			{
				TintColor = LayerTints[i];
			}
			float TexSizeX = LayerTex->GetSizeX();
			float TexSizeY = LayerTex->GetSizeY();

			Canvas->SetDrawColor(TintColor);
			Canvas->DrawTile(LayerTex, Offsets[i].X, Offsets[i].Y, TexSizeX, TexSizeY, 0, 0, TexSizeX, TexSizeY);
		}
	}
}

UCompositeTexture* UCompositeTexture::Create(UObject* WorldContextObject, const TArray<UTexture2D*>& Layers, const TArray<FVector2D> Offsets)
{
	TArray<FColor> Colors;
	Colors.SetNum(Layers.Num());
	for (auto& Color : Colors)
	{
		Color = FColor::White;
	}

	return UCompositeTexture::Create(WorldContextObject, Layers, Offsets, Colors);
}

UCompositeTexture* UCompositeTexture::Create(UObject* WorldContextObject, const TArray<UTexture2D*>& Layers, const TArray<FVector2D> Offsets, const TArray<FColor>& LayerTints)
{
	if (Layers.Num() <= 0 || Layers.Num() != Offsets.Num())
	{
		return NULL;
	}

	UTexture2D* BaseTexture = Layers[0];

	UCompositeTexture* RenderTarget = Cast<UCompositeTexture>(UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(WorldContextObject, UCompositeTexture::StaticClass(), BaseTexture->GetSizeX(), BaseTexture->GetSizeY()));

	RenderTarget->Layers.Append(Layers);
	RenderTarget->Offsets.Append(Offsets);
	RenderTarget->LayerTints.Append(LayerTints);

	RenderTarget->UpdateResource();
	return RenderTarget;
}

UCompositeTexture* UCompositeTexture::UpdateLayer(UCompositeTexture* RenderTarget, UTexture2D* Layer, FVector2D Offset, const int32 Index)
{
	if ((RenderTarget->Layers.Num() <= 0) || (Index >= RenderTarget->Layers.Num()))
	{
		return NULL;
	}

	RenderTarget->Layers[Index] = Layer;
	RenderTarget->Offsets[Index] = Offset;

	RenderTarget->UpdateResource();
	return RenderTarget;
}

UCompositeTexture* UCompositeTexture::UpdateAllLayers(UCompositeTexture* RenderTarget, TArray<UTexture2D*>& Layers, const TArray<FVector2D> Offsets)
{
	if (RenderTarget->Layers.Num() <= 0 || Layers.Num() != Offsets.Num())
	{
		return NULL;
	}

	for (int32 i = 0; i < RenderTarget->Layers.Num(); ++i)
	{
		RenderTarget->Layers[i] = Layers[i];
	}
	for (int32 i = 0; i < RenderTarget->Offsets.Num(); ++i)
	{
		RenderTarget->Offsets[i] = Offsets[i];
	}

	RenderTarget->UpdateResource();
	return RenderTarget;
}

UCompositeTexture* UCompositeTexture::UpdateTint(UCompositeTexture* RenderTarget, const FColor& Tint, const int32 Index)
{
	if ((RenderTarget->LayerTints.Num() <= 0) || (Index >= RenderTarget->LayerTints.Num()))
	{
		return NULL;
	}

	RenderTarget->LayerTints[Index] = Tint;

	RenderTarget->UpdateResource();
	return RenderTarget;
}

UCompositeTexture* UCompositeTexture::UpdateAllTints(UCompositeTexture* RenderTarget, const TArray<FColor>& LayerTints)
{
	if (RenderTarget->LayerTints.Num() <= 0)
	{
		return NULL;
	}

	for (int32 i = 0; i < RenderTarget->LayerTints.Num(); ++i)
	{
		RenderTarget->LayerTints[i] = LayerTints[i];
	}

	RenderTarget->UpdateResource();
	return RenderTarget;
}

