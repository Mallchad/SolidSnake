
#include "NPCController.h"
#include "CodeHelpers.h"

#include <CoreMinimal.h>
#include <AITypes.h>
#include <NavFilters/NavigationQueryFilter.h>
#include <Components/CapsuleComponent.h>
#include <Actions/PawnActionsComponent.h>
#include <Perception/AIPerceptionComponent.h>
#include <NavigationSystem.h>

#include <Engine/Canvas.h>
#include <Kismet/GameplayStatics.h>
#include <DisplayDebugHelpers.h>
#include <VisualLogger/VisualLoggerTypes.h>
#include <VisualLogger/VisualLogger.h>

// -- Logging Declarations --
DECLARE_LOG_CATEGORY_EXTERN(LogNPCNavigation, All, All);
DEFINE_LOG_CATEGORY(LogNPCNavigation);

DECLARE_CYCLE_STAT(TEXT("MoveTo"), STAT_MoveTo, STATGROUP_AI);

// -- Member Functions Definitions --

CONSTRUCTOR ANPCController::ANPCController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bSetControlRotationFromPawnOrientation = true;
    PathFollowingComponent =
        CreateDefaultSubobject<UPathFollowingComponent>(TEXT("PathFollowingComponent"));
    PathFollowingComponent->OnRequestFinished.
        AddUObject(this, &ANPCController::OnMoveCompleted);

    ActionsComp = CreateDefaultSubobject<UPawnActionsComponent>("ActionsComp");

    bSkipExtraLOSChecks = true;
    bWantsPlayerState = false;

    bStartAILogicOnPossess = false;
    bStopAILogicOnUnposses = true;

    // DefaultNavigationFilterClass = UNavigationQueryFilter::StaticClass;
}

void
FUNCTION ANPCController::SetPawn(APawn* InPawn)
{
    Super::SetPawn(InPawn);
}

EPathFollowingRequestResult::Type
FUNCTION ANPCController::MoveToActor(AActor* Goal,
                                     float AcceptanceRadius,
                                     bool bStopOnOverlap,
                                     bool bUsePathfinding,
                                     bool bCanStrafe,
                                     bool bAllowPartialPath)
{
    // Abort active movement to keep only one request running
    if (PathFollowingComponent != nullptr &&
        PathFollowingComponent->GetStatus() != EPathFollowingStatus::Idle)
    {
        PathFollowingComponent->AbortMove(*this,
                                          FPathFollowingResultFlags::ForcedScript |
                                          FPathFollowingResultFlags::NewRequest,
                                          FAIRequestID::CurrentRequest,
                                          EPathFollowingVelocityMode::Keep);
    }

    FAIMoveRequest MoveReq(Goal);
    MoveReq.SetUsePathfinding(bUsePathfinding);
    MoveReq.SetAllowPartialPath(bAllowPartialPath);
    MoveReq.SetNavigationFilter(DefaultNavigationFilterClass);
    MoveReq.SetAcceptanceRadius(AcceptanceRadius);
    MoveReq.SetReachTestIncludesAgentRadius(bStopOnOverlap);
    MoveReq.SetCanStrafe(bCanStrafe);

    return MoveTo(MoveReq);
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
    // Abort active movement to keep only one request running
    if (PathFollowingComponent != nullptr &&
        PathFollowingComponent->GetStatus() != EPathFollowingStatus::Idle)
    {
        PathFollowingComponent->AbortMove(*this,
                                          FPathFollowingResultFlags::ForcedScript |
                                          FPathFollowingResultFlags::NewRequest,
                                          FAIRequestID::CurrentRequest,
                                          EPathFollowingVelocityMode::Keep);
    }
    FAIMoveRequest MoveReq(Dest);
    MoveReq.SetUsePathfinding(bUsePathfinding);
    MoveReq.SetAllowPartialPath(bAllowPartialPath);
    MoveReq.SetNavigationFilter(DefaultNavigationFilterClass);
    MoveReq.SetAcceptanceRadius(AcceptanceRadius);
    MoveReq.SetReachTestIncludesAgentRadius(bStopOnOverlap);
    MoveReq.SetCanStrafe(bCanStrafe);

    return MoveTo(MoveReq);
}

