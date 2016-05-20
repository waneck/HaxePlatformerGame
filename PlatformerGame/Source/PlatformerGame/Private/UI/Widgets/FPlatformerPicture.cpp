// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "PlatformerGame.h"
#include "FPlatformerPicture.h"

FPlatformerPicture::FPlatformerPicture(UWorld* World)
{
	OwnerWorld = World;
	TintColor = FLinearColor::White;
	TintColor.A = 0;
	bIsHiding = false;
}

void FPlatformerPicture::Tick(UCanvas* Canvas)
{
	if (bIsVisible)
	{
		float AnimPercentage = 0.0f;

		if (bIsHiding)
		{
			AnimPercentage = 1.0f - FMath::Min(1.0f, (OwnerWorld->GetTimeSeconds() - AnimStartedTime) / AnimTime);
			if (AnimPercentage == 0.0f)
			{
				bIsVisible = false;
			}
		}
		else
		{
			AnimPercentage = FMath::Min(1.0f, (OwnerWorld->GetTimeSeconds() - AnimStartedTime) / AnimTime);
		}
		TintColor.A = AnimPercentage;
		Canvas->SetDrawColor(TintColor.ToFColor(true));
		float width = Image->GetSurfaceWidth();
		float height = Image->GetSurfaceHeight();
		float imageAspectRatio = width / height;
		float viewAspectRatio = Canvas->ClipX / Canvas->ClipY;
		if (imageAspectRatio >= viewAspectRatio)
		{
			width = Canvas->ClipX * ScreenCoverage;
			height = bKeepAspectRatio ? (width / imageAspectRatio) : Canvas->ClipY * ScreenCoverage;
		}
		else
		{
			height = Canvas->ClipY * ScreenCoverage;
			width = bKeepAspectRatio ? (height * imageAspectRatio) : Canvas->ClipX * ScreenCoverage;
		}
		FCanvasTileItem TileItem(FVector2D((Canvas->ClipX - width) / 2.0f , (Canvas->ClipY - height) / 2.0f), Image->Resource, FVector2D(width, height), TintColor);
		Canvas->DrawItem( TileItem );
	}
}

void FPlatformerPicture::Show(UTexture2D* InTexture, float InFadeInTime, float InScreenCoverage, bool bInKeepAspectRatio)
{
	if (InTexture != NULL)
	{
		Image = InTexture;
		AnimStartedTime = OwnerWorld->GetTimeSeconds();
		bIsHiding = false;
		AnimTime = InFadeInTime;
		bIsVisible = true;
		ScreenCoverage = InScreenCoverage / 2.0f;
		bKeepAspectRatio = bInKeepAspectRatio;
	}
}

void FPlatformerPicture::Hide(float FadeOutTime)
{
	AnimTime = FadeOutTime;
	AnimStartedTime = OwnerWorld->GetTimeSeconds();
	bIsHiding = true;
}

bool FPlatformerPicture::IsVisible() const
{
	return bIsVisible;
}