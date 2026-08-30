#include "pch.hpp"
#include "RayCastingSystem.hpp"
#include "Components.hpp"

#include "GameScene.hpp"
#include "AssetsLoads.hpp"
#include "Map.hpp"



ObjectRayHit RayCastingSystem::objectRays[GFX::NUM_RAYS];
ObjectRayHit RayCastingSystem::middleRay;
std::vector<Entity*> RayCastingSystem::renderDotEnsOrdered;


bool RayCastingSystem::intersectBox(
    const Vector2f& origin,
    const Vector2f& dir,
    const SDL_FRect& rect,
    ObjectRayHit& outHit)
{
    const float EPS = 1e-6f;

    // tmin = the closest distance where the ray enters the box
    // tmax = the furthest distance where the ray leaves the box
    float tmin = -FLT_MAX;
    float tmax = FLT_MAX;

    // Keeps track of which side of the box was hit first.
    // This is needed later for texture mapping.
    BoxFace enterFace = FACE_LEFT;


    // -------------------------------------------------
    // X SLAB TEST
    //
    // A rectangle has an X range:
    //
    // rect.x ---------------- rect.x + rect.w
    //
    // We calculate when the ray enters and leaves this range.
    // -------------------------------------------------

    if (fabs(dir.x) < EPS)
    {
        // Ray is parallel to X sides.
        // If the origin is outside the X range,
        // it can never hit the box.
        if (origin.x < rect.x || origin.x > rect.x + rect.w)
            return false;
    }
    else
    {
        // Calculate distance to left and right sides.
        //
        // ray position:
        // x = origin.x + dir.x * t
        //
        // solve for t:
        // t = (side - origin.x) / dir.x

        float tx1 = (rect.x - origin.x) / dir.x;
        float tx2 = (rect.x + rect.w - origin.x) / dir.x;


        // These are the faces the ray hits when entering/leaving X.
        BoxFace face1 = FACE_LEFT;
        BoxFace face2 = FACE_RIGHT;


        // If the ray is moving backwards,
        // swap so tx1 is always the entry point.
        if (tx1 > tx2)
        {   
            std::swap(tx1, tx2);
            std::swap(face1, face2);
        }


        // The largest entry distance is the actual entry point.
        if (tx1 > tmin)
        {
            tmin = tx1;
            enterFace = face1;
        }


        // The smallest exit distance is the actual exit point.
        tmax = std::min(tmax, tx2);


        // If entry happens after exit,
        // the ray misses the box.
        if (tmin > tmax)
            return false;
    }



    // -------------------------------------------------
    // Y SLAB TEST
    //
    // Same idea as X, but for:
    //
    // rect.y ---------------- rect.y + rect.h
    //
    // -------------------------------------------------

    if (fabs(dir.y) < EPS)
    {
        // Ray is parallel to Y sides.
        // Outside Y range means no intersection.
        if (origin.y < rect.y || origin.y > rect.y + rect.h)
            return false;
    }
    else
    {
        // Calculate distance to top and bottom sides.
        float ty1 = (rect.y - origin.y) / dir.y;
        float ty2 = (rect.y + rect.h - origin.y) / dir.y;


        BoxFace face1 = FACE_TOP;
        BoxFace face2 = FACE_BOTTOM;


        // Make ty1 the entry distance.
        if (ty1 > ty2)
        {
            std::swap(ty1, ty2);
            std::swap(face1, face2);
        }


        // Update closest entry point.
        if (ty1 > tmin)
        {
            tmin = ty1;
            enterFace = face1;
        }


        // Update furthest exit point.
        tmax = std::min(tmax, ty2);


        // No overlap between X and Y intervals.
        if (tmin > tmax)
            return false;
    }



    // If the intersection is behind the player,
    // it is not visible.
    if (tmin < 0.0f)
        return false;



    // Calculate the exact hit position.
    Vector2f hit =
    {
        origin.x + dir.x * tmin,
        origin.y + dir.y * tmin
    };


    outHit.rayHit.hit = true;
    outHit.rayHit.dist = tmin;
    outHit.face = enterFace;



    // -------------------------------------------------
    // TEXTURE COORDINATE
    //
    // Depending on which face was hit,
    // use the other axis to find the texture position.
    //
    // Example:
    //
    // Hit LEFT/RIGHT face:
    //
    // +---------+
    // |         |
    // |    X    |  <- use Y position
    // |         |
    // +---------+
    //
    // Hit TOP/BOTTOM face:
    //
    // +----X----+
    // |         |
    // +---------+
    //      ^
    //    use X
    //
    // -------------------------------------------------

    switch (enterFace)
    {
    case FACE_LEFT:
    case FACE_RIGHT:

        // Convert hit Y position into 0..1 range
        outHit.rayHit.texOffset =
            (hit.y - rect.y) / rect.h;

        break;


    case FACE_TOP:
    case FACE_BOTTOM:

        // Convert hit X position into 0..1 range
        outHit.rayHit.texOffset =
            (hit.x - rect.x) / rect.w;

        break;
    }


    // Prevent floating point errors from
    // producing values outside texture range.
    outHit.rayHit.texOffset = std::clamp(outHit.rayHit.texOffset, 0.0f, 1.0f);


    return true;
}


