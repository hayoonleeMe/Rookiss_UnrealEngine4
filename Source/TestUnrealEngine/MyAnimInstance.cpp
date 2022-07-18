// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAnimInstance.h"

// �� �������� ȣ������� ������ �Է½ý����� ���� ȣ���� ������ ������ ������ �� �����ϰ�, �� ������ ���������� AnimInstance�� �̷� ������Ʈ �Լ��� ȣ���Ѵ�.
void UMyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	auto pawn = TryGetPawnOwner();	
	if (IsValid(pawn))
	{
		Speed = pawn->GetVelocity().Size();
	}
}
