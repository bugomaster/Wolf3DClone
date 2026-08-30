#pragma once
#include "GFX.hpp"
#include "SpriteSheet.hpp"


#include <functional>
#include <vector>


struct SDL_Texture;
class Entity;
enum class Collectible
{
    AMMO,

};

struct InputComponent : public Component {
    InputComponent():
        right(false),
        left(false),
        up(false),
        down(false),
        w(false),
        s(false),
        space(false),
        disabled(false){}
    void reset() 
    {
        this->right = false;
        this->left = false;
        this->down = false;
        this->up = false;
        this->s = false;
        this->w = false;
        this->space = false;
        this->disabled = false;
        this->e = false;

    }
    void disable() {
        reset();
        this->disabled = true;
    }
    bool isActive() { return !disabled; }




    bool right;
    bool left;
    bool up;
    bool down;
    bool w;
    bool s;
    bool space;
    bool e;

private:
    bool disabled = false;
};
struct PlayerComponent : public Component {
    bool shoot = false;
    enum class Weapon : int
    {
        KNIFE,
        PISTOL,
        RIFLE,
        MACHINE_GUN,
    };

    Weapon weapon;
    int faceExpression = 0;
    int ammo = 1000;
    int health = 100;
};
struct DoorComponent : Component {
    DoorComponent(Vector2i openDir, int timer, Vector2f originalPos):
    openDir(openDir), timer(timer), originalPos(originalPos)
    {
        velDoor = { openDir.x / 100.f, openDir.y / 100.f };
        openPos.x = openDir.x * 0.99f + originalPos.x;
        openPos.y = openDir.y * 0.99f + originalPos.y;
    }


    Vector2i openDir = {};
    Vector2f originalPos = {};
    Vector2f openPos = {};
    int timer = 120;
    bool opening = false;
    bool closing = false;
    Vector2f velDoor;
    bool open = false;

};
struct VelocityComponent : Component {

    VelocityComponent(float dx, float dy, float dAngle = 0.f):
    dx(dx), dy(dy), dAngle(dAngle)
    {}


    float dx; float dy;
    float dAngle = 0.f;// in degrees

    void reset()
    {
        this->dx = 0.f;
        this->dy = 0.f;
        this->dAngle = 0.f;
    }
};
struct CantCollideAllComponent : Component {};
struct WasCollisionComponent : Component {
    WasCollisionComponent(const MoveData& data)
    :data(data)
    { }
    MoveData data;


};
struct CantCollideWithComponent : Component{

    CantCollideWithComponent(const ComponentID& compID)
    {
        this->compIDs.push_back(compID);
    }
    CantCollideWithComponent(const std::vector<ComponentID>& compIDs)
        : compIDs(compIDs)
    {
    }

    bool checkEntityForComponent(Entity* entity) const
    {
        if (compIDs.size() == 0)
            return false;

        bool containOne = false;
        for (auto& compID : compIDs)
        {
            containOne |= compID < MAX_COMPONENTS && entity->compBitset().test(compID);
            if (containOne) return containOne;
        }
        return containOne;
    }
    std::vector<ComponentID> compIDs = {};

};


struct NotCollideableComponent : Component{

    NotCollideableComponent() : compIDs({})
    {

    }
    //exceptions
    NotCollideableComponent(std::vector<ComponentID> compIDs) 
    : compIDs(compIDs)
    {
    }

    bool checkEntityForComponent(Entity* entity) const 
    {
        if (compIDs.size() == 0)
            return false;

        bool containOne = false;
        for (auto& compID : compIDs)
        {
            containOne |= compID < MAX_COMPONENTS && entity->compBitset().test(compID);
            if (containOne) return containOne;
        }
        return containOne;
    }

    std::vector<ComponentID> compIDs = {};


};

struct ColNotEffectMovement : Component
{

};


struct InitEntityComponenet : Component {

    InitEntityComponenet(int range,std::function<void(Entity*)> initFunc):
    range(range),initFunc(initFunc)
    {}
    int range;
    std::function<void(Entity*)> initFunc = {};
};
struct PositionComponent : Component
{
    PositionComponent(const Vector2f& position) :position(position)
    {}
    PositionComponent(const Vector2f& position, float radius):position(position),radius(radius)
    {}


