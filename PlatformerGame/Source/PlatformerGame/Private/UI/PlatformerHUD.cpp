// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "PlatformerGame.h"
#include "PlatformerHUD.h"
#include "Widgets/FPlatformerPicture.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "PlatformerBlueprintLibrary.h"
#include "PlatformerGameMode.h"

#define LOCTEXT_NAMESPACE "PlatformerGame.HUD"

APlatformerHUD::APlatformerHUD(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UFont> HUDFontOb(TEXT("/Game/UI/HUD/RobotoLight48"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> BorderTextureOb(TEXT("/Game/UI/HUD/Frame/Border"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> BorderBackgroundTextureOb(TEXT("/Game/UI/HUD/Frame/Background"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> LeftBorderTextureOb(TEXT("/Game/UI/HUD/Frame/BorderLeft"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> RightBorderTextureOb(TEXT("/Game/UI/HUD/Frame/BorderRight"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> TopBorderTextureOb(TEXT("/Game/UI/HUD/Frame/BorderTop"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> BottomBorderTextureOb(TEXT("/Game/UI/HUD/Frame/BorderBottom"));

	static ConstructorHelpers::FObjectFinder<UTexture2D> BorderRedTextureOb(TEXT("/Game/UI/HUD/Frame/BorderRed"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> BorderBackgroundRedTextureOb(TEXT("/Game/UI/HUD/Frame/BackgroundRed"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> LeftBorderRedTextureOb(TEXT("/Game/UI/HUD/Frame/BorderLeftRed"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> RightBorderRedTextureOb(TEXT("/Game/UI/HUD/Frame/BorderRightRed"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> TopBorderRedTextureOb(TEXT("/Game/UI/HUD/Frame/BorderTopRed"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> BottomBorderRedTextureOb(TEXT("/Game/UI/HUD/Frame/BorderBottomRed"));

	static ConstructorHelpers::FObjectFinder<UTexture2D> UpButtonTextureOb(TEXT("/Game/UI/HUD/UpButton"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> DownButtonTextureOb(TEXT("/Game/UI/HUD/DownButton"));

	UpButtonTexture = UpButtonTextureOb.Object;
	DownButtonTexture = DownButtonTextureOb.Object;

	HUDFont = HUDFontOb.Object;
	
	BlueBorder.Background = BorderBackgroundTextureOb.Object;
	BlueBorder.Border = BorderTextureOb.Object;
	BlueBorder.BottomBorder = BottomBorderTextureOb.Object;
	BlueBorder.TopBorder = TopBorderTextureOb.Object;
	BlueBorder.LeftBorder = LeftBorderTextureOb.Object;
	BlueBorder.RightBorder =  RightBorderTextureOb.Object;

	RedBorder.Background = BorderBackgroundRedTextureOb.Object;
	RedBorder.Border = BorderRedTextureOb.Object;
	RedBorder.BottomBorder = BottomBorderRedTextureOb.Object;
	RedBorder.TopBorder = TopBorderRedTextureOb.Object;
	RedBorder.LeftBorder = LeftBorderRedTextureOb.Object;
	RedBorder.RightBorder =  RightBorderRedTextureOb.Object;

	HighScoreName.Init('A',3);
	CurrentLetter = 0;
	bEnterNamePromptActive = false;
	bHighscoreActive = false;

	HighscoreNames.Init("TST",10);
	HighscoreTimes.Init(60.0f,10);
}

void APlatformerHUD::NotifyHitBoxClick(FName BoxName)
{
	Super::NotifyHitBoxClick(BoxName);

	if (BoxName.GetPlainNameString() == "Letter")
	{
		CurrentLetter = BoxName.GetNumber();
	}
	if (BoxName == "Up")
	{
		if (HighScoreName[CurrentLetter] < 'Z')
		{
			HighScoreName[CurrentLetter]++;
		}
	}
	if (BoxName == "Down")
	{
		if (HighScoreName[CurrentLetter] > 'A')
		{
			HighScoreName[CurrentLetter]--;
		}
	}
	if (BoxName == "OK")
	{
		bEnterNamePromptActive = false;
		if (PlayerOwner)
		{
			PlayerOwner->bShowMouseCursor = bEnterNamePromptActive;
		}

		FString EnteredName = FString();
		for (int32 i=0; i < HighScoreName.Num(); i++)
		{
			EnteredName.AppendChar(HighScoreName[i]);
		}

		OnHighscoreNameAccepted.Broadcast(EnteredName);
	}
}


void APlatformerHUD::ShowHighscore(TArray<float> Times, TArray<FString> Names)
{
	HighscoreNames = MoveTemp(Names);
	HighscoreTimes = MoveTemp(Times);
	bHighscoreActive = true;
}

void APlatformerHUD::ShowHighscorePrompt()
{
	bEnterNamePromptActive = true;
	if (PlayerOwner)
	{
		PlayerOwner->bShowMouseCursor = bEnterNamePromptActive;
	}
}

void APlatformerHUD::HideHighscore()
{
	bHighscoreActive = false;
}

void APlatformerHUD::DrawHighscoreEntryPrompt()
{
	const float SizeX = 90 * UIScale;
	const float SizeY = 90 * UIScale;
	const float DrawX = (Canvas->ClipX - SizeX*3) / 2.0f;
	const float DrawY = (Canvas->ClipY - SizeY) / 2.0f;

	const float UVL = 90.0f/128.0f;
	const float TextScale = 1.8f;

	FCanvasTileItem TileItem(FVector2D(DrawX+SizeX*CurrentLetter, DrawY), UpButtonTexture->Resource, 
		FVector2D( SizeX, SizeY ),
		FVector2D(0,0),FVector2D(UVL,UVL), FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	if (HighScoreName[CurrentLetter] < 'Z')
	{
		Canvas->DrawItem(TileItem);
		AddHitBox(TileItem.Position,TileItem.Size,"Up",true,255);
	}

	if (HighScoreName[CurrentLetter] > 'A')
	{
		TileItem.Position = FVector2D(DrawX+SizeX*CurrentLetter,DrawY+SizeY*2);
		TileItem.Texture = DownButtonTexture->Resource;

		AddHitBox(TileItem.Position,TileItem.Size,"Down",true,255);
		Canvas->DrawItem(TileItem);
	}

	FCanvasTextItem TextItem(FVector2D(DrawX, DrawY), FText(), HUDFont, FLinearColor::White);
	TextItem.Scale = FVector2D(TextScale * UIScale, TextScale * UIScale);
	TextItem.EnableShadow( FLinearColor::Transparent );


	float StrSizeX, StrSizeY;
	float Offset = 0;

	for (int32 i = 0; i < HighScoreName.Num(); i++)
	{
		TextItem.Text = FText::FromString(FString::Chr(HighScoreName[i]));
		Canvas->StrLen(HUDFont, TextItem.Text.ToString(), StrSizeX, StrSizeY);
		Canvas->StrLen(HUDFont, TextItem.Text.ToString(), StrSizeX, StrSizeY);
		Canvas->StrLen(HUDFont, TextItem.Text.ToString(), StrSizeX, StrSizeY);
		StrSizeX = StrSizeX * TextScale * UIScale;
		StrSizeY = StrSizeY * TextScale * UIScale;
		TextItem.Position = FVector2D(DrawX+Offset+(SizeX-StrSizeX)/2.0f, DrawY + SizeY);

		DrawBorder(DrawX + Offset, DrawY + SizeY,  SizeX, SizeY, 0.4f, CurrentLetter == i ? BlueBorder : RedBorder);
		AddHitBox(FVector2D(DrawX + Offset,DrawY + SizeY),TileItem.Size,FName(TEXT("Letter"),i),true,255);

		Canvas->DrawItem( TextItem );
		Offset += SizeX;
	}

	const float ButtonWidth = 200 * UIScale;
	TextItem.Text = LOCTEXT("OK", "OK");
	Canvas->StrLen(HUDFont, TextItem.Text.ToString(), StrSizeX, StrSizeY);
	StrSizeX = StrSizeX * TextScale * UIScale;
	StrSizeY = StrSizeY * TextScale * UIScale;
	TextItem.Position = FVector2D((Canvas->ClipX - StrSizeX) / 2.0f, DrawY + SizeY*4);

	DrawBorder((Canvas->ClipX - ButtonWidth) / 2.0f, DrawY + SizeY*4,  ButtonWidth, SizeY, 0.4f, BlueBorder);
	AddHitBox(FVector2D((Canvas->ClipX - ButtonWidth) / 2.0f,DrawY + SizeY*4),FVector2D(ButtonWidth,SizeY),FName("OK"),true,255);
	Canvas->DrawItem( TextItem );
}

/** draws high score */
void APlatformerHUD::DrawHighscore()
{
	const FText Highscore = LOCTEXT("Highscore", "High score");
	const float SizeX = Canvas->ClipX * 0.4f;
	const float SizeY = 1000 * UIScale;
	const float DrawX = (Canvas->ClipX - SizeX) / 2.0f;
	const float DrawY = (Canvas->ClipY - SizeY) / 2.0f;

	DrawBorder(DrawX, DrawY,  SizeX, SizeY, 1.0f, BlueBorder);

	const float TextScale = 1.4f;
	const float TextMargin = 0.03f;
	float StrSizeX, StrSizeY;
	Canvas->StrLen(HUDFont, Highscore.ToString(), StrSizeX, StrSizeY);
	StrSizeX = StrSizeX * TextScale * UIScale;
	StrSizeY = StrSizeY * TextScale * UIScale;

	FCanvasTextItem TextItem( FVector2D( (Canvas->ClipX - StrSizeX) / 2.0f, DrawY + SizeY * TextMargin ), 
		Highscore, HUDFont, FLinearColor::White );
	TextItem.Scale = FVector2D( TextScale * UIScale, TextScale * UIScale );
	TextItem.EnableShadow( FLinearColor::Transparent );
	Canvas->DrawItem( TextItem );

	const float BorderSize = BlueBorder.LeftBorder->Resource->GetSizeX() * UIScale;
	FCanvasTileItem TileItem(FVector2D(  DrawX + BorderSize, DrawY + SizeY * TextMargin + StrSizeY ), BlueBorder.TopBorder->Resource, 
		FVector2D( SizeX - 2* BorderSize, BlueBorder.TopBorder->Resource->GetSizeY() * UIScale ),
		FVector2D(0,0),FVector2D((SizeX - 2* BorderSize) / BlueBorder.TopBorder->Resource->GetSizeX() * UIScale,1), FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TileItem);


	const float StartY = DrawY + SizeY * TextMargin*3 + StrSizeY * TextScale * UIScale;

	const float ColWidths[] = {70*UIScale, 340*UIScale, 200*UIScale};
	
	const float TotalWidth = ColWidths[0]+ColWidths[1]+ColWidths[2];

	for (int32 i=0; i < 10; i++ )
	{
		FText Texts[] = { FText::Format(FText::FromString("{0}."), FText::AsNumber(i+1)),
			FText::FromString(UPlatformerBlueprintLibrary::DescribeTime(HighscoreTimes[i], false)),
			FText::FromString(HighscoreNames[i]) };
		float Offset = 0;
		for (uint8 column=0; column < 3; column++)
		{
			TextItem.Text = Texts[column]; 
			Canvas->StrLen(HUDFont, TextItem.Text.ToString(), StrSizeX, StrSizeY);
			StrSizeX = StrSizeX * TextScale * UIScale;
			StrSizeY = StrSizeY * TextScale * UIScale;
			TextItem.Position = FVector2D((Canvas->ClipX - TotalWidth) / 2.0f + Offset + ColWidths[column] - StrSizeX, StartY + i * StrSizeY);
			Canvas->DrawItem( TextItem );
			Offset += ColWidths[column];
		}

	}

}


void APlatformerHUD::DrawHUD()
{
	if ( GEngine && GEngine->GameViewport )
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		UIScale = ViewportSize.X / 2048.0f;
	}

	Super::DrawHUD();
	
	APlatformerGameMode* MyGame = GetWorld()->GetAuthGameMode<APlatformerGameMode>();	
	if (MyGame)
	{
		// active messages
		DrawActiveMessages();

		// round timer
		EGameState GameState = MyGame->GetGameState();
		if (GameState == EGameState::Playing)
		{
			DisplayRoundTimer();
			DisplayRoundTimeModification();
		}

		//When game is finished, draw the summary screen when picture is shown from Blueprints
		if (GameState == EGameState::Finished && MyGame->PlatformerPicture && MyGame->PlatformerPicture->IsVisible())
		{
			const float SizeX = Canvas->ClipX * 0.75f;
			const float SizeY = Canvas->ClipY * 0.7f;
			const float DrawX = (Canvas->ClipX - SizeX) / 2.0f;
			const float DrawY = (Canvas->ClipY - SizeY) / 2.0f;

			DrawBorder(DrawX, DrawY,  SizeX, SizeY, 1.0f, MyGame->IsRoundWon() ? BlueBorder : RedBorder);

			MyGame->PlatformerPicture->Tick(Canvas);

			if (EndingMessages.Num() > 0)
			{
				//First message with total time
				const float EndingMessagesScale = 1.6f;
				const float TextMargin = 0.03f;
				float StrSizeX, StrSizeY;
				Canvas->StrLen(HUDFont, EndingMessages[0].Message, StrSizeX, StrSizeY);
				StrSizeX = StrSizeX * EndingMessagesScale * UIScale;
				StrSizeY = StrSizeY * EndingMessagesScale * UIScale;
				
				FCanvasTextItem TextItem( FVector2D( (Canvas->ClipX - StrSizeX) / 2.0f, DrawY + SizeY * TextMargin ), 
					FText::FromString( EndingMessages[0].Message ), HUDFont, FLinearColor::White );
				TextItem.Scale = FVector2D( EndingMessagesScale * UIScale, EndingMessagesScale * UIScale );
				TextItem.EnableShadow( FLinearColor::Transparent );
				Canvas->DrawItem( TextItem );
				
				//2nd message will only be shown after making at least 2 runs
				//displays time difference + NEW RECORD! or TRY AGAIN
				if (EndingMessages.Num() > 1)
				{
					Canvas->StrLen(HUDFont, EndingMessages[1].Message, StrSizeX, StrSizeY);
					StrSizeX = StrSizeX * EndingMessagesScale * UIScale;
					StrSizeY = StrSizeY * EndingMessagesScale * UIScale;
					TextItem.Position = FVector2D( (Canvas->ClipX - StrSizeX) / 2.0f ,
						DrawY + SizeY * (1.0f-TextMargin) - StrSizeY );
					TextItem.Text =  FText::FromString( EndingMessages[1].Message );
					Canvas->DrawItem( TextItem );
				}
			}

		}

		// game state related messages, make it pulse 1 Hz
		if (GameState == EGameState::Waiting || MyGame->CanBeRestarted())
		{
			const int32 GameTime = FMath::TruncToInt(1.0f * GetWorld()->GetTimeSeconds());
			const bool bShowInputMessage = (GameTime % 2) == 0;
			if (bShowInputMessage)
			{
				FString InputMessage = TEXT("Jump or Slide");
				switch (GameState)
				{
				case EGameState::Waiting:	InputMessage += TEXT(" to start running"); break;
				case EGameState::Finished:	InputMessage += TEXT(" to play again"); break;
				}

				DrawMessage(InputMessage, 0.5f, 0.9f, 1.0f, FLinearColor::White);
			}
		}

		if (GameState == EGameState::Finished)
		{
			if (bEnterNamePromptActive)
			{
				DrawHighscoreEntryPrompt();
			}
			if (bHighscoreActive)
			{
				DrawHighscore();
			}
		}
	}
}

void APlatformerHUD::NotifyRoundTimeModified(float DeltaTime)
{
	RoundTimeModification = DeltaTime;
	RoundTimeModificationTime = GetWorld()->GetTimeSeconds();
}

void APlatformerHUD::DisplayRoundTimeModification()
{
	const float ModificationDisplayDuration = 0.5f;
	const float CurrTime = GetWorld()->GetTimeSeconds();
	if (RoundTimeModification != 0.0f &&
		CurrTime - RoundTimeModificationTime <= ModificationDisplayDuration)
	{
		FString DisplayText = UPlatformerBlueprintLibrary::DescribeTime(RoundTimeModification, true);

		const float Delta = FMath::Clamp((CurrTime - RoundTimeModificationTime) / ModificationDisplayDuration, 0.0f, 1.0f);
		const float PosY = 0.11f + Delta * 0.24f;

		DrawMessage(DisplayText, 0.5f, PosY, 1.0f, FLinearColor::White);
	}
}

void APlatformerHUD::DisplayRoundTimer()
{
	APlatformerGameMode* GI = GetWorld()->GetAuthGameMode<APlatformerGameMode>();	
	if (GI)
	{
		const float RoundDuration = GI->GetRoundDuration();
		const bool bIncludeSign = false;
		FString RoundDurationText = FString(TEXT("Time: ")) + UPlatformerBlueprintLibrary::DescribeTime(RoundDuration, bIncludeSign);

		DrawMessage(RoundDurationText, 0.5f, 0.1f, 1.0f, FLinearColor::White);
	}
}

void APlatformerHUD::DrawMessage(FString Message, float PosX, float PosY, float TextScale, FLinearColor TextColor, bool bRedBorder)
{
	if (Canvas)
	{
		float SizeX, SizeY;
		Canvas->StrLen(HUDFont, Message, SizeX, SizeY);

		const float DrawX = Canvas->ClipX * FMath::Clamp(PosX, 0.0f, 1.0f) - (SizeX * TextScale * 0.5f * UIScale);
		const float DrawY = Canvas->ClipY * FMath::Clamp(PosY, 0.0f, 1.0f) - (SizeY * TextScale * 0.5f * UIScale);
		const float BoxPadding = 8.0f * UIScale;

		DrawBorder(DrawX - BoxPadding, DrawY - BoxPadding,  (SizeX * TextScale * UIScale) + (BoxPadding * 2.0f), (SizeY * TextScale * UIScale) + (BoxPadding * 2.0f), 0.4f, bRedBorder ? RedBorder : BlueBorder);

		FCanvasTextItem TextItem( FVector2D( DrawX, DrawY ), FText::FromString( Message ), HUDFont, TextColor );
		TextItem.Scale = FVector2D( TextScale*UIScale, TextScale*UIScale );
		TextItem.EnableShadow( FLinearColor::Transparent );
		Canvas->DrawItem( TextItem );
	}
}

void APlatformerHUD::DrawBorder(float PosX, float PosY, float Width, float Height, float BorderScale, FBorderTextures& BorderTextures)
{
	FCanvasBorderItem BorderItem(FVector2D(PosX, PosY), BorderTextures.Border->Resource, BorderTextures.Background->Resource,
		BorderTextures.LeftBorder->Resource, BorderTextures.RightBorder->Resource, BorderTextures.TopBorder->Resource, BorderTextures.BottomBorder->Resource,
		FVector2D(Width, Height), FLinearColor::White);
	BorderItem.BlendMode = SE_BLEND_Translucent;
	BorderItem.CornerSize = FVector2D(85.0f/256.0f,95.0f/256.0f);
	BorderItem.BorderScale = FVector2D(BorderScale*UIScale,BorderScale*UIScale);
	BorderItem.BackgroundScale = FVector2D(1.0f*UIScale,1.0f*UIScale);
	Canvas->DrawItem(BorderItem);
}

void APlatformerHUD::AddMessage(FString Message, float DisplayDuration, float PosX, float PosY, float TextScale, bool bRedBorder)
{
	APlatformerGameMode* MyGame = GetWorld()->GetAuthGameMode<APlatformerGameMode>();	

	if (MyGame)
	{
		FPlatformerMessageData MsgData;

		MsgData.Message = Message;
		MsgData.DisplayDuration = DisplayDuration;
		MsgData.DisplayStartTime = GetWorld()->GetTimeSeconds();
		MsgData.PosX = PosX;
		MsgData.PosY = PosY;
		MsgData.TextScale = TextScale;
		MsgData.bRedBorder = bRedBorder;

		EGameState GameState = MyGame->GetGameState();

		if (GameState == EGameState::Finished)
		{
			EndingMessages.Add(MsgData);
		}
		else
		{
			if (EndingMessages.Num() > 0)
			{
				EndingMessages.Empty();
			}
			ActiveMessages.Add(MsgData);
		}
	}
	
}

void APlatformerHUD::DrawActiveMessages()
{
	const float CurrTime = GetWorld()->GetTimeSeconds();
	for (int32 i = ActiveMessages.Num() - 1; i >= 0; --i)
	{
		const FPlatformerMessageData& Message = ActiveMessages[i];
		const bool bIsActive = (CurrTime < Message.DisplayDuration + Message.DisplayStartTime);
		if (bIsActive)
		{
			DrawMessage(Message.Message, Message.PosX, Message.PosY, Message.TextScale, FLinearColor::White, Message.bRedBorder);
		}
		else
		{
			ActiveMessages.RemoveAt(i);
		}
	}
}

#undef LOCTEXT_NAMESPACE
