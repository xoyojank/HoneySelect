// Copyright Tencent Games, Inc. All Rights Reserved
#include "AvatarBoneInfo.h"
#include "HoneySelect.h"
#include "Components/PoseableMeshComponent.h"

void UAvatarBoneInfo::LoadBoneCategory(const FString& Filename, const FString& NameFilename)
{
	check(SourceBoneNames.Num() > 0);
	if (BoneCategoryMap.Num() > 0)
		return;
	FString FullPath = FPaths::ProjectContentDir() + Filename;
	FString FileContent;
	if (FFileHelper::LoadFileToString(FileContent, *FullPath))
	{
		TArray<FString> Rows;
		TArray<FString> Columns;
		FileContent.ParseIntoArrayLines(Rows);
		Columns.SetNum(0, false);
		for (auto& Row : Rows)
		{
			Row.ParseIntoArrayWS(Columns);
			check(Columns.Num() == 11);

			int32 CategoryID = FCString::Atoi(*Columns[0]);
			FAvatarBoneCategory& BoneCategory = BoneCategoryMap.Add(CategoryID);;
			BoneCategory.BoneName = *Columns[1];
			BoneCategory.BoneIndex = SourceBoneNames.Find(BoneCategory.BoneName);
			check(BoneCategory.BoneIndex != INDEX_NONE);
			BoneCategory.MaskLocationX = Columns[2][0] != '0';
			BoneCategory.MaskLocationY = Columns[3][0] != '0';
			BoneCategory.MaskLocationZ = Columns[4][0] != '0';
			BoneCategory.MaskRotationX = Columns[5][0] != '0';
			BoneCategory.MaskRotationY = Columns[6][0] != '0';
			BoneCategory.MaskRotationZ = Columns[7][0] != '0';
			BoneCategory.MaskScaleX = Columns[8][0] != '0';
			BoneCategory.MaskScaleY = Columns[9][0] != '0';
			BoneCategory.MaskScaleZ = Columns[10][0] != '0';
		}
	}
	LoadNamesToArray(NameFilename, BoneCategoryNames);
	BoneCategoryValues.SetNum(BoneCategoryNames.Num());
}

void UAvatarBoneInfo::BeginPlay()
{
	Super::BeginPlay();

	LoadBoneNames(BoneNamesFilename, SourceBoneNamesFilename);
	LoadBoneKeys(BoneKeysFilename);
	LoadBoneCategory(CateoryFilename, CategoryNamesFilename);

	auto SkinnedMesh = Cast<USkinnedMeshComponent>(GetAttachParent());
	if (SkinnedMesh)
	{
		SetEditMesh(SkinnedMesh);
	}

	LoadPreset(0);
}

void UAvatarBoneInfo::LoadBoneNames(const FString& Filename, const FString& SourceFilename)
{
	LoadNamesToArray(Filename, BoneNames);
	LoadNamesToArray(SourceFilename, SourceBoneNames);

	BoneTransforms.SetNum(BoneNames.Num());
	SourceBoneTransforms.SetNum(SourceBoneNames.Num());
}

void UAvatarBoneInfo::SetEditMesh(class USkinnedMeshComponent* Mesh)
{
	EditMesh = Mesh;

	auto PoseableMesh = Cast<UPoseableMeshComponent>(EditMesh);
	if (PoseableMesh)
	{
		for (int i = 0; i < BoneNames.Num(); ++i)
		{
			int32 BoneIndex = PoseableMesh->GetBoneIndex(BoneNames[i]);
			if (BoneIndex != INDEX_NONE)
			{
				FTransform Transform = PoseableMesh->BoneSpaceTransforms[BoneIndex];
				BoneTransforms[i].Location = Transform.GetLocation();
				BoneTransforms[i].Rotation = Transform.GetRotation().Euler();
				BoneTransforms[i].Scale = Transform.GetScale3D();
			}
		}
	}
	auto SkeletalMesh = Cast<USkeletalMeshComponent>(EditMesh);
	if (SkeletalMesh)
	{
		for (int i = 0; i <BoneNames.Num(); ++i)
		{
			int32 BoneIndex = SkeletalMesh->GetBoneIndex(BoneNames[i]);
			if (BoneIndex != INDEX_NONE)
			{
				FTransform Transform = SkeletalMesh->BoneSpaceTransforms[BoneIndex];
				BoneTransforms[i].Location = Transform.GetLocation();
				BoneTransforms[i].Rotation = Transform.GetRotation().Euler();
				BoneTransforms[i].Scale = Transform.GetScale3D();
			}
		}
	}
}

void UAvatarBoneInfo::LoadPreset(int32 Index)
{
	FString Filename = FPaths::ProjectSavedDir() + TEXT("Avatar/") + EditMesh->GetName() + FString::FromInt(Index);
	TArray<uint8> DataBuffer;
	if (FFileHelper::LoadFileToArray(DataBuffer, *Filename))
	{
		check(DataBuffer.Num() == BoneCategoryValues.Num() * sizeof(float));
		FMemory::Memcpy(BoneCategoryValues.GetData(), DataBuffer.GetData(), DataBuffer.Num());

		for (int32 CategoryID = 0; CategoryID < BoneCategoryValues.Num(); ++CategoryID)
		{
			SetCategoryValue(CategoryID, BoneCategoryValues[CategoryID]);
		}
	}
	else
	{
		for (int32 CategoryID = 0; CategoryID < BoneCategoryValues.Num(); ++CategoryID)
		{
			BoneCategoryValues[CategoryID] = 0.5f;
			SetCategoryValue(CategoryID, BoneCategoryValues[CategoryID]);
		}
	}

	UpdateBoneTransforms();
}

void UAvatarBoneInfo::SavePreset(int32 Index)
{
	FString Filename = FPaths::ProjectSavedDir() + TEXT("Avatar/") + EditMesh->GetName() + FString::FromInt(Index);
	TArray<uint8> DataBuffer;
	DataBuffer.SetNum(BoneCategoryValues.Num() * sizeof(float));
	FMemory::Memcpy(DataBuffer.GetData(), BoneCategoryValues.GetData(), DataBuffer.Num());
	FFileHelper::SaveArrayToFile(DataBuffer, *Filename);
}

void UAvatarBoneInfo::ChangeValue(int32 CategoryID, float Value)
{
	BoneCategoryValues[CategoryID] = Value;
	SetCategoryValue(CategoryID, Value);

	UpdateBoneTransforms();
}

void UAvatarBoneInfo::UpdateBoneTransforms()
{
	if (ComparingBoneCategoryID >= 0)
		BackupBoneTransforms();
	{
		// @todo: hack code from ILSpy
		if (EditMesh->GetName().StartsWith("head"))
			UpdateHeadBoneTransforms();
		else
			UpdateBodyBoneTransforms();
	}
	if (ComparingBoneCategoryID >= 0)
		CompareBoneTransforms();

	ApplyToMesh();
}

void UAvatarBoneInfo::SetCategoryValue(int32 CategoryID, float Value)
{
	TArray<FAvatarBoneCategory*> BoneCategories;
	BoneCategoryMap.MultiFindPointer(CategoryID, BoneCategories);
	for (FAvatarBoneCategory* BoneCategory : BoneCategories)
	{
		LerpBoneCategory(BoneCategory, Value);
	}
}

void UAvatarBoneInfo::LerpBoneCategory(FAvatarBoneCategory* BoneCategory, float Value)
{
	FVector BlendLocation;
	FRotator BlendRotator;
	FVector BlendScale;
	LerpBoneTransform(BoneCategory->BoneName, Value, &BlendLocation, &BlendRotator, &BlendScale);
	if (BoneCategory->MaskLocationX)
		SourceBoneTransforms[BoneCategory->BoneIndex].Location.X = -BlendLocation.X;
	if (BoneCategory->MaskLocationY)
		SourceBoneTransforms[BoneCategory->BoneIndex].Location.Y = -BlendLocation.Y;
	if (BoneCategory->MaskLocationZ)
		SourceBoneTransforms[BoneCategory->BoneIndex].Location.Z = BlendLocation.Z;
	if (BoneCategory->MaskRotationX)
		SourceBoneTransforms[BoneCategory->BoneIndex].Rotation.X = BlendRotator.Roll;
	if (BoneCategory->MaskRotationY)
		SourceBoneTransforms[BoneCategory->BoneIndex].Rotation.Y = BlendRotator.Pitch;
	if (BoneCategory->MaskRotationZ)
		SourceBoneTransforms[BoneCategory->BoneIndex].Rotation.Z = BlendRotator.Yaw;
	if (BoneCategory->MaskScaleX)
		SourceBoneTransforms[BoneCategory->BoneIndex].Scale.X = BlendScale.X;
	if (BoneCategory->MaskScaleY)
		SourceBoneTransforms[BoneCategory->BoneIndex].Scale.Y = BlendScale.Y;
	if (BoneCategory->MaskScaleZ)
		SourceBoneTransforms[BoneCategory->BoneIndex].Scale.Z = BlendScale.Z;
}

inline FRotator RotatorFromEular(float X, float Y, float Z)
{
	return FRotator(Y, Z, X);
}

void UAvatarBoneInfo::ApplyToMesh()
{
	auto PoseableMesh = Cast<UPoseableMeshComponent>(EditMesh);
	if (PoseableMesh)
	{
		for (int32 i = 0; i < BoneNames.Num(); ++i)
		{
			int32 BoneIndex = PoseableMesh->GetBoneIndex(BoneNames[i]);
			if (BoneIndex != INDEX_NONE)
			{
				PoseableMesh->BoneSpaceTransforms[BoneIndex] = BoneTransforms[i].ToUnrealTransform();
			}
		}
		EditMesh->RefreshBoneTransforms();
	}
}