FAIRequestID
FUNCTION ANPCController::RequestMove(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr Path)
{
    uint32 RequestID = FAIRequestID::InvalidRequest;
    if (PathFollowingComponent != nullptr)
    {
        RequestID = PathFollowingComponent->RequestMove(MoveRequest, Path);
    }

    return RequestID;
}
FPathFollowingRequestResult
FUNCTION ANPCController::MoveTo(const FAIMoveRequest& MoveRequest,
                                FNavPathSharedPtr* OutPath)
{
    /* both MoveToActor and MoveToLocation can be called from blueprints/script,
       and should keep only single movement request at the same time.

       this function is entry point of all movement mechanics - do NOT abort in here,
       since movement may be handled by AITasks, which support stacking */

    SCOPE_CYCLE_COUNTER(STAT_MoveTo);

    UE_VLOG(this, LogNPCNavigation, Log, TEXT("MoveTo: %s"), *MoveRequest.ToString());

	FPathFollowingRequestResult ResultData;
	ResultData.Code = EPathFollowingRequestResult::Failed;

	if (MoveRequest.IsValid() == false)
	{
		UE_VLOG(this, LogNPCNavigation, Error,
                TEXT("MoveTo request failed, MoveRequest is invalid. likely Goal Actor no longer exists"),
                *MoveRequest.ToString());
		return ResultData;
	}

	if (PathFollowingComponent == nullptr)
	{
		UE_VLOG(this, LogNPCNavigation, Error,
                TEXT("MoveTo request failed due missing PathFollowingComponent"));
		return ResultData;
	}

    ensure(MoveRequest.GetNavigationFilter());

    bool bCanRequestMove = true;
    bool bAlreadyAtGoal = false;

    if (!MoveRequest.IsMoveToActorRequest())
    {
        if (MoveRequest.GetGoalLocation().ContainsNaN() ||
            FAISystem::IsValidLocation(MoveRequest.GetGoalLocation()) == false)
        {
            UE_VLOG(this, LogNPCNavigation, Error,
                    TEXT("AAIController::MoveTo: Destination is not valid! Goal(%s)"),
                    TEXT_AI_LOCATION(MoveRequest.GetGoalLocation()));
            bCanRequestMove = false;
        }

        // Fail if projection to navigation is required but it failed
        if (bCanRequestMove && MoveRequest.IsProjectingGoal())
        {
            UNavigationSystemV1* NavSys =
                FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
            const FNavAgentProperties& AgentProps = GetNavAgentPropertiesRef();
            FNavLocation ProjectedLocation;

            if ( NavSys != nullptr &&
                 !NavSys->ProjectPointToNavigation(MoveRequest.GetGoalLocation(),
                                                  ProjectedLocation,
                                                   INVALID_NAVEXTENT, &AgentProps) )
            {
                if (MoveRequest.IsUsingPathfinding())
                {
                    UE_VLOG_LOCATION(this, LogNPCNavigation, Error,
                                     MoveRequest.GetGoalLocation(), 30.f,
                                     FColor::Red,
                                     TEXT("AAIController::MoveTo failed to project destination location to navmesh"));
                }
                else
                {
                    UE_VLOG_LOCATION(this, LogNPCNavigation, Error,
                                     MoveRequest.GetGoalLocation(), 30.f, FColor::Red,
                                     TEXT("AAIController::MoveTo failed to project destination location to navmesh, path finding is disabled perhaps disable goal projection ?"));
                }

                bCanRequestMove = false;
            }

            MoveRequest.UpdateGoalLocation(ProjectedLocation.Location);
        }

        bAlreadyAtGoal = bCanRequestMove && PathFollowingComponent->HasReached(MoveRequest);
    }
    else
    {
        bAlreadyAtGoal = bCanRequestMove && PathFollowingComponent->HasReached(MoveRequest);
    }

    if (bAlreadyAtGoal)
    {
        UE_VLOG(this, LogNPCNavigation, Log, TEXT("MoveTo: already at goal!"));
        ResultData.MoveId =
            PathFollowingComponent->RequestMoveWithImmediateFinish(EPathFollowingResult::Success);
        ResultData.Code = EPathFollowingRequestResult::AlreadyAtGoal;
    }
    else if (bCanRequestMove)
    {
        FPathFindingQuery PFQuery;

        const bool bValidQuery = BuildPathFindingQuery(MoveRequest, PFQuery);
        if (bValidQuery)
        {
            FNavPathSharedPtr Path;
            FindPathForMoveRequest(MoveRequest, PFQuery, Path);

            const FAIRequestID RequestID = Path.IsValid() ?
                RequestMove(MoveRequest, Path) :
                FAIRequestID::InvalidRequest;
            if (RequestID.IsValid())
            {
                bAllowStrafe = MoveRequest.CanStrafe();
                ResultData.MoveId = RequestID;
                ResultData.Code = EPathFollowingRequestResult::RequestSuccessful;

                if (OutPath)
                {
                    *OutPath = Path;
                }
            }
        }
    }

    if (ResultData.Code == EPathFollowingRequestResult::Failed)
    {
        ResultData.MoveId = PathFollowingComponent->RequestMoveWithImmediateFinish(EPathFollowingResult::Invalid);
    }

    return ResultData;
}

