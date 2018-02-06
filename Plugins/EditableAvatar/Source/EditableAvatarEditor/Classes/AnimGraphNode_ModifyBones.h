// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "AnimNode_ModifyBones.h"
#include "AnimGraphNode_Base.h"
#include "AnimGraphNode_ModifyBones.generated.h"

class FCompilerResultsLog;

UCLASS(meta = (Keywords = "Modify Transforms"))
class EDITABLEAVATAREDITOR_API UAnimGraphNode_ModifyBones : public UAnimGraphNode_Base
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_ModifyBones Node;

	// UEdGraphNode interface
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	// End of UEdGraphNode interface

	// UAnimGraphNode_Base interface
	virtual FString GetNodeCategory() const override;
	// End of UAnimGraphNode_Base interface
};

