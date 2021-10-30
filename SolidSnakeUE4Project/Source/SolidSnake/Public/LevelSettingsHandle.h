// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <GameFramework/Actor.h>
#include "LevelSettingsHandle.generated.h"

class AActor;


UCLASS()
class ALevelSettingsHandle : public AActor
{
    GENERATED_BODY()

    public:
    // Sets default values for this actor's properties
    ALevelSettingsHandle();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay();

public:
    // Called every frame
    virtual void Tick(float DeltaTime);



};
