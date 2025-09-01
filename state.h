#pragma once

#include "types.h"

// ================================================================================================

struct poiRecipe
{
	int Id{ -1 };
	std::string Name = { "" };
	std::string Class = { "" };
	vec2 Position{ 0.0f, 0.0f };
};

struct movementRecipe
{
	vec2 Move{ 0.0f, 0.0f };
	float Rotation{ 0.0f };
	float Time{ 0.0f };
	easingId EasingId{ easingId::LINEAR };
	std::string ActionsAtEndOfMove{ "NONE" };
};

struct objectRecipe
{
	int Id{ -1 };
	std::string Name = { "" };
	std::string Class = { "" };
	std::string Filename = { "" };
	vec2 Position{ 0.0f, 0.0f };
	vec2 Velocity{ 0.0f, 0.0f };
	vec2 Size{ 0.0f, 0.0f };
	bool bHorizontalFlip = false;
	float Rotation = 0.0f;
	int PivotPoiId{ -1 };
	std::vector<movementRecipe> MovementRecipes{};
	vec2 ParallaxFactor{ 1.0f, 1.0f };
	int Value{ 1 }; // Eg, Projectile strength or gibbon appetite
};

struct platformRecipe
{
	int Id{ -1 };
	std::string Name = { "" };
	std::string Class = { "" };
	std::vector<vec2> Vertices{};
	bool bIsGroundTopper{ false };
};

struct colliderRecipe
{
	struct polygon { std::vector<vec2> Vertices; }; // Expressed as % Size, relative to centre
	struct circle { vec2 Position; float Radius; }; // Expressed as % Size, relative to centre
	std::vector<polygon> Polygons{};
	std::vector<circle> Circles{};
};

struct levelRecipe
{
	std::string Filename{ "" };
	vec2 Size{ 0.0f, 0.0f };
	std::string DefaultPlatformClass{ "" };
	std::vector<poiRecipe> PoiRecipes{};
	std::vector<objectRecipe> ObjectRecipes{};
	std::vector<platformRecipe> PlatformRecipes{};
	std::vector<objectRecipe> BackgroundObjectRecipes{};
	std::vector<objectRecipe> ForegroundObjectRecipes{};
	std::unordered_map<std::string, colliderRecipe> ColliderRecipes{};
	posSize CameraStartWorldPositionSize{ {0.0f, 0.0f}, {SCREEN_SIZE.x / TILE_SIZE.x, SCREEN_SIZE.y / TILE_SIZE.y} };
	
};

struct particleRecipe
{
	int ParticleCount{ 1 };
	std::string ParticleTag{ "DOT" };
	vec2 SourcePosition{ 0.0f, 0.0f };
	vec2 SourceVelocity{ 0.0f, 0.0f };
	float VelocityDampening{ 0.0f };
	lowHigh Speed{ 0.0f, 0.0f };
	lowHigh MovementAngle{ 0.0f, 0.0f };
	lowHigh RotationStartingAngle{ 0.0f, 0.0f };
	lowHigh RotationSpeed{ 0.0f, 0.0f };
	float RotationDampening{};
	olc::Pixel Tint{ olc::WHITE };
	std::string TintTag{ "FIXED" };
	vec2 WorldSize{ 1.0f, 1.0f };
	lowHigh TimeBeforeDeletion{ 0.0f, 0.0f };
};

// ================================================================================================

struct mipLevel
{
	vec2 FrameSize{ 1.0f, 1.0f };
	std::string Filename{};
	std::shared_ptr<olc::Renderable> Gfx{};
};

struct advancedGfx
{
	std::vector<mipLevel> MipMap{};
};

// ================================================================================================

struct collisionMessage
{
	std::string OtherObjectClass{ "" }; // ** Move to enum class later?
	float SpeedOfCollision{ 0.0f };
	int OtherObjectValue{ 0 }; // Eg, number of bananas
	int OtherObjectValue2{ 0 }; // Eg, net number of overlaps
};

// ================================================================================================

struct bodyUserData
{
	std::string ObjectClass{ "" };
	std::unordered_map<uintptr_t, collisionMessage>CollisionMessages{};
	int Value{ 0 }; // Eg, Projectile strength or gibbon appetite
};

struct objectState
{
	std::string Name{ "" };
	std::string Class{ "" }; // ** Move to enum class later?
	b2BodyId Body{ b2_nullBodyId };
	bodyUserData* BodyUserData{};

	vec2 WorldPosition{ 0.0f, 0.0f };
	vec2 WorldVelocity{ 0.0f, 0.0f };
	vec2 WorldSize{ 0.0f, 0.0f };
	float WorldRotation = 0.0f;