void
FUNCTION ANPCController::FindPathForMoveRequest(const FAIMoveRequest& MoveRequest,
                                                FPathFindingQuery& Query,
                                                FNavPathSharedPtr& OutPath) const
{
    SCOPE_CYCLE_COUNTER(STAT_AI_Overall);

    UNavigationSystemV1* NavSys =
        FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

    if (NavSys != nullptr)
    {
        FPathFindingResult PathResult = NavSys->FindPathSync(Query);
        if (PathResult.Result != ENavigationQueryResult::Error)
        {
            if (PathResult.IsSuccessful() && PathResult.Path.IsValid())
            {
                if ( MoveRequest.IsMoveToActorRequest() )
                {
                    PathResult.Path->SetGoalActorObservation(*MoveRequest.GetGoalActor(), 100.f);
                }

                PathResult.Path->EnableRecalculationOnInvalidation(true);
                OutPath = PathResult.Path;
            }
        }
        else
        {
            UE_VLOG(this, LogNPCNavigation, Error,
                    TEXT("Trying to find path to %s resulted in Error"),
                    MoveRequest.IsMoveToActorRequest() ?
                    *GetNameSafe(MoveRequest.GetGoalActor()) :
                    *MoveRequest.GetGoalLocation().ToString());
            UE_VLOG_SEGMENT(this, LogNPCNavigation, Error,
                            GetPawn() ?
                            GetPawn()->GetActorLocation() :
                            FAISystem::InvalidLocation,
                            MoveRequest.GetGoalLocation(),
                            FColor::Red, TEXT("Failed move to %s"),
                            *GetNameSafe(MoveRequest.GetGoalActor()));
        }
    }
}

bool
FUNCTION ANPCController::BuildPathFindingQuery(const FAIMoveRequest& MoveRequest,
                                               FPathFindingQuery& Query) const
{
    bool bResult = false;

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    const ANavigationData* NavData = (NavSys == nullptr) ? nullptr :
        MoveRequest.IsUsingPathfinding() ?
        NavSys->GetNavDataForProps(GetNavAgentPropertiesRef(), GetNavAgentLocation()) :
        NavSys->GetAbstractNavData();

    if (NavData != nullptr)
    {
        FVector GoalLocation = MoveRequest.GetGoalLocation();
        if ( MoveRequest.IsMoveToActorRequest() )
        {
            const INavAgentInterface* NavGoal =
                Cast<const INavAgentInterface>( MoveRequest.GetGoalActor() );
            if (NavGoal != nullptr)
            {
                const FVector Offset = NavGoal->GetMoveGoalOffset(this);
                GoalLocation =
                    FQuatRotationTranslationMatrix(MoveRequest.GetGoalActor()->GetActorQuat(),
                                                   NavGoal->GetNavAgentLocation())
                    .TransformPosition(Offset);
            }
            else
            {
                GoalLocation = MoveRequest.GetGoalActor()->GetActorLocation();
            }

            FSharedConstNavQueryFilter NavFilter =
                UNavigationQueryFilter::GetQueryFilter(*NavData,
                                                       this,
                                                       MoveRequest.GetNavigationFilter());
            Query = FPathFindingQuery(*this,
                                      *NavData,
                                      GetNavAgentLocation(),
                                      GoalLocation,
                                      NavFilter);
            Query.SetAllowPartialPaths(MoveRequest.IsUsingPartialPaths());

            if (PathFollowingComponent)
            {
                PathFollowingComponent->OnPathfindingQuery(Query);
            }

            bResult = true;
        }
        else
        {
            UE_VLOG(this, LogNPCNavigation, Warning,
                    TEXT("Unable to find NavigationData instance while calling AAIController::BuildPathfindingQuery"));
        }
    }
    return bResult;
}

