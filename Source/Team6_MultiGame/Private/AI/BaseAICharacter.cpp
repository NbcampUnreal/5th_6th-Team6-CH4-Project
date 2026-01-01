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
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetMesh()->bNoSkeletonUpdate = false;

        if (DeathAnim)
        {
            PlayAnimMontage(DeathAnim);

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
    if (!MeshComp) return;

   
    MeshComp->bNoSkeletonUpdate = true;
    MeshComp->SetComponentTickEnabled(false);

   
    MeshComp->SetAnimInstanceClass(nullptr);

    if (DissolveMasterMaterial)
    {
        DynamicDissolveMaterial = MeshComp->CreateDynamicMaterialInstance(0, DissolveMasterMaterial);
        if (DynamicDissolveMaterial)
        {
            DynamicDissolveMaterial->SetScalarParameterValue(TEXT("DissolveAmount"), 0.0f);
            BP_StartDissolveEffect(); 
        }
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
    }
}

void ABaseAICharacter::FinishDying()
{
    if (HasAuthority())
    {
        Destroy();
    }
}

void ABaseAICharacter::OnRep_CurrentHP()
{
   
}