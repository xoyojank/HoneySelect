// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "EditableAvatarPrivatePCH.h"
#include "AnimNode_ModifyBones.h"
#include "AnimationRuntime.h"
#include "AnimInstanceProxy.h"
#include "EditableAvatarComponent.h"
/////////////////////////////////////////////////////
// FAnimNode_ModifyBones

FAnimNode_ModifyBones::FAnimNode_ModifyBones()
{
}

void FAnimNode_ModifyBones::Initialize(const FAnimationInitializeContext& Context)
{
	LocalPose.Initialize(Context);

	ReinitBoneIndices(Context.AnimInstanceProxy);
}

void FAnimNode_ModifyBones::ReinitBoneIndices(struct FAnimInstanceProxy* AnimInstanceProxy)
{
	auto Mesh = AnimInstanceProxy->GetSkelMeshComponent();
	UEditableAvatarComponent* AvatarBoneInfo = nullptr;
	auto& Children = Mesh->GetAttachChildren();
	for (auto Component : Children)
	{
		AvatarBoneInfo = Cast<UEditableAvatarComponent>(Component);
		if (AvatarBoneInfo)
			break;
	}
	if (AvatarBoneInfo)
	{
		auto& BoneNames = AvatarBoneInfo->BoneNames;
		BoneIndices.SetNum(BoneNames.Num());
		for (int i = 0; i < BoneNames.Num(); ++i)
		{
			BoneIndices[i] = Mesh->GetBoneIndex(BoneNames[i]);
		}
	}
}

void FAnimNode_ModifyBones::CacheBones(const FAnimationCacheBonesContext& Context)
{
	LocalPose.CacheBones(Context);
}

void FAnimNode_ModifyBones::Update(const FAnimationUpdateContext& Context)
{
	LocalPose.Update(Context);

	ReinitBoneIndices(Context.AnimInstanceProxy);
	auto Mesh = Context.AnimInstanceProxy->GetSkelMeshComponent();
	UEditableAvatarComponent* AvatarBoneInfo = nullptr;
	auto& Children = Mesh->GetAttachChildren();
	for (auto Component : Children)
	{
		AvatarBoneInfo = Cast<UEditableAvatarComponent>(Component);
		if (AvatarBoneInfo)
			break;
	}
	if (AvatarBoneInfo)
		Transforms = AvatarBoneInfo->BoneTransforms;
}

void FAnimNode_ModifyBones::GatherDebugData(FNodeDebugData& DebugData)
{
	FString DebugLine = DebugData.GetNodeName(this);
	DebugData.AddDebugItem(DebugLine);
	LocalPose.GatherDebugData(DebugData);
}

void FAnimNode_ModifyBones::Evaluate(FPoseContext& Output)
{
	LocalPose.Evaluate(Output);
	FCompactPose& OutPose = Output.Pose;

	for (int i = 0; i < BoneIndices.Num(); ++i)
	{
		int BoneIndex = BoneIndices[i];
		if (BoneIndex >=0 )
			OutPose[FCompactPoseBoneIndex(BoneIndex)] = Transforms[i].ToUnrealTransform();
	}
}

