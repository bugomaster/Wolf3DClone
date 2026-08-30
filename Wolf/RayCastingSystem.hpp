#pragma once
#include "GFX.hpp"
#include "System.hpp"
class GameScene;
class Entity;

enum BoxFace : int
{
    FACE_LEFT,
    FACE_RIGHT,
    FACE_TOP,
    FACE_BOTTOM
};



struct RayHit {
    bool hit = false;
    float dist;
    float projHeight;
    float texOffset;
    Vector2i mapCoords;
};
//struct WallRayHit {
//    RayHit rayHit;
//    int wallID;
//
//};
struct ObjectRayHit
{
    RayHit rayHit;
    BoxFace face;
    Entity* entity;
};

class RayCastingSystem : public System {
public:
    RayCastingSystem(GameScene* scene) :
        scene(scene)
    {
    }
    //static WallRayHit wallRays[GFX::NUM_RAYS];
    static ObjectRayHit objectRays[GFX::NUM_RAYS];

    void update(World* world) override;
    void onAddedToWorld(World* world) override;
    static ObjectRayHit middleRay;
    static std::vector<Entity*> renderDotEnsOrdered;
private:


    bool intersectBox(const Vector2f& origin,const Vector2f& dir,const SDL_FRect& rect,ObjectRayHit& outHit);
    void castRays(World* world);
    void updateDotObjectRays(World* world);
    GameScene* scene;
};
