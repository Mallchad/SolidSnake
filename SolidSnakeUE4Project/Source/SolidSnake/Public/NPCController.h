// MIT License
// Comes with no warrenty to speak of

#pragma once

#include "CodeHelpers.h"
#include <UObject/ObjectMacros.h>
#include <UObject/UObjectGlobals.h>
#include <EngineDefines.h>

#include <CoreMinimal.h>
#include <GameFramework/Pawn.h>
#include <GameFramework/Controller.h>
#include <AITypes.h>
#include <NavFilters/NavigationQueryFilter.h>
#include <Actions/PawnAction.h>
#include <Navigation/PathFollowingComponent.h>
#include <Perception/AIPerceptionComponent.h>
#include <Perception/AIPerceptionListenerInterface.h>
#include <VisualLogger/VisualLoggerDebugSnapshotInterface.h>
// generated must be included for all UObject files
#include "NPCController.generated.h"

FWD_CLASS UPawnAction;
FWD_CLASS UPawnActionsComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAIMoveCompletedSignature, FAIRequestID, RequestID, EPathFollowingResult::Type, Result);

struct FFocusInfo
{
    struct FFocusItem
    {
        TWeakObjectPtr<AActor> Actor;
        FVector Position;

        FFocusItem()
        {
            Actor = nullptr;
            Position = FAISystem::InvalidLocation;
        }
    };

    // An array of priorities isn't needed, just a few items

    /** Where has been decided to go */
    FFocusItem TargetFocus;

    /** Where we need to look/go right now */
    FFocusItem AlertFocus;

    /** If AlertFocus is he current target */
    bool bFollowingAlert;

};

/**
 *
 * NPCController is a stripped back reimplimentation of AIController
 *
 * Controllers are non-physical actors that can be attached to a pawn to control its actions.
 * AIControllers manage the artificial intelligence for the pawns they control.
 * In networked games, they only exist on the server.
 *
 * @see https://docs.unrealengine.com/latest/INT/Gameplay/Framework/Controller/
 */
UCLASS(ClassGroup = AI, BlueprintType, Blueprintable)
DECLARE class ANPCController final :
    public AController,
    public IAIPerceptionListenerInterface,
    public IVisualLoggerDebugSnapshotInterface
{
    GENERATED_BODY()

    FFocusInfo FocusInformation;

protected: // internal data
    /** By default AI's logic does not start when controlled Pawn is possessed.
        Setting this flag to true will make AI logic start when pawn is possessed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NPC)
    uint32 bStartAILogicOnPossess : 1; DATA

                                       /** By default AI's logic gets stopped when controlled Pawn is unpossessed.
                                        * Setting this flag to false will make AI logic persist past losing
                                        * control over a pawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NPC)
    uint32 bStopAILogicOnUnposses : 1; DATA

    public: // public data
    /** used for alternating LineOfSight traces */
    UPROPERTY()
    mutable uint32 bLOSflag : 1; DATA

                                 /** Skip extra line of sight traces to extremities of target being checked. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NPC)
    uint32 bSkipExtraLOSChecks : 1; DATA

                                    /** Is strafing allowed during movement? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NPC)
    uint32 bAllowStrafe : 1; DATA

                             /** Specifies if this AI wants its own PlayerState. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NPC)
    uint32 bWantsPlayerState : 1; DATA

                                  /** Copy Pawn rotation to ControlRotation, if there is no focus point. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NPC)
    uint32 bSetControlRotationFromPawnOrientation:1; DATA

    UPROPERTY(BlueprintReadWrite, Category = NPC)
    UAIPerceptionComponent* PerceptionComponent; DATA

                                                 /** Blueprint notification that we've completed the current movement request */
    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "MoveCompleted"))
    FAIMoveCompletedSignature ReceiveMoveCompleted;

protected: // Internal data
    UPROPERTY(VisibleDefaultsOnly, Category = NPC)
    UPathFollowingComponent* PathFollowingComponent; DATA

    UPROPERTY(BlueprintReadOnly, Category = NPC, meta = (AllowPrivateAccess = "true"))
    UPawnActionsComponent* ActionsComp;

    TSubclassOf<UNavigationQueryFilter> DefaultNavigationFilterClass;

    // FUNCTIONS
