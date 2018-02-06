#pragma once
#include "AvatarBoneTransform.generated.h"

USTRUCT(BlueprintType)
struct EDITABLEAVATAR_API FAvatarBoneTransform
{
	GENERATED_USTRUCT_BODY()

	FAvatarBoneTransform();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector Location;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector Rotation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector Scale;

	int32 Equals(const FAvatarBoneTransform& Transform) const;

	FAvatarBoneTransform& operator=(const FAvatarBoneTransform& Transform);

public:
	void SetPositionX(float X);
	void SetPositionY(float Y);
	void SetPositionZ(float Z);
	void SetPosition(float X, float Y, float Z);
	void SetRotation(float X, float Y, float Z);
	void SetScale(float X, float Y, float Z);

	FTransform ToUnrealTransform() const;
};

