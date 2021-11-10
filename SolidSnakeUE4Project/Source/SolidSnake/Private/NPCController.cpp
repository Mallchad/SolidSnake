
#include "NPCController.h"
#include "CodeHelpers.h"


// CONSTRUCTOR
ANPCController::ANPCController(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{

}

void
FUNCTION ANPCController::SetPawn(APawn* InPawn)
{ }

EPathFollowingRequestResult::Type
FUNCTION ANPCController::MoveToActor(AActor* Goal,
                                     float AcceptanceRadius,
                                     bool bStopOnOverlap,
                                     bool bUsePathfinding,
                                     bool bCanStrafe,
                                     bool bAllowPartialPath)
{
    return EPathFollowingRequestResult::Failed;
}

EPathFollowingRequestResult::Type
FUNCTION ANPCController::MoveToLocation(const FVector& Dest,
                                        float AcceptanceRadius,
                                        bool bStopOnOverlap,
                                        bool bUsePathfinding,
                                        bool bProjectDestinationToNavigation,
                                        bool bCanStrafe,
                                        bool bAllowPartialPath)
{
    return EPathFollowingRequestResult::Failed;
}

FPathFollowingRequestResult
FUNCTION ANPCController::MoveTo(const FAIMoveRequest& MoveRequest,
                                FNavPathSharedPtr* OutPath)
{
    FPathFollowingRequestResult Stub;
    return Stub;
}

void
FUNCTION ANPCController::FindPathForMoveRequest(const FAIMoveRequest& MoveRequest,
                                                FPathFindingQuery& Query,
                                                FNavPathSharedPtr& OutPath) const
{}

bool
FUNCTION ANPCController::BuildPathFindingQuery(const FAIMoveRequest& MoveRequest,
                                               FPathFindingQuery& Query) const
{
    return false;
}

bool
FUNCTION ANPCController::PauseMove(FAIRequestID RequestToPause)
{
    return false;
}

bool 
FUNCTION ANPCController::ResumeMove(FAIRequestID RequestToResume)
{
    return false;
}

void
FUNCTION ANPCController::StopMovement()
{}

void
FUNCTION ANPCController::OnMoveCompleted(FAIRequestID RequestID,
                                         const FPathFollowingResult& Result)
{ }

EPathFollowingStatus::Type
FUNCTION ANPCController::GetMoveStatus() const
{
    return EPathFollowingStatus::Idle;
}

bool
FUNCTION ANPCController::HasPartialPath() const
{
    return false;
}

FVector
FUNCTION ANPCController::GetImmediateMoveDestination() const
{
    return FVector::ZeroVector;
}

void 
FUNCTION ANPCController::SetMoveBlockDetection(bool bEnable)
{}

FVector
FUNCTION ANPCController::GetFocalPoint() const
{
    return FVector::ZeroVector;
}

FVector
FUNCTION ANPCController::GetFocalPointOnActor(const AActor *Actor) const
{
    return FVector::ZeroVector;
}

void 
FUNCTION ANPCController::K2_SetFocalPoint(FVector FP)
{}

void
FUNCTION ANPCController::K2_SetFocus(AActor* NewFocus)
{}

AActor*
FUNCTION ANPCController::GetFocusActor() const
{
    return nullptr;
}

void
FUNCTION ANPCController::K2_ClearFocus()
{}

bool
FUNCTION ANPCController::SuggestTossVelocity(FVector& OutTossVelocity,
                                             FVector Start,
                                             FVector End,
                                             float TossSpeed,
                                             bool bPreferHighArc,
                                             float CollisionRadius,
                                             bool bOnlyTraceUp)
{
    return false;
}

void
FUNCTION ANPCController::BeginPlay()
{}

void
FUNCTION ANPCController::Tick(float DeltaTime)
{}

void
FUNCTION ANPCController::PostInitializeComponents()
{}

void
FUNCTION ANPCController::PostRegisterAllComponents()
{}

void
FUNCTION ANPCController::OnPossess(APawn* InPawn)
{}

void
FUNCTION ANPCController::OnUnPossess()
{}

bool
FUNCTION ANPCController::ShouldPostponePathUpdates() const
{
    return true;
}

void
FUNCTION ANPCController::DisplayDebug(UCanvas* Canvas,
                                      const FDebugDisplayInfo& DebugDisplay,
                                      float& YL,
                                      float& YPos)
{}

#if ENABLE_VISUAL_LOG
void
FUNCTION ANPCController::GrabDebugSnapshot(FVisualLogEntry* Snapshot) const
{}
#endif

void 
FUNCTION ANPCController::Reset()
{}

bool
FUNCTION ANPCController::LineOfSightTo(const AActor* other, FVector ViewPoint, bool bAlternativeChecks) const
{
    return false;
}

void
FUNCTION ANPCController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{}

void
FUNCTION ANPCController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{}

void
FUNCTION ANPCController::SetFocalPoint(FVector NewFocus)
{}

void 
FUNCTION ANPCController::SetFocus(AActor* NewFocus)
{}

void
FUNCTION ANPCController::ClearFocus()
{}

void
FUNCTION ANPCController::SetPerceptionComponent(
    UAIPerceptionComponent& InPerceptionComponent)
{}

bool
FUNCTION ANPCController::IsFollowingAPath() const
{
    return false;
}

bool
FUNCTION ANPCController::PerformAction(UPawnAction& Action, UObject* const Insigator)
{
    return false;
}

// -- Debug/Dev-Time --
FString
FUNCTION ANPCController::GetDebugIcon() const
{
    return FString(TEXT("stub"));
}

void
FUNCTION ANPCController::SetPathFollowingComponent(
    UPathFollowingComponent* NewPFCOmponent)
    {
    }
