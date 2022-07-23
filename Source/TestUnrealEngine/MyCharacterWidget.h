// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyCharacterWidget.generated.h"

/**
 * 
 */
UCLASS()
class TESTUNREALENGINE_API UMyCharacterWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void BindHp(class UMyStatComponent* StatComp);

	void UpdateHp();
	
private:
	TWeakObjectPtr<class UMyStatComponent> CurrentStatComp;

	// 에디터에서 만든 프로그레스 바 위젯과 PB_HpBar 변수가 바로 연동되도록 한다.
	// 이를 위해선 프로그레스 바 위젯과 변수의 이름이 동일해야 한다.
	UPROPERTY(meta=(BindWidget))
	class UProgressBar* PB_HpBar;

	//class UProgressBar* PB_HpBar123;
};
