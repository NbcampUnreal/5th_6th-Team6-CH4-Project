//KYGTestCharacter.cpp

#include "KYG/KYGTestCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "KYG/ALCGunBase.h"
#include "GameFramework/CharacterMovementComponent.h"


AKYGTestCharacter::AKYGTestCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

    //캐릭터 회전 기본 값
    bUseControllerRotationYaw = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AKYGTestCharacter::BeginPlay()
{
	Super::BeginPlay();
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(IMC_Default, 0);
        }
    }
}

void AKYGTestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

    // WASD 이동
    Input->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AKYGTestCharacter::Move);

    // 마우스 Look
    Input->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AKYGTestCharacter::Look);

    //발사 
    /*PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AKYGTestCharacter::InputFire);*/
    Input->BindAction(
        IA_Fire,
        ETriggerEvent::Started, //  Pressed일 때 딱 1번
        this,
        &AKYGTestCharacter::InputFire
    );
}

//void AKYGTestCharacter::Heal(float Amout)
//{
//}
//
//void AKYGTestCharacter::RestoreStamina(float Amout)
//{
//}





void AKYGTestCharacter::Move(const FInputActionValue& Value)
{
    if (!Controller)
    {return;}

    const FVector2D MoveVector = Value.Get<FVector2D>();

    const FRotator ControlRot = Controller->GetControlRotation();
    const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

    const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
    const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

    AddMovementInput(Forward, MoveVector.Y);
    AddMovementInput(Right, MoveVector.X);
}

void AKYGTestCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxis = Value.Get<FVector2D>();
    if (!Controller)
    {return;}

    AddControllerYawInput(LookAxis.X);
    AddControllerPitchInput(LookAxis.Y);
}

//발사
void AKYGTestCharacter::InputFire()
{
    if (!EquippedGun)
    {
        UE_LOG(LogTemp, Warning, TEXT("[InputFire] No EquippedGun"));
        return;
    }
   /* EquippedGun->Fire();*/ // 내부에서 서버 RPC 처리됨
}



void AKYGTestCharacter::ServerEquipGun_Implementation(AALCGunBase* NewGun)
{
    if (!HasAuthority() || !NewGun) { return; }

      //기존 총이 있다면 제거
    if(EquippedGun)
    {
        EquippedGun->Destroy();
        EquippedGun = nullptr;
    }

    //새 총 장착
    EquippedGun = NewGun;

    //손 소켓 부착
    NewGun->AttachToComponent(
        GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        TEXT("Hand_R_Socket")
    );

    UE_LOG(LogTemp, Warning, TEXT("[Server] Weapon Switched -> %s"), *NewGun->GetName());
}

void AKYGTestCharacter::SetEquippedGun_Server(AALCGunBase* NewGun)
{
    if (!HasAuthority()) return;

    EquippedGun = NewGun;

    UE_LOG(LogTemp, Warning, TEXT("[SERVER] Gun Equipped: %s"),
        NewGun ? *NewGun->GetName() : TEXT("None"));
}

//네트워크
void AKYGTestCharacter::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AKYGTestCharacter, EquippedGun);
}

void AKYGTestCharacter::OnRep_EquippedGun()
{
    if (!EquippedGun) { return; }

    EquippedGun->AttachToComponent(
        GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        TEXT("Hand_R_Socket")
    );
}

void AKYGTestCharacter::ReceiveHeal_Implementation(float HealAmount)
{   //서버만
    if (!HasAuthority()) { return; }

    float OldHP = CurrentHP;
    CurrentHP = FMath::Clamp(CurrentHP + HealAmount, 0.f, MaxHP);

    UE_LOG(LogTemp, Warning, TEXT("[Heal] %s : %.1f -> %.1f"), *GetName(), OldHP, CurrentHP);
}