// Mjnorms -- 2024


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,						// World context object
				Damage,						// BaseDamage
				10.f,						// MinimumDamage
				GetActorLocation(),			// Origin
				200.f,						// DamageInnerRadius
				500.f,						// DamageOuterRadius
				1.f,						// DamageFalloff
				UDamageType::StaticClass(), // DamageTypeClass
				TArray<AActor*>(),			// IgnoreActors
				this,						// DamageCauser
				FiringController			// InstigatorController
			);
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
