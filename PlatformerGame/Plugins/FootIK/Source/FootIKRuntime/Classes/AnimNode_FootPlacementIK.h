// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_FootPlacementIK.generated.h"

USTRUCT()
struct FOOTIKRUNTIME_API FAnimNode_FootPlacementIK : public FAnimNode_SkeletalControlBase
{
	GENERATED_USTRUCT_BODY()

	/** Name of bone to control. This is the main bone chain to modify from. **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=IK)
	FBoneReference IKBone;

	/** Effector Location. **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=JointTarget, meta=(PinShownByDefault))
	FVector JointTargetLocation;

	/** Reference frame of Effector Location. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=JointTarget)
	TEnumAsByte<enum EBoneControlSpace> JointTargetLocationSpace;

	/** If EffectorLocationSpace is a bone, this is the bone to use. **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=JointTarget)
	FName JointTargetSpaceBoneName;

	/** if set, node can stretch as much as possible, bone to catch effector location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=IK)
	uint32 bAllowStretching:1;

	/** limits for bone stretching */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=IK)
	FVector2D StretchLimits;

	/** Z offset from hit point, to correct effector location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=IK)
	float HitZOffset;

	/** time to blend in/out node's influence */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=IK)
	float BlendTime;

	/** Internal use - activation time for node blending */
	float ActivationTime;

	FAnimNode_FootPlacementIK();

	// FAnimNode_Base interface
	virtual void UpdateInternal(const FAnimationUpdateContext& Context) override;
	// End of FAnimNode_Base interface

	// FAnimNode_SkeletalControlBase interface
	virtual void EvaluateBoneTransforms(USkeletalMeshComponent* SkelComp, FCSPose<FCompactPose>& MeshBases, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	// End of FAnimNode_SkeletalControlBase interface

private:
	enum EMyBlendState
	{
		STATE_UNKNOWN = 0,
		STATE_BLEND_IN = 1,
		STATE_BLEND_OUT = 2
	};

	/** state of our node (init, blend-in, blend-out) */
	EMyBlendState BlendState;

	/** effector location calculated dynamically */
	FVector EffectorLocation;

	// FAnimNode_SkeletalControlBase interface
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	// End of FAnimNode_SkeletalControlBase interface

	/** calculate current effector location and blend alpha */
	void CalculateEffector(float DeltaTime, USkeletalMeshComponent* SkelComp, FCSPose<FCompactPose>& MeshBases);
};