	vec2 SourceCentre{ 0.0f, 0.0f };
	vec2 ParallaxFactor{ 1.0f, 1.0f };

	advancedGfx AdvancedGfx{};
	std::string AnimationTag{ "IDLE" };
	int AnimationFrameIndex{ 0 };
	float TargetFrameTime{ 1.0f };
	float CurrentFrameTime{ 0.0f };
	bool bEndFrameEarly{ false };
	bool HFlip{ false }; // ** Only flips graphics, not physics

	// For player:
	int ControllerIndex{ 0 }; // ** Will need to update if add multiplayer
	float bAimCooldownRemaining{ 0.0f };
	float CurrentAimAngle{ 0.5f * PI };
	float CurrentAimWobble{ 0.0f };
	std::pair<float, float> AimBoundsMinMax{-1.0f * PI, 1.0f * PI}; // {0.1f * PI, 0.9f * PI};
	bool bLoopAimBounds{ true }; // false
	bool bHasButtonAirGapBeforeThrowing{ false };
	bool bPreparingThrow{ false };
	double TimeStartedPreparingToThrow{ 0.0 };
	float ThrowStengthAsProportionMaximum{ 0.0f };

	// For player upgrades:
	float ThrowSpeedMultiple{ 1.0f };
	float AimRotationSpeedMultiple{ 1.0f };
	float AimCooldownMultiple{ 1.0f };
	float TimeAtMaxStrengthBeforeWobble{ 0.1f };

	// For gibbons:
	int GibbonTotalAppetite{ 1 }; // ** Needed?
	int GibbonCurrentAppetite{ 1 };
	int GibbonBananasEaten{ 0 };
	int GibbonBananasInHand{ 0 };
	int GibbonBananasDiscarded{ 0 };
	bool bIsBaby{ false };

	// For projectiles:
	bool bThisProjectileJustThrown{ true };
	int ProjectileStrength{ 1 };
	int CountGravityFieldOverlaps{ false };
	vec2 RecentPosition{ ZERO };
	double TimeLastAtRecentPosition{ 0.0 };
	bool bIsProjectileStillMoving{ true };

	// For level objects
	int GravityFieldDirectionIndex{ 0 };
	std::string PinTag{ "NORMAL" };

	// For kinetic movements:
	struct movementLeg
	{
		float StartTime{ 0.0f };
		float LegTime{ 0.0f };
		vec2 StartPosition{ 0.0f, 0.0f };
		vec2 Velocity{ 0.0f, 0.0f };
		float StartRotation{ 0.0f };
		float RotationSpeed{ 0.0f };
		easingId EasingId{ easingId::LINEAR };
	};
	vec2 StartingWorldPosition{ 0.0f, 0.0f };
	float KineticMovementTimer{ 0.0f };
	int CurrentMovementLegIndex{ 0 };
	std::vector<movementLeg> KineticMovementLegs;

	bool bDestroyOnNextUpdate{ false };
};

struct platformState
{
	std::string Name{ "" };
	std::string Class{ "" }; // ** Move to enum class later?
	b2BodyId Body{ b2_nullBodyId };
	std::shared_ptr<olc::Renderable> Gfx{};
	std::vector<collisionMessage> CollisionMessages{};
	bodyUserData* BodyUserData{}; // **

	vec2 WorldPosition{ 0.0f, 0.0f }; // ** ??
	std::vector<vec2> Vertices{}; // ** ??
	olc::Pixel Tint{ olc::WHITE };
};

struct particleState
{
	vec2 Position{ 0.0f, 0.0f };
	vec2 Velocity{ 0.0f, 0.0f };
	float VelocityDampening{};

	float RotationAngle{};
	float RotationSpeed{};
	float RotationDampening{};

	olc::vi2d Frame{ 0, 0 };
	olc::Pixel Tint{ olc::WHITE };
	vec2 WorldSize{ 1.0f, 1.0f };
	float TimeRemaining{ 0.0f };
};

// ================================================================================================

struct state
{
	olc::ResourcePack* AssetPack;

	// Stage:
	olc::PixelGameEngine* PGE{};
	std::map<std::string, std::shared_ptr<olc::Renderable>> GfxMap;
	
	olc::MiniAudio* MiniAudio{ nullptr };
	std::vector<std::string> WavFilenames;
	std::vector<std::pair<float, double>> WavsToPlay;
	std::map<std::string, unsigned int> Sounds;
	unsigned int MusicInstanceId{ 999 }; // ** - Hardcoding