bool
FUNCTION ANPCController::PauseMove(FAIRequestID RequestToPause)
{
    if ( PathFollowingComponent != nullptr &&
         RequestToPause.IsEquivalent( PathFollowingComponent->GetCurrentRequestId() ))
    {
        PathFollowingComponent->PauseMove(RequestToPause, EPathFollowingVelocityMode::Reset);
        return true;
    }
    return false;
}

bool 
FUNCTION ANPCController::ResumeMove(FAIRequestID RequestToResume)
{
    if ( PathFollowingComponent != nullptr &&
         RequestToResume.IsEquivalent( PathFollowingComponent->GetCurrentRequestId() ))
    {
        PathFollowingComponent->ResumeMove(RequestToResume);
        return true;
    }
    return false;
}

void
FUNCTION ANPCController::StopMovement()
{
/** @note FPathFollowingResultFlags::ForcedScript added to make AITask_MoveTo instances
    not ignore OnRequestFinished notify that's going to be sent out due to this call */
    PathFollowingComponent->AbortMove(*this, FPathFollowingResultFlags::MovementStop | FPathFollowingResultFlags::ForcedScript);
}

void
FUNCTION ANPCController::OnMoveCompleted(FAIRequestID RequestID,
                                         const FPathFollowingResult& Result)
{
    ReceiveMoveCompleted.Broadcast(RequestID, Result.Code.GetValue());
}

EPathFollowingStatus::Type
FUNCTION ANPCController::GetMoveStatus() const
{
    return (PathFollowingComponent != nullptr) ?
        PathFollowingComponent->GetStatus() :
        EPathFollowingStatus::Idle;
}

bool
FUNCTION ANPCController::HasPartialPath() const
{
    return (PathFollowingComponent != nullptr) && (PathFollowingComponent->HasPartialPath());
}

FVector
FUNCTION ANPCController::GetImmediateMoveDestination() const
{
    return (PathFollowingComponent != nullptr) ?
        PathFollowingComponent->GetCurrentTargetLocation() :
        FVector::ZeroVector;
}

void 
FUNCTION ANPCController::SetMoveBlockDetection(bool bEnable)
{
    if (PathFollowingComponent != nullptr)
    {
        PathFollowingComponent->SetBlockDetectionState(bEnable);
    }
}

FVector
FUNCTION ANPCController::GetFocalPoint() const
{
    FVector Result = FAISystem::InvalidLocation;
    const FFocusInfo::FFocusItem& FocusItem = FocusInformation.TargetFocus;
    const AActor* FocusActor = FocusItem.Actor.Get();

    if (FocusActor != nullptr)
    {
        Result = GetFocalPointOnActor(FocusActor);
    }
    else if ( FAISystem::IsValidLocation(FocusItem.Position) )
    {
        Result = FocusItem.Position;
    }

    return Result;
}

FVector
FUNCTION ANPCController::GetFocalPointOnActor(const AActor *Actor) const
{
    return FAISystem::InvalidLocation;
}

void 
FUNCTION ANPCController::K2_SetFocalPoint(FVector FP)
{
    SetFocalPoint(FP);
}

void
FUNCTION ANPCController::K2_SetFocus(AActor* NewFocus)
{
    SetFocus(NewFocus);
}

AActor*
FUNCTION ANPCController::GetFocusActor() const
{
    const FFocusInfo::FFocusItem& FocusItem = FocusInformation.TargetFocus;
    AActor* FocusActor = FocusItem.Actor.Get();
    return  (FocusActor != nullptr) ? FocusActor : nullptr;
}

void
FUNCTION ANPCController::K2_ClearFocus()
{
    ClearFocus();
}

void
FUNCTION ANPCController::BeginPlay()
{
    Super::BeginPlay();
}

