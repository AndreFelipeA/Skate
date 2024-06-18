// Copyright Epic Games, Inc. All Rights Reserved.

#include "SkateCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Obstacles/Obstacles.h"

#include "Kismet/KismetMathLibrary.h"
#include "WIdgets/PlayerHUD.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ASkateCharacter



ASkateCharacter::ASkateCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ASkateCharacter::OnOverlap);
	
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	Score = 0;
	SkateboardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkateBoardMesh"));
	SkateboardMesh->SetupAttachment(RootComponent);
	MrValue = 0.4f;

	OptionsMenu = false;
	//
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}



void ASkateCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	PlayerHUD = Cast<APlayerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
	
	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			PlayerController->SetInputMode(FInputModeGameOnly());
		}
	}
}

FVector ASkateCharacter::WheelTrace(FVector SocketLocation)
{
	const FVector OffSet(0.0f,0.0f,40.0f);
	
	FVector LineForward = SocketLocation + OffSet;
	FVector LineBack = SocketLocation - OffSet;

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(Hit, LineForward, LineBack, ECC_Visibility, QueryParams);
	if(Hit.bBlockingHit)
	{
		return Hit.Location;
	}

	return SocketLocation;
}

void ASkateCharacter::Update()
{
	const FVector FWLocation = SkateboardMesh->GetSocketLocation("FW");
	const FVector BWLocation = SkateboardMesh->GetSocketLocation("BW");


	FwHit =	WheelTrace(FWLocation);
	BwHit =	WheelTrace(BWLocation);

	if(FwHit == FWLocation && BwHit == BWLocation)
	{
		return;
	}
	FVector Direction = (BwHit - FwHit).GetSafeNormal();

	// Calcular a rotação de olhar usando UKismetMathLibrary::FindLookAtRotation
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(BwHit, FwHit);

	FRotator Rotation = FMath::RInterpTo(SkateboardMesh->GetComponentRotation(), LookAtRotation, GetWorld()->GetDeltaSeconds(), 20.f);
	
	SkateboardMesh->SetWorldRotation(Rotation);
}

void ASkateCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Update();

	float Rotation = FMath::Clamp((SkateboardMesh->GetForwardVector().Z*3.0f), -0.9f, 0.9f);

	
	
	if(Rotation < 0 && !FMath::IsNearlyZero(Rotation, 0.00020))
	{
		float Movement = 0 - Rotation;
		MfValue = FMath::Lerp(MfValue, Movement, 0.008f);
		
		// add movement 
		AddMovementInput(SkateboardMesh->GetForwardVector(), MfValue);
	}

	
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASkateCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASkateCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASkateCharacter::Look);

		EnhancedInputComponent->BindAction(OptionAction, ETriggerEvent::Started, this, &ASkateCharacter::Options);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}



void ASkateCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
	
		float Rotation = FMath::Clamp((SkateboardMesh->GetForwardVector().Z*3.0f), -1.f, 1.f);
		float Movement = MovementVector.Y - Rotation;

		MfValue = FMath::Lerp(MfValue, Movement, 0.01f);
		
		// add movement 
		AddMovementInput(SkateboardMesh->GetForwardVector(), MfValue);
		AddMovementInput(SkateboardMesh->GetRightVector(), (MovementVector.X*MrValue));
	

	}
}

void ASkateCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}



void ASkateCharacter::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AObstacles* Obstacles = Cast<AObstacles>(OtherActor);

	if(Obstacles)
	{
		Score += 10;
		OnScoreChangedDelegate.Broadcast(Score);
	}
}


void ASkateCharacter::Options()
{
	OptionsMenu = !OptionsMenu;
	
	PlayerHUD->ShowOptionsMenu(OptionsMenu);
}