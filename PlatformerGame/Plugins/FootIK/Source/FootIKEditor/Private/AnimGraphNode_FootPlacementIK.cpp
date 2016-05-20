// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "FootIKEditorPrivatePCH.h"

/////////////////////////////////////////////////////
// UAnimGraphNode_FootPlacementIK

#define LOCTEXT_NAMESPACE "A3Nodes"

UAnimGraphNode_FootPlacementIK::UAnimGraphNode_FootPlacementIK(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FText UAnimGraphNode_FootPlacementIK::GetControllerDescription() const
{
	return LOCTEXT("FootPlacementIK", "Foot placement IK");
}

FText UAnimGraphNode_FootPlacementIK::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("ControllerDescription"), GetControllerDescription());
	Args.Add(TEXT("BoneName"), FText::FromName(Node.IKBone.BoneName));

	if(TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle)
	{
		if (Node.IKBone.BoneName == NAME_None)
		{
			return FText::Format(LOCTEXT("FootPlacementIK_MenuTitle", "{ControllerDescription}"), Args);
		}
		return FText::Format(LOCTEXT("FootPlacementIK_ListTitle", "{ControllerDescription} - Bone: {BoneName}"), Args);
	}
	else
	{
		return FText::Format(LOCTEXT("FootPlacementIK_FullTitle", "{ControllerDescription}\nBone: {BoneName}"), Args);
	}
}

#undef LOCTEXT_NAMESPACE