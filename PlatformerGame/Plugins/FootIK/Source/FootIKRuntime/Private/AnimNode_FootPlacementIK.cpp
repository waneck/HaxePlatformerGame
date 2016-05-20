// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "FootIKRuntimePrivatePCH.h"
#include "AnimationRuntime.h"
#include "Animation/AnimInstanceProxy.h"

FAnimNode_FootPlacementIK::FAnimNode_FootPlacementIK()
	: FAnimNode_SkeletalControlBase()
	, BlendTime(0.2f)
	, ActivationTime(0)
	, BlendState(STATE_UNKNOWN)
{
}

void FAnimNode_FootPlacementIK::UpdateInternal(const FAnimationUpdateContext& Context)
{
	check(Context.AnimInstanceProxy);
	FAnimNode_SkeletalControlBase::UpdateInternal(Context);

	FCSPose<FCompactPose> MeshRefPose;
	MeshRefPose.InitPose(&Context.AnimInstanceProxy->GetRequiredBones());

	CalculateEffector(Context.GetDeltaTime(), Context.AnimInstanceProxy->GetSkelMeshComponent(), MeshRefPose);
}

void FAnimNode_FootPlacementIK::CalculateEffector(float DeltaTime, USkeletalMeshComponent* SkelComp, FCSPose<FCompactPose>& MeshBases)
{
	FCompactPoseBoneIndex IKBoneIndex = IKBone.GetCompactPoseIndex(MeshBases.GetPose().GetBoneContainer());
	if (!MeshBases.GetPose().IsValidIndex(IKBoneIndex))
	{
		return;
	}

	FTransform EndBoneWorldTransform = MeshBases.GetComponentSpaceTransform(IKBoneIndex);
	FAnimationRuntime::ConvertCSTransformToBoneSpace(SkelComp, MeshBases, EndBoneWorldTransform, FCompactPoseBoneIndex(INDEX_NONE), BCS_WorldSpace);
	const FVector EndBoneWorldPos = EndBoneWorldTransform.GetTranslation();
	const FVector TraceOffset(0,0,50);
	FHitResult Hit;
	FVector DesiredEffectorLocation = EndBoneWorldPos;
	SkelComp->GetWorld()->LineTraceSingleByChannel(Hit, EndBoneWorldPos + TraceOffset, EndBoneWorldPos - TraceOffset, ECC_Pawn, FCollisionQueryParams(NAME_None, true, SkelComp->GetOwner()));

	// check if we should blend-in or blend-out this node
	const EMyBlendState OldBlendState = BlendState;
	if (Hit.Actor.IsValid())
	{
		DesiredEffectorLocation = Hit.Location + FVector(0,0,HitZOffset);
		BlendState = STATE_BLEND_IN; 
		if ((DesiredEffectorLocation - EndBoneWorldPos).GetSafeNormal().Z <= 0 && !bAllowStretching)
		{
			BlendState = STATE_BLEND_OUT;
		}
	}
	else
	{
		DesiredEffectorLocation = EndBoneWorldPos;
		BlendState = STATE_BLEND_OUT;
	}

	if (OldBlendState != BlendState)
	{
		ActivationTime = SkelComp->GetWorld()->GetTimeSeconds();
	}

	if (BlendState == STATE_BLEND_IN)
	{
		AlphaScaleBias.Scale = ((SkelComp->GetWorld()->GetTimeSeconds() - ActivationTime) / BlendTime);
	}
	else if (BlendState == STATE_BLEND_OUT)
	{
		AlphaScaleBias.Scale = 1-((SkelComp->GetWorld()->GetTimeSeconds() - ActivationTime) / BlendTime);
	}

	// base on node internal state, calculate effector location (or keep current location)
	if ((BlendState == STATE_BLEND_IN) && (AlphaScaleBias.Scale >= 0.9f))
	{
		// we are almost fully blended in so blend effector location to new position (to avoid rapid effector location changes)
		const float Delta = FMath::Clamp<float>(DeltaTime / BlendTime, 0.0f, 1.0f);
		EffectorLocation = FVector(DesiredEffectorLocation.X, DesiredEffectorLocation.Y, EffectorLocation.Z + Delta * (DesiredEffectorLocation.Z - EffectorLocation.Z) );
	}
	else if (BlendState != STATE_BLEND_OUT)
	{
		EffectorLocation = DesiredEffectorLocation;
	}
	else if (EffectorLocation == FVector::ZeroVector)
	{
		EffectorLocation = EndBoneWorldPos;
	}

	AlphaScaleBias.Scale = FMath::Clamp<float>(AlphaScaleBias.Scale, 0.0f, 1.0f);
	DeltaTime = 0.0f;
}

