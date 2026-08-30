#include "pch.hpp"
#include "RenderSystem.hpp"

#include "Components.hpp"
#include "AppWindow.hpp"
#include "GFX.hpp"
#include "GameScene.hpp"
#include "RayCastingSystem.hpp"
#include "AssetsLoads.hpp"

//GUI
void RenderSystem::renderPlayerStats() 
{

    auto* playerComp = scene->playerEntity->getComponent<PlayerComponent>();
    SDL_Rect dstRect = {0, GFX::SCREEN_HEIGHT - 100, GFX::SCREEN_WIDTH, 100};
    scene->getScreen()->blitTextureScale(g_assets.statsBar.texture, dstRect);

    std::string ammoText = std::to_string(playerComp->ammo);
    if (playerComp->ammo > 99)
    {
        ammoText = "99";
    }
    scene->getScreen()->renderText(560, 560, 70, ammoText, COLORS::FONT);

    scene->getScreen()->renderText(460, 560, 70, std::to_string(playerComp->health), COLORS::FONT);
    Vector2i tMapPicPos = { 0,0 };
    float scale = 3.f;
    SDL_Rect srcRect = { 0,0, 48, 22 };
    {
        switch (playerComp->weapon)
        {
        case PlayerComponent::Weapon::KNIFE: {}break;
        case PlayerComponent::Weapon::PISTOL: {
            tMapPicPos = { 0, 23 };
        }break;
        case PlayerComponent::Weapon::RIFLE: {
            tMapPicPos = { 49, 0 };
        }break;
        case PlayerComponent::Weapon::MACHINE_GUN: {
            tMapPicPos = { 49, 23 };
        }break;
        default:
            break;
        }

        dstRect.x = 640;
        dstRect.y = 512;
        dstRect.w = (int)(48 * scale);
        dstRect.h = (int)(22 * scale);
        srcRect.x = tMapPicPos.x;
        srcRect.y = tMapPicPos.y;
        scene->getScreen()->blitPixelsFromTextureScale(g_assets.gunsStatsTMap.texture, srcRect, dstRect);

    }

    {
        tMapPicPos = { 0, 0 };
        if (playerComp->health < 10)
        {
            tMapPicPos = { 6,1 };
        }
        else if (playerComp->health < 25)
        {
            tMapPicPos = { 3,1 };

        }
        else if (playerComp->health < 40)
        {
            tMapPicPos = { 0,1 };

        }
        else if (playerComp->health < 55)
        {
            tMapPicPos = { 9,0 };

        }
        else if (playerComp->health < 70)
        {
            tMapPicPos = { 6,0 };

        }
        else if (playerComp->health < 85)
        {
            tMapPicPos = { 3,0 };
        }
        tMapPicPos.x += playerComp->faceExpression;
        srcRect.x = tMapPicPos.x * 25 + 1;
        srcRect.y = tMapPicPos.y * 34 + 2;
        srcRect.w = 24;
        srcRect.h = 31;
        scale = 3.f;
        dstRect = { 337, 510, (int)(24.f * scale), (int)(31.f * scale)};
        scene->getScreen()->blitPixelsFromTextureScale(g_assets.facesTMap.texture, srcRect, dstRect);
        
    }



    
    
    
    //    15, 540      floor 
    //    120, 540 score 
    //    260, 540 lives
    //    330, 506 face
    //    431, 540 health 460, 560
    //    530, 540 ammo -> 560, 560, 70
}
void RenderSystem::drawBackground() {

    float yScreenOffset =
        scene->yScreenOffset;


    int screenOffset = static_cast<int>(std::round(yScreenOffset));

    // Ceiling
    SDL_Rect ceiling = {0,0,GFX::SCREEN_WIDTH,GFX::SCREEN_HEIGHT / 2 + screenOffset };
    scene->getScreen()->drawRect(ceiling, SDL_Color{ 150, 200, 250, 255 });

    // Floor
    SDL_Rect floor = {0,GFX::SCREEN_HEIGHT / 2 + screenOffset,GFX::SCREEN_WIDTH,GFX::SCREEN_HEIGHT / 2 - screenOffset};
    scene->getScreen()->drawRect(floor, SDL_Color{ 40, 40, 40, 255});


}
void RenderSystem::renderDotEntity(Entity* entity) {
    float cameraYScreen =
        scene->yScreenOffset;
    const auto* pos = entity->getComponent<PositionComponent>();
    const auto* rayCastObj = entity->getComponent<RayCastDotObjectComponent>();
    const auto* texture = entity->getComponent<TextureComponent>();

    if (entity->hasComponent<SpritesheetComponent>())
    {
        const auto* sprSheet = entity->getComponent<SpritesheetComponent>();
        if (rayCastObj->normDist <= 0.1f)
            return;

        float proj = SCREEN_DIST / rayCastObj->normDist;

        float projHeight = proj;
        float projWidth = proj;

        float gap = projWidth / sprSheet->sprSheetData.gridWidth;

        int startX = (int)(rayCastObj->screenX - projWidth * 0.5f);

        int y = (int)(GFX::SCREEN_HEIGHT * 0.5f - projHeight * 0.5f + cameraYScreen + pos->yScreenOffset);

        Vector2i sprSheetCoords = sprSheet->getCoords(pos->getAngle());
        if (RayCastingSystem::middleRay.entity == entity)
        {
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
            targetRect.y = y + (rayCastObj->proj - targetRect.h);


            scene->getScreen()->drawRect(targetRect,COLORS::BLACK, false, 3);

        }

        for (int x = 0; x < (int)projWidth; x++)
        {
            int screenX = startX + x;

            if (screenX < 0 || screenX >= GFX::SCREEN_WIDTH)
                continue;

            int ray = screenX / GFX::SCALE;

            if (ray >= 0 && ray < GFX::NUM_RAYS)
            {
                if (RayCastingSystem::objectRays[ray].rayHit.dist < rayCastObj->normDist)
                    continue;
            }

            int texX = (int)(x / gap);
            SDL_Rect srcRect =
            {
                sprSheetCoords.x + texX,
                sprSheetCoords.y,
                1,
                sprSheet->sprSheetData.gridHeight
            };

            SDL_Rect dstRect =
            {
                screenX,
                y,
                1,
                (int)projHeight
            };

            scene->getScreen()->blitPixelsFromTextureScale(
                texture->texture,
                srcRect,
                dstRect);
        }
    }
    else
    {

        int texW, texH;
        SDL_QueryTexture(texture->texture, nullptr, nullptr, &texW, &texH);

        if (rayCastObj->normDist <= 0.1f)
            return;

        float proj = SCREEN_DIST / rayCastObj->normDist;

        float projHeight = proj;
        float projWidth = proj;

        float gap = projWidth / texW;

        int startX = (int)(rayCastObj->screenX - projWidth * 0.5f);

        int y = (int)(GFX::SCREEN_HEIGHT * 0.5f - projHeight * 0.5f + cameraYScreen + pos->yScreenOffset);

        Vector2i sprSheetCoords = { 0,0 };


        for (int x = 0; x < (int)projWidth; x++)
        {
            int screenX = startX + x;

            if (screenX < 0 || screenX >= GFX::SCREEN_WIDTH)
                continue;

            int ray = screenX / GFX::SCALE;

            if (ray >= 0 && ray < GFX::NUM_RAYS)
            {
                if (RayCastingSystem::objectRays[ray].rayHit.dist < rayCastObj->normDist)
                    continue;
            }

            int texX = (int)(x / gap);
            SDL_Rect srcRect =
            {
                sprSheetCoords.x + texX,
                sprSheetCoords.y,
                1,
                texH
            };

            SDL_Rect dstRect =
            {
                screenX,
                y,
                1,
                (int)projHeight
            };

            scene->getScreen()->blitPixelsFromTextureScale(
                texture->texture,
                srcRect,
                dstRect);
        }
    }

}
void RenderSystem::renderDotEntities(World* world)
{
    //first entities renders 
    for (Entity* dotEntity : RayCastingSystem::renderDotEnsOrdered)
    {
        if (dotEntity->hasComponent<DecorativeObjectComponent>())
            renderDotEntity(dotEntity);

    }
    for (Entity* dotEntity : RayCastingSystem::renderDotEnsOrdered)
    {
        if (!dotEntity->hasComponent<DecorativeObjectComponent>())
            renderDotEntity(dotEntity);

    }




}