public:
    ANPCController(
        const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual void SetPawn(APawn* InPawn) override;


    /** Makes AI go toward specified Goal actor (destination will be continuously updated), aborts any active path following
     *  @param AcceptanceRadius - finish move if pawn gets close enough
     *  @param bStopOnOverlap - add pawn's radius to AcceptanceRadius
     *  @param bUsePathfinding - use navigation data to calculate path (otherwise it will go in straight line)
     *  @param bCanStrafe - set focus related flag: bAllowStrafe
     *  @param bAllowPartialPath - use incomplete path when goal can't be reached
     *  @note AcceptanceRadius has default value or -1 due to Header Parser not being able to recognize UPathFollowingComponent::DefaultAcceptanceRadius
     */
    UFUNCTION(BlueprintCallable, Category = "AI|Navigation", Meta = (AdvancedDisplay = "bStopOnOverlap,bCanStrafe,bAllowPartialPath"))
    EPathFollowingRequestResult::Type
    MoveToActor(AActor* Goal,
                float AcceptanceRadius = -1,
                bool bStopOnOverlap = true,
                bool bUsePathfinding = true,
                bool bCanStrafe = true,
                bool bAllowPartialPath = true);

    /** Makes AI go toward specified Dest location, aborts any active path following
     *  @param AcceptanceRadius - finish move if pawn gets close enough
     *  @param bStopOnOverlap - add pawn's radius to AcceptanceRadius
     *  @param bUsePathfinding - use navigation data to calculate path (otherwise it will go in straight line)
     *  @param bProjectDestinationToNavigation - project location on navigation data before using it
     *  @param bCanStrafe - set focus related flag: bAllowStrafe
     *  @param bAllowPartialPath - use incomplete path when goal can't be reached
     *  @note AcceptanceRadius has default value or -1 due to Header Parser not being able to recognize UPathFollowingComponent::DefaultAcceptanceRadius
     */
    UFUNCTION(BlueprintCallable, Category = "NPC|Navigation", Meta = (AdvancedDisplay = "bStopOnOverlap,bCanStrafe,bAllowPartialPath"))
    EPathFollowingRequestResult::Type
    FUNCTION MoveToLocation(const FVector& Dest,
                            float AcceptanceRadius = -1,
                            bool bStopOnOverlap = true,
                            bool bUsePathfinding = true,
                            bool bProjectDestinationToNavigation = false,
                            bool bCanStrafe = true,
                            bool bAllowPartialPath = true);

	/** Passes move request and path object to path following */
	virtual FAIRequestID
    FUNCTION RequestMove(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr Path);

    /** Makes AI go toward specified destination
     *  @param MoveRequest - details about move
     *  @param OutPath - optional output param, filled in with assigned path
     *  @return struct holding MoveId and enum code
     */
    virtual FPathFollowingRequestResult
    FUNCTION MoveTo(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath = nullptr);

/** Finds path for given move request
 *  @param MoveRequest - details about move
 *  @param Query - pathfinding query for navigation system
 *  @param OutPath - generated path
 */
    void
    FUNCTION FindPathForMoveRequest(const FAIMoveRequest& MoveRequest,
                                    FPathFindingQuery& Query,
                                    FNavPathSharedPtr& OutPath) const;

    /** Helper function for creating pathfinding query for this agent from move request data */
    bool
    FUNCTION BuildPathFindingQuery(const FAIMoveRequest& MoveRequest, FPathFindingQuery& Query) const;

    /** if AI is currently moving due to request given by RequestToPause, then the move will be paused */
    bool
    FUNCTION PauseMove(FAIRequestID RequestToPause);

    /** resumes last AI-performed, paused request provided it's ID was equivalent to RequestToResume */
    bool
    FUNCTION ResumeMove(FAIRequestID RequestToResume);

    /** Aborts the move the controller is currently performing */
    virtual void
    FUNCTION StopMovement() override;

    /** Called on completing current movement request */
    virtual void
    FUNCTION OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);

    /** Returns the Move Request ID for the current move */
    FORCEINLINE FAIRequestID
    FUNCTION GetCurrentMoveRequestID() const { return GetPathFollowingComponent() ? GetPathFollowingComponent()->GetCurrentRequestId() : FAIRequestID::InvalidRequest; }

    /** Returns status of path following */
    UFUNCTION(BlueprintCallable, Category = "NPC|Navigation") EPathFollowingStatus::Type
    FUNCTION GetMoveStatus() const;

    /** Returns true if the current PathFollowingComponent's path is partial (does not reach desired destination). */
    UFUNCTION(BlueprintCallable, Category = "NPC|Navigation")
    bool HasPartialPath() const;

    /** Returns position of current path segment's end. */
    UFUNCTION(BlueprintCallable, Category = "NPC|Navigation")
    FVector GetImmediateMoveDestination() const;

    /** Updates state of movement block detection. */
    UFUNCTION(BlueprintCallable, Category = "NPC|Navigation")
    FUNCTION void SetMoveBlockDetection(bool bEnable);

    /** Retrieve the final position that controller should be looking at. */
    UFUNCTION(BlueprintCallable, Category = "NPC")
    FUNCTION FVector GetFocalPoint() const;

    /** Retrieve the focal point this controller should focus to on given actor. */
    UFUNCTION(BlueprintCallable, Category = "NPC")
    FUNCTION virtual FVector GetFocalPointOnActor(const AActor *Actor) const;

    /** Set the position that controller should be looking at. */
    UFUNCTION(BlueprintCallable, Category = "NPC", meta = (DisplayName = "SetFocalPoint", ScriptName = "SetFocalPoint", Keywords = "focus"))
    void
    FUNCTION K2_SetFocalPoint(FVector FP);

    /** Set Focus for actor, will set FocalPoint as a result. */
    UFUNCTION(BlueprintCallable, Category = "NPC", meta = (DisplayName = "SetFocus", ScriptName = "SetFocus"))
    FUNCTION void K2_SetFocus(AActor* NewFocus);

    /** Get the focused actor. */
    UFUNCTION(BlueprintCallable, Category = "NPC")
    AActor*
    FUNCTION GetFocusActor() const;

    /** Clears Focus, will also clear FocalPoint as a result */
    UFUNCTION(BlueprintCallable, Category = "NPC", meta = (DisplayName = "ClearFocus", ScriptName = "ClearFocus"))
    void
    FUNCTION K2_ClearFocus();

    // -- Begin AActor Interface --
    /// Runs once when both the game is started and the actor enters the world
    void
    FUNCTION BeginPlay() override;

    /// Runs once per game update
    virtual void
    FUNCTION Tick(float DeltaTime) override;

    virtual void
    FUNCTION PostInitializeComponents() override;
    virtual void
    FUNCTION PostRegisterAllComponents() override;
    // --End AActor Interface --

    // -- Begin AController Interface --
