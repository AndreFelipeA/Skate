// Fill out your copyright notice in the Description page of Project Settings.


#include "PawnSkate.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Widgets/PlayerHUD.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"



// Sets default values
APawnSkate::APawnSkate()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(35.f, 96.0f);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &APawnSkate::OnOverlap);



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
	

	SkateboardMesh = CreateDefaultSubobject<UStaticMeshComponent>("SkateboardMesh");
	SkateboardMesh->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void APawnSkate::BeginPlay()
{
	Super::BeginPlay();

	PlayerHUD = Cast<APlayerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());

	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
}

// Called every frame
void APawnSkate::Tick(float DeltaTime)
{


	Super::Tick(DeltaTime);
	// Update();
}

FVector APawnSkate::WheelTrace(FVector SocketLocation)
{
	const FVector OffSet(0.0f,0.0f,40.0f);
	
	FVector LineForward = SocketLocation + OffSet;
	FVector LineBack = SocketLocation - OffSet;

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(Hit, LineForward, LineBack, ECC_Visibility, QueryParams);
	DrawDebugLine(GetWorld(), LineForward, LineBack, Hit.bBlockingHit ? FColor::Blue : FColor::Red, false, 0.1f, 0, 1.0f);
	if(Hit.bBlockingHit)
	{
		if(GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Hit")));
		
		return Hit.Location;
	}
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Socket")));
	return SocketLocation;
		
}



// Called to bind functionality to input
void APawnSkate::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APawnSkate::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APawnSkate::Look);

		// Change Instance
		EnhancedInputComponent->BindAction(ChangeInstanceAction, ETriggerEvent::Completed, this, &APawnSkate::ChangeInstance);}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void APawnSkate::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// float Clamp = FMath::Clamp((SkateboardMesh->GetForwardVector().Z*3.0f), -0.7f, 0.7f);
		// float Movement = MovementVector.Y - Clamp;
		//
		// MfValue = FMath::Lerp(MfValue, Movement, 0.003f);
		//
		// // add movement 
		// AddMovementInput(SkateboardMesh->GetForwardVector(), MfValue);
		// AddMovementInput(SkateboardMesh->GetRightVector(), (MovementVector.X*0.3f));
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);

	}
}
void APawnSkate::Look(const FInputActionValue& Value)
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

void APawnSkate::Update()
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


void APawnSkate::ChangeInstance()
{
	
}

void APawnSkate::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}