void UAvatarBoneInfo::LerpBoneTransform(FName BoneName, float Alpha, FVector* OutLocation, FRotator* OutRotaion, FVector* OutScale)
{
	FAvatarBoneKeys* BoneKeys = BoneKeyMap.Find(BoneName);
	if (BoneKeys == nullptr)
		return;
	int NumKeys = BoneKeys->LocationKeys.Num();
	if (Alpha <= 0.0f)
	{
		*OutLocation = BoneKeys->LocationKeys[0];
		*OutRotaion = BoneKeys->RotationKeys[0];
		*OutScale = BoneKeys->ScaleKeys[0];
	}
	else if (Alpha >= 1.0f)
	{
		*OutLocation = BoneKeys->LocationKeys.Last();
		*OutRotaion = BoneKeys->RotationKeys.Last();
		*OutScale = BoneKeys->ScaleKeys.Last();
	}
	else
	{
		float IndexAlpha = (NumKeys - 1) * Alpha;
		int Index = FMath::FloorToInt(IndexAlpha);
		float LerpAlpha = IndexAlpha - Index;
		*OutLocation = FMath::Lerp(BoneKeys->LocationKeys[Index], BoneKeys->LocationKeys[Index + 1], LerpAlpha);
		*OutRotaion = FMath::Lerp(BoneKeys->RotationKeys[Index], BoneKeys->RotationKeys[Index + 1], LerpAlpha);
		*OutScale = FMath::Lerp(BoneKeys->ScaleKeys[Index], BoneKeys->ScaleKeys[Index + 1], LerpAlpha);
	}
}

void UAvatarBoneInfo::LoadNamesToArray(const FString& Filename, TArray<FName>& Names)
{
	if (Names.Num() > 0)
		return;
	FString FullPath = FPaths::ProjectContentDir() + Filename;
	FString FileContent;
	if (FFileHelper::LoadFileToString(FileContent, *FullPath))
	{
		TArray<FString> Rows;
		FileContent.ParseIntoArrayLines(Rows);
		for (auto& Row : Rows)
		{
			Names.Add(*Row);
		}
	}
}

void UAvatarBoneInfo::LoadBoneKeys(const FString& Filename)
{
	if (BoneKeyMap.Num() > 0)
		return;
	FString FullPath = FPaths::ProjectContentDir() + Filename;
	FString FileContent;
	if (FFileHelper::LoadFileToString(FileContent, *FullPath))
	{
		TArray<FString> Rows;
		TArray<FString> Columns;
		FileContent.ParseIntoArrayLines(Rows);
		Columns.SetNum(0, false);
		for (auto& Row : Rows)
		{
			Row.ParseIntoArrayWS(Columns);
			check((Columns.Num() - 1) % 10 == 0);

			int32 NumKeys = (Columns.Num() - 1) / 10;
			FName BoneName = *Columns[0];
			FAvatarBoneKeys& BoneKey = BoneKeyMap.Add(BoneName);
			BoneKey.BoneName = BoneName;
			BoneKey.LocationKeys.SetNum(NumKeys);
			BoneKey.RotationKeys.SetNum(NumKeys);
			BoneKey.ScaleKeys.SetNum(NumKeys);
			for (int32 i = 0; i < NumKeys; ++i)
			{
				BoneKey.LocationKeys[i].X = FCString::Atof(*Columns[i * 10 + 2]);
				BoneKey.LocationKeys[i].Y = FCString::Atof(*Columns[i * 10 + 3]);
				BoneKey.LocationKeys[i].Z = FCString::Atof(*Columns[i * 10 + 4]);
				BoneKey.RotationKeys[i].Roll = FCString::Atof(*Columns[i * 10 + 5]);
				BoneKey.RotationKeys[i].Pitch = FCString::Atof(*Columns[i * 10 + 6]);
				BoneKey.RotationKeys[i].Yaw = FCString::Atof(*Columns[i * 10 + 7]);
				BoneKey.ScaleKeys[i].X = FCString::Atof(*Columns[i * 10 + 8]);
				BoneKey.ScaleKeys[i].Y = FCString::Atof(*Columns[i * 10 + 9]);
				BoneKey.ScaleKeys[i].Z = FCString::Atof(*Columns[i * 10 + 10]);
			}
		}
	}
}

