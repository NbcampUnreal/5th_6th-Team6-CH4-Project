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

    // 데미지 적용 및 체력 클램핑
    CurrentHP = FMath::Clamp(CurrentHP - ActualDamage, 0.f, MaxHP);

    if (ActualDamage > 0.f)
    {
        // 1. 모든 클라이언트에서 피격 애니메이션 재생
        MulticastPlayHitMontage();

        // 2. 공격자가 있을 경우 컨트롤러 로직 실행 (서버에서만 실행)
        if (DamageCauser)
        {
            ABaseAIController* AIC = Cast<ABaseAIController>(GetController());
            if (AIC)
            {
                AIC->OnDamagedByPlayer(DamageCauser);
            }
        }
    }

    // 사망 판정
    if (CurrentHP <= 0.f)
    {
        Die();
    }

    return ActualDamage;
}

// 모든 클라이언트에서 피격 몽타주 재생
void ABaseAICharacter::MulticastPlayHitMontage_Implementation()
{
   
    if (bIsDead || !GetMesh()) return;

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (!AnimInst) return;

    
    if (AppearancePresets.IsValidIndex(SelectedAppearanceIndex))
    {
        UAnimMontage* HitMontage = AppearancePresets[SelectedAppearanceIndex].HitMontage;

        if (HitMontage)
        {
            
            AnimInst->Montage_Play(HitMontage);
        }
    }
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
    
    if (DynamicDissolveMaterial || !DissolveMasterMaterial) return;

    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp) return;

   
    MeshComp->SetPlayRate(0.0f); 
    UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
    if (AnimInst)
    {
        AnimInst->Montage_Pause(nullptr); 
    }

  
    MeshComp->bNoSkeletonUpdate = true;


    DynamicDissolveMaterial = MeshComp->CreateDynamicMaterialInstance(0, DissolveMasterMaterial);

    if (DynamicDissolveMaterial)
    {
        
        BP_StartDissolveEffect();
    }
}


void ABaseAICharacter::OnRep_CurrentHP()
{
   
}