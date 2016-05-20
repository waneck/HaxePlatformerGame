// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "PlatformerGameLoadingScreen.h"

#include "SlateBasics.h"
#include "SlateExtras.h"
#include "MoviePlayer.h"
#include "SThrobber.h"

// This module must be loaded "PreLoadingScreen" in the .uproject file, otherwise it will not hook in time!

struct FPlatformerGameLoadingScreenBrush : public FSlateDynamicImageBrush, public FGCObject
{
	FPlatformerGameLoadingScreenBrush( const FName InTextureName, const FVector2D& InImageSize )
		: FSlateDynamicImageBrush( InTextureName, InImageSize )
	{
		ResourceObject = LoadObject<UObject>( NULL, *InTextureName.ToString() );
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector)
	{
		if( ResourceObject )
		{
			Collector.AddReferencedObject(ResourceObject);
		}
	}
};

class SPlatformerLoadingScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPlatformerLoadingScreen) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		static const FName LoadingScreenName(TEXT("/Game/UI/Menu/LoadingScreen.LoadingScreen"));

		//since we are not using game styles here, just load one image
		LoadingScreenBrush = MakeShareable( new FPlatformerGameLoadingScreenBrush( LoadingScreenName, FVector2D(1920,1080) ) );

		ChildSlot
		[
			SNew(SOverlay)
			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SImage)
				.Image(LoadingScreenBrush.Get())
			]
			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.VAlign(VAlign_Bottom)
				.HAlign(HAlign_Right)
				.Padding(FMargin(10.0f))
				[
					SNew(SThrobber)
					.Visibility(this, &SPlatformerLoadingScreen::GetLoadIndicatorVisibility)
				]
			]
		];
	}

private:
	EVisibility GetLoadIndicatorVisibility() const
	{
		return GetMoviePlayer()->IsLoadingFinished() ? EVisibility::Collapsed : EVisibility::Visible;
	}
	
	/** loading screen image brush */
	TSharedPtr<FSlateDynamicImageBrush> LoadingScreenBrush;
};

class FPlatformerGameLoadingScreenModule : public IPlatformerGameLoadingScreenModule
{
public:
	virtual void StartupModule() override
	{		
		// Load for cooker reference
		LoadObject<UObject>(NULL, TEXT("/Game/UI/Menu/LoadingScreen.LoadingScreen") ); 

		if (IsMoviePlayerEnabled())
		{
			CreateLoadingScreen();
		}
	}


	virtual bool IsGameModule() const override
	{
		return true;
	}

	virtual void StartInGameLoadingScreen() override
	{
		CreateLoadingScreen();
	}

	void CreateLoadingScreen()
	{
		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
		LoadingScreen.WidgetLoadingScreen = SNew(SPlatformerLoadingScreen);
		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	}
};

IMPLEMENT_GAME_MODULE(FPlatformerGameLoadingScreenModule, PlatformerGameLoadingScreen);
