// Copyright Tencent Games, Inc. All Rights Reserved
#pragma once

#include "EditableAvatarComponent.h"
#include "AvatarBoneInfo.generated.h"


USTRUCT(BlueprintType)
struct FAvatarBoneKeys
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FVector> LocationKeys;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRotator> RotationKeys;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FVector> ScaleKeys;
};

USTRUCT(BlueprintType)
struct FAvatarBoneCategory
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BoneIndex;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint32 MaskLocationX : 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint32 MaskLocationY : 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint32 MaskLocationZ : 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint32 MaskRotationX : 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint32 MaskRotationY : 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint32 MaskRotationZ : 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint32 MaskScaleX : 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint32 MaskScaleY : 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint32 MaskScaleZ : 1;

	FAvatarBoneCategory()
		: BoneIndex(-1)
		, MaskLocationX(0)
		, MaskLocationY(0)
		, MaskLocationZ(0)
		, MaskRotationX(0)
		, MaskRotationY(0)
		, MaskRotationZ(0)
		, MaskScaleX(0)
		, MaskScaleY(0)
		, MaskScaleZ(0)
	{
	}

	bool operator==(const FAvatarBoneCategory& RHS) const
	{
		return BoneIndex == RHS.BoneIndex && BoneName == RHS.BoneName;
	}

	void MergeMask(int32 Mask)
	{
		if (Mask & (1 << 0)) MaskLocationX = 1;
		if (Mask & (1 << 1)) MaskLocationY = 1;
		if (Mask & (1 << 2)) MaskLocationZ = 1;
		if (Mask & (1 << 3)) MaskRotationX = 1;
		if (Mask & (1 << 4)) MaskRotationY = 1;
		if (Mask & (1 << 5)) MaskRotationZ = 1;
		if (Mask & (1 << 6)) MaskScaleX = 1;
		if (Mask & (1 << 7)) MaskScaleY = 1;
		if (Mask & (1 << 8)) MaskScaleZ = 1;
	}
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UAvatarBoneInfo : public UEditableAvatarComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay();

public:
	UFUNCTION(Category = Avatar, BlueprintCallable)
	void LoadBoneNames(const FString& Filename, const FString& SourceFilename);
	UFUNCTION(Category = Avatar, BlueprintCallable)
	void LoadBoneKeys(const FString& Filename);
	UFUNCTION(Category = Avatar, BlueprintCallable)
	void LoadBoneCategory(const FString& Filename, const FString& NameFilename);
	UFUNCTION(Category = Avatar, BlueprintCallable)
	void SetEditMesh(class USkinnedMeshComponent* Mesh);

	UFUNCTION(Category = Avatar, BlueprintCallable)
	void LoadPreset(int32 Index);
	UFUNCTION(Category = Avatar, BlueprintCallable)
	void SavePreset(int32 Index);

	UFUNCTION(Category = Avatar, BlueprintCallable)
	void ChangeValue(int32 CategoryID, float Value);

	UFUNCTION(Category = Avatar, BlueprintCallable)
	void DumpNewBoneCategory();
	UFUNCTION(Category = Avatar, BlueprintCallable)
	void DumpNewBoneKeys();

	void RecordNewBoneKeys();

private:
	void LoadNamesToArray(const FString& Filename, TArray<FName>& Names);

	void LerpBoneCategory(FAvatarBoneCategory* BoneCategory, float Value);
	void LerpBoneTransform(FName BoneName, float Alpha, FVector* OutLocation, FRotator* OutRotaion, FVector* OutScale);

	void UpdateBoneTransforms();
	void SetCategoryValue(int32 CategoryID, float Value);

	void UpdateHeadBoneTransforms();

	void BackupBoneTransforms();
	void CompareBoneTransforms();

	void UpdateBodyBoneTransforms();
	void ApplyToMesh();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataFile)
	FString BoneNamesFilename;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataFile)
	FString SourceBoneNamesFilename;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataFile)
	FString BoneKeysFilename;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataFile)
	FString CateoryFilename;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataFile)
	FString CategoryNamesFilename;


	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<FName> BoneCategoryNames;
	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<float> BoneCategoryValues;
	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<FName> SourceBoneNames;
	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<FAvatarBoneTransform> SourceBoneTransforms;

	UPROPERTY(Transient)
	class USkinnedMeshComponent* EditMesh;

	TMap<FName, FAvatarBoneKeys> BoneKeyMap;
	TMultiMap<int32, FAvatarBoneCategory> BoneCategoryMap;

	UPROPERTY(Transient)
	TArray<FAvatarBoneTransform> BoneBackupTransforms;
	int32 ComparingBoneCategoryID = -1;
	TMultiMap<int32, FAvatarBoneCategory> BoneNewCategoryMap;
	TMap<FName, FAvatarBoneKeys> BoneNewKeyMap;
};