// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AnimGraphNode_SkeletalControlBase.h"
#include "AnimGraphDefinitions.h"
#include "Kismet2/BlueprintEditorUtils.h"

#include "AnimGraphNode_FootPlacementIK.generated.h"

UCLASS(MinimalAPI)
class UAnimGraphNode_FootPlacementIK : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category=Settings)
	FAnimNode_FootPlacementIK Node;

public:
	// UEdGraphNode interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	// End of UEdGraphNode interface

protected:
	// UAnimGraphNode_SkeletalControlBase interface
	virtual FText GetControllerDescription() const override;
	// End of UAnimGraphNode_SkeletalControlBase interface
};