    Vector2f position = { 0.f, 0.f };
    float yScreenOffset = 0.f;
    float radius = 0.f;
    SDL_FRect hitBox = { 0.f,0.f,0.f,0.f };
    SDL_FRect getRelativeHB() const
    {
        if (hitBox.w != 0.f && hitBox.w != 0.f)
        {
            return SDL_FRect{ position.x + hitBox.x, position.y + hitBox.y, hitBox.w, hitBox.h };    
        }
        return SDL_FRect{ position.x - radius, position.y - radius, radius*2, radius * 2 };
   
    }


    // input dt angle in radians
    void changeAngle(float delta) {
        angle = std::fmod(angle + delta, 2.0f *GFX::PI);
        if (angle < 0.0f)
            angle += 2.0f *GFX::PI;
    }
    void setAngle(float newAngle) {
        angle = std::fmod(newAngle, 2.0f *GFX::PI);
        if (angle < 0.0f)
            angle += 2.0f *GFX::PI;
    }


    float getAngle() const {
        return angle;
    }
private:
    float angle = 0.f;// in degrees

};


struct RayCastRectObjectComponent : public Component {
    RayCastRectObjectComponent(Vector2f wh, float height = 1.f, float zPos = 0.f) :
        wh(wh), height(height), zPos(zPos)
    { }
    
    float screenX;
    float dist;
    float normDist;
    Vector2f wh;
    float height = 1.f;
    float zPos = 0.f;

};
struct RayCastDotObjectComponent : public Component {    
    
    float screenX;
    float dist;
    float normDist;
    float proj;

};
//src rect
struct AnimationComponent : public Component {
    AnimationComponent(std::vector<int> frameIDS, int ticksPerFrame,
        bool repeated = true) :
        frameIDS(frameIDS),
        frameCount((int)frameIDS.size()),
        ticksPerFrame(ticksPerFrame),
        repeated(repeated),
        frameTimer(ticksPerFrame)
    {}
    AnimationComponent(const AnimationComponent& other) :
        frameIDS(other.frameIDS),
        ticksPerFrame(other.ticksPerFrame),
        repeated(other.repeated),
        frameCount((int)frameIDS.size()),
        frameTimer(ticksPerFrame)
    {   
    }
    std::vector<int> frameIDS;
    int frameCount;
    int ticksPerFrame;
    int frameTimer = 0;

    int currentFrame = 0;

    bool repeated = true;

};



enum class EnemyType : int {
    GUARD,
};
struct Target {
    Vector2f pos;
    float dist;
    float angle;
};
struct EnemyState;
struct PathNode
{
    int x;
    int y;

    float g; // distance from start
    float h; // distance to goal
    float f; // g + h

    PathNode* parent;

    PathNode(int x, int y)
        : x(x), y(y), g(0), h(0), f(0), parent(nullptr)
    {
    }
};
struct EnemyComponent : public Component
{
    EnemyComponent(EnemyType type, int reactionTime,int maxWalkingTime,
        EnemyState* currentState , Collectible drop = Collectible::AMMO)
        :
        type(type),
        reactionTime(reactionTime),
        maxWalkingTime(maxWalkingTime),
        seeTargetTimer(reactionTime), currentState(currentState),
        drop(drop)
    {
        switch (type)
        {
        case EnemyType::GUARD:
            this->lives = 1;
            break;
        default:
            break;
        }
    }
    EnemyState* currentState = nullptr;
    EnemyState* prevState = nullptr;

    bool hearShot = false;


    EnemyType type;
    int lives;
    Collectible drop;



    std::vector<Vector2f> path;
    int iPath = 0;


    //patrolAlert
    int seeTargetTimer = 0;
    int reactionTime = 60;


    //chase
    int walkingTime = 0;
    int maxWalkingTime = 0;
    float dirAngle = -1.f;

    
    float angleOffset = 0.1f;
    bool walkingStraight = true;


    //attack
    int counterShots = 0;

    
};


struct DecorativeObjectComponent : public Component {};
struct TextureComponent : public Component {
    TextureComponent(SDL_Texture* texture):
    texture(texture)
    {}
    SDL_Texture* texture;

};


