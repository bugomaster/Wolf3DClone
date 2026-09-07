#pragma once
#include <algorithm>
#include <cmath>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <array>
#include <bitset>
#include <memory>

#include <random>
#include <type_traits>
#include <numbers>
#include "System.hpp"
#include "MathWolf.hpp"
template <typename T, std::size_t N>
std::vector<T> arrToVec(const T(&arr)[N])
{
    return { std::begin(arr), std::end(arr) };
}






constexpr int TERMINAL_COLORS[] = {
    31, // red
    32, // green
    33, // yellow
    34, // blue
    35, // magenta
    36, // cyan
    91, // bright red
    92, // bright green
    93, // bright yellow
    94, // bright blue
    95, // bright magenta
    96  // bright cyan
};

template <typename T>
inline T getRandomRange(T min, T max)
{
    if (min == max)
    {
        return min;
    }
    static std::random_device rd;
    static std::mt19937 gen(rd());

    if constexpr (std::is_integral_v<T>)
    {
        std::uniform_int_distribution<T> dist(min, max);
        return dist(gen);
    }
    else
    {
        std::uniform_real_distribution<T> dist(min, max);
        return dist(gen);
    }
}
template<typename T>
const T& getRandomFromVector(const std::vector<T>& vec)
{
    if (vec.empty())
        throw std::runtime_error("Vector is empty");

    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<std::size_t> dist(0, vec.size() - 1);
    return vec[dist(gen)];
}
template <typename T>
inline void println(const T& line, bool randomColor = false) {
    if (randomColor)
    {
        int color = TERMINAL_COLORS[
            getRandomRange(0, (int)(sizeof(TERMINAL_COLORS) / sizeof(TERMINAL_COLORS[0])) - 1)
        ];

        std::cout << "\033[" << color << "m" << line << "\033[0m\n";
    }
    else
        std::cout << line << '\n';


}
inline void printRect(const SDL_FRect& rect, bool randomColor = false) {
    std::cout << "x: " << rect.x << ", y:" << rect.y << ", w:" << rect.w << ", h:" << rect.h << std::endl;

}




class Entity;
class World;
class System;
class Map;

//

using ComponentID = std::uint8_t;
inline ComponentID getNewComponentTypeID() {
    static ComponentID lastID = 0;
    return lastID++;
}

template <typename T>
inline ComponentID getComponentTypeID() {
    static ComponentID typeID = getNewComponentTypeID();
    return typeID;
}


constexpr std::uint16_t MAX_COMPONENTS = 256;
constexpr std::uint8_t MAX_SYSTEMS = 16;

struct Component {
    virtual ~Component() = default;
};

using ComponentArray = std::array<Component*, MAX_COMPONENTS>;
using ComponentBitset = std::bitset<MAX_COMPONENTS>;


class Entity {
public:


    Entity()
    {
        componentBitset.reset();
    }
    template<typename... Components>
    bool hasComponent() const {
        return (componentBitset[getComponentTypeID<Components>()] && ...);
    }

    template<typename T>
    T* getComponent() {
        return static_cast<T*>(components[getComponentTypeID<T>()].get());
    }

    template<typename T, typename... Args>
    T* addComponent(Args&&... args) {

        ComponentID id = getComponentTypeID<T>();

        auto component = std::make_unique<T>(std::forward<Args>(args)...);

        T* ptr = component.get();

        components[id] = std::move(component);

        componentBitset.set(id);

        return ptr;
    }

    template<typename T>
    void removeComponent() {

        ComponentID id = getComponentTypeID<T>();

        components[id].reset();

        componentBitset.set(id, false);
    }
    //void removeAllComponents()
    //{
    //    for (auto& component : components)
    //    {
    //        component.reset();
    //    }
    //    componentBitset.reset();
    //}
    const ComponentBitset& compBitset() {
        return this->componentBitset;
    }
private:
    std::array<std::unique_ptr<Component>, MAX_COMPONENTS> components;

    ComponentBitset componentBitset;
};





class GameScene;
class World {
public:

    ~World() {
        for (auto* e : entities)
            delete e;
        for (auto* e : pendingEntities)
            delete e;
    }

    std::vector<Entity*> pendingEntities;
    Entity* createEntity() {
        Entity* e = new Entity();
        pendingEntities.push_back(e);
        return e;
    }

    Entity* createImmidiateEntity() {
        Entity* e = new Entity();
        entities.push_back(e);
        return e;
    }

    void destroyEntity(Entity* e)
    {

        println("destroy Entity", true);
        if (std::find(destroyQueue.begin(), destroyQueue.end(), e) == destroyQueue.end())
        {
            destroyQueue.push_back(e);
        }
    }
    void processDestroyQueue() {
        for (auto* e : destroyQueue) {

            entities.erase(
                std::remove(entities.begin(), entities.end(), e),
                entities.end()
            );

            delete e;
        }

        destroyQueue.clear();
    }

    template <typename T, typename... Args>
    T* registerSystem(Args&&... args)
    {
        systems.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        systems.back()->onAddedToWorld(this);

        return static_cast<T*>(systems.back().get());
    }

    void tick() {
        for (Entity* e : pendingEntities) {
            this->entities.push_back(e);
        }
        pendingEntities.clear();
        for (auto& system : systems) {
            if (system->enabled) {
                system->update(this);
            }
        }
        processDestroyQueue();
    }
    std::vector<Entity*>& getEntities() {
        return entities;
    }
    std::vector<Entity*>& getEntitiesWithPos() {
        return entitiesWithPos;
    }
    template <typename... Component, typename Func>
    void find(Func func) {
        for (auto* entity : entities) {
            if ((entity->hasComponent<Component>() && ...)) {
                func(entity);
            }
        }
    }
    template <typename... Component, typename Func>
    void findStop(Func func) {
        for (auto* entity : entities) {
            if ((entity->hasComponent<Component>() && ...)) {
                if (func(entity))
                {
                    break;
                }
            }
        }
    }
    template <typename... Component, typename Func>
    void findFirst(Func func) {
        for (auto* entity : entities) {
            if ((entity->hasComponent<Component>() && ...)) {
                func(entity);
                break;
            }
        }
    }
private:
    std::vector<Entity*> entities;
    std::vector<Entity*> entitiesWithPos;


    std::vector<Entity*> destroyQueue;
    std::vector<std::unique_ptr<System>> systems;
public:
    void loadScene(GameScene* gameScene) {
        this->gameScene = gameScene;

    }
    GameScene* gameScene = nullptr;
};



struct ColData
{
    bool wasCol = false;
    Entity* entity = nullptr;
    SDL_FRect overLapRect = { 0.f,0.f,0.f,0.f };
};

struct MoveData
{
    bool wasCollsion;
    Vector2f newPos;
    Vector2f newVel;
    std::vector<ColData> collisions;
    ColData resolvedCol;
    ColData xCollision;
    ColData yCollision;
};