void
FUNCTION ANPCController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    const float DeltaSeconds = DeltaTime;

    UpdateControlRotation(DeltaSeconds);
}

void
FUNCTION ANPCController::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    const bool bInitServerPlayerState =
        bWantsPlayerState && IsPendingKill() && (GetNetMode() != NM_Client);

    if (bInitServerPlayerState)
    {
        InitPlayerState();
    }

#if ENABLE_VISUAL_LOG
    for (UActorComponent* Component : GetComponents())
    {
        if (Component)
        {
            REDIRECT_OBJECT_TO_VLOG(Component, this);
        }
    }
#endif
}

void
FUNCTION ANPCController::PostRegisterAllComponents()
{
    Super::PostRegisterAllComponents();

    // cache PerceptionComponent if not already set
    // note that it's possible for an AI to not have a perception component at all
    const bool bPerceptionCompInvalid =
        PerceptionComponent == NULL || PerceptionComponent->IsPendingKill() == true;

    if (bPerceptionCompInvalid)
    {
        PerceptionComponent = FindComponentByClass<UAIPerceptionComponent>();
    }
}

void
FUNCTION ANPCController::OnPossess(APawn* InPawn)
{
    // Don't try to process pending-kill pawns
    if (InPawn != nullptr && InPawn->IsPendingKill())
    {
        return;
    }

    Super::OnPossess(InPawn);

    if (GetPawn() == nullptr || InPawn == nullptr)
    {
        return;
    }

    // -- Start Processing --
    // Not calling UpdateNavigationComponents() anymore.
    // The PathFollowingComponent is now observing newly possessed pawns (via OnNewPawn)

    AttachToActor(InPawn, FAttachmentTransformRules::KeepRelativeTransform);
    if (PathFollowingComponent)
    {
        PathFollowingComponent->Initialize();
    }

    if (bWantsPlayerState)
    {
        ChangeState(NAME_Playing);
    }

}


void
FUNCTION ANPCController::OnUnPossess()
{
    APawn* CurrentPawn = GetPawn();

    Super::OnUnPossess();

    if (PathFollowingComponent)
    {
        PathFollowingComponent->Cleanup();
    }
}

bool
FUNCTION ANPCController::ShouldPostponePathUpdates() const
{
    return GetPathFollowingComponent()->HasStartedNavLinkMove() ||
        Super::ShouldPostponePathUpdates();
}

void
FUNCTION ANPCController::DisplayDebug(UCanvas* Canvas,
                                      const FDebugDisplayInfo& DebugDisplay,
                                      float& YL,
                                      float& YPos)
{
    Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

    static FName NAME_AI = FName(TEXT("NPC AI"));
    if (DebugDisplay.IsDisplayOn(NAME_AI))
    {
        if (PathFollowingComponent != nullptr)
        {
            PathFollowingComponent->DisplayDebug(Canvas, DebugDisplay, YL, YPos);
        }

        AActor* FocusActor = GetFocusActor();
        if (FocusActor != nullptr)
        {
            FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
            DisplayDebugManager.DrawString(
                FString::Printf(TEXT("Focus %s"), *FocusActor->GetName()));
        }
    }

}

#if ENABLE_VISUAL_LOG
void
FUNCTION ANPCController::GrabDebugSnapshot(FVisualLogEntry* Snapshot) const
{
    FVisualLogStatusCategory MyCategory;
    MyCategory.Category = TEXT("NPC AI Controller");
    MyCategory.Add(TEXT("Pawn"), GetNameSafe(GetPawn()));
    const AActor* FocusActor = GetFocusActor();
    MyCategory.Add(TEXT("Focus"), GetDebugName(FocusActor));

    if (FocusActor == nullptr)
    {
        MyCategory.Add(TEXT("Focus Location"), TEXT_AI_LOCATION(GetFocalPoint()));
    }
    Snapshot->Status.Add(MyCategory);

    if (GetPawn() != nullptr)
    {
        Snapshot->Location = GetPawn()->GetActorLocation();
    }

    if (PathFollowingComponent != nullptr)
    {
        PathFollowingComponent->DescribeSelfToVisLog(Snapshot);
    }

    if (PerceptionComponent != nullptr)
    {
        PerceptionComponent->DescribeSelfToVisLog(Snapshot);
    }
}
#endif

