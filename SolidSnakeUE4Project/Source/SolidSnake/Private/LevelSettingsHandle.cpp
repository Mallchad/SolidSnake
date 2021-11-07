// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelSettingsHandle.h"


// // Sets default values
ALevelSettingsHandle::ALevelSettingsHandle()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALevelSettingsHandle::BeginPlay()
{
    Super::BeginPlay();

}

// Called every frame
void ALevelSettingsHandle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
