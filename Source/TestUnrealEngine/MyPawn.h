// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyPawn.generated.h"

UCLASS()
class TESTUNREALENGINE_API AMyPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMyPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void UpDown(float Value);
	void LeftRight(float Value);

private:
	// MyPawn을 나타낼 메쉬
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	// 가장 간단하고 기본적인 이동을 제공하는 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class UFloatingPawnMovement* Movement;
};
