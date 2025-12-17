// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Controller/PlayerController_WASD.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "EngineUtils.h"
#include "Character/MoveManager.h"
#include "Engine/Engine.h"
#include "Character/Squirrel.h"
#include "Camera/CameraComponent.h"


void APlayerController_WASD::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("PlayerController_WASD"));

    if (ULocalPlayer* LP = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
        {
            Subsystem->ClearAllMappings();
            Subsystem->AddMappingContext(ClientIMC, 0);
        }
    }


    // 0.1초마다 다람쥐와 카메라 체크
    GetWorldTimerManager().SetTimer(TimerHandle_CheckSquirrel, this, &APlayerController_WASD::CheckSquirrelAndCamera, 0.1f, true);

}
// --- 타이머 1: 다람쥐와 카메라 확인 ---
void APlayerController_WASD::CheckSquirrelAndCamera()
{
    if (bSquirrelReady) return; // 이미 준비 완료면 체크 종료

    for (TActorIterator<ASquirrel> It(GetWorld()); It; ++It)
    {
        TargetSquirrel = *It;
        break;
    }

    if (!TargetSquirrel) return;

    TargetCamera = TargetSquirrel->FindComponentByClass<UCameraComponent>();
    if (!TargetCamera) return;

    // 준비 완료
    bSquirrelReady = true;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
            FString::Printf(TEXT("[Debug] Squirrel and Camera Ready: %s"), *TargetSquirrel->GetName()));
    }

    // 다람쥐와 카메라 확인 후, 0.1초마다 SetViewTarget 호출
    //GetWorldTimerManager().SetTimer(TimerHandle_UpdateViewTarget, this, &APlayerController_WASD::UpdateViewTargetToSquirrel, 0.1f, true);
    UpdateViewTargetToSquirrel();
    // 체크 타이머 종료
    GetWorldTimerManager().ClearTimer(TimerHandle_CheckSquirrel);
}

// --- 타이머 2: 다람쥐 시점 업데이트 ---
void APlayerController_WASD::UpdateViewTargetToSquirrel()
{
    if (!bSquirrelReady || !TargetSquirrel) return;
    if (!IsLocalController()) return;

    FViewTargetTransitionParams Params;
    Params.BlendTime = 0.f;
    SetViewTarget(TargetSquirrel, Params);
}


void APlayerController_WASD::SetupInputComponent()
{
    Super::SetupInputComponent();

    UE_LOG(LogTemp, Warning, TEXT("SetupInputComponent CALLED"));

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
    {
        UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent FOUND"));
        EIC->BindAction(IA_MoveForward,
            ETriggerEvent::Triggered,
            this,
            &APlayerController_WASD::OnMoveForwardTriggered);

        UE_LOG(LogTemp, Warning, TEXT("BindAction DONE"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent NOT FOUND"));
    }
}

void APlayerController_WASD::OnMoveForwardTriggered(const FInputActionValue& Value)
{
    // Axis2D 값을 가져온다
    FVector2D MoveInput = Value.Get<FVector2D>();

    // 서버에 전송
    Server_SendWInput(MoveInput);
}




void APlayerController_WASD::Server_SendWInput_Implementation(FVector2D MoveInput)
{
    for (TActorIterator<AMoveManager> It(GetWorld()); It; ++It)
    {
        (*It)->OnClientWInput(this, MoveInput);
        break;
    }
}