void 
FUNCTION ANPCController::Reset()
{
    Super::Reset();
    if (PathFollowingComponent != nullptr)
    {
        PathFollowingComponent->AbortMove(*this,
                                          FPathFollowingResultFlags::OwnerFinished |
                                          FPathFollowingResultFlags::ForcedScript);
    }
}

bool
FUNCTION ANPCController::LineOfSightTo(const AActor* Other, FVector ViewPoint, bool bAlternativeChecks) const
{
    if (Other == nullptr)
    {
        return false;
    }

    if (ViewPoint.IsZero())
    {
        FRotator ViewRotation;
        GetActorEyesViewPoint(ViewPoint, ViewRotation);

        const bool bFailOnInvalidViewPoint = ViewPoint.IsZero();
        if (bFailOnInvalidViewPoint)
        {
            return false;
        }
    }

    FVector TargetLocation = Other->GetTargetLocation(GetPawn());

    FCollisionQueryParams CollisionParams(SCENE_QUERY_STAT(LineOfSight),
                                          true,
                                          this->GetPawn());
    CollisionParams.AddIgnoredActor(Other);

    bool bHit = GetWorld()->LineTraceTestByChannel(ViewPoint,
                                                   TargetLocation,
                                                   ECC_Visibility,
                                                   CollisionParams);
    if (bHit != true)
    {
        return true;
    }

    const APawn* OtherPawn = Cast<const APawn>(Other);
    const bool bTargetUsesCapsule =
        Cast<UCapsuleComponent>(Other->GetRootComponent()) != nullptr;
    // Bail early, traces will likely be inaccurate and trace off target
    if (OtherPawn != nullptr && bTargetUsesCapsule)
    {
        return false;
    }

    const FVector OtherActorLocation = Other->GetActorLocation();
    const float DistanceSquared = (OtherActorLocation - ViewPoint).SizeSquared();
    if (DistanceSquared > FAR_SIGHT_THRESHOLD_SQUARED)
    {
        return false;
        }
    if ( OtherPawn != nullptr && (DistanceSquared > FAR_SIGHT_THRESHOLD_SQUARED) )
        {
            return false;
        }
    float OtherRadius;
    float OtherHeight;
    Other->GetSimpleCollisionCylinder(OtherRadius, OtherHeight);

    if ( !bAlternativeChecks || !bLOSflag )
    {
        // Try viewpoint to head
        bHit = GetWorld()->LineTraceTestByChannel(ViewPoint, OtherActorLocation + FVector(0.f, 0.f, OtherHeight), ECC_Visibility, CollisionParams);
        if (!bHit)
        {
            return true;
        }
    }
    if ( !bSkipExtraLOSChecks && ( !bAlternativeChecks || bLOSflag) )
    {
        // Only check sides if width of other is sinigifcant compared to distance
        float SignificanceTolerance = 0.0001f;
        FVector VectorToOther = OtherActorLocation - ViewPoint;
        float DistanceToOther = (OtherRadius * OtherRadius) / VectorToOther.SizeSquared();
        bool DistanceSignificant = DistanceToOther < SignificanceTolerance;
        if (DistanceSignificant)
        {
            return false;
        }

        // Try checking sides-
        // Use distance to four side ponits, culling furthest and closest
         FVector Points[4];
         Points[0] = OtherActorLocation - FVector(OtherRadius, -1 * OtherRadius, 0);
         Points[1] = OtherActorLocation + FVector(OtherRadius,      OtherRadius, 0);
         Points[2] = OtherActorLocation - FVector(OtherRadius,      OtherRadius, 0);
         Points[3] = OtherActorLocation + FVector(OtherRadius, -1 * OtherRadius, 0);

         FVector DistanceToPoint = (Points[0] - ViewPoint);
         int32 ClosestIndex = 0;
         int32 FarthestIndex = 0;
         float CurrentMin = DistanceToPoint.SizeSquared();
         float CurrentMax = CurrentMin;
         float NextSize = 0;

         for (int32 IPoint=1; IPoint<4; ++IPoint)
         {
             DistanceToPoint = ( Points[IPoint] - ViewPoint);
             NextSize = DistanceToPoint.SizeSquared();

             if (NextSize > CurrentMin)
             {
                 CurrentMin = NextSize;
                 FarthestIndex = IPoint;
             }
             else if (NextSize < CurrentMax)
             {
                 CurrentMax = NextSize;
                 ClosestIndex = IPoint;
             }
         }
         for (int32 IPoint=0; IPoint<4; ++IPoint)
         {
             if ( (IPoint != ClosestIndex) && (IPoint != FarthestIndex) )
             {
                 bHit = GetWorld()->LineTraceTestByChannel(
                     ViewPoint,
                     Points[IPoint],
                     ECC_Visibility,
                     CollisionParams);
             }
             if (bHit != true)
             {
                 return true;
             }
         }
    }
    return false;
}

