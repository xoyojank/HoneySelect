// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#include "AnimGraphNode_ModifyBones.h"
#include "EditableAvatarEditorPrivatePCH.h"
#include "UnrealWidget.h"
#include "AnimNodeEditModes.h"
#include "Kismet2/CompilerResultsLog.h"

/////////////////////////////////////////////////////
// UAnimGraphNode_ModifyBones

#define LOCTEXT_NAMESPACE "A3Nodes"

UAnimGraphNode_ModifyBones::UAnimGraphNode_ModifyBones(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FLinearColor UAnimGraphNode_ModifyBones::GetNodeTitleColor() const
{
	return FLinearColor(0.7f, 0.7f, 0.7f);
}

FText UAnimGraphNode_ModifyBones::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_ModifyBones_Tooltip", "The Transform Bone node alters the transform - i.e. Translation, Rotation, or Scale - of the bone");
}

FText UAnimGraphNode_ModifyBones::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("UAnimGraphNode_ModifyBones_Title", "Modify Transforms");
}

FString UAnimGraphNode_ModifyBones::GetNodeCategory() const
{
	return TEXT("Skeletal Controls");
}

#undef LOCTEXT_NAMESPACE
