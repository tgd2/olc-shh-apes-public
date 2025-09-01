#pragma once

#include "types.h"
#include "state.h"

#include "geometry.h"

extern state State;

namespace Camera
{
	void CalculateTargetCameraPositionSize(float ElapsedTime)
	{
		if (State.Camera.CameraStateTag == "FIXED")
		{
			State.Camera.TargetWorldPositionSize = State.Camera.CurrentWorldPositionSize;
		}
		else if (State.Camera.CameraStateTag == "TRACK_BODIES")
		{
			if (State.Camera.BodiesToTrack.size() == 0)
			{
				State.Camera.CameraStateTag = "RETURN_TO_START";
				State.Camera.BodiesToTrack.clear();
			}
			else
			{
				rectCorners RectCorners{ {MAX_FLOAT, MAX_FLOAT}, { -MAX_FLOAT, -MAX_FLOAT} };
				b2BodyId FastestBodyId{ State.Camera.BodiesToTrack.at(0) };
				vec2 FastestBodyPosition{ 0.0f };
				vec2 FastestBodyVelocity{ 0.0f };
				float FastestBodySpeedSquared{ -1.0f };

				int CountValidBodies{ 0 };

				for (auto& BodyId : State.Camera.BodiesToTrack)
				{
					if (b2Body_IsValid(BodyId))
					{
						vec2 BodyVelocity{ b2Body_GetLinearVelocity(BodyId) };
						vec2 BodyPosition{ b2Body_GetPosition(BodyId) + 0.5f * BodyVelocity }; // ** Looking forward 0.5 seconds
						float BodySpeedSquared{ b2LengthSquared(BodyVelocity) };

						RectCorners.TopLeft.x = std::min(RectCorners.TopLeft.x, BodyPosition.x);
						RectCorners.TopLeft.y = std::min(RectCorners.TopLeft.y, BodyPosition.y);
						RectCorners.BottomRight.x = std::max(RectCorners.BottomRight.x, BodyPosition.x);
						RectCorners.BottomRight.y = std::max(RectCorners.BottomRight.y, BodyPosition.y);

						if (BodySpeedSquared > FastestBodySpeedSquared)
						{
							FastestBodyId = BodyId;
							FastestBodyPosition = BodyPosition;
							FastestBodyVelocity = BodyVelocity;
							FastestBodySpeedSquared = BodySpeedSquared;
						}

						++CountValidBodies;
					}
				}

				if (CountValidBodies <= 1) // Player is always the first body to track
				{
					State.Camera.CameraStateTag = "RETURN_TO_START";
					State.Camera.BodiesToTrack.clear();
				}
				else if (RectCorners.TopLeft == vec2{ MAX_FLOAT, MAX_FLOAT } && RectCorners.BottomRight == vec2{ -MAX_FLOAT, -MAX_FLOAT })
				{
					State.Camera.CameraStateTag = "RETURN_TO_START";
					State.Camera.BodiesToTrack.clear();
				}
				else
				{
					posSize PosSize{ rectCorners_to_posSize(RectCorners) };
					if (PosSize.Size.y / SCREEN_SIZE.y > PosSize.Size.x / SCREEN_SIZE.x)
					{
						PosSize.Size.x = PosSize.Size.y / SCREEN_SIZE.y * SCREEN_SIZE.x;
					}
					else
					{
						PosSize.Size.y = PosSize.Size.x / SCREEN_SIZE.x * SCREEN_SIZE.y;
					}

					RectCorners = posSize_to_rectCorners(PosSize);
					if (FastestBodyPosition.x < RectCorners.TopLeft.x) PosSize.Size.x -= (RectCorners.TopLeft.x - FastestBodyPosition.x);
					if (FastestBodyPosition.y < RectCorners.TopLeft.y) PosSize.Size.y -= (RectCorners.TopLeft.y - FastestBodyPosition.y);
					if (FastestBodyPosition.x > RectCorners.BottomRight.x) PosSize.Size.x += (FastestBodyPosition.x - RectCorners.BottomRight.x);
					if (FastestBodyPosition.y > RectCorners.BottomRight.y) PosSize.Size.y += (FastestBodyPosition.y - RectCorners.BottomRight.y);

					PosSize.Size = PosSize.Size + 0.25f * vec2{ SCREEN_SIZE.x / TILE_SIZE.x, SCREEN_SIZE.y / TILE_SIZE.y }; // ** To add a margin around the object
					State.Camera.TargetWorldPositionSize = PosSize;
				}
			}
		}
		else if (State.Camera.CameraStateTag == "ZOOM_OUT")
		{
			State.Camera.TargetWorldPositionSize = State.Game.StageRecipe.CameraStartWorldPositionSize;
		}
		else if (State.Camera.CameraStateTag == "RETURN_TO_START")
		{
			State.Camera.TargetWorldPositionSize = State.Camera.StartWorldPositionSize;
		}
		else
		{
			std::cout << "Invalid camera state tag " << State.Camera.CameraStateTag << '\n';
		}
	}

