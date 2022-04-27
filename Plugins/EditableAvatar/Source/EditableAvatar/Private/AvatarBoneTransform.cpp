#include "AvatarBoneTransform.h"
#include "EditableAvatarPrivatePCH.h"


FAvatarBoneTransform::FAvatarBoneTransform()
	: Location(FVector::ZeroVector)
	, Rotation(FVector::ZeroVector)
	, Scale(1.0f)
{
}

int32 FAvatarBoneTransform::Equals(const FAvatarBoneTransform& Transform) const
{
	int32 Mask = 0;
	Mask |= FMath::IsNearlyEqual(Location.X, Transform.Location.X, KINDA_SMALL_NUMBER) ? 0 : (1 << 0);
	Mask |= FMath::IsNearlyEqual(Location.Y, Transform.Location.Y, KINDA_SMALL_NUMBER) ? 0 : (1 << 1);
	Mask |= FMath::IsNearlyEqual(Location.Z, Transform.Location.Z, KINDA_SMALL_NUMBER) ? 0 : (1 << 2);
	Mask |= FMath::IsNearlyEqual(Rotation.X, Transform.Rotation.X, KINDA_SMALL_NUMBER) ? 0 : (1 << 3);
	Mask |= FMath::IsNearlyEqual(Rotation.Y, Transform.Rotation.Y, KINDA_SMALL_NUMBER) ? 0 : (1 << 4);
	Mask |= FMath::IsNearlyEqual(Rotation.Z, Transform.Rotation.Z, KINDA_SMALL_NUMBER) ? 0 : (1 << 5);
	Mask |= FMath::IsNearlyEqual(Scale.X, Transform.Scale.X, KINDA_SMALL_NUMBER) ? 0 : (1 << 6);
	Mask |= FMath::IsNearlyEqual(Scale.Y, Transform.Scale.Y, KINDA_SMALL_NUMBER) ? 0 : (1 << 7);
	Mask |= FMath::IsNearlyEqual(Scale.Z, Transform.Scale.Z, KINDA_SMALL_NUMBER) ? 0 : (1 << 8);
	return Mask;
}

FAvatarBoneTransform& FAvatarBoneTransform::operator=(const FAvatarBoneTransform& Transform)
{
	Location = Transform.Location;
	Rotation = Transform.Rotation;
	Scale = Transform.Scale;
	return *this;
}

void FAvatarBoneTransform::SetPositionX(float X)
{
	Location.X = X;
}

void FAvatarBoneTransform::SetPositionY(float Y)
{
	Location.Y = Y;
}

void FAvatarBoneTransform::SetPositionZ(float Z)
{
	Location.Z = Z;
}

void FAvatarBoneTransform::SetPosition(float X, float Y, float Z)
{
	Location.X = X;
	Location.Y = Y;
	Location.Z = Z;
}

void FAvatarBoneTransform::SetRotation(float X, float Y, float Z)
{
	Rotation.X = X;
	Rotation.Y = Y;
	Rotation.Z = Z;
}

void FAvatarBoneTransform::SetScale(float X, float Y, float Z)
{
	Scale.X = X;
	Scale.Y = Y;
	Scale.Z = Z;
}

FTransform FAvatarBoneTransform::ToUnrealTransform() const
{
	return FTransform(FQuat::MakeFromEuler(Rotation), Location, Scale);
}

