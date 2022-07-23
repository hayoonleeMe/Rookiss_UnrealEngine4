// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "MyAnimInstance.h"
#include "DrawDebugHelpers.h"
#include "MyWeapon.h"
#include "MyStatComponent.h"
#include "Components/WidgetComponent.h"
#include "MyCharacterWidget.h"
#include "MyAIController.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SPRINGARM"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CAMERA"));

	// Character 클래스의 RootComponent는 캡슐 컴포넌트이므로 SpringArm을 캡슐 컴포넌트에 붙인다.
	SpringArm->SetupAttachment(GetCapsuleComponent());
	Camera->SetupAttachment(SpringArm);

	// 스프링암의 길이, 회전 조정
	SpringArm->TargetArmLength = 500.f;
	SpringArm->SetRelativeRotation(FRotator(-35.f, 0.f, 0.f));

	// 캐릭터의 초기 위치, 회전 조정
	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.f, 0.f, -88.f), FRotator(0.f, -90.f, 0.f));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SM(TEXT("SkeletalMesh'/Game/ParagonGreystone/Characters/Heroes/Greystone/Meshes/Greystone.Greystone'"));
	if (SM.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SM.Object);
	}

	Stat = CreateDefaultSubobject<UMyStatComponent>(TEXT("STAT"));

	// Hp바 설정
	HpBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBAR"));
	HpBar->SetupAttachment(GetMesh());	
	HpBar->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	// Screen은 절대 잘리지 않게 화면상에 렌더링된다.
	HpBar->SetWidgetSpace(EWidgetSpace::Screen);
	
	static ConstructorHelpers::FClassFinder<UUserWidget> UW(TEXT("WidgetBlueprint'/Game/UI/WBP_HpBar.WBP_HpBar_C'"));
	if (UW.Succeeded())
	{
		HpBar->SetWidgetClass(UW.Class);
		HpBar->SetDrawSize(FVector2D(200.f, 50.f));
	}

	// APawn 클래스에 기본으로 정의되어 있는 AIControllerClass를 설정한다.
	// MyCharacter가 AI로 조종될 때 어떤 컨트롤러에 의해 조종되는지를 설정한다.
	AIControllerClass = AMyAIController::StaticClass();
	// 어떤 상황에서 빙의되어 AI로 조종될지를 정한다.
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	//FName WeaponSocket(TEXT("hand_l_socket"));

	// 게임 실행시 MyWeapon을 화면 정중앙에 생성한다.
	// 기본으로 맵에 배치된 캐릭터 1개와 게임 시작시 생성되는 캐릭터 1개이므로 맵에는 총 2개가 생성될 것
	/*auto CurrentWeapon = GetWorld()->SpawnActor<AMyWeapon>(FVector::ZeroVector, FRotator::ZeroRotator);

	if (CurrentWeapon)
	{
		CurrentWeapon->AttachToComponent(GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			WeaponSocket);
	}*/
}

void AMyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AnimInstance = Cast<UMyAnimInstance>(GetMesh()->GetAnimInstance());
	if (AnimInstance)
	{
		// OnMontageEnded 델레게이트는 Dynamic Multicast Delegate이므로 AddDynamic 매크로를 통해 바인딩한다.
		AnimInstance->OnMontageEnded.AddDynamic(this, &AMyCharacter::OnAttackMontageEnded);
		AnimInstance->OnAttackHit.AddUObject(this, &AMyCharacter::AttackCheck);
	}

	// 컴포넌트 초기화 전에 InitWidget을 수행해 위젯이 반드시 초기화되도록 보장해준다.
	// 버전에 따라서 이 메소드를 수행하지 않으면 작동안되는 경우도 있으므로 필수
	HpBar->InitWidget();

	auto HpWidget = Cast<UMyCharacterWidget>(HpBar->GetUserWidgetObject());
	if (HpWidget)
		HpWidget->BindHp(Stat);
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &AMyCharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Attack"), EInputEvent::IE_Pressed, this, &AMyCharacter::Attack);
	PlayerInputComponent->BindAxis(TEXT("UpDown"), this, &AMyCharacter::UpDown);
	PlayerInputComponent->BindAxis(TEXT("LeftRight"), this, &AMyCharacter::LeftRight);
	PlayerInputComponent->BindAxis(TEXT("Yaw"), this, &AMyCharacter::Yaw);
}

void AMyCharacter::Attack()
{
	if (IsAttacking)
		return;

	AnimInstance->PlayAttackMontage();

	AnimInstance->JumpToSection(AttackIndex);

	AttackIndex = (AttackIndex + 1) % 3;

	IsAttacking = true;
}

void AMyCharacter::AttackCheck()
{
	// 충돌을 판정하는 과정
	FHitResult HitResult;
	FCollisionQueryParams Params(NAME_None, false, this);

	float AttackRange = 100.f;
	float AttackRadius = 50.f;

	bool bResult = GetWorld()->SweepSingleByChannel(
		OUT HitResult,
		GetActorLocation(),
		GetActorLocation() + GetActorForwardVector() * AttackRange,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(AttackRadius),
		Params);

	FVector Vec = GetActorForwardVector() * AttackRange;
	FVector Center = GetActorLocation() + Vec * 0.5f;
	float HalfHeight = AttackRange * 0.5f + AttackRadius;
	FQuat Rotation = FRotationMatrix::MakeFromZ(Vec).ToQuat();
	FColor DrawColor;
	if (bResult)
		DrawColor = FColor::Green;
	else
		DrawColor = FColor::Red;

	DrawDebugCapsule(GetWorld(), Center, HalfHeight, AttackRadius, Rotation, DrawColor, false, 2.f);

	// UE5에서 FHitResult.Actor가 FHitResult.GetActor()로 대체되었다.
	// 이제 더이상 IsValid()로 체크하지 않아도 된다.
	if (bResult && HitResult.GetActor())
	{
		UE_LOG(LogTemp, Log, TEXT("Hit Actor : %s"), *HitResult.GetActor()->GetName());

		FDamageEvent DamageEvent;
		HitResult.GetActor()->TakeDamage(Stat->GetAttack(), DamageEvent, GetController(), this);
	}
}

void AMyCharacter::UpDown(float Value)
{
	UpDownValue = Value;
	AddMovementInput(GetActorForwardVector(), Value);
}

void AMyCharacter::LeftRight(float Value)
{
	LeftRightValue = Value;
	AddMovementInput(GetActorRightVector(), Value);
}

void AMyCharacter::Yaw(float Value)
{
	AddControllerYawInput(Value);
}

void AMyCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	IsAttacking = false;
}

// 피격당한 쪽의 TakeDamage가 호출되므로 여기서 Stat은 피격당한 쪽의 Stat이다.
float AMyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Stat->OnAttacked(DamageAmount);

	return DamageAmount;
}