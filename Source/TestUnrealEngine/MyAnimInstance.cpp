// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAnimInstance.h"

// 한 프레임의 호출순서상 엔진이 입력시스템을 먼저 호출한 다음에 콘텐츠 로직을 다 실행하고, 그 다음에 최종적으로 AnimInstance의 이런 업데이트 함수를 호출한다.
void UMyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	auto pawn = TryGetPawnOwner();	
	if (IsValid(pawn))
	{
		Speed = pawn->GetVelocity().Size();
	}
}