void RenderSystem::drawWalls() {
    float yScreenOffset =
        scene->yScreenOffset;
    static const auto& sprSheetWall = SPRSHEET_DATA::WALLTMAP;

    for (int x = 0; x < GFX::NUM_RAYS; x++)
    {
        const auto& objRay = RayCastingSystem::objectRays[x];
        if (objRay.rayHit.hit)
        {
            Entity* objEntity = objRay.entity;



            auto* spriteSheet = objEntity->getComponent<SpritesheetComponent>();
            auto* rayCastRectComp = objEntity->getComponent<RayCastRectObjectComponent>();
            Vector2i sprsheetCoords = spriteSheet->getCoords();
            if (objEntity->hasComponent<RectFacesComponent>())
            {
                int textureID = objEntity->getComponent<RectFacesComponent>()->faceIDs[(int)objRay.face];
                sprsheetCoords = spriteSheet->getCoordsByID(textureID);
            }            

            int screenX = x * GFX::SCREEN_WIDTH / GFX::NUM_RAYS;
            int columnWidth = GFX::SCREEN_WIDTH / GFX::NUM_RAYS + 1;

            float height = objRay.rayHit.projHeight * rayCastRectComp->height;

            int yStart = (GFX::SCREEN_HEIGHT / 2) - (int)(height / 2) + (int)(rayCastRectComp->zPos* objRay.rayHit.projHeight);
            int yEnd = (GFX::SCREEN_HEIGHT / 2) + (int)(height / 2);


            float offset = objRay.rayHit.texOffset;
            SDL_Rect srcRect = 
                { sprsheetCoords.x + (int)(offset * sprSheetWall.gridWidth),
                sprsheetCoords.y,1 ,sprSheetWall.gridHeight };


            SDL_Rect dstRect = { screenX, yStart, columnWidth, yEnd - yStart };


            dstRect.y += (int)yScreenOffset;
            scene->getScreen()->blitPixelsFromTextureScale(g_assets.wallTMap.texture, srcRect, dstRect);

        }


    }

}
void RenderSystem::renderWeapon()
{
    int indicatorScale = 4;

    scene->getScreen()->drawRect(
        SDL_Rect{
            GFX::SCREEN_WIDTH / 2 - indicatorScale / 2,
            GFX::SCREEN_HEIGHT / 2 - indicatorScale / 2,
            indicatorScale,
            indicatorScale
        },
        COLORS::CYAN
    );


    //===========
    auto* sprSheetComp = scene->playerEntity->getComponent<SpritesheetComponent>();
    auto* texture = scene->playerEntity->getComponent<TextureComponent>();

    Vector2i sprSheetCoords = sprSheetComp->getCoords();
    SDL_Rect srcRect =
    {
        sprSheetCoords.x,
        sprSheetCoords.y,
        sprSheetComp->sprSheetData.gridWidth,
        sprSheetComp->sprSheetData.gridHeight
    };

    int size = 400;


    SDL_Rect dstRect =
    {
        GFX::SCREEN_WIDTH/2 - size/2,GFX::SCREEN_HEIGHT -size - 100 ,
        size ,size
    };
    scene->getScreen()->blitPixelsFromTextureScale(texture->texture,srcRect,dstRect);



}

void RenderSystem::update(World* world) {

    drawBackground();
    drawWalls();

    renderDotEntities(world);
    renderWeapon();
    renderPlayerStats();
}