void FAnimNode_FootPlacementIK::EvaluateBoneTransforms(USkeletalMeshComponent* SkelComp, FCSPose<FCompactPose>& MeshBases, TArray<FBoneTransform>& OutBoneTransforms)
{
	// Get indices of the lower and upper limb bones and check validity.
	bool bInvalidLimb = false;

	const FBoneContainer BoneContainer = MeshBases.GetPose().GetBoneContainer();

	const FCompactPoseBoneIndex IKBoneIndex = IKBone.GetCompactPoseIndex(BoneContainer);
	const FCompactPoseBoneIndex LowerLimbIndex = MeshBases.GetPose().GetParentBoneIndex(IKBoneIndex);

	if (LowerLimbIndex == INDEX_NONE)
	{
		bInvalidLimb = true;
	}

	const FCompactPoseBoneIndex UpperLimbIndex = MeshBases.GetPose().GetParentBoneIndex(LowerLimbIndex);
	if (UpperLimbIndex == INDEX_NONE)
	{
		bInvalidLimb = true;
	}

	// If we walked past the root, this controlled is invalid, so return no affected bones.
	if( bInvalidLimb )
	{
		return;
	}

	// Get Local Space transforms for our bones. We do this first in case they already are local.
	// As right after we get them in component space. (And that does the auto conversion).
	// We might save one transform by doing local first...
	const FTransform UpperLimbLocalTransform = MeshBases.GetLocalSpaceTransform(UpperLimbIndex);
	const FTransform LowerLimbLocalTransform = MeshBases.GetLocalSpaceTransform(LowerLimbIndex);
	const FTransform EndBoneLocalTransform = MeshBases.GetLocalSpaceTransform(IKBoneIndex);

	// Now get those in component space...
	FTransform UpperLimbCSTransform = MeshBases.GetComponentSpaceTransform(UpperLimbIndex);
	FTransform LowerLimbCSTransform = MeshBases.GetComponentSpaceTransform(LowerLimbIndex);
	FTransform EndBoneCSTransform = MeshBases.GetComponentSpaceTransform(IKBoneIndex);

	// Get current position of root of limb.
	// All position are in Component space.
	const FVector RootPos = UpperLimbCSTransform.GetTranslation();
	const FVector InitialJointPos = LowerLimbCSTransform.GetTranslation();
	const FVector InitialEndPos = EndBoneCSTransform.GetTranslation();

	FTransform EffectorTransform(EffectorLocation);
	FAnimationRuntime::ConvertBoneSpaceTransformToCS(SkelComp, MeshBases, EffectorTransform, FCompactPoseBoneIndex(INDEX_NONE), BCS_WorldSpace);

	// This is our reach goal.
	FVector DesiredPos = EffectorTransform.GetTranslation();;
	FVector DesiredDelta = DesiredPos - RootPos;
	float DesiredLength = DesiredDelta.Size();

	// Check to handle case where DesiredPos is the same as RootPos.
	FVector	DesiredDir;
	if (DesiredLength < (float)KINDA_SMALL_NUMBER)
	{
		DesiredLength = (float)KINDA_SMALL_NUMBER;
		DesiredDir = FVector(1,0,0);
	}
	else
	{
		DesiredDir = DesiredDelta / DesiredLength;
	}

	// Get joint target (used for defining plane that joint should be in).
	FTransform JointTargetTransform(JointTargetLocation);
	const int32 JointTargetSpaceBoneIndexInt = (JointTargetLocationSpace == BCS_ParentBoneSpace || JointTargetLocationSpace == BCS_BoneSpace) ? BoneContainer.GetPoseBoneIndexForBoneName(JointTargetSpaceBoneName) : INDEX_NONE;
	const FCompactPoseBoneIndex JointTargetSpaceBoneIndex = BoneContainer.MakeCompactPoseIndex(FMeshPoseBoneIndex(JointTargetSpaceBoneIndexInt));
	FAnimationRuntime::ConvertBoneSpaceTransformToCS(SkelComp, MeshBases, JointTargetTransform, JointTargetSpaceBoneIndex, JointTargetLocationSpace);

	FVector	JointTargetPos = JointTargetTransform.GetTranslation();
	FVector JointTargetDelta = JointTargetPos - RootPos;

	// Same check as above, to cover case when JointTarget position is the same as RootPos.
	FVector JointPlaneNormal, JointBendDir;
	if (JointTargetDelta.SizeSquared() < FMath::Square((float)KINDA_SMALL_NUMBER))
	{
		JointBendDir = FVector(0,1,0);
		JointPlaneNormal = FVector(0,0,1);
	}
	else
	{
		JointPlaneNormal = DesiredDir ^ JointTargetDelta;

		// If we are trying to point the limb in the same direction that we are supposed to displace the joint in, 
		// we have to just pick 2 random vector perp to DesiredDir and each other.
		if (JointPlaneNormal.SizeSquared() < FMath::Square((float)KINDA_SMALL_NUMBER))
		{
			DesiredDir.FindBestAxisVectors(JointPlaneNormal, JointBendDir);
		}
		else
		{
			JointPlaneNormal.Normalize();

			// Find the final member of the reference frame by removing any component of JointTargetDelta along DesiredDir.
			// This should never leave a zero vector, because we've checked DesiredDir and JointTargetDelta are not parallel.
			JointBendDir = JointTargetDelta - ((JointTargetDelta | DesiredDir) * DesiredDir);
			JointBendDir.Normalize();
		}
	}

	// Find lengths of upper and lower limb in the ref skeleton.
	// Use actual sizes instead of ref skeleton, so we take into account translation and scaling from other bone controllers.
	float LowerLimbLength = EndBoneLocalTransform.GetTranslation().Size();
	float UpperLimbLength = LowerLimbLocalTransform.GetTranslation().Size();
	float MaxLimbLength	= LowerLimbLength + UpperLimbLength;

	if (bAllowStretching)
	{
		const float ScaleRange = StretchLimits.Y - StretchLimits.X;
		if( ScaleRange > KINDA_SMALL_NUMBER && MaxLimbLength > KINDA_SMALL_NUMBER )
		{
			const float ReachRatio = DesiredLength / MaxLimbLength;
			const float ScalingFactor = (StretchLimits.Y - 1.f) * FMath::Clamp<float>((ReachRatio - StretchLimits.X) / ScaleRange, 0.f, 1.f);
			if (ScalingFactor > KINDA_SMALL_NUMBER)
			{
				const float AdjustedFactor = 1.0f + ScalingFactor;

				LowerLimbLength *= AdjustedFactor;
				UpperLimbLength *= AdjustedFactor;
				MaxLimbLength *= AdjustedFactor;
			}
		}
	}

	FVector OutEndPos = DesiredPos;
	FVector OutJointPos = InitialJointPos;

	// If we are trying to reach a goal beyond the length of the limb, clamp it to something solvable and extend limb fully.
	if (DesiredLength > MaxLimbLength)
	{
		OutEndPos = RootPos + (MaxLimbLength * DesiredDir);
		OutJointPos = RootPos + (UpperLimbLength * DesiredDir);
	}
	else
	{
		// So we have a triangle we know the side lengths of. We can work out the angle between DesiredDir and the direction of the upper limb
		// using the sin rule:
		const float TwoAB = 2.f * UpperLimbLength * DesiredLength;

		const float CosAngle = (TwoAB != 0.f) ? ((UpperLimbLength*UpperLimbLength) + (DesiredLength*DesiredLength) - (LowerLimbLength*LowerLimbLength)) / TwoAB : 0.f;

		// If CosAngle is less than 0, the upper arm actually points the opposite way to DesiredDir, so we handle that.
		const bool bReverseUpperBone = (CosAngle < 0.f);

		// If CosAngle is greater than 1.f, the triangle could not be made - we cannot reach the target.
		// We just have the two limbs double back on themselves, and EndPos will not equal the desired EffectorLocation.
		if ((CosAngle > 1.f) || (CosAngle < -1.f))
		{
			// Because we want the effector to be a positive distance down DesiredDir, we go back by the smaller section.
			if (UpperLimbLength > LowerLimbLength)
			{
				OutJointPos = RootPos + (UpperLimbLength * DesiredDir);
				OutEndPos = OutJointPos - (LowerLimbLength * DesiredDir);
			}
			else
			{
				OutJointPos = RootPos - (UpperLimbLength * DesiredDir);
				OutEndPos = OutJointPos + (LowerLimbLength * DesiredDir);
			}
		}
		else
		{
			// Angle between upper limb and DesiredDir
			const float Angle = FMath::Acos(CosAngle);

			// Now we calculate the distance of the joint from the root -> effector line.
			// This forms a right-angle triangle, with the upper limb as the hypotenuse.
			const float JointLineDist = UpperLimbLength * FMath::Sin(Angle);

			// And the final side of that triangle - distance along DesiredDir of perpendicular.
			// ProjJointDistSqr can't be neg, because JointLineDist must be <= UpperLimbLength because appSin(Angle) is <= 1.
			const float ProjJointDistSqr = (UpperLimbLength*UpperLimbLength) - (JointLineDist*JointLineDist);
			// although this shouldn't be ever negative, sometimes Xbox release produces -0.f, causing ProjJointDist to be NaN
			// so now I branch it. 						
			float ProjJointDist = (ProjJointDistSqr>0.f)? FMath::Sqrt(ProjJointDistSqr) : 0.f;
			if( bReverseUpperBone )
			{
				ProjJointDist *= -1.f;
			}

			// So now we can work out where to put the joint!
			OutJointPos = RootPos + (ProjJointDist * DesiredDir) + (JointLineDist * JointBendDir);
		}
	}

	// Update transform for upper bone.
	{
		// Get difference in direction for old and new joint orientations
		FVector const OldDir = (InitialJointPos - RootPos).GetSafeNormal();
		FVector const NewDir = (OutJointPos - RootPos).GetSafeNormal();
		// That was done in Component space, so turn that into local space.
		// Find Delta Rotation take takes us from Old to New dir
		FQuat const DeltaRotation = FQuat::FindBetweenNormals(OldDir, NewDir);
		// Rotate our Joint quaternion by this delta rotation
		UpperLimbCSTransform.SetRotation( DeltaRotation * UpperLimbCSTransform.GetRotation() );
		// And put joint where it should be.
		UpperLimbCSTransform.SetTranslation( RootPos );

		// Order important. First bone is upper limb.
		OutBoneTransforms.Add( FBoneTransform(UpperLimbIndex, UpperLimbCSTransform) );
	}

	// Update transform for lower bone.
	{
		// Get difference in direction for old and new joint orientations
		FVector const OldDir = (InitialEndPos - InitialJointPos).GetSafeNormal();
		FVector const NewDir = (OutEndPos - OutJointPos).GetSafeNormal();
		// That was done in Component space, so turn that into local space.
		FVector const OldDirLocal = LowerLimbCSTransform.InverseTransformVectorNoScale(OldDir);
		FVector const NewDirLocal = LowerLimbCSTransform.InverseTransformVectorNoScale(NewDir);
		// Find Delta Rotation take takes us from Old to New dir
		FQuat const DeltaRotation = FQuat::FindBetweenNormals(OldDir, NewDir);
		// Rotate our Joint quaternion by this delta rotation
		LowerLimbCSTransform.SetRotation( DeltaRotation * LowerLimbCSTransform.GetRotation() );
		// And put joint where it should be.
		LowerLimbCSTransform.SetTranslation( OutJointPos );

		// Order important. Second bone is lower limb.
		OutBoneTransforms.Add( FBoneTransform(LowerLimbIndex, LowerLimbCSTransform) );
	}

	// Update transform for end bone.
	{
		// Set correct location for end bone.
		EndBoneCSTransform.SetTranslation(OutEndPos);

		// Order important. Third bone is End Bone.
		OutBoneTransforms.Add(FBoneTransform(IKBoneIndex, EndBoneCSTransform));
	}

	// Make sure we have correct number of bones
	check(OutBoneTransforms.Num() == 3);
}

bool FAnimNode_FootPlacementIK::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	// if both bones are valid
	return (IKBone.IsValid(RequiredBones));
}

void FAnimNode_FootPlacementIK::InitializeBoneReferences(const FBoneContainer& RequiredBones) 
{
	if (!IKBone.Initialize(RequiredBones))
	{
		UE_LOG(LogAnimation, Warning, TEXT("FAnimNode_FootPlacementIK::InitializeBoneReferences BoneIndex for Bone (%s) is not found in the Asset (%s)"), 
			*IKBone.BoneName.ToString(), *GetNameSafe(RequiredBones.GetAsset()));
	}
}
