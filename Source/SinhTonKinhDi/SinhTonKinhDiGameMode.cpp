// Copyright Epic Games, Inc. All Rights Reserved.

#include "SinhTonKinhDiGameMode.h"
#include "SinhTonKinhDiCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASinhTonKinhDiGameMode::ASinhTonKinhDiGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