void RayCastingSystem::castRays(World* world)
{
    for (auto& r : objectRays) {
        r = {};
    }
    

    const auto* playerPos = scene->playerEntity->getComponent<PositionComponent>();

    float ox = playerPos->position.x;
    float oy = playerPos->position.y;
    int mapX = (int)ox;
    int mapY = (int)oy;

    float rayAngle = playerPos->getAngle() - GFX::FOV * 0.5f;

    for (int ray = 0; ray < GFX::NUM_RAYS;
        ray++, rayAngle += GFX::DELTA_ANGLE)
    {
        Vector2f dir = { cosf(rayAngle), sinf(rayAngle) };


        float deltaDistX = (dir.x == 0) ? 1e30f : std::abs(1.0f / dir.x);
        float deltaDistY = (dir.y == 0) ? 1e30f : std::abs(1.0f / dir.y);


        int stepX, stepY;
        float sideDistX, sideDistY;
        {
            if (dir.x < 0)
            {
                stepX = -1;
                sideDistX = (ox - mapX) * deltaDistX;
            }
            else
            {
                stepX = 1;
                sideDistX = (mapX + 1.0f - ox) * deltaDistX;
            }

            if (dir.y < 0)
            {
                stepY = -1;
                sideDistY = (oy - mapY) * deltaDistY;
            }
            else
            {
                stepY = 1;
                sideDistY = (mapY + 1.0f - oy) * deltaDistY;
            }
        }

        int texture = 0;



        // loop through raycast rect Objects
        ObjectRayHit bestObject{};
        world->find<RayCastRectObjectComponent, PositionComponent>(
            [&](Entity* entity)
            {
                const auto* obj = entity->getComponent<RayCastRectObjectComponent>();
                const auto* pos = entity->getComponent<PositionComponent>();
                SDL_FRect positionRect = { pos->position.x, pos->position.y };
                positionRect.w = obj->wh.x;
                positionRect.h = obj->wh.y;
                ObjectRayHit hit;

                if (intersectBox({ ox, oy }, dir, positionRect, hit))
                {
                    // fish eye 
                    float corrected = hit.rayHit.dist * cosf(rayAngle - playerPos->getAngle());

                    hit.rayHit.dist = corrected;
                    hit.rayHit.projHeight = SCREEN_DIST / corrected;

                    if (!bestObject.rayHit.hit || corrected < bestObject.rayHit.dist)
                    {
                        hit.entity = entity;
                        bestObject = hit;
                    }
                }
            });
        if (bestObject.rayHit.hit)
        {
            objectRays[ray] = bestObject;
            if (ray == GFX::NUM_RAYS / 2)//if middle ray
            {
                auto* pos = bestObject.entity->getComponent<PositionComponent>();
                middleRay = bestObject;
                middleRay.rayHit.mapCoords = Vector2i
                { (int)(pos->position.x),(int)(pos->position.y) };

            }

        }

    }
}
void RayCastingSystem::updateDotObjectRays(World* world) {
    const auto* playerPos =
        scene->playerEntity->getComponent<PositionComponent>();

    
    renderDotEnsOrdered = {};
    world->find<RayCastDotObjectComponent, PositionComponent>(
        [&](Entity* entity)
        {

            auto* rayCastObj = entity->getComponent<RayCastDotObjectComponent>();
            auto* pos = entity->getComponent<PositionComponent>();
            const float dx = pos->position.x - playerPos->position.x;
            const float dy = pos->position.y - playerPos->position.y;
            const float theta = std::atan2(dy, dx);
            float delta = theta - playerPos->getAngle();

            delta = std::fmod(delta, 2.0f * GFX::PI);

            if (delta > GFX::PI)
                delta -= 2.0f * GFX::PI;
            else if (delta < -GFX::PI)
                delta += 2.0f * GFX::PI;


            // Actual FOV
            const float halfFov = GFX::NUM_RAYS * GFX::DELTA_ANGLE * 0.7f;

            // Outside screen FOV -> don't render
            if (std::abs(delta) > halfFov)
                return;

            renderDotEnsOrdered.push_back(entity);



            //middle of the sprite
            rayCastObj->screenX = (GFX::NUM_RAYS * 0.5f + delta / GFX::DELTA_ANGLE) * GFX::SCALE;

            rayCastObj->dist = std::hypot(dx, dy);
            rayCastObj->normDist = rayCastObj->dist * std::cos(delta);
            rayCastObj->proj = SCREEN_DIST / rayCastObj->normDist;





            // 22/64 ration of the guard img
            auto* spriteSheet = entity->getComponent<SpritesheetComponent>();
            float ratioPadX =
                (float)spriteSheet->sprSheetData.entityXPad /
                (float)spriteSheet->sprSheetData.gridWidth;
            float ratioPadY =
                (float)spriteSheet->sprSheetData.entityYPad /
                (float)spriteSheet->sprSheetData.gridHeight;

            float ratioWidth =
                (float)spriteSheet->sprSheetData.entityWidth /
                (float)spriteSheet->sprSheetData.gridWidth;

            float ratioHeight =
                (float)spriteSheet->sprSheetData.entityHeight /
                (float)spriteSheet->sprSheetData.gridHeight;


            // hitbox for cursor
            SDL_FRect targetRect = {};
            targetRect.w = rayCastObj->proj * ratioWidth;
            targetRect.h = rayCastObj->proj * ratioHeight;


            float CleftX = rayCastObj->screenX - rayCastObj->proj / 2.f;
            float CtopY = GFX::SCREEN_WIDTH * 0.5f - rayCastObj->proj / 2.f;

            targetRect.x = CleftX + ratioPadX * rayCastObj->proj;
            targetRect.y = (GFX::SCREEN_HEIGHT * 0.5f - rayCastObj->proj * 0.5f + scene->yScreenOffset + pos->yScreenOffset);
            targetRect.y += (rayCastObj->proj - targetRect.h);

            if (inFRect(targetRect, Vector2f{ GFX::SCREEN_WIDTH * 0.5f, GFX::SCREEN_HEIGHT * 0.5f }))
            {
                if (!middleRay.rayHit.hit || rayCastObj->dist < middleRay.rayHit.dist)
                {
                    middleRay.entity = entity;
                    middleRay.rayHit.dist = rayCastObj->dist;
                    middleRay.rayHit.hit = true;
                }
            }


        });

    std::sort(renderDotEnsOrdered.begin(), renderDotEnsOrdered.end(),
    []( Entity* a,  Entity* b)
    {

        auto* dotCompA = a->getComponent<RayCastDotObjectComponent>();
        auto* dotCompB = b->getComponent<RayCastDotObjectComponent>();

        bool bigger = dotCompA->normDist > dotCompB->normDist;
        return bigger; // biggest -> smallest
    });
}

void RayCastingSystem::onAddedToWorld(World* world) {

}
void RayCastingSystem::update(World* world)
{
    middleRay = {};
    castRays(world);
    updateDotObjectRays(world);
}