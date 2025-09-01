#pragma once

#include "types.h"
#include "state.h"

extern state State;

namespace Collisions
{
    // ============================================================================

    void ProcessContact_AtoB(b2ShapeId ShapeIdA, b2ShapeId ShapeIdB, bool bIsBegin)
    {
        if (!bIsBegin) return;

        b2BodyId BodyIdA{ b2Shape_GetBody(ShapeIdA) };
        b2BodyId BodyIdB{ b2Shape_GetBody(ShapeIdB) };

        bodyUserData* UserDataA = static_cast<bodyUserData*>(b2Body_GetUserData(BodyIdA));
        bodyUserData* UserDataB = static_cast<bodyUserData*>(b2Body_GetUserData(BodyIdB));

        // ** Jam hacky - this could be refactored to be much simpler

        if (UserDataA->ObjectClass == "PROJECTILE")
        {
            if (UserDataB->ObjectClass == "GIBBON")
            {
                UserDataB->CollisionMessages.insert({
                    (uintptr_t)UserDataA,
                    collisionMessage{
                    .OtherObjectClass = "PROJECTILE",
                    .OtherObjectValue = UserDataA->Value
                    }
                    }
                );
            }
            else if (UserDataB->ObjectClass == "PIN")
            {
                UserDataB->CollisionMessages.insert({
                    (uintptr_t)UserDataA,
                    collisionMessage{
                    .OtherObjectClass = "PROJECTILE",
                    .OtherObjectValue = UserDataA->Value
                    }
                    }
                );
            }
        }
        else if (UserDataA->ObjectClass == "GIBBON")
        {
            if (UserDataB->ObjectClass == "PROJECTILE")
            {
                UserDataB->CollisionMessages.insert({
                    (uintptr_t)UserDataA,
                    collisionMessage{
                    .OtherObjectClass = "GIBBON",
                    .OtherObjectValue = UserDataA->Value
                    }
                    }
                );
            }
        }
        else if (UserDataA->ObjectClass == "PIN")
        {
            if (UserDataB->ObjectClass == "PROJECTILE")
            {
                UserDataB->CollisionMessages.insert({
                    (uintptr_t)UserDataA,
                    collisionMessage{
                    .OtherObjectClass = "PIN",
                    .OtherObjectValue = UserDataA->Value
                    }
                    }
                );
            }
        }
        else if (UserDataA->ObjectClass == "GROUND")
        {
            if (UserDataB->ObjectClass == "PROJECTILE")
            {
                UserDataB->CollisionMessages.insert({
                    (uintptr_t)UserDataA,
                    collisionMessage{
                    .OtherObjectClass = "GROUND",
                    .SpeedOfCollision = 0.0f, // ** To add speed measure
                    .OtherObjectValue = UserDataA->Value
                    }
                    }
                );
            }
        }
    }

    // ============================================================================

    void ProcessSensor(b2ShapeId SensorShapeId, b2ShapeId VisitorShapeId, bool bIsBegin)
    {
        b2BodyId SensorBodyId = b2Shape_GetBody(SensorShapeId);
        b2BodyId VisitorBodyId = b2Shape_GetBody(VisitorShapeId);

        bodyUserData* SensorUserData = static_cast<bodyUserData*>(b2Body_GetUserData(SensorBodyId));
        bodyUserData* VisitorUserData = static_cast<bodyUserData*>(b2Body_GetUserData(VisitorBodyId));

        if (!SensorUserData || !VisitorUserData)
        {
            std::cout << "Missing BodyUserData \n";
        }
        else
        {
            if (VisitorUserData->ObjectClass == "PROJECTILE")
            {
                if (SensorUserData->ObjectClass == "GRAVITY_FIELD")
                {
                    if (VisitorUserData->CollisionMessages.contains((uintptr_t)VisitorUserData))
                    {
                        VisitorUserData->CollisionMessages.at((uintptr_t)VisitorUserData).OtherObjectValue2 += (bIsBegin ? 1 : -1);
                    }
                    else
                    {
                        VisitorUserData->CollisionMessages.insert({
                            (uintptr_t)VisitorUserData,
                            collisionMessage{
                            .OtherObjectClass = "GRAVITY_FIELD",
                            .OtherObjectValue = SensorUserData->Value,
                            .OtherObjectValue2 = (bIsBegin ? 1 : -1),
                            }
                            }
                        );
                    }
                }
            }
        }
    }

    // ============================================================================

    void ProcessCollisions()
    {
        b2ContactEvents ContactEvents = b2World_GetContactEvents(State.World);
        b2SensorEvents SensorEvents = b2World_GetSensorEvents(State.World);

        for (int i = 0; i < ContactEvents.beginCount; ++i)
        {
            b2ContactBeginTouchEvent BeginEvent = ContactEvents.beginEvents[i];
            ProcessContact_AtoB(BeginEvent.shapeIdA, BeginEvent.shapeIdB, true);
            ProcessContact_AtoB(BeginEvent.shapeIdB, BeginEvent.shapeIdA, true);
        }

        for (int i = 0; i < ContactEvents.endCount; ++i)
        {
            b2ContactEndTouchEvent EndEvent = ContactEvents.endEvents[i];
            ProcessContact_AtoB(EndEvent.shapeIdA, EndEvent.shapeIdB, false);
            ProcessContact_AtoB(EndEvent.shapeIdB, EndEvent.shapeIdA, false);
        }

        for (int i = 0; i < SensorEvents.beginCount; ++i)
        {
            b2SensorBeginTouchEvent BeginEvent = SensorEvents.beginEvents[i];
            ProcessSensor(BeginEvent.sensorShapeId, BeginEvent.visitorShapeId, true);
        }

        for (int i = 0; i < SensorEvents.endCount; ++i)
        {
            b2SensorEndTouchEvent EndEvent = SensorEvents.endEvents[i];
            ProcessSensor(EndEvent.sensorShapeId, EndEvent.visitorShapeId, false);
        }
    }

    // ============================================================================

};
