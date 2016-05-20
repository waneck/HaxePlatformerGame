// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "PlatformerGame.h"
#include "PlatformerClimbMarker.h"

APlatformerClimbMarker::APlatformerClimbMarker(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ClimbMarkerOb(TEXT("/Game/Environment/meshes/ClimbMarker"));

	USceneComponent* SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	RootComponent = SceneComp;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ClimbMesh"));
	Mesh->StaticMesh = ClimbMarkerOb.Object;
	Mesh->RelativeScale3D = FVector(0.25f, 1.0f, 0.25f);
	Mesh->AttachParent = SceneComp;
}