	double CumulativeTime{ 0.0 };
	b2WorldId World;
	olc::TileTransformedView LevelView{ vec2_to_vf2d(SCREEN_SIZE), vec2_to_vf2d(TILE_SIZE) };
	struct camera
	{
		std::vector<b2BodyId> BodiesToTrack{};
		posSize StartWorldPositionSize{ {0.0f, 0.0f}, {SCREEN_SIZE.x / TILE_SIZE.x, SCREEN_SIZE.y / TILE_SIZE.y} };
		posSize TargetWorldPositionSize{ StartWorldPositionSize };
		posSize CurrentWorldPositionSize{ StartWorldPositionSize };
		posSize VelocityPositionSize{ StartWorldPositionSize };
		std::string CameraStateTag{ "RETURN_TO_START" };
		std::string CameraSpeedTag{ "IMMEDIATE" };

		// Camera shake:
		struct shakeComponent
		{
			olc::vf2d MaximumOffset{ 0.0f, 0.0f };
			float Frequency{ 0.0f };
			float ProportionReductionRate{ 1.0f };
			float ProportionRemaining{ 1.0f };
		};
		std::vector<shakeComponent> ShakeComponents;
		olc::vf2d CumulativeShake{ 0.0f, 0.0f };
		olc::vf2d TransientCumulativeShake{ 0.0f, 0.0f };
	};
	camera Camera{};

	// Jam hacky .. this really shouldn't be here - START
	bool bShowAppMessage{ false };
	std::string AppMessage{""};
	float AppMessageScale{ 1.0f };

	void ShowAppMessage(std::string NewMessage, float Scale)
	{
		bShowAppMessage = true;
		AppMessage = NewMessage;
		AppMessageScale = Scale;
	}
	void HideAppMessage()
	{
		bShowAppMessage = false;
	}
	// Jam hacky .. this really shouldn't be here - END

	std::vector<debugLine> DebugLines{};
	bool bDebugDraw{ false };

	// **************************************************************************************************************

	appStateId AppStateId{ appStateId::MENU };
	double TimeCurrentAppStateStarted{ 0.0 };
	bool bJustEnteredNewAppState{ true };
	bool bIsFirstFrameInNewAppState{ true };

	std::vector<olc::GamePad*> GamePads{};
	std::vector<playerInputs> PlayerInputs{ {} };

	// **************************************************************************************************************

	struct gameState
	{
		gameStateId GameStateId{ gameStateId::START_NEW_GAME };
		double TimeCurrentGameStateStarted{ 0 };
		bool bJustEnteredNewGameState{ true };
		bool bIsFirstFrameInNewGameState{ true };

		bool bIsPaused{ false };
		bool bIsReturnToTitleMenu{ false };


		int CurrentLevelIndex{ 0 };
		int StartingLevelIndex{ 0 };
		int TotalLevelIndex{ 0 };
		int TimesLevelsWrappedAround{ 0 };

		levelRecipe StageRecipe{};
		levelRecipe LevelRecipe{};

		struct levelState
		{
			rectCorners LevelCorners{ -SCREEN_SIZE, SCREEN_SIZE };

			std::vector<objectState> BackgroundObjects{};
			std::vector<objectState> Objects{};
			std::vector<particleState> Particles{};
			std::vector<platformState> Platforms{};
			std::vector<objectState> ForegroundObjects{};

			std::vector<objectRecipe> NewObjectRecipes{};
			std::vector<particleRecipe> NewParticleRecipes{};

			float CurrentVolume{ 0.0f };
			float MaxVolume{ 350.0f };
			float VolumeFlatIncreaseRate{ 2.0f };
			float VolumeIncreaseRatePerActiveGibbon{ 1.0f };
			float VolumeDecreasePerHitGibbon{ 50.0f };

			double TimeLastMovingBananaSpotted{ 0.0 };
			bool bUpdateActiveGibbonCountOnNextFrame{ true };
			int GibbonCallsRemainingBeforeSfx{ 1 };
			int ActiveGibbonCount{ 0 };
			vec2 PositionInstructions3{ZERO};
		};
		levelState Level{};
	};
	gameState Game{};

	// **************************************************************************************************************

	void Create(olc::PixelGameEngine* PGE_)
	{
		PGE = PGE_;
		srand(time(0));

		b2WorldDef WorldDef = b2DefaultWorldDef();
		WorldDef.gravity = { 0.0f, 0.0f };
		World = b2CreateWorld(&WorldDef);

		LevelView.SetWorldOffset(-0.5f * olc::vf2d{ SCREEN_WIDTH / TILE_WIDTH, SCREEN_HEIGHT / TILE_HEIGHT });
		LevelView.SetScaleExtents(0.4f * vec2_to_vf2d(TILE_SIZE), 3.0f * vec2_to_vf2d(TILE_SIZE));
		LevelView.EnableScaleClamp(true);
	}

	void Destroy()
	{
		b2DestroyWorld(World);
	}
};

// ================================================================================================