void UAvatarBoneInfo::UpdateHeadBoneTransforms()
{
	BoneTransforms[61].SetPositionY(SourceBoneTransforms[121].Location.Y);
	BoneTransforms[61].SetPositionZ(SourceBoneTransforms[122].Location.Z + SourceBoneTransforms[121].Location.Z);
	BoneTransforms[38].SetPositionX(SourceBoneTransforms[72].Location.X);
	BoneTransforms[38].SetPositionY(SourceBoneTransforms[74].Location.Y);
	BoneTransforms[38].SetPositionZ(SourceBoneTransforms[74].Location.Z + SourceBoneTransforms[70].Location.Z + SourceBoneTransforms[72].Location.Z + SourceBoneTransforms[75].Location.Z);
	BoneTransforms[38].SetRotation(SourceBoneTransforms[74].Rotation.X + SourceBoneTransforms[70].Rotation.X + SourceBoneTransforms[75].Rotation.X, SourceBoneTransforms[70].Rotation.Y + SourceBoneTransforms[72].Rotation.Y, SourceBoneTransforms[70].Rotation.Z);
	BoneTransforms[39].SetPositionX(SourceBoneTransforms[73].Location.X);
	BoneTransforms[39].SetPositionY(SourceBoneTransforms[74].Location.Y);
	BoneTransforms[39].SetPositionZ(SourceBoneTransforms[74].Location.Z + SourceBoneTransforms[71].Location.Z + SourceBoneTransforms[73].Location.Z + SourceBoneTransforms[76].Location.Z);
	BoneTransforms[39].SetRotation(SourceBoneTransforms[74].Rotation.X + SourceBoneTransforms[71].Rotation.X + SourceBoneTransforms[76].Rotation.X, SourceBoneTransforms[71].Rotation.Y + SourceBoneTransforms[73].Rotation.Y, SourceBoneTransforms[71].Rotation.Z);
	BoneTransforms[40].SetPositionY(SourceBoneTransforms[77].Location.Y);
	BoneTransforms[40].SetPositionZ(SourceBoneTransforms[77].Location.Z + SourceBoneTransforms[123].Location.Z + SourceBoneTransforms[124].Location.Z + SourceBoneTransforms[125].Location.Z);
	BoneTransforms[40].SetRotation(SourceBoneTransforms[77].Rotation.X + SourceBoneTransforms[123].Rotation.X + SourceBoneTransforms[124].Rotation.X + SourceBoneTransforms[125].Rotation.X, SourceBoneTransforms[77].Rotation.Y + SourceBoneTransforms[125].Rotation.Y, SourceBoneTransforms[77].Rotation.Z);
	BoneTransforms[41].SetPositionY(SourceBoneTransforms[78].Location.Y);
	BoneTransforms[41].SetPositionZ(SourceBoneTransforms[78].Location.Z + SourceBoneTransforms[123].Location.Z + SourceBoneTransforms[124].Location.Z + SourceBoneTransforms[125].Location.Z);
	BoneTransforms[41].SetRotation(SourceBoneTransforms[78].Rotation.X + SourceBoneTransforms[123].Rotation.X + SourceBoneTransforms[124].Rotation.X + SourceBoneTransforms[125].Rotation.X, SourceBoneTransforms[78].Rotation.Y - SourceBoneTransforms[125].Rotation.Y, SourceBoneTransforms[78].Rotation.Z);
	BoneTransforms[42].SetPositionY(SourceBoneTransforms[79].Location.Y);
	BoneTransforms[42].SetPositionZ(SourceBoneTransforms[79].Location.Z + SourceBoneTransforms[126].Location.Z);
	BoneTransforms[42].SetRotation(SourceBoneTransforms[79].Rotation.X + SourceBoneTransforms[126].Rotation.X, SourceBoneTransforms[79].Rotation.Y, SourceBoneTransforms[79].Rotation.Z);
	BoneTransforms[43].SetPositionY(SourceBoneTransforms[80].Location.Y);
	BoneTransforms[43].SetPositionZ(SourceBoneTransforms[80].Location.Z + SourceBoneTransforms[126].Location.Z);
	BoneTransforms[43].SetRotation(SourceBoneTransforms[80].Rotation.X + SourceBoneTransforms[126].Rotation.X, SourceBoneTransforms[80].Rotation.Y, SourceBoneTransforms[80].Rotation.Z);
	BoneTransforms[21].SetPositionX(SourceBoneTransforms[41].Location.X);
	BoneTransforms[21].SetPositionY(SourceBoneTransforms[43].Location.Y);
	BoneTransforms[21].SetPositionZ(SourceBoneTransforms[44].Location.Z);
	BoneTransforms[21].SetRotation(0.0f, 0.0f, SourceBoneTransforms[35].Rotation.Z);
	BoneTransforms[22].SetPositionX(SourceBoneTransforms[42].Location.X);
	BoneTransforms[22].SetPositionY(SourceBoneTransforms[43].Location.Y);
	BoneTransforms[22].SetPositionZ(SourceBoneTransforms[44].Location.Z);
	BoneTransforms[22].SetRotation(0.0f, 0.0f, SourceBoneTransforms[36].Rotation.Z);
	BoneTransforms[19].SetScale(SourceBoneTransforms[37].Scale.X, SourceBoneTransforms[39].Scale.Y, 1.0f);
	BoneTransforms[15].SetRotation(0.0f, SourceBoneTransforms[33].Rotation.Y, 0.0f);
	BoneTransforms[20].SetScale(SourceBoneTransforms[38].Scale.X, SourceBoneTransforms[40].Scale.Y, 1.0f);
	BoneTransforms[16].SetRotation(0.0f, SourceBoneTransforms[34].Rotation.Y, 0.0f);
	BoneTransforms[23].SetRotation(SourceBoneTransforms[47].Rotation.X, SourceBoneTransforms[45].Rotation.Y + SourceBoneTransforms[47].Rotation.Y, 0.0f);
	BoneTransforms[25].SetRotation(SourceBoneTransforms[49].Rotation.X, SourceBoneTransforms[51].Rotation.Y, SourceBoneTransforms[51].Rotation.Z);
	BoneTransforms[27].SetPositionX(SourceBoneTransforms[53].Location.X);
	BoneTransforms[27].SetRotation(SourceBoneTransforms[55].Rotation.X, SourceBoneTransforms[53].Rotation.Y, 0.0f);
	BoneTransforms[29].SetRotation(SourceBoneTransforms[57].Rotation.X, SourceBoneTransforms[59].Rotation.Y, SourceBoneTransforms[59].Rotation.Z);
	BoneTransforms[24].SetRotation(SourceBoneTransforms[48].Rotation.X, SourceBoneTransforms[46].Rotation.Y + SourceBoneTransforms[48].Rotation.Y, 0.0f);
	BoneTransforms[26].SetRotation(SourceBoneTransforms[50].Rotation.X, SourceBoneTransforms[52].Rotation.Y, SourceBoneTransforms[52].Rotation.Z);
	BoneTransforms[28].SetPositionX(SourceBoneTransforms[54].Location.X);
	BoneTransforms[28].SetRotation(SourceBoneTransforms[56].Rotation.X, SourceBoneTransforms[54].Rotation.Y, 0.0f);
	BoneTransforms[30].SetRotation(SourceBoneTransforms[58].Rotation.X, SourceBoneTransforms[60].Rotation.Y, SourceBoneTransforms[60].Rotation.Z);
	BoneTransforms[31].SetRotation(0.0f, 0.0f, SourceBoneTransforms[63].Rotation.Z);
	BoneTransforms[17].SetRotation(SourceBoneTransforms[61].Rotation.X, 0.0f, 0.0f);
	BoneTransforms[32].SetRotation(0.0f, 0.0f, SourceBoneTransforms[64].Rotation.Z);
	BoneTransforms[18].SetRotation(SourceBoneTransforms[62].Rotation.X, 0.0f, 0.0f);
	BoneTransforms[59].SetPositionY(SourceBoneTransforms[119].Location.Y);
	BoneTransforms[59].SetPositionZ(SourceBoneTransforms[113].Location.Z + SourceBoneTransforms[115].Location.Z + SourceBoneTransforms[117].Location.Z + SourceBoneTransforms[119].Location.Z);
	BoneTransforms[59].SetScale(SourceBoneTransforms[113].Scale.X * SourceBoneTransforms[115].Scale.X * SourceBoneTransforms[117].Scale.X, SourceBoneTransforms[113].Scale.Y * SourceBoneTransforms[115].Scale.Y * SourceBoneTransforms[119].Scale.Y, SourceBoneTransforms[117].Scale.Z * SourceBoneTransforms[119].Scale.Z);
	BoneTransforms[60].SetPositionY(SourceBoneTransforms[120].Location.Y);
	BoneTransforms[60].SetPositionZ(SourceBoneTransforms[114].Location.Z + SourceBoneTransforms[116].Location.Z + SourceBoneTransforms[118].Location.Z + SourceBoneTransforms[120].Location.Z);
	BoneTransforms[60].SetScale(SourceBoneTransforms[114].Scale.X * SourceBoneTransforms[116].Scale.X * SourceBoneTransforms[118].Scale.X, SourceBoneTransforms[114].Scale.Y * SourceBoneTransforms[116].Scale.Y * SourceBoneTransforms[120].Scale.Y, SourceBoneTransforms[118].Scale.Z * SourceBoneTransforms[120].Scale.Z);
	BoneTransforms[33].SetScale(SourceBoneTransforms[65].Scale.X, 1.0f, 1.0f);
	BoneTransforms[36].SetPositionY(SourceBoneTransforms[68].Location.Y);
	BoneTransforms[37].SetPositionZ(SourceBoneTransforms[69].Location.Z);
	BoneTransforms[2].SetPositionX(SourceBoneTransforms[4].Location.X);
	BoneTransforms[2].SetPositionY(SourceBoneTransforms[6].Location.Y);
	BoneTransforms[2].SetPositionZ(SourceBoneTransforms[7].Location.Z + SourceBoneTransforms[8].Location.Z);
	BoneTransforms[3].SetPositionX(SourceBoneTransforms[5].Location.X);
	BoneTransforms[3].SetPositionY(SourceBoneTransforms[6].Location.Y);
	BoneTransforms[3].SetPositionZ(SourceBoneTransforms[7].Location.Z + SourceBoneTransforms[8].Location.Z);
	BoneTransforms[0].SetPositionX(SourceBoneTransforms[0].Location.X);
	BoneTransforms[0].SetPositionY(SourceBoneTransforms[2].Location.Y);
	BoneTransforms[0].SetPositionZ(SourceBoneTransforms[3].Location.Z);
	BoneTransforms[1].SetPositionX(SourceBoneTransforms[1].Location.X);
	BoneTransforms[1].SetPositionY(SourceBoneTransforms[2].Location.Y);
	BoneTransforms[1].SetPositionZ(SourceBoneTransforms[3].Location.Z);
	BoneTransforms[35].SetPositionZ(SourceBoneTransforms[67].Location.Z);
	BoneTransforms[34].SetScale(SourceBoneTransforms[66].Scale.X, 1.0f, 1.0f);
	BoneTransforms[50].SetPositionY(SourceBoneTransforms[91].Location.Y);
	BoneTransforms[50].SetPositionZ(SourceBoneTransforms[91].Location.Z + SourceBoneTransforms[85].Location.Z);
	BoneTransforms[49].SetScale(SourceBoneTransforms[89].Scale.X, SourceBoneTransforms[90].Scale.Y, 1.0f);
	BoneTransforms[48].SetPositionY(SourceBoneTransforms[86].Location.Y);
	BoneTransforms[47].SetPositionY(SourceBoneTransforms[92].Location.Y);
	BoneTransforms[47].SetPositionZ(SourceBoneTransforms[92].Location.Z);
	BoneTransforms[47].SetScale(SourceBoneTransforms[92].Scale.X, 1.0f, 1.0f);
	BoneTransforms[45].SetPositionY(SourceBoneTransforms[87].Location.Y);
	BoneTransforms[45].SetRotation(0.0f, 0.0f, SourceBoneTransforms[87].Rotation.Z);
	BoneTransforms[46].SetPositionY(SourceBoneTransforms[88].Location.Y);
	BoneTransforms[46].SetRotation(0.0f, 0.0f, SourceBoneTransforms[88].Rotation.Z);
	BoneTransforms[5].SetPositionY(SourceBoneTransforms[13].Location.Y);
	BoneTransforms[4].SetPositionY(SourceBoneTransforms[11].Location.Y + SourceBoneTransforms[9].Location.Y);
	BoneTransforms[4].SetPositionZ(SourceBoneTransforms[12].Location.Z + SourceBoneTransforms[9].Location.Z);
	BoneTransforms[4].SetRotation(SourceBoneTransforms[9].Rotation.X, 0.0f, 0.0f);
	BoneTransforms[4].SetScale(SourceBoneTransforms[10].Scale.X, 1.0f, 1.0f);
	BoneTransforms[6].SetPositionY(SourceBoneTransforms[15].Location.Y);
	BoneTransforms[6].SetPositionZ(SourceBoneTransforms[16].Location.Z);
	BoneTransforms[6].SetScale(SourceBoneTransforms[14].Scale.X, SourceBoneTransforms[14].Scale.Y, SourceBoneTransforms[14].Scale.Z);
	BoneTransforms[56].SetPositionY(SourceBoneTransforms[103].Location.Y);
	BoneTransforms[56].SetPositionZ(SourceBoneTransforms[104].Location.Z + SourceBoneTransforms[105].Location.Z + SourceBoneTransforms[103].Location.Z + SourceBoneTransforms[101].Location.Z);
	BoneTransforms[56].SetRotation(SourceBoneTransforms[101].Rotation.X, 0.0f, 0.0f);
	BoneTransforms[55].SetScale(SourceBoneTransforms[102].Scale.X, 1.0f, 1.0f);
	BoneTransforms[54].SetPositionY(SourceBoneTransforms[97].Location.Y + SourceBoneTransforms[99].Location.Y + SourceBoneTransforms[96].Location.Y);
	BoneTransforms[54].SetPositionZ(SourceBoneTransforms[97].Location.Z + SourceBoneTransforms[100].Location.Z + SourceBoneTransforms[96].Location.Z);
	BoneTransforms[53].SetRotation(SourceBoneTransforms[97].Rotation.X + SourceBoneTransforms[96].Rotation.X, 0.0f, 0.0f);
	BoneTransforms[53].SetScale(SourceBoneTransforms[98].Scale.X, SourceBoneTransforms[98].Scale.Y, SourceBoneTransforms[98].Scale.Z);
	BoneTransforms[57].SetPositionX(SourceBoneTransforms[109].Location.X);
	BoneTransforms[57].SetPositionY(SourceBoneTransforms[111].Location.Y);
	BoneTransforms[57].SetPositionZ(SourceBoneTransforms[112].Location.Z);
	BoneTransforms[57].SetRotation(SourceBoneTransforms[106].Rotation.X, 0.0f, SourceBoneTransforms[107].Rotation.Z);
	BoneTransforms[58].SetPositionX(SourceBoneTransforms[110].Location.X);
	BoneTransforms[58].SetPositionY(SourceBoneTransforms[111].Location.Y);
	BoneTransforms[58].SetPositionZ(SourceBoneTransforms[112].Location.Z);
	BoneTransforms[58].SetRotation(SourceBoneTransforms[106].Rotation.X, 0.0f, SourceBoneTransforms[108].Rotation.Z);
	BoneTransforms[52].SetPositionY(SourceBoneTransforms[94].Location.Y);
	BoneTransforms[52].SetPositionZ(SourceBoneTransforms[94].Location.Z);
	BoneTransforms[52].SetScale(SourceBoneTransforms[94].Scale.X, SourceBoneTransforms[94].Scale.Y, SourceBoneTransforms[94].Scale.Z);
	BoneTransforms[51].SetPositionY(SourceBoneTransforms[93].Location.Y);
	BoneTransforms[51].SetPositionZ(SourceBoneTransforms[95].Location.Z);
	BoneTransforms[51].SetRotation(SourceBoneTransforms[93].Rotation.X, 0.0f, 0.0f);
	BoneTransforms[44].SetPositionY(SourceBoneTransforms[83].Location.Y + SourceBoneTransforms[81].Location.Y + SourceBoneTransforms[82].Location.Y);
	BoneTransforms[44].SetPositionZ(SourceBoneTransforms[83].Location.Z + SourceBoneTransforms[84].Location.Z + SourceBoneTransforms[82].Location.Z);
	BoneTransforms[44].SetRotation(SourceBoneTransforms[83].Rotation.X + SourceBoneTransforms[81].Rotation.X + SourceBoneTransforms[82].Rotation.X, 0.0f, 0.0f);
	BoneTransforms[7].SetRotation(0.0f, SourceBoneTransforms[17].Rotation.Y, SourceBoneTransforms[19].Rotation.Z);
	BoneTransforms[7].SetScale(SourceBoneTransforms[21].Scale.X, SourceBoneTransforms[21].Scale.Y, SourceBoneTransforms[21].Scale.Z);
	BoneTransforms[13].SetPositionX(SourceBoneTransforms[31].Location.X);
	BoneTransforms[13].SetPositionY(SourceBoneTransforms[31].Location.Y);
	BoneTransforms[13].SetPositionZ(SourceBoneTransforms[31].Location.Z);
	BoneTransforms[13].SetRotation(SourceBoneTransforms[31].Rotation.X, SourceBoneTransforms[31].Rotation.Y, 0.0f);
	BoneTransforms[13].SetScale(SourceBoneTransforms[31].Scale.X, SourceBoneTransforms[31].Scale.Y, SourceBoneTransforms[31].Scale.Z);
	BoneTransforms[9].SetPositionY(SourceBoneTransforms[23].Location.Y);
	BoneTransforms[9].SetPositionZ(SourceBoneTransforms[23].Location.Z);
	BoneTransforms[9].SetScale(SourceBoneTransforms[23].Scale.X, SourceBoneTransforms[23].Scale.Y, SourceBoneTransforms[23].Scale.Z);
	BoneTransforms[8].SetRotation(0.0f, SourceBoneTransforms[18].Rotation.Y, SourceBoneTransforms[20].Rotation.Z);
	BoneTransforms[8].SetScale(SourceBoneTransforms[22].Scale.X, SourceBoneTransforms[22].Scale.Y, SourceBoneTransforms[22].Scale.Z);
	BoneTransforms[14].SetPositionX(SourceBoneTransforms[32].Location.X);
	BoneTransforms[14].SetPositionY(SourceBoneTransforms[32].Location.Y);
	BoneTransforms[14].SetPositionZ(SourceBoneTransforms[32].Location.Z);
	BoneTransforms[14].SetRotation(SourceBoneTransforms[32].Rotation.X, SourceBoneTransforms[32].Rotation.Y, 0.0f);
	BoneTransforms[14].SetScale(SourceBoneTransforms[32].Scale.X, SourceBoneTransforms[32].Scale.Y, SourceBoneTransforms[32].Scale.Z);
	BoneTransforms[10].SetPositionY(SourceBoneTransforms[24].Location.Y);
	BoneTransforms[10].SetPositionZ(SourceBoneTransforms[24].Location.Z);
	BoneTransforms[10].SetScale(SourceBoneTransforms[24].Scale.X, SourceBoneTransforms[24].Scale.Y, SourceBoneTransforms[24].Scale.Z);
	BoneTransforms[11].SetPositionY(SourceBoneTransforms[25].Location.Y);
	BoneTransforms[11].SetRotation(0.0f, 0.0f, SourceBoneTransforms[27].Rotation.Z);
	BoneTransforms[11].SetScale(SourceBoneTransforms[29].Scale.X, SourceBoneTransforms[29].Scale.Y, SourceBoneTransforms[29].Scale.Z);
	BoneTransforms[12].SetPositionY(SourceBoneTransforms[26].Location.Y);
	BoneTransforms[12].SetRotation(0.0f, 0.0f, SourceBoneTransforms[28].Rotation.Z);
	BoneTransforms[12].SetScale(SourceBoneTransforms[30].Scale.X, SourceBoneTransforms[30].Scale.Y, SourceBoneTransforms[30].Scale.Z);
}

