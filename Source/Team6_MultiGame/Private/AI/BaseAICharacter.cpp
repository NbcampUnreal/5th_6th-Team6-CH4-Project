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

    MaxHP = 100.f;
    CurrentHP = MaxHP;
    GetCharacterMovement()->MaxWalkSpeed = 300.f;
    AttackRange = 150.f;
    AttackDamage = 10.f;
    bIsDead = false;
    SelectedAppearanceIndex = -1;
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
            SelectedAppearanceIndex = FMath::RandRange(0, AppearancePresets.Num() - 1);
            ApplyAppearance();
        }
    }
}

void ABaseAICharacter::PostNetInit()
{
    Super::PostNetInit();
    if (!HasAuthority() && SelectedAppearanceIndex != -1)
    {
        ApplyAppearance();
    }
}

void ABaseAICharacter::OnRep_SelectedAppearanceIndex()
{
    ApplyAppearance();
}

void ABaseAICharacter::ApplyAppearance()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp || !AppearancePresets.IsValidIndex(SelectedAppearanceIndex)) return;

    const FAIAppearanceSet& SelectedSet = AppearancePresets[SelectedAppearanceIndex];

    if (SelectedSet.Mesh) MeshComp->SetSkeletalMeshAsset(SelectedSet.Mesh);
    if (SelectedSet.AnimBlueprint) MeshComp->SetAnimInstanceClass(SelectedSet.AnimBlueprint);

    MeshComp->InitAnim(true);
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

           
            if (HitActor && HitActor != this && HitActor->ActorHasTag(TEXT("Player")))
            {
                UGameplayStatics::ApplyDamage(HitActor, AttackDamage, GetController(), this, UDamageType::StaticClass());
            }
        }
    }
}

// 데미지 및 사망 로직 

float ABaseAICharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHP = FMath::Clamp(CurrentHP - ActualDamage, 0.f, MaxHP);

    // 피격 시 AI 컨트롤러를 깨워서 공격자를 보게 함 (경계 상태 돌입)
    if (ActualDamage > 0.f && DamageCauser)
    {
        ABaseAIController* AIC = Cast<ABaseAIController>(GetController());
        if (AIC)
        {
            AIC->OnDamagedByPlayer(DamageCauser);
        }
    }

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
    if (AICon) AICon->OnAICharacterDead();

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    MulticastPlayDeath();
}
void ABaseAICharacter::MulticastPlayDeath_Implementation()
{
    if (AppearancePresets.IsValidIndex(SelectedAppearanceIndex))
    {
        UAnimMontage* DeathAnim = AppearancePresets[SelectedAppearanceIndex].DeathMontage;
        if (!DeathAnim) return;

        UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
        if (AnimInst && AnimInst->Montage_IsPlaying(DeathAnim)) return;

        GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetMesh()->bNoSkeletonUpdate = false;

        PlayAnimMontage(DeathAnim);

        // [수정] 아래의 MontageEndedDelegate 관련 코드들은 삭제하세요.
        // 이제 몽타주 안에 심어놓은 Notify가 TriggerDissolveEffect를 호출합니다.
    }
}

void ABaseAICharacter::UpdateDissolveParameter(float DissolveValue)
{
    if (DynamicDissolveMaterial)
    {
        DynamicDissolveMaterial->SetScalarParameterValue(TEXT("DissolveAmount"), DissolveValue);
    }
}

void ABaseAICharacter::OnRep_IsDead()
{
    if (bIsDead)
    {
       
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        // MulticastPlayDeath(); 
    }
}

void ABaseAICharacter::FinishDying()
{
    if (HasAuthority())
    {
        Destroy();
    }
}


void ABaseAICharacter::TriggerDissolveEffect()
{
    // 1. 중복 실행 방지 및 유효성 검사
    if (DynamicDissolveMaterial || !DissolveMasterMaterial) return;

    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp) return;

    // 2. 애니메이션 확실히 멈추기 (박제)
    MeshComp->SetPlayRate(0.0f); // 재생 속도 0
    UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
    if (AnimInst)
    {
        AnimInst->Montage_Pause(nullptr); // 현재 재생 중인 모든 몽타주 일시정지
    }

    // 포즈 업데이트 중단 (이걸 해야 완전히 박제됨)
    MeshComp->bNoSkeletonUpdate = true;

    // 3. 디졸브 머티리얼 생성 및 적용 (슬롯 0번 고정)
    DynamicDissolveMaterial = MeshComp->CreateDynamicMaterialInstance(0, DissolveMasterMaterial);

    if (DynamicDissolveMaterial)
    {
        // 4. 블루프린트 타임라인 시작 호출
        BP_StartDissolveEffect();
    }
}


void ABaseAICharacter::OnRep_CurrentHP()
{
   
}