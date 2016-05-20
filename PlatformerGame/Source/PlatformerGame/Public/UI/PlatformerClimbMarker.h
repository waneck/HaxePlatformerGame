// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PlatformerClimbMarker.generated.h"

UCLASS(Blueprintable)
class APlatformerClimbMarker : public AActor
{
	GENERATED_UCLASS_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category=Mesh, meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* Mesh;

public:
	/** Returns Mesh subobject **/
	FORCEINLINE UStaticMeshComponent* GetMesh() const { return Mesh; }
};