protected:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

public:
    virtual bool ShouldPostponePathUpdates() const override;
    virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;

#if ENABLE_VISUAL_LOG
    virtual void GrabDebugSnapshot(FVisualLogEntry* Snapshot) const override;
#endif

    virtual void Reset() override;

    /**
     * Checks line to center and top of other actor
     * @param Other is the actor whose visibility is being checked.
     * @param ViewPoint is eye position visibility is being checked from.  If vect(0,0,0) passed in, uses current viewtarget's eye position.
     * @param bAlternateChecks used only in AIController implementation
     * @return true if controller's pawn can see Other actor.
     */
    virtual bool LineOfSightTo(const AActor* Other, FVector ViewPoint = FVector(ForceInit), bool bAlternateChecks = false) const override;
    // -- End AController Interface --

    /** Notifies AIController of changes in given actors' perception */
    virtual void ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    /** Update direction AI is looking based on FocalPoint */
    virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn = true);

    /** Set FocalPoint as absolute position or offset from base. */
    virtual void SetFocalPoint(FVector NewFocus);

    /* Set Focus actor, will set FocalPoint as a result. */
    virtual void SetFocus(AActor* NewFocus);

    /** Clears Focus, will also clear FocalPoint as a result */
    virtual void ClearFocus();

    void SetPerceptionComponent(UAIPerceptionComponent& InPerceptionComponent);

    /// IAIPerceptionListenerInterface
    virtual UAIPerceptionComponent* GetPerceptionComponent() override
    { return GetAIPerceptionComponent(); }

    /// INavAgentInterface
    virtual bool IsFollowingAPath() const override;
    virtual IPathFollowingAgentInterface* GetPathFollowingAgent() const override { return PathFollowingComponent; }

	// Actions
	bool PerformAction(UPawnAction& Action, UObject* const Instigator = NULL);

	// debug/dev-time
	virtual FString GetDebugIcon() const;

	// Cheat/debugging functions
	static void ToggleAIIgnorePlayers() { bAIIgnorePlayers = !bAIIgnorePlayers; }
	static bool AreAIIgnoringPlayers() { return bAIIgnorePlayers; }

	/** If true, AI controllers will ignore players. */
	static bool bAIIgnorePlayers;

	/** Returns PathFollowingComponent subobject **/
	UFUNCTION(BlueprintCallable, Category="AI|Navigation")
	UPathFollowingComponent* GetPathFollowingComponent() const { return PathFollowingComponent; }
	/** Returns ActionsComp subobject **/
	UPawnActionsComponent* GetActionsComp() const { return ActionsComp; }
	UFUNCTION(BlueprintPure, Category = "NPC|Perception")
	UAIPerceptionComponent* GetAIPerceptionComponent() { return PerceptionComponent; }

	const UAIPerceptionComponent* GetAIPerceptionComponent() const { return PerceptionComponent; }

	/** Note that his function does not do any pathfollowing state transfer.
	 *	Intended to be called as part of initialization/setup process */
	UFUNCTION(BlueprintCallable, Category = "NPC|Navigation")
	void SetPathFollowingComponent(UPathFollowingComponent* NewPFComponent);


protected: // internal functions
};

/*
  Footnotes
  This reimplimentation will go through a lot of divergence from the original
  AAIController class, here some of the choices will be expanded upon.

  No depreceated functions or variables will be used.

  Inherted class 'IGameplayTaskOwnerInterface': Its not clear why anybody should
  about this, it appears to primarily drive a blueprint-like task system, which is
  easilly rendererd obsolete by a little bit of programming.
  This also means any and all task and blackboard related members will be rejected.

  Inherted class 'IGenericTeamAgentInterface': Describes AI response to "teams",
  basically unimplimented, waste of time if not for compatbility.

  UNavigationQueryFilter is never used, it's not apartent why it would be useful.

*/