void UAvatarBoneInfo::BackupBoneTransforms()
{
	BoneBackupTransforms.SetNum(BoneTransforms.Num());
	for (int i = 0; i < BoneTransforms.Num(); ++i)
	{
		BoneBackupTransforms[i] = BoneTransforms[i];
	}
}

void UAvatarBoneInfo::CompareBoneTransforms()
{
	for (int i = 0; i < BoneTransforms.Num(); ++i)
	{
		int32 Mask = BoneTransforms[i].Equals(BoneBackupTransforms[i]);
		int32 BoneIndex = EditMesh->GetBoneIndex(BoneNames[i]);
		if (Mask != 0 && BoneIndex >= 0)
		{
			FAvatarBoneCategory BoneCategory;
			BoneCategory.BoneIndex = BoneIndex;
			BoneCategory.BoneName = BoneNames[i];
			BoneNewCategoryMap.AddUnique(ComparingBoneCategoryID, BoneCategory).MergeMask(Mask);
		}
	}
}

void UAvatarBoneInfo::DumpNewBoneCategory()
{
	for (int i = 0; i < BoneCategoryNames.Num(); ++i)
	{
		ComparingBoneCategoryID = i;
		ChangeValue(i, 0.0f);
		ChangeValue(i, 0.25f);
		ChangeValue(i, 0.75f);
		ChangeValue(i, 1.0f);
	}
	ComparingBoneCategoryID = -1;

	for (auto& Pair : BoneNewCategoryMap)
	{
		int32 BoneCategoryID = Pair.Key;
		auto& BoneCategory = Pair.Value;
		UE_LOG(Solar, Log, TEXT("%d,%s,%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
		       BoneCategoryID, *BoneCategoryNames[BoneCategoryID].ToString(),
		       BoneCategory.BoneIndex, *BoneCategory.BoneName.ToString(),
		       BoneCategory.MaskLocationX, BoneCategory.MaskLocationY, BoneCategory.MaskLocationZ,
		       BoneCategory.MaskRotationX, BoneCategory.MaskRotationY, BoneCategory.MaskRotationZ,
		       BoneCategory.MaskScaleX, BoneCategory.MaskScaleY, BoneCategory.MaskScaleZ);
	}
}


void UAvatarBoneInfo::DumpNewBoneKeys()
{
	for (int i = 0; i < BoneCategoryNames.Num(); ++i)
	{
		ChangeValue(i, 0.0f);
	}
	RecordNewBoneKeys();
	for (int i = 0; i < BoneCategoryNames.Num(); ++i)
	{
		ChangeValue(i, 1.0f);
	}
	RecordNewBoneKeys();
	UE_LOG(Solar, Log, TEXT("BoneName,Tx,Ty,Tz,Rx,Ry,Rz,Sx,Sy,Sz,Tx,Ty,Tz,Rx,Ry,Rz,Sx,Sy,Sz"));
	for (auto& Pair : BoneNewKeyMap)
	{
		FAvatarBoneKeys& BoneKeys = Pair.Value;
		UE_LOG(Solar, Log, TEXT("%s,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f"),
		       *BoneKeys.BoneName.ToString(),
		       BoneKeys.LocationKeys[0].X, BoneKeys.LocationKeys[0].Y, BoneKeys.LocationKeys[0].Z,
		       BoneKeys.RotationKeys[0].Roll, BoneKeys.RotationKeys[0].Pitch, BoneKeys.RotationKeys[0].Yaw,
		       BoneKeys.ScaleKeys[0].X, BoneKeys.ScaleKeys[0].Y, BoneKeys.ScaleKeys[0].Z,
		       BoneKeys.LocationKeys[1].X, BoneKeys.LocationKeys[1].Y, BoneKeys.LocationKeys[1].Z,
		       BoneKeys.RotationKeys[1].Roll, BoneKeys.RotationKeys[1].Pitch, BoneKeys.RotationKeys[1].Yaw,
		       BoneKeys.ScaleKeys[1].X, BoneKeys.ScaleKeys[1].Y, BoneKeys.ScaleKeys[1].Z);
	}
}

void UAvatarBoneInfo::RecordNewBoneKeys()
{
	for (int i = 0; i < BoneNames.Num(); ++i)
	{
		FAvatarBoneKeys& BoneKeys = BoneNewKeyMap.FindOrAdd(BoneNames[i]);
		BoneKeys.BoneName = BoneNames[i];
		BoneKeys.LocationKeys.Add(BoneTransforms[i].Location);
		BoneKeys.RotationKeys.Add(FRotator::MakeFromEuler(BoneTransforms[i].Rotation));
		BoneKeys.ScaleKeys.Add(BoneTransforms[i].Scale);
	}
}

void UAvatarBoneInfo::UpdateBodyBoneTransforms()
{
	float num = SourceBoneTransforms[195].Location.Y + SourceBoneTransforms[196].Location.Y;
	float num2 = SourceBoneTransforms[195].Scale.Y * SourceBoneTransforms[196].Scale.Y;
	float x = SourceBoneTransforms[196].Scale.X;
	float z = SourceBoneTransforms[196].Location.Z;
	float num3 = SourceBoneTransforms[197].Location.Y + SourceBoneTransforms[198].Location.Y;
	float num4 = SourceBoneTransforms[197].Location.Z + SourceBoneTransforms[198].Location.Z;
	float num5 = SourceBoneTransforms[197].Scale.Y * SourceBoneTransforms[198].Scale.Y;
	float num6 = SourceBoneTransforms[197].Scale.X * SourceBoneTransforms[198].Scale.X;
	float num7 = SourceBoneTransforms[199].Location.X + SourceBoneTransforms[200].Location.X;
	float num8 = SourceBoneTransforms[199].Rotation.Z + SourceBoneTransforms[200].Rotation.Z;
	float x2 = SourceBoneTransforms[199].Scale.X * SourceBoneTransforms[200].Scale.X;
	float y = SourceBoneTransforms[199].Scale.Y * SourceBoneTransforms[200].Scale.Y;
	float num9 = SourceBoneTransforms[201].Scale.X * SourceBoneTransforms[202].Scale.X;
	float num10 = SourceBoneTransforms[201].Scale.Y * SourceBoneTransforms[202].Scale.Y;
	float x3 = SourceBoneTransforms[202].Location.X;
	float num11 = SourceBoneTransforms[201].Location.Z + SourceBoneTransforms[202].Location.Z;
	float x4 = SourceBoneTransforms[203].Location.X;
	float y2 = SourceBoneTransforms[203].Location.Y;
	float z2 = SourceBoneTransforms[203].Location.Z;
	float x5 = SourceBoneTransforms[203].Scale.X;
	float y3 = SourceBoneTransforms[203].Scale.Y;
	float x6 = SourceBoneTransforms[204].Location.X;
	float y4 = SourceBoneTransforms[204].Location.Y;
	float z3 = SourceBoneTransforms[204].Location.Z;
	float x7 = SourceBoneTransforms[204].Scale.X;
	float x8 = SourceBoneTransforms[204].Rotation.X;
	float z4 = SourceBoneTransforms[206].Location.Z;
	float x9 = SourceBoneTransforms[206].Scale.X;
	float x10 = SourceBoneTransforms[205].Location.X;
	float y5 = SourceBoneTransforms[205].Location.Y;
	float z5 = SourceBoneTransforms[205].Location.Z;
	float x11 = SourceBoneTransforms[205].Scale.X;
	float x12 = SourceBoneTransforms[205].Location.X;
	float y6 = SourceBoneTransforms[205].Location.Y;
	float z6 = SourceBoneTransforms[205].Location.Z;
	float x13 = SourceBoneTransforms[205].Scale.X;
	{
		BoneTransforms[71].SetScale(SourceBoneTransforms[34].Scale.X, SourceBoneTransforms[34].Scale.Y, SourceBoneTransforms[34].Scale.Z);
		BoneTransforms[67].SetScale(SourceBoneTransforms[152].Scale.X, 1.0f, SourceBoneTransforms[152].Scale.Z);
		BoneTransforms[72].SetPositionZ(SourceBoneTransforms[163].Location.Z + SourceBoneTransforms[164].Location.Z + SourceBoneTransforms[165].Location.Z + SourceBoneTransforms[166].Location.Z);
		BoneTransforms[72].SetRotation(SourceBoneTransforms[163].Rotation.X + SourceBoneTransforms[165].Rotation.X + SourceBoneTransforms[166].Rotation.X, SourceBoneTransforms[163].Rotation.Y, 0.0f);
		BoneTransforms[73].SetPositionX(SourceBoneTransforms[167].Location.X);
		BoneTransforms[73].SetPositionZ(SourceBoneTransforms[167].Location.Z + SourceBoneTransforms[168].Location.Z + SourceBoneTransforms[169].Location.Z + SourceBoneTransforms[170].Location.Z);
		BoneTransforms[73].SetRotation(SourceBoneTransforms[167].Rotation.X + SourceBoneTransforms[168].Rotation.X + SourceBoneTransforms[169].Rotation.X + SourceBoneTransforms[170].Rotation.X, SourceBoneTransforms[167].Rotation.Y, 0.0f);
		BoneTransforms[74].SetPositionX(SourceBoneTransforms[171].Location.X + SourceBoneTransforms[172].Location.X);
		BoneTransforms[74].SetPositionZ(SourceBoneTransforms[171].Location.Z + SourceBoneTransforms[172].Location.Z + SourceBoneTransforms[173].Location.Z + SourceBoneTransforms[174].Location.Z);
		BoneTransforms[74].SetRotation(SourceBoneTransforms[171].Rotation.X + SourceBoneTransforms[172].Rotation.X, SourceBoneTransforms[171].Rotation.Y, 0.0f);
		BoneTransforms[75].SetPositionX(SourceBoneTransforms[175].Location.X);
		BoneTransforms[75].SetPositionZ(SourceBoneTransforms[175].Location.Z + SourceBoneTransforms[176].Location.Z + SourceBoneTransforms[177].Location.Z + SourceBoneTransforms[178].Location.Z);
		BoneTransforms[75].SetRotation(SourceBoneTransforms[175].Rotation.X + SourceBoneTransforms[176].Rotation.X + SourceBoneTransforms[177].Rotation.X + SourceBoneTransforms[178].Rotation.X, SourceBoneTransforms[175].Rotation.Y, 0.0f);
		BoneTransforms[76].SetPositionZ(SourceBoneTransforms[179].Location.Z + SourceBoneTransforms[180].Location.Z + SourceBoneTransforms[181].Location.Z + SourceBoneTransforms[182].Location.Z);
		BoneTransforms[76].SetRotation(SourceBoneTransforms[179].Rotation.X + SourceBoneTransforms[181].Rotation.X + SourceBoneTransforms[182].Rotation.X, SourceBoneTransforms[179].Rotation.Y, 0.0f);
		BoneTransforms[77].SetPositionX(SourceBoneTransforms[183].Location.X + SourceBoneTransforms[184].Location.X);
		BoneTransforms[77].SetPositionZ(SourceBoneTransforms[183].Location.Z + SourceBoneTransforms[184].Location.Z + SourceBoneTransforms[185].Location.Z + SourceBoneTransforms[186].Location.Z);
		BoneTransforms[77].SetRotation(SourceBoneTransforms[183].Rotation.X + SourceBoneTransforms[184].Rotation.X + SourceBoneTransforms[185].Rotation.X + SourceBoneTransforms[186].Rotation.X, SourceBoneTransforms[183].Rotation.Y, 0.0f);
		BoneTransforms[78].SetPositionX(SourceBoneTransforms[187].Location.X + SourceBoneTransforms[188].Location.X);
		BoneTransforms[78].SetPositionZ(SourceBoneTransforms[187].Location.Z + SourceBoneTransforms[188].Location.Z + SourceBoneTransforms[189].Location.Z + SourceBoneTransforms[190].Location.Z);
		BoneTransforms[78].SetRotation(SourceBoneTransforms[187].Rotation.X + SourceBoneTransforms[188].Rotation.X, SourceBoneTransforms[187].Rotation.Y, 0.0f);
		BoneTransforms[79].SetPositionX(SourceBoneTransforms[191].Location.X + SourceBoneTransforms[192].Location.X);
		BoneTransforms[79].SetPositionZ(SourceBoneTransforms[191].Location.Z + SourceBoneTransforms[192].Location.Z + SourceBoneTransforms[193].Location.Z + SourceBoneTransforms[194].Location.Z);
		BoneTransforms[79].SetRotation(SourceBoneTransforms[191].Rotation.X + SourceBoneTransforms[192].Rotation.X + SourceBoneTransforms[193].Rotation.X + SourceBoneTransforms[194].Rotation.X, SourceBoneTransforms[191].Rotation.Y, 0.0f);
		BoneTransforms[66].SetPositionZ(SourceBoneTransforms[151].Location.Z + SourceBoneTransforms[150].Location.Z);
		BoneTransforms[66].SetRotation(SourceBoneTransforms[151].Rotation.X, 0.0f, 0.0f);
		BoneTransforms[62].SetPositionX(SourceBoneTransforms[138].Location.X);
		BoneTransforms[62].SetScale(SourceBoneTransforms[134].Scale.X, SourceBoneTransforms[136].Scale.Y * SourceBoneTransforms[134].Scale.Z, SourceBoneTransforms[136].Scale.Z * SourceBoneTransforms[134].Scale.Y);
		BoneTransforms[63].SetPositionX(SourceBoneTransforms[139].Location.X);
		BoneTransforms[63].SetScale(SourceBoneTransforms[135].Scale.X, SourceBoneTransforms[137].Scale.Y * SourceBoneTransforms[135].Scale.Z, SourceBoneTransforms[137].Scale.Z * SourceBoneTransforms[135].Scale.Y);
		BoneTransforms[6].SetPositionX(SourceBoneTransforms[16].Location.X);
		BoneTransforms[6].SetPositionY(SourceBoneTransforms[14].Location.Y + SourceBoneTransforms[16].Location.Y);
		BoneTransforms[6].SetRotation(0.0f, SourceBoneTransforms[14].Rotation.Y, SourceBoneTransforms[14].Rotation.Z + SourceBoneTransforms[16].Rotation.Z);
		BoneTransforms[6].SetScale(1.0f, SourceBoneTransforms[14].Scale.Y * SourceBoneTransforms[12].Scale.Y, SourceBoneTransforms[14].Scale.Z * SourceBoneTransforms[12].Scale.Z);
		BoneTransforms[7].SetPositionX(SourceBoneTransforms[17].Location.X);
		BoneTransforms[7].SetPositionY(SourceBoneTransforms[15].Location.Y + SourceBoneTransforms[17].Location.Y);
		BoneTransforms[7].SetRotation(0.0f, SourceBoneTransforms[15].Rotation.Y, SourceBoneTransforms[15].Rotation.Z + SourceBoneTransforms[17].Rotation.Z);
		BoneTransforms[7].SetScale(1.0f, SourceBoneTransforms[15].Scale.Y * SourceBoneTransforms[13].Scale.Y, SourceBoneTransforms[15].Scale.Z * SourceBoneTransforms[13].Scale.Z);
		BoneTransforms[8].SetScale(1.0f, SourceBoneTransforms[20].Scale.Y * SourceBoneTransforms[18].Scale.Y, SourceBoneTransforms[20].Scale.Z * SourceBoneTransforms[18].Scale.Z);
		BoneTransforms[9].SetScale(1.0f, SourceBoneTransforms[21].Scale.Y * SourceBoneTransforms[19].Scale.Y, SourceBoneTransforms[21].Scale.Z * SourceBoneTransforms[19].Scale.Z);
		BoneTransforms[10].SetScale(1.0f, SourceBoneTransforms[24].Scale.Y * SourceBoneTransforms[22].Scale.Y, SourceBoneTransforms[24].Scale.Z * SourceBoneTransforms[22].Scale.Z);
		BoneTransforms[11].SetScale(1.0f, SourceBoneTransforms[25].Scale.Y * SourceBoneTransforms[23].Scale.Y, SourceBoneTransforms[25].Scale.Z * SourceBoneTransforms[23].Scale.Z);
		BoneTransforms[0].SetScale(1.0f, SourceBoneTransforms[2].Scale.Y * SourceBoneTransforms[0].Scale.Y, SourceBoneTransforms[2].Scale.Z * SourceBoneTransforms[0].Scale.Z);
		BoneTransforms[1].SetScale(1.0f, SourceBoneTransforms[3].Scale.Y * SourceBoneTransforms[1].Scale.Y, SourceBoneTransforms[3].Scale.Z * SourceBoneTransforms[1].Scale.Z);
		BoneTransforms[2].SetScale(1.0f, SourceBoneTransforms[6].Scale.Y * SourceBoneTransforms[4].Scale.Y, SourceBoneTransforms[6].Scale.Z * SourceBoneTransforms[4].Scale.Z);
		BoneTransforms[3].SetScale(1.0f, SourceBoneTransforms[7].Scale.Y * SourceBoneTransforms[5].Scale.Y, SourceBoneTransforms[7].Scale.Z * SourceBoneTransforms[5].Scale.Z);
		BoneTransforms[4].SetScale(1.0f, SourceBoneTransforms[10].Scale.Y * SourceBoneTransforms[8].Scale.Y, SourceBoneTransforms[10].Scale.Z * SourceBoneTransforms[8].Scale.Z);
		BoneTransforms[5].SetScale(1.0f, SourceBoneTransforms[11].Scale.Y * SourceBoneTransforms[9].Scale.Y, SourceBoneTransforms[11].Scale.Z * SourceBoneTransforms[9].Scale.Z);
		BoneTransforms[12].SetScale(SourceBoneTransforms[26].Scale.X, SourceBoneTransforms[26].Scale.Y, SourceBoneTransforms[26].Scale.Z);
		BoneTransforms[13].SetScale(SourceBoneTransforms[27].Scale.X, SourceBoneTransforms[27].Scale.Y, SourceBoneTransforms[27].Scale.Z);
		BoneTransforms[14].SetScale(1.0f, SourceBoneTransforms[30].Scale.Y * SourceBoneTransforms[28].Scale.Y, SourceBoneTransforms[30].Scale.Z * SourceBoneTransforms[28].Scale.Z);
		BoneTransforms[15].SetScale(1.0f, SourceBoneTransforms[31].Scale.Y * SourceBoneTransforms[29].Scale.Y, SourceBoneTransforms[31].Scale.Z * SourceBoneTransforms[29].Scale.Z);
		BoneTransforms[17].SetScale(SourceBoneTransforms[35].Scale.X * SourceBoneTransforms[36].Scale.X, 1.0f, SourceBoneTransforms[35].Scale.Z * SourceBoneTransforms[37].Scale.Z);
		BoneTransforms[18].SetScale(SourceBoneTransforms[38].Scale.X * SourceBoneTransforms[39].Scale.X, 1.0f, SourceBoneTransforms[38].Scale.Z * SourceBoneTransforms[40].Scale.Z);
		BoneTransforms[68].SetScale(SourceBoneTransforms[153].Scale.X * SourceBoneTransforms[154].Scale.X, 1.0f, SourceBoneTransforms[153].Scale.Z * SourceBoneTransforms[155].Scale.Z);
		BoneTransforms[68].SetPositionY(SourceBoneTransforms[156].Location.Y);
		BoneTransforms[68].SetPositionZ(SourceBoneTransforms[154].Location.Z + SourceBoneTransforms[155].Location.Z);
		BoneTransforms[69].SetScale(SourceBoneTransforms[157].Scale.X * SourceBoneTransforms[158].Scale.X, 1.0f, SourceBoneTransforms[157].Scale.Z * SourceBoneTransforms[159].Scale.Z);
		BoneTransforms[70].SetScale(SourceBoneTransforms[160].Scale.X * SourceBoneTransforms[161].Scale.X, 1.0f, SourceBoneTransforms[160].Scale.Z * SourceBoneTransforms[162].Scale.Z);
		BoneTransforms[61].SetScale(SourceBoneTransforms[129].Scale.X * SourceBoneTransforms[130].Scale.X, 1.0f, SourceBoneTransforms[129].Scale.Z * SourceBoneTransforms[131].Scale.Z);
		BoneTransforms[16].SetScale(SourceBoneTransforms[32].Scale.X * SourceBoneTransforms[33].Scale.X, SourceBoneTransforms[32].Scale.Y * SourceBoneTransforms[33].Scale.Y, SourceBoneTransforms[32].Scale.Z * SourceBoneTransforms[33].Scale.Z);
	}
	{
		BoneTransforms[47].SetPositionX(SourceBoneTransforms[97].Location.X + SourceBoneTransforms[101].Location.X);
		BoneTransforms[47].SetPositionY(SourceBoneTransforms[99].Location.Y + SourceBoneTransforms[105].Location.Y);
		BoneTransforms[47].SetPositionZ(SourceBoneTransforms[99].Location.Z + SourceBoneTransforms[103].Location.Z);
		BoneTransforms[47].SetRotation(SourceBoneTransforms[99].Rotation.X + SourceBoneTransforms[103].Rotation.X + SourceBoneTransforms[105].Rotation.X, SourceBoneTransforms[101].Rotation.Y + SourceBoneTransforms[105].Rotation.Y, 0.0f);
		BoneTransforms[47].SetScale(SourceBoneTransforms[93].Scale.X, SourceBoneTransforms[93].Scale.Y, SourceBoneTransforms[93].Scale.Z);
		BoneTransforms[45].SetPositionY(SourceBoneTransforms[95].Location.Y);
		BoneTransforms[45].SetPositionZ(SourceBoneTransforms[95].Location.Z);
		BoneTransforms[45].SetRotation(0.0f, 0.0f, SourceBoneTransforms[95].Rotation.Z);
		BoneTransforms[45].SetScale(SourceBoneTransforms[95].Scale.X, SourceBoneTransforms[95].Scale.Y, SourceBoneTransforms[95].Scale.Z);
		BoneTransforms[43].SetPositionX(SourceBoneTransforms[113].Location.X + SourceBoneTransforms[111].Location.X);
		BoneTransforms[43].SetPositionZ(0.072f + SourceBoneTransforms[109].Location.Z + SourceBoneTransforms[111].Location.Z);
		BoneTransforms[43].SetRotation(SourceBoneTransforms[109].Rotation.X, SourceBoneTransforms[111].Rotation.Y, 0.0f);
		BoneTransforms[49].SetPositionY(SourceBoneTransforms[107].Location.Y);
		BoneTransforms[49].SetRotation(SourceBoneTransforms[107].Rotation.X, SourceBoneTransforms[107].Rotation.Y, SourceBoneTransforms[107].Rotation.Z);
		BoneTransforms[49].SetScale(SourceBoneTransforms[107].Scale.X, SourceBoneTransforms[107].Scale.Y, SourceBoneTransforms[107].Scale.Z);
		BoneTransforms[51].SetPositionX(SourceBoneTransforms[107].Location.X);
		BoneTransforms[51].SetPositionZ(0.03f + SourceBoneTransforms[115].Location.Z);
		BoneTransforms[51].SetRotation(SourceBoneTransforms[119].Rotation.X, 0.0f, 0.0f);
		BoneTransforms[53].SetPositionY(SourceBoneTransforms[117].Location.Y);
		BoneTransforms[53].SetPositionZ(SourceBoneTransforms[117].Location.Z);
		BoneTransforms[53].SetRotation(SourceBoneTransforms[117].Rotation.X, 0.0f, 0.0f);
		BoneTransforms[53].SetScale(SourceBoneTransforms[117].Scale.X, SourceBoneTransforms[117].Scale.Y, SourceBoneTransforms[117].Scale.Z);
		BoneTransforms[55].SetPositionZ(0.03f + SourceBoneTransforms[121].Location.Z);
		BoneTransforms[55].SetRotation(SourceBoneTransforms[125].Rotation.X, 0.0f, 0.0f);
		BoneTransforms[57].SetPositionZ(SourceBoneTransforms[123].Location.Z);
		BoneTransforms[57].SetScale(SourceBoneTransforms[123].Scale.X, SourceBoneTransforms[123].Scale.Y, SourceBoneTransforms[123].Scale.Z);
		BoneTransforms[59].SetPositionZ(SourceBoneTransforms[127].Location.Z);
		BoneTransforms[59].SetScale(SourceBoneTransforms[127].Scale.X, SourceBoneTransforms[127].Scale.Y, SourceBoneTransforms[127].Scale.Z);
		BoneTransforms[37].SetPositionZ(SourceBoneTransforms[85].Location.Z + SourceBoneTransforms[83].Location.Z);
		BoneTransforms[37].SetScale(SourceBoneTransforms[85].Scale.X * SourceBoneTransforms[83].Scale.X, SourceBoneTransforms[85].Scale.Y * SourceBoneTransforms[83].Scale.Y, SourceBoneTransforms[85].Scale.Z);
		BoneTransforms[80].SetPositionX(x10);
		BoneTransforms[80].SetPositionY(y5);
		BoneTransforms[80].SetPositionZ(z5);
		BoneTransforms[80].SetScale(x11, 1.0f, x11);
	}
	{
		BoneTransforms[39].SetPositionZ(SourceBoneTransforms[87].Location.Z);
		BoneTransforms[39].SetScale(SourceBoneTransforms[87].Scale.X, SourceBoneTransforms[87].Scale.Y, SourceBoneTransforms[87].Scale.Z);
		BoneTransforms[41].SetPositionZ(SourceBoneTransforms[89].Location.Z + SourceBoneTransforms[91].Location.Z);
		BoneTransforms[41].SetScale(SourceBoneTransforms[81].Scale.X * SourceBoneTransforms[91].Scale.X, SourceBoneTransforms[81].Scale.Y * SourceBoneTransforms[91].Scale.Y, SourceBoneTransforms[81].Scale.Z * SourceBoneTransforms[91].Scale.Z);
	}
	{
		BoneTransforms[48].SetPositionX(SourceBoneTransforms[98].Location.X + SourceBoneTransforms[102].Location.X);
		BoneTransforms[48].SetPositionY(SourceBoneTransforms[100].Location.Y + SourceBoneTransforms[106].Location.Y);
		BoneTransforms[48].SetPositionZ(SourceBoneTransforms[100].Location.Z + SourceBoneTransforms[104].Location.Z);
		BoneTransforms[48].SetRotation(SourceBoneTransforms[100].Rotation.X + SourceBoneTransforms[104].Rotation.X + SourceBoneTransforms[106].Rotation.X, SourceBoneTransforms[102].Rotation.Y + SourceBoneTransforms[106].Rotation.Y, 0.0f);
		BoneTransforms[48].SetScale(SourceBoneTransforms[94].Scale.X, SourceBoneTransforms[94].Scale.Y, SourceBoneTransforms[94].Scale.Z);
		BoneTransforms[46].SetPositionY(SourceBoneTransforms[96].Location.Y);
		BoneTransforms[46].SetPositionZ(SourceBoneTransforms[96].Location.Z);
		BoneTransforms[46].SetRotation(0.0f, 0.0f, SourceBoneTransforms[96].Rotation.Z);
		BoneTransforms[46].SetScale(SourceBoneTransforms[96].Scale.X, SourceBoneTransforms[96].Scale.Y, SourceBoneTransforms[96].Scale.Z);
		BoneTransforms[44].SetPositionX(SourceBoneTransforms[114].Location.X + SourceBoneTransforms[112].Location.X);
		BoneTransforms[44].SetPositionZ(0.072f + SourceBoneTransforms[110].Location.Z + SourceBoneTransforms[112].Location.Z);
		BoneTransforms[44].SetRotation(SourceBoneTransforms[110].Rotation.X, SourceBoneTransforms[112].Rotation.Y, 0.0f);
		BoneTransforms[50].SetPositionY(SourceBoneTransforms[108].Location.Y);
		BoneTransforms[50].SetRotation(SourceBoneTransforms[108].Rotation.X, SourceBoneTransforms[108].Rotation.Y, SourceBoneTransforms[108].Rotation.Z);
		BoneTransforms[50].SetScale(SourceBoneTransforms[108].Scale.X, SourceBoneTransforms[108].Scale.Y, SourceBoneTransforms[108].Scale.Z);
		BoneTransforms[52].SetPositionX(SourceBoneTransforms[108].Location.X);
		BoneTransforms[52].SetPositionZ(0.03f + SourceBoneTransforms[116].Location.Z);
		BoneTransforms[52].SetRotation(SourceBoneTransforms[120].Rotation.X, 0.0f, 0.0f);
		BoneTransforms[54].SetPositionY(SourceBoneTransforms[118].Location.Y);
		BoneTransforms[54].SetPositionZ(SourceBoneTransforms[118].Location.Z);
		BoneTransforms[54].SetRotation(SourceBoneTransforms[118].Rotation.X, 0.0f, 0.0f);
		BoneTransforms[54].SetScale(SourceBoneTransforms[118].Scale.X, SourceBoneTransforms[118].Scale.Y, SourceBoneTransforms[118].Scale.Z);
		BoneTransforms[56].SetPositionZ(0.03f + SourceBoneTransforms[122].Location.Z);
		BoneTransforms[56].SetRotation(SourceBoneTransforms[126].Rotation.X, 0.0f, 0.0f);
		BoneTransforms[58].SetPositionZ(SourceBoneTransforms[124].Location.Z);
		BoneTransforms[58].SetScale(SourceBoneTransforms[124].Scale.X, SourceBoneTransforms[124].Scale.Y, SourceBoneTransforms[124].Scale.Z);
		BoneTransforms[60].SetPositionZ(SourceBoneTransforms[128].Location.Z);
		BoneTransforms[60].SetScale(SourceBoneTransforms[128].Scale.X, SourceBoneTransforms[128].Scale.Y, SourceBoneTransforms[128].Scale.Z);
		BoneTransforms[38].SetPositionZ(SourceBoneTransforms[86].Location.Z + SourceBoneTransforms[84].Location.Z);
		BoneTransforms[38].SetScale(SourceBoneTransforms[86].Scale.X * SourceBoneTransforms[84].Scale.X, SourceBoneTransforms[86].Scale.Y * SourceBoneTransforms[84].Scale.Y, SourceBoneTransforms[86].Scale.Z);
		BoneTransforms[81].SetPositionX(x12);
		BoneTransforms[81].SetPositionY(y6);
		BoneTransforms[81].SetPositionZ(z6);
		BoneTransforms[81].SetScale(x13, 1.0f, x13);
	}
	{
		BoneTransforms[40].SetPositionZ(SourceBoneTransforms[88].Location.Z);
		BoneTransforms[40].SetScale(SourceBoneTransforms[88].Scale.X, SourceBoneTransforms[88].Scale.Y, SourceBoneTransforms[88].Scale.Z);
		BoneTransforms[42].SetPositionZ(SourceBoneTransforms[90].Location.Z + SourceBoneTransforms[92].Location.Z);
		BoneTransforms[42].SetScale(SourceBoneTransforms[82].Scale.X * SourceBoneTransforms[92].Scale.X, SourceBoneTransforms[82].Scale.Y * SourceBoneTransforms[92].Scale.Y, SourceBoneTransforms[82].Scale.Z * SourceBoneTransforms[92].Scale.Z);
	}
	{
		BoneTransforms[35].SetScale(SourceBoneTransforms[79].Scale.X, 1.0f, SourceBoneTransforms[79].Scale.Z);
		BoneTransforms[36].SetScale(SourceBoneTransforms[80].Scale.X, 1.0f, SourceBoneTransforms[80].Scale.Z);
		BoneTransforms[29].SetPositionX(SourceBoneTransforms[65].Location.X + SourceBoneTransforms[59].Location.X);
		BoneTransforms[29].SetRotation(0.0f, 0.0f, SourceBoneTransforms[59].Rotation.Z);
		BoneTransforms[29].SetScale(SourceBoneTransforms[63].Scale.X * SourceBoneTransforms[65].Scale.X * SourceBoneTransforms[59].Scale.X, 1.0f, SourceBoneTransforms[63].Scale.Z * SourceBoneTransforms[65].Scale.Z * SourceBoneTransforms[61].Scale.Z);
		BoneTransforms[30].SetPositionX(SourceBoneTransforms[66].Location.X + SourceBoneTransforms[60].Location.X);
		BoneTransforms[30].SetRotation(0.0f, 0.0f, SourceBoneTransforms[60].Rotation.Z);
		BoneTransforms[30].SetScale(SourceBoneTransforms[64].Scale.X * SourceBoneTransforms[66].Scale.X * SourceBoneTransforms[60].Scale.X, 1.0f, SourceBoneTransforms[64].Scale.Z * SourceBoneTransforms[66].Scale.Z * SourceBoneTransforms[62].Scale.Z);
		BoneTransforms[31].SetScale(SourceBoneTransforms[69].Scale.X * SourceBoneTransforms[71].Scale.X * SourceBoneTransforms[67].Scale.X, 1.0f, SourceBoneTransforms[69].Scale.Z * SourceBoneTransforms[71].Scale.Z * SourceBoneTransforms[67].Scale.Z);
		BoneTransforms[32].SetScale(SourceBoneTransforms[70].Scale.X * SourceBoneTransforms[72].Scale.X * SourceBoneTransforms[68].Scale.X, 1.0f, SourceBoneTransforms[70].Scale.Z * SourceBoneTransforms[72].Scale.Z * SourceBoneTransforms[68].Scale.Z);
		BoneTransforms[33].SetScale(SourceBoneTransforms[75].Scale.X * SourceBoneTransforms[77].Scale.X * SourceBoneTransforms[73].Scale.X, 1.0f, SourceBoneTransforms[75].Scale.Z * SourceBoneTransforms[77].Scale.Z * SourceBoneTransforms[73].Scale.Z);
		BoneTransforms[34].SetScale(SourceBoneTransforms[76].Scale.X * SourceBoneTransforms[78].Scale.X * SourceBoneTransforms[74].Scale.X, 1.0f, SourceBoneTransforms[76].Scale.Z * SourceBoneTransforms[78].Scale.Z * SourceBoneTransforms[74].Scale.Z);
		BoneTransforms[19].SetPositionZ(SourceBoneTransforms[41].Location.Z);
		BoneTransforms[19].SetScale(SourceBoneTransforms[41].Scale.X, 1.0f, SourceBoneTransforms[41].Scale.Z);
		BoneTransforms[20].SetPositionZ(SourceBoneTransforms[42].Location.Z);
		BoneTransforms[20].SetScale(SourceBoneTransforms[42].Scale.X, 1.0f, SourceBoneTransforms[42].Scale.Z);
		BoneTransforms[21].SetPositionZ(SourceBoneTransforms[45].Location.Z);
		BoneTransforms[21].SetScale(SourceBoneTransforms[47].Scale.X * SourceBoneTransforms[45].Scale.X * SourceBoneTransforms[43].Scale.X, 1.0f, SourceBoneTransforms[47].Scale.Z * SourceBoneTransforms[45].Scale.Z * SourceBoneTransforms[43].Scale.Z);
		BoneTransforms[22].SetPositionZ(SourceBoneTransforms[46].Location.Z);
		BoneTransforms[22].SetScale(SourceBoneTransforms[48].Scale.X * SourceBoneTransforms[46].Scale.X * SourceBoneTransforms[44].Scale.X, 1.0f, SourceBoneTransforms[48].Scale.Z * SourceBoneTransforms[46].Scale.Z * SourceBoneTransforms[44].Scale.Z);
		BoneTransforms[23].SetRotation(SourceBoneTransforms[51].Rotation.X, 0.0f, 0.0f);
		BoneTransforms[23].SetScale(SourceBoneTransforms[49].Scale.X * SourceBoneTransforms[51].Scale.X, 1.0f, SourceBoneTransforms[49].Scale.Z * SourceBoneTransforms[51].Scale.Z);
		BoneTransforms[24].SetRotation(SourceBoneTransforms[52].Rotation.X, 0.0f, 0.0f);
		BoneTransforms[24].SetScale(SourceBoneTransforms[50].Scale.X * SourceBoneTransforms[52].Scale.X, 1.0f, SourceBoneTransforms[50].Scale.Z * SourceBoneTransforms[52].Scale.Z);
		BoneTransforms[25].SetScale(SourceBoneTransforms[53].Scale.X * SourceBoneTransforms[55].Scale.X, 1.0f, SourceBoneTransforms[53].Scale.Z * SourceBoneTransforms[55].Scale.Z);
		BoneTransforms[26].SetScale(SourceBoneTransforms[54].Scale.X * SourceBoneTransforms[56].Scale.X, 1.0f, SourceBoneTransforms[54].Scale.Z * SourceBoneTransforms[56].Scale.Z);
		BoneTransforms[27].SetPositionX(SourceBoneTransforms[57].Location.X);
		BoneTransforms[27].SetPositionZ(SourceBoneTransforms[57].Location.Z);
		BoneTransforms[27].SetRotation(SourceBoneTransforms[57].Rotation.X, 0.0f, SourceBoneTransforms[57].Rotation.Z);
		BoneTransforms[27].SetScale(SourceBoneTransforms[57].Scale.X, 1.0f, SourceBoneTransforms[57].Scale.Z);
		BoneTransforms[28].SetPositionX(SourceBoneTransforms[58].Location.X);
		BoneTransforms[28].SetPositionZ(SourceBoneTransforms[57].Location.Z);
		BoneTransforms[28].SetRotation(SourceBoneTransforms[58].Rotation.X, 0.0f, SourceBoneTransforms[58].Rotation.Z);
		BoneTransforms[28].SetScale(SourceBoneTransforms[58].Scale.X, 1.0f, SourceBoneTransforms[58].Scale.Z);
		BoneTransforms[64].SetPosition(SourceBoneTransforms[146].Location.X, SourceBoneTransforms[148].Location.Y + SourceBoneTransforms[146].Location.Y, SourceBoneTransforms[144].Location.Z + SourceBoneTransforms[146].Location.Z);
		BoneTransforms[64].SetRotation(SourceBoneTransforms[148].Rotation.X, 0.0f, 0.0f);
		BoneTransforms[64].SetScale(SourceBoneTransforms[142].Scale.X * SourceBoneTransforms[144].Scale.X * SourceBoneTransforms[146].Scale.X, SourceBoneTransforms[146].Scale.Y, SourceBoneTransforms[140].Scale.Z * SourceBoneTransforms[142].Scale.Z * SourceBoneTransforms[144].Scale.Z * SourceBoneTransforms[146].Scale.Z);
		BoneTransforms[65].SetPosition(SourceBoneTransforms[147].Location.X, SourceBoneTransforms[149].Location.Y + SourceBoneTransforms[147].Location.Y, SourceBoneTransforms[145].Location.Z + SourceBoneTransforms[147].Location.Z);
		BoneTransforms[65].SetRotation(SourceBoneTransforms[149].Rotation.X, 0.0f, 0.0f);
		BoneTransforms[65].SetScale(SourceBoneTransforms[143].Scale.X * SourceBoneTransforms[145].Scale.X * SourceBoneTransforms[147].Scale.X, SourceBoneTransforms[147].Scale.Y, SourceBoneTransforms[141].Scale.Z * SourceBoneTransforms[143].Scale.Z * SourceBoneTransforms[145].Scale.Z * SourceBoneTransforms[147].Scale.Z);
		BoneTransforms[82].SetPositionY(num + num3);
		BoneTransforms[82].SetPositionZ(z + num4);
		BoneTransforms[82].SetScale(x * num6 * SourceBoneTransforms[207].Scale.X, num2 * num5, x * num6);
		BoneTransforms[83].SetPositionX(-num7);
		BoneTransforms[83].SetPositionY(SourceBoneTransforms[199].Location.Y);
		BoneTransforms[83].SetRotation(SourceBoneTransforms[199].Rotation.X, 0.0f, -num8);
		BoneTransforms[83].SetScale(x2, y, 1.0f);
		BoneTransforms[84].SetPositionX(num7);
		BoneTransforms[84].SetPositionY(SourceBoneTransforms[199].Location.Y);
		BoneTransforms[84].SetRotation(SourceBoneTransforms[199].Rotation.X, 0.0f, num8);
		BoneTransforms[84].SetScale(x2, y, 1.0f);
		BoneTransforms[85].SetPositionX(-(x3 + x4 + x6));
		BoneTransforms[85].SetPositionY(y2 + y4);
		BoneTransforms[85].SetPositionZ(num11 + z2 + z3 + z4);
		BoneTransforms[85].SetRotation(x8, 0.0f, 0.0f);
		BoneTransforms[85].SetScale(num9 * x5 * x7 * x9 / SourceBoneTransforms[207].Scale.X, num10 * y3, num9 * x5 * x7);
		BoneTransforms[86].SetPositionX(x3 + x4 + x6);
		BoneTransforms[86].SetPositionY(y2 + y4);
		BoneTransforms[86].SetPositionZ(num11 + z2 + z3 + z4);
		BoneTransforms[86].SetRotation(x8, 0.0f, 0.0f);
		BoneTransforms[86].SetScale(num9 * x5 * x7 * x9 / SourceBoneTransforms[207].Scale.X, num10 * y3, num9 * x5 * x7);
	}
}
