#include "AI/BaseAICharacter.h"
#include "AI/BaseAIController.h"
#include "Net/UnrealNetwork.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ABaseAICharacter::ABaseAICharacter()
{
    bReplicates = true;
    SetReplicateMovement(true);

    // 기본 스탯 설정
    MaxHP = 100.f;
    CurrentHP = MaxHP;
    AttackRange = 150.f;
    AttackDamage = 10.f;
    bIsDead = false;
    SelectedAppearanceIndex = -1; // 초기값 설정
}

void ABaseAICharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABaseAICharacter, SelectedAppearanceIndex);
    DOREPLIFETIME(ABaseAICharacter, CurrentHP);
    DOREPLIFETIME(ABaseAICharacter, bIsDead);
}

void ABaseAICharacter::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        CurrentHP = MaxHP;

        if (AppearancePresets.Num() > 0)
        {
            // 서버에서 외형 결정
            SelectedAppearanceIndex = FMath::RandRange(0, AppearancePresets.Num() - 1);
            ApplyAppearance();
        }
    }
}

// 클라이언트가 서버로부터 초기 데이터를 모두 받은 후 호출됨 (동기화 보강)
void ABaseAICharacter::PostNetInit()
{
    Super::PostNetInit();

    if (!HasAuthority() && SelectedAppearanceIndex != -1)
    {
        ApplyAppearance();
    }
}

// 변수가 서버로부터 복제될 때 호출되는 콜백
void ABaseAICharacter::OnRep_SelectedAppearanceIndex()
{
    ApplyAppearance();
}

void ABaseAICharacter::ApplyAppearance()
{
    USkeletalMeshComponent* MeshComp = GetMesh();

    // 데이터 유효성 검사
    if (!MeshComp || !AppearancePresets.IsValidIndex(SelectedAppearanceIndex))
    {
        return;
    }

    const FAIAppearanceSet& SelectedSet = AppearancePresets[SelectedAppearanceIndex];

    if (SelectedSet.Mesh)
    {
        MeshComp->SetSkeletalMeshAsset(SelectedSet.Mesh);
    }

    if (SelectedSet.AnimBlueprint)
    {
        MeshComp->SetAnimInstanceClass(SelectedSet.AnimBlueprint);
    }

    MeshComp->InitAnim(true);

    // 로그 확인용
    FString RoleStr = HasAuthority() ? TEXT("Server") : TEXT("Client");
    UE_LOG(LogTemp, Log, TEXT("[%s] Appearance Applied Index: %d"), *RoleStr, SelectedAppearanceIndex);
}

// --- 공격 관련 로직 ---

void ABaseAICharacter::PlayAttackMontage()
{
    if (!HasAuthority() || bIsDead) return;
    MulticastPlayAttackMontage();
}

void ABaseAICharacter::MulticastPlayAttackMontage_Implementation()
{
    if (bIsDead) return;

    if (AppearancePresets.IsValidIndex(SelectedAppearanceIndex))
    {
        UAnimMontage* CurrentAttack = AppearancePresets[SelectedAppearanceIndex].AttackMontage;
        if (CurrentAttack && GetMesh()->GetAnimInstance())
        {
            PlayAnimMontage(CurrentAttack);
        }
    }
}

void ABaseAICharacter::OnAttackHitCheck()
{
    if (!HasAuthority() || bIsDead) return;

    FVector TraceStart = GetActorLocation();
    FVector TraceEnd = TraceStart + (GetActorForwardVector() * AttackRange);
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(50.f);
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHasHit = GetWorld()->OverlapMultiByChannel(OverlapResults, TraceEnd, FQuat::Identity, ECC_Pawn, SphereShape, Params);

    if (bHasHit)
    {
        for (auto& Result : OverlapResults)
        {
            AActor* HitActor = Result.GetActor();
            if (HitActor && HitActor != this)
            {
                UGameplayStatics::ApplyDamage(HitActor, AttackDamage, GetController(), this, UDamageType::StaticClass());
            }
        }
    }
}

// --- 데미지 및 사망 로직 ---

float ABaseAICharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHP = FMath::Clamp(CurrentHP - ActualDamage, 0.f, MaxHP);

    if (CurrentHP <= 0.f)
    {
        Die();
    }
    return ActualDamage;
}

void ABaseAICharacter::Die()
{
    if (!HasAuthority() || bIsDead) return;

    bIsDead = true;

    ABaseAIController* AICon = Cast<ABaseAIController>(GetController());
    if (AICon)
    {
        AICon->OnAICharacterDead();
    }

    // [서버] 이동 중지 및 중력 영향 최소화
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }

    // [서버] 충돌 설정 (캡슐만 먼저 끄고 메시는 애니메이션을 위해 둡니다)
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    MulticastPlayDeath();
}

void ABaseAICharacter::MulticastPlayDeath_Implementation()
{
    if (AppearancePresets.IsValidIndex(SelectedAppearanceIndex))
    {
        UAnimMontage* DeathAnim = AppearancePresets[SelectedAppearanceIndex].DeathMontage;

        // 1. 애니메이션 재생 전, 메시가 애니메이션을 끝까지 보여주도록 설정
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 쿼리만 끄거나 아예 건드리지 않음
        GetMesh()->bNoSkeletonUpdate = false; // 업데이트 보장

        if (DeathAnim)
        {
            float Duration = PlayAnimMontage(DeathAnim);

            // 델리게이트 바인딩
            UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
            if (AnimInstance)
            {
                FOnMontageEnded MontageEndedDelegate;
                MontageEndedDelegate.BindUObject(this, &ABaseAICharacter::StartDissolveAfterAnim);
                AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, DeathAnim);
            }
        }
        else
        {
            StartDissolveAfterAnim(nullptr, false);
        }
    }
}

void ABaseAICharacter::StartDissolveAfterAnim(UAnimMontage* Montage, bool bInterrupted)
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp && DissolveMasterMaterial)
    {
        // 다이내믹 인스턴스 생성
        DynamicDissolveMaterial = MeshComp->CreateDynamicMaterialInstance(0, DissolveMasterMaterial);

        if (DynamicDissolveMaterial)
        {
            // 시작 시 파라미터 초기화 (완전 불투명 상태에서 시작)
            DynamicDissolveMaterial->SetScalarParameterValue(TEXT("DissolveAmount"), 0.0f);

            // 블루프린트 타임라인 시작
            BP_StartDissolveEffect();
        }
    }
}

// 블루프린트 타임라인의 'Update' 핀에 연결될 함수
void ABaseAICharacter::UpdateDissolveParameter(float DissolveValue)
{
    if (DynamicDissolveMaterial)
    {
		// 'DissolveAmount' 파라미터 업데이트
        DynamicDissolveMaterial->SetScalarParameterValue(TEXT("DissolveAmount"), DissolveValue);
    }
}

void ABaseAICharacter::OnRep_IsDead()
{
    if (bIsDead)
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void ABaseAICharacter::FinishDying()
{
    
    if (HasAuthority())
    {
        Destroy();
    }
}

// UI 업데이트 필요 시 구현
void ABaseAICharacter::OnRep_CurrentHP()
{
    
}