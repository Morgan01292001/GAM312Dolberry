// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerChar2.h"

// Sets default values
APlayerChar2::APlayerChar2()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// this camera component is implemented here.
	PlayerCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Cam"));
	// the camera is attached to the head bone of the player character's mesh. This allows the camera to move with the character's head movements.
	PlayerCamComp->SetupAttachment(GetMesh(), "Head");
	PlayerCamComp->bUsePawnControlRotation = true;

	BuildingArray.SetNum(3);
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

	if (objWidget) {
		objWidget->UpdatebuildOBJ(0.0f);
		objWidget->UpdatematOBJ(0.0f);
	}
}

// Called every frame
void APlayerChar2::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	playerUI->UpdateBars(Health, Hunger, Stamina);

	if (isBuilding) {
		if (spawnedPart) {
			FVector StartLocation = PlayerCamComp->GetComponentLocation();
			FVector Direction = PlayerCamComp->GetForwardVector() * 800.0f;
			FVector EndLocation = StartLocation + Direction;
			spawnedPart->SetActorLocation(EndLocation);
		}
	}
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
	PlayerInputComponent->BindAction("RotPart", IE_Pressed, this, &APlayerChar2::RotateBuilding);
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
	// explaining the linear algebra here, it's multiplying and adding numbers in a set of 3 to target a location in 3D space.
	// use of trace and collision here is to detect interactions with objects in the game world.
	FHitResult HitResult;
	FVector StartLocation = PlayerCamComp->GetComponentLocation();
	FVector Direction = PlayerCamComp->GetForwardVector() * 800.0f;
	FVector EndLocation = StartLocation + Direction;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnFaceIndex = true;
	
	if (!isBuilding) {
		if(GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
			{
				AResource_M* HitResource = Cast<AResource_M>(HitResult.GetActor());
				if (Stamina > 5) {
					if (HitResource) {
						FString hitName = HitResource->resourceName;
						int resourceValue = HitResource->resourceAmount;
				
						HitResource->totalResource -= resourceValue;
						if(HitResource->totalResource > resourceValue)
						{
							GiveResource(resourceValue, hitName);
							matsCollected += resourceValue;
							objWidget->UpdatematOBJ(matsCollected);
							check(GEngine != nullptr);
							GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("You have collected %d %s. %d remaining."), resourceValue, *hitName, HitResource->totalResource));

							UGameplayStatics::SpawnDecalAtLocation(GetWorld(), hitDecal, FVector(10.0f, 10.0f, 10.0f), HitResult.Location, FRotator(-90, 0, 0), 2.0f);
							SetStamina(-5);
						}
						else
						{
							HitResource->Destroy();
							check(GEngine != nullptr);
							GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("You have collected %d %s. Resource Depleted."), resourceValue, *hitName));
						}
					}
				}
			}
	}
	else {
		isBuilding = false;
		objectsBuilt++;
		objWidget->UpdatebuildOBJ(objectsBuilt);
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
	if(resourceType == "Berries")
	{
		ResourcesArray[2] = ResourcesArray[2] + amount;
	}
}

void APlayerChar2::UpdateResources(float woodAmount, float stoneAmount, FString buildingObject)
{
	if (woodAmount <= ResourcesArray[0]) {
		if (stoneAmount <= ResourcesArray[1]) {
			ResourcesArray[0] -= woodAmount;
			ResourcesArray[1] -= stoneAmount;
			if (buildingObject == "Wall") {
				BuildingArray[0]++;
			}
			if (buildingObject == "Floor") {
				BuildingArray[1]++;
			}
			if (buildingObject == "Ceiling") {
				BuildingArray[2]++;
			}
		}
	}
}

void APlayerChar2::SpawnBuilding(int buildingID, bool& isSuccess)
{
	if (!isBuilding && BuildingArray[buildingID] >= 1) {
		isBuilding = true;
		FActorSpawnParameters SpawnParams;
		FVector StartLocation = PlayerCamComp->GetComponentLocation();
		FVector Direction = PlayerCamComp->GetForwardVector() * 400.0f;
		FVector EndLocation = StartLocation + Direction;
		FRotator myRot = FRotator(0.0f, 0.0f, 0.0f);
		BuildingArray[buildingID]--;
		spawnedPart = GetWorld()->SpawnActor<ABuildingPart>(BuildPartClass, EndLocation, myRot, SpawnParams);
		isSuccess = true;
	}
	isSuccess = false;
}

void APlayerChar2::RotateBuilding()
{
	if (isBuilding) {
		spawnedPart->AddActorWorldRotation(FRotator(0.0f, 90.0f, 0.0f));
	}
}
