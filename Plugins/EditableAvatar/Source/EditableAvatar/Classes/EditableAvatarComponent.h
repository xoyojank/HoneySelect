#pragma once

#include "AvatarBoneTransform.h"
#include "Components/SceneComponent.h"
#include "EditableAvatarComponent.generated.h"


UCLASS(ClassGroup = (Custom))
class EDITABLEAVATAR_API UEditableAvatarComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<FName> BoneNames;
	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<FAvatarBoneTransform> BoneTransforms;
};