	void UpdateCameraPositionSize(float ElapsedTime)
	{
		if (State.Camera.CameraSpeedTag == "IMMEDIATE")
		{
			State.Camera.CurrentWorldPositionSize = State.Camera.TargetWorldPositionSize;
		}
		else if (State.Camera.CameraSpeedTag == "URGENT" || State.Camera.CameraSpeedTag == "RELAXED")
		{
			float CameraMultiplier = State.Camera.CameraStateTag == "TRACK_BODIES" ? 3.5f : 3.0f; // ** Jam hacky .. using the camera state tag, rather than speed tag here!

			vec2 DistanceToTravel{ State.Camera.TargetWorldPositionSize.Pos - State.Camera.CurrentWorldPositionSize.Pos };
			State.Camera.VelocityPositionSize.Pos = Geometry::MoveTowards2d(State.Camera.VelocityPositionSize.Pos, DistanceToTravel, ElapsedTime * (State.Camera.CameraStateTag == "TRACK_BODIES" ? b2Length(DistanceToTravel) : b2LengthSquared(DistanceToTravel)));

			vec2 SizeToChange{ State.Camera.TargetWorldPositionSize.Size - State.Camera.CurrentWorldPositionSize.Size };
			State.Camera.VelocityPositionSize.Size = Geometry::MoveTowards2d(State.Camera.VelocityPositionSize.Size, SizeToChange, ElapsedTime * (State.Camera.CameraStateTag == "TRACK_BODIES" ? b2Length(SizeToChange) : b2LengthSquared(SizeToChange)));

			State.Camera.CurrentWorldPositionSize.Pos = Geometry::MoveTowards2d(State.Camera.CurrentWorldPositionSize.Pos, State.Camera.CurrentWorldPositionSize.Pos + DistanceToTravel, CameraMultiplier * ElapsedTime * b2Length(DistanceToTravel));
			State.Camera.CurrentWorldPositionSize.Size = Geometry::MoveTowards2d(State.Camera.CurrentWorldPositionSize.Size, State.Camera.CurrentWorldPositionSize.Size + SizeToChange, CameraMultiplier * ElapsedTime * b2Length(SizeToChange));
		}
		else
		{
//			std::cout << "Invalid camera speed tag " << State.Camera.CameraSpeedTag << '\n';
		}
	}

	void Update(float ElapsedTime)
	{
		State.LevelView.SetWorldOffset((State.LevelView.GetWorldOffset() - State.Camera.TransientCumulativeShake));

		State.Camera.CurrentWorldPositionSize.Size = vf2d_to_vec2(State.LevelView.GetWorldVisibleArea());
		State.Camera.CurrentWorldPositionSize.Pos = vf2d_to_vec2(State.LevelView.GetWorldOffset() + 0.5f * State.LevelView.GetWorldVisibleArea());

		CalculateTargetCameraPositionSize(ElapsedTime);
		UpdateCameraPositionSize(ElapsedTime);

		State.LevelView.SetWorldScale(olc::vf2d{ 
			SCREEN_SIZE.x / State.Camera.CurrentWorldPositionSize.Size.x,
			SCREEN_SIZE.y / State.Camera.CurrentWorldPositionSize.Size.y });
		olc::vf2d CurrentCameraSize{ State.LevelView.GetWorldVisibleArea() };

		// Clamp to level size:
		olc::vf2d CurrentCameraTopLeft{ vec2_to_vf2d(State.Camera.CurrentWorldPositionSize.Pos) - 0.5f * CurrentCameraSize };
		CurrentCameraTopLeft.x = std::max(CurrentCameraTopLeft.x, State.Game.Level.LevelCorners.TopLeft.x);
		CurrentCameraTopLeft.y = std::max(CurrentCameraTopLeft.y, State.Game.Level.LevelCorners.TopLeft.y);
		CurrentCameraTopLeft.x = std::min(CurrentCameraTopLeft.x, State.Game.Level.LevelCorners.BottomRight.x - CurrentCameraSize.x);
		CurrentCameraTopLeft.y = std::min(CurrentCameraTopLeft.y, State.Game.Level.LevelCorners.BottomRight.y - CurrentCameraSize.y);

		State.LevelView.SetWorldOffset(CurrentCameraTopLeft + State.Camera.CumulativeShake);
		State.Camera.TransientCumulativeShake = State.Camera.CumulativeShake;
	}

}

