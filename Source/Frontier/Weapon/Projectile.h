// Mjnorms -- 2024

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"


UCLASS()
class FRONTIER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

private:
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent = nullptr;


	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* Tracer = nullptr;
	class UNiagaraComponent* TracerComponent = nullptr;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* ImpactFX = nullptr;
	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound = nullptr;
};
