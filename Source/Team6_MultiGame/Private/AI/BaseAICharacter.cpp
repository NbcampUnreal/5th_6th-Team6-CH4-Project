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

    MulticastPlayDeath();

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
    }
}

void ABaseAICharacter::MulticastPlayDeath_Implementation()
{
    // 1. 애니메이션 재생
    if (AppearancePresets.IsValidIndex(SelectedAppearanceIndex))
    {
        UAnimMontage* DeathAnim = AppearancePresets[SelectedAppearanceIndex].DeathMontage;
        if (DeathAnim) PlayAnimMontage(DeathAnim);
    }

    // 2. 메시 컴포넌트 가져오기
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp)
    {
        // [중요] 에디터에서 설정한 마스터 머티리얼이 있다면 메시의 0번 슬롯에 덮어씌웁니다.
        if (DissolveMasterMaterial)
        {
            MeshComp->SetMaterial(0, DissolveMasterMaterial);
        }

        // 이제 그 0번 슬롯(디졸브 머티리얼)을 제어할 다이내믹 인스턴스 생성
        DynamicDissolveMaterial = MeshComp->CreateDynamicMaterialInstance(0);
    }

    // 3. 블루프린트 타임라인 이벤트 호출
    BP_StartDissolveEffect();
}

// 블루프린트 타임라인의 'Update' 핀에 연결될 함수
void ABaseAICharacter::UpdateDissolveParameter(float DissolveValue)
{
    if (DynamicDissolveMaterial)
    {
        // 머티리얼에 설정된 파라미터 이름(예: DissolveAmount)과 일치해야 합니다.
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