// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerChar2.h"

// Sets default values
APlayerChar2::APlayerChar2()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PlayerCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Cam"));

	PlayerCamComp->SetupAttachment(GetMesh(), "Head");
	PlayerCamComp->bUsePawnControlRotation = true;

	ResourcesArray.SetNum(3);
	ResourcesNameArray.Add(TEXT("Wood"));
	ResourcesNameArray.Add(TEXT("Stone"));
	ResourcesNameArray.Add(TEXT("Berry"));
}

// Called when the game starts or when spawned
void APlayerChar2::BeginPlay()
{
	Super::BeginPlay();
	
	FTimerHandle StatsTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(StatsTimerHandle, this, &APlayerChar2::DecreaseStats, 2.0f, true);
}

// Called every frame
void APlayerChar2::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	playerUI->UpdateBars(Health, Hunger, Stamina);

	/*if (isBuilding) {
		if (spawnedPart) {
			FVector StartLocation = PlayerCamComp->GetComponentLocation();
			FVector Direction = PlayerCamComp->GetForwardVector() * 800.0f;
			FVector EndLocation = StartLocation + Direction;
			spawnedPart->SetActorLocation(EndLocation);
		}
	}*/
}

// Called to bind functionality to input. 
void APlayerChar2::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerChar2::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerChar2::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerChar2::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &APlayerChar2::AddControllerYawInput);
	PlayerInputComponent->BindAction("JumpEvent", IE_Pressed, this, &APlayerChar2::StartJump);
	PlayerInputComponent->BindAction("JumpEvent", IE_Released, this, &APlayerChar2::StopJump);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerChar2::FindObject);
}

void APlayerChar2::MoveForward(float axisValue) { // that was another VS studio feature that didn't follow the resource video.
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
	AddMovementInput(Direction, axisValue);
}

void APlayerChar2::MoveRight(float axisValue) {
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	AddMovementInput(Direction, axisValue);
}

// this start jump is called by the imput component because
void APlayerChar2::StartJump()
{
	bPressedJump = true;
}

void APlayerChar2::StopJump()
{
	bPressedJump = false;
}

void APlayerChar2::FindObject()
{
	FHitResult HitResult;
	FVector StartLocation = PlayerCamComp->GetComponentLocation();
	FVector Direction = PlayerCamComp->GetForwardVector() * 800.0f;
	FVector EndLocation = StartLocation + Direction;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnFaceIndex = true;
	
	if(GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
	{
		AResource_M* HitResource = Cast<AResource_M>(HitResult.GetActor());
		if (Stamina > 5.0f) {
			if (HitResource) {
				FString hitName = HitResource->resourceName;
				int resourceValue = HitResource->resourceAmount;

				if(HitResource->totalResource > resourceValue)
				{
					HitResource->totalResource -= resourceValue;
					GiveResource(resourceValue, hitName);
					check(GEngine != nullptr);
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("You have collected %d %s. %d remaining."), resourceValue, *hitName, HitResource->totalResource));

					UGameplayStatics::SpawnDecalAtLocation(GetWorld(), hitDecal, FVector(10.0f, 10.0f, 10.0f), HitResult.Location, FRotator(-90, 0, 0), 2.0f);
					SetStamina(resourceValue);
				}
				else
				{
					GiveResource(HitResource->totalResource, hitName);
					HitResource->Destroy();
					check(GEngine != nullptr);
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("You have collected %d %s. Resource Depleted."), resourceValue, *hitName));

					SetStamina(HitResource->totalResource);
				}
			}
		}
	}
}

void APlayerChar2::SetHealth(float Amount)
{
	if(Health + Amount > 100.0f)
	{
		Health = 100.0f;
	}
	else if(Health + Amount < 0.0f)
	{
		Health = 0.0f;
	}
	else
	{
		Health += Amount;
	}
}

void APlayerChar2::SetHunger(float Amount)
{
	if(Hunger + Amount > 100.0f)
	{
		Hunger = 100.0f;
	}
	else if(Hunger + Amount < 0.0f)
	{
		Hunger = 0.0f;
	}
	else
	{
		Hunger += Amount;
	}
}

void APlayerChar2::SetStamina(float Amount)
{
	if(Stamina + Amount > 100.0f)
	{
		Stamina = 100.0f;
	}
	else if(Stamina + Amount < 0.0f)
	{
		Stamina = 0.0f;
	}
	else
	{
		Stamina += Amount;
	}
}

void APlayerChar2::DecreaseStats()
{
	SetHunger(-1.0f);
	SetStamina(10.0f);

	if(Hunger <= 0)
	{
		SetHealth(-3.0f);
	}
}

void APlayerChar2::GiveResource(float amount, FString resourceType)
{
	if(resourceType == "Wood")
	{
		//resourcearray's index size should be defined 
		ResourcesArray[0] = ResourcesArray[0] + amount;
	}
	if(resourceType == "Stone")
	{
		ResourcesArray[1] = ResourcesArray[1] + amount;
	}
	if(resourceType == "Food")
	{
		ResourcesArray[2] = ResourcesArray[2] + amount;
	}
}
