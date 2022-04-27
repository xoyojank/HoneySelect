// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimNodeBase.h"
#include "AvatarBoneTransform.h"
#include "AnimNode_ModifyBones.generated.h"

class USkeletalMeshComponent;

/**
*	Simple controller that replaces or adds to the translation/rotation of a single bone.
*/
USTRUCT(BlueprintType)
struct EDITABLEAVATAR_API FAnimNode_ModifyBones : public FAnimNode_Base
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
	FPoseLink LocalPose;

	/** Name of bone to control. This is the main bone chain to modify from. **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> BoneIndices;

	/** New translation of bone to apply. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAvatarBoneTransform> Transforms;

	FAnimNode_ModifyBones();

	// FAnimNode_Base interface
	virtual void Initialize(const FAnimationInitializeContext& Context) override;

	virtual void CacheBones(const FAnimationCacheBonesContext& Context) override;
	virtual void Update(const FAnimationUpdateContext& Context) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	virtual void Evaluate(FPoseContext& Output) override;
	// End of FAnimNode_Base interface

	void ReinitBoneIndices(struct FAnimInstanceProxy* AnimInstanceProxy);
};
