
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/WidgetComponent.h" 
#include "Squirrel.generated.h"

class USpringArmComponent;
class UCameraComponent;

//UI
class UUW_HPBar;

UCLASS()
class TEAM6_MULTIGAME_API ASquirrel : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASquirrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void Move(const FVector2D& Value);

	UFUNCTION()
	void Look(const FVector2D& value);

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

#pragma region UI

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HP")
	float MaxHP = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HP")
	float CurrentHP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> HPWidgetComponent;

	void UpdateHPUI();

#pragma endregion
};