struct RectFacesComponent : public Component {

    RectFacesComponent(const std::vector<int>& faceIDs):
    faceIDs(faceIDs)
    {}
    //    
    //    FACE_LEFT,
    //    FACE_RIGHT,
    //    FACE_TOP,
    //    FACE_BOTTOM

    std::vector<int> faceIDs;
};
struct SpritesheetComponent : public Component
{
    SpritesheetComponent(
        const SpriteSheetData& sprSheetData, int frameID = 0) :
        sprSheetData(sprSheetData), frameID(frameID)
    {
    }

    void setSpriteSheetWithAngle(bool val) {
        if (val)
        {
            useAngle = true;
            this->deltaAngle = (2.0f * GFX::PI) / (float)sprSheetData.cols;
        }
        else
        {
            useAngle = false;
            this->deltaAngle = 1.f;
        }
        
    }

    Vector2i getCoords(float angle = -1.f) const {
        int fid = frameID;

        if (angle != -1.f && useAngle)
        {
            int rowStartID = frameID - (frameID % sprSheetData.cols);

            // normalize angle to [0, 2π)
            float a = std::fmod(angle, 2.0f *GFX::PI);
            if (a < 0.0f)
                a += 2.0f *GFX::PI;

            fid = rowStartID + (int)(a / deltaAngle);
        }

        return getTileMapTexCoord(fid, sprSheetData);
    }
    Vector2i getCoordsByID(int frameID) const {
        int fid = frameID;
        return getTileMapTexCoord(fid, sprSheetData);
    }

    SpriteSheetData sprSheetData = {};
    int frameID = 0;
    float deltaAngle = 1.f;
    bool useAngle = false;
};


//callbacks
struct DestroyDelayComponent : Component
{
    DestroyDelayComponent(int frames) :
        accFrames(0), frames(frames)
    {
    }

    int accFrames = 0;
    int frames = 0;
};
struct RepeatedTimerNTimesComponent : Component
{
    RepeatedTimerNTimesComponent(int frameGap, int times, std::function<void(Entity*)> onFinish) :
        accFrames(0), frameGap(frameGap), times(times), onFinish(onFinish)
    {

    }
    std::function<void(Entity*)> onFinish = [&](Entity* entity) {};


    int accFrames = 0;
    int frameGap = 0;
    int times = 1;
    int counterTimes = 0;

};
struct Timer
{
    int frames = 0;
    std::function<void(Entity*)> onFinish;

    Timer(int frames, std::function<void(Entity*)> onFinish = [](Entity*) {}) :
        frames(frames),
        onFinish(std::move(onFinish))
    {
    }
    int accFrames = 0;
};
struct RepeatedTimerComponent : Component
{
    RepeatedTimerComponent(int frames, std::function<void(Entity*)> onFinish) :
        accFrames(0), frames(frames), onFinish(onFinish)
    {

    }
    std::function<void(Entity*)> onFinish = [&](Entity* entity) {};


    int accFrames = 0;
    int frames = 0;




};
struct TimerComponent : Component
{
    TimerComponent()
    {
        timers.reserve(4);
    }
    TimerComponent(int frames, std::function<void(Entity*)> onFinish) :
        TimerComponent()
    {
        timers.emplace_back(frames, std::move(onFinish));
    }
    void addTimer(int frames, std::function<void(Entity*)> onFinish)
    {
        timers.emplace_back(frames, std::move(onFinish));
    }

    std::vector<Timer> timers;
};
struct WaitUntilComponent : public Component {
    WaitUntilComponent() = default;
    WaitUntilComponent(std::function<bool(Entity*)> condition, std::function<void(Entity*)> doAfter)
        : condition{ condition }, doAfter{ doAfter } {
    }

    std::function<bool(Entity*)> condition;
    std::function<void(Entity*)> doAfter;
};


//
struct CollectibleComponent : public Component {
    CollectibleComponent(Collectible type) :
        type(type)
    {
    }
    Collectible type;
};
struct KillEnemyComponent : public Component {

    KillEnemyComponent(Collectible drop) :
        drop(drop)
    {
    }
    Collectible drop;
};
struct WallComponent : Component {};