void
FUNCTION ANPCController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{

}

void
FUNCTION ANPCController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
    APawn* const ControlledPawn = GetPawn();
    if (ControlledPawn != nullptr)
    {
        FRotator NewControlRotation = GetControlRotation();

        // Look Towards Focus
        FVector FocalPoint = GetFocalPoint();
        if (FAISystem::IsValidLocation(FocalPoint))
        {
            NewControlRotation = (FocalPoint - ControlledPawn->GetPawnViewLocation()).Rotation();
        }
        else if (bSetControlRotationFromPawnOrientation)
        {

            NewControlRotation = ControlledPawn->GetActorRotation();
        }

        // Don't pitch unless looking at another pawn
        if (NewControlRotation.Pitch != 0 && Cast<APawn>(GetFocusActor()) == nullptr)
        {
            NewControlRotation.Pitch = 0.f;
        }
        SetControlRotation(NewControlRotation);

        if (bUpdatePawn)
        {
            const FRotator CuurentPawnRotation = ControlledPawn->GetActorRotation();
            if (CuurentPawnRotation.Equals(NewControlRotation, 1e-3f) == false)
            {
                ControlledPawn->FaceRotation(NewControlRotation, DeltaTime);
            }
        }
    }

}

void
FUNCTION ANPCController::SetFocalPoint(FVector NewFocus)
{
    ClearFocus();

    /* Always be second most important priority in-case critical focus like
       self-presevation wants to occur without clobbering focus */
    FFocusInfo::FFocusItem& FocusItem = FocusInformation.TargetFocus;
    FocusItem.Position = NewFocus;
}

void
FUNCTION ANPCController::SetFocus(AActor* NewFocus)
{
    ClearFocus();

    if (NewFocus != nullptr)
    {
        /* Always be second most important priority in-case critical focus like
           self-presevation wants to occur without clobbering focus */
        FocusInformation.TargetFocus.Actor = NewFocus;
    }
}

void
FUNCTION ANPCController::ClearFocus()
{
    FocusInformation.TargetFocus.Actor = nullptr;
    FocusInformation.TargetFocus.Position = FAISystem::InvalidLocation;
}

void
FUNCTION ANPCController::SetPerceptionComponent(
    UAIPerceptionComponent& InPerceptionComponent)
{
    if (PerceptionComponent != nullptr)
    {
        UE_VLOG(this, LogAIPerception, Warning, TEXT("Setting perception component while AIController already has one!"));
    }
    PerceptionComponent = &InPerceptionComponent;
}

bool
FUNCTION ANPCController::IsFollowingAPath() const
{
    return (PathFollowingComponent != nullptr) &&
        (PathFollowingComponent->GetStatus() != EPathFollowingStatus::Idle);
}

bool
FUNCTION ANPCController::PerformAction(UPawnAction& Action, UObject* const Insigator)
{
    EAIRequestPriority::Type Stub = EAIRequestPriority::SoftScript;
    return (ActionsComp != nullptr && ActionsComp->PushAction(Action, Stub));
}

// -- Debug/Dev-Time --
FString
FUNCTION ANPCController::GetDebugIcon() const
{
    return TEXT("/Engine/EngineResources/AICON-Green.AICON-Green");
}

void
FUNCTION ANPCController::SetPathFollowingComponent(UPathFollowingComponent* NewPFComponent)
{
    PathFollowingComponent = NewPFComponent;

#if ENABLE_VISUAL_LOG
    if (NewPFComponent)
    {
        REDIRECT_OBJECT_TO_VLOG(NewPFComponent, this);
    }
#endif // ENABLE_VISUAL_LOG
}
