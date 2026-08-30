#pragma once
#include <algorithm>
#include <cmath>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#include <vector>
#include <array>
#include <bitset>
#include <memory>

#include <random>
#include <type_traits>
#include <numbers>
#include "System.hpp"
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


//================================================ MATH

inline int clamp(int v, int lo, int hi) {
    return std::max(lo, std::min(v, hi));
}
template <typename T>
class Vector2
{
public:
    T x;
    T y;

public:
    constexpr Vector2() : x(0), y(0) {}
    constexpr Vector2(T x, T y) : x(x), y(y) {}
    constexpr explicit Vector2(T v) : x(v), y(v) {}
    // Convert to another Vector2 type
    template <typename U>
    constexpr Vector2<U> to() const
    {
        return Vector2<U>(
            static_cast<U>(x),
            static_cast<U>(y)
        );
    }
    // Cast
    template <typename U>
    constexpr Vector2<U> cast() const
    {
        return Vector2<U>(static_cast<U>(x), static_cast<U>(y));
    }

    // Operators
    constexpr Vector2 operator+(const Vector2& rhs) const
    {
        return { x + rhs.x, y + rhs.y };
    }

    constexpr Vector2 operator-(const Vector2& rhs) const
    {
        return { x - rhs.x, y - rhs.y };
    }

    constexpr Vector2 operator*(T scalar) const
    {
        return { x * scalar, y * scalar };
    }

    constexpr Vector2 operator/(T scalar) const
    {
        return { x / scalar, y / scalar };
    }

    Vector2& operator+=(const Vector2& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    Vector2& operator-=(const Vector2& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    Vector2& operator*=(T scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    bool operator==(const Vector2& rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }

    bool operator!=(const Vector2& rhs) const
    {
        return !(*this == rhs);
    }

    // Math
    float length() const
    {
        return std::sqrt(static_cast<float>(x * x + y * y));
    }

    float lengthSquared() const
    {
        return static_cast<float>(x * x + y * y);
    }

    float distance(const Vector2& rhs) const
    {
        return (*this - rhs).length();
    }

    Vector2<float> normalized() const
    {
        float len = length();
        if (len <= 0.00001f)
            return { 0.f, 0.f };

        return {
            static_cast<float>(x) / len,
            static_cast<float>(y) / len
        };
    }

    Vector2<float> withLength(float speed) const
    {
        return normalized() * speed;
    }

    T dot(const Vector2& rhs) const
    {
        return x * rhs.x + y * rhs.y;
    }

    static Vector2 lerp(const Vector2& a, const Vector2& b, float t)
    {
        return {
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t
        };
    }
};
template <typename T>
constexpr Vector2<T> operator*(T scalar, const Vector2<T>& v)
{
    return { v.x * scalar, v.y * scalar };
}

using Vector2i = Vector2<int>;
using Vector2u = Vector2<unsigned int>;
using Vector2f = Vector2<float>;

inline bool wentPast(Vector2f vel,Vector2f pos ,Vector2f targetPos)
{
    Vector2f delta = targetPos - pos;
    if ((vel.x > 0.f && delta.x <= 0.f) ||
        (vel.x < 0.f && delta.x >= 0.f) ||
        (vel.y > 0.f && delta.y <= 0.f) ||
        (vel.y < 0.f && delta.y >= 0.f))
    {
        return true;
    }
    return false;
}


class PIDController {
public:
    PIDController() = default;
    PIDController(double kP, double kI, double kD) : m_kP{ kP }, m_kI{ kI }, m_kD{ kD } {}
    PIDController(double kP, double kI, double kD, double period)
        : m_kP{ kP }, m_kI{ kI }, m_kD{ kD }, m_period{ period } {
    }

    double calculate(double measurement) {
        m_positionError = m_setpoint - measurement;
        m_velocityError = (m_setpoint - measurement) / m_period;

        if (m_kI != 0) {
            m_totalError = clamp(m_totalError + m_velocityError * m_period, m_minIntegral / m_kI,
                m_maxIntegral / m_kI);
        }

        return m_positionError * m_kP + m_totalError * m_kI + m_velocityError * m_kD;
    }

    double calculate(double measurement, double setpoint) {
        m_positionError = setpoint - measurement;
        m_velocityError = (setpoint - measurement) / m_period;

        if (m_kI != 0) {
            m_totalError = clamp(m_totalError + m_velocityError * m_period, m_minIntegral / m_kI,
                m_maxIntegral / m_kI);
        }

        return (m_positionError * m_kP) + (m_totalError * m_kI) + (m_velocityError * m_kD);
    }

    void setSetpoint(double setpoint) {
        m_setpoint = setpoint;
    }

private:
    double m_kP = 0;
    double m_kI = 0;
    double m_kD = 0;

    double m_period = 1.0;

    double m_minIntegral = -1.0;
    double m_maxIntegral = 1.0;

    double m_setpoint = 0;
    double m_totalError = 0;
    double m_velocityError = 0;
    double m_positionError = 0;

    double clamp(double value, double low, double high) {
        return std::max(low, std::min(value, high));
    }
};
template <typename Rect>
inline bool AABBOverlaps(const Rect& a, const Rect& b)
{


    const Rect& ra = a;
    const Rect& rb = b;

    auto leftA = ra.x;
    auto rightA = ra.x + ra.w;
    auto topA = ra.y;
    auto bottomA = ra.y + ra.h;

    auto leftB = rb.x;
    auto rightB = rb.x + rb.w;
    auto topB = rb.y;
    auto bottomB = rb.y + rb.h;

    return leftA < rightB &&
        rightA > leftB &&
        topA < bottomB &&
        bottomA > topB;
}
template <typename Rect>
inline Rect AABBOverlapRect(const Rect& a, const Rect& b)
{
    auto left = std::max(std::min(a.x, a.x + a.w), std::min(b.x, b.x + b.w));
    auto right = std::min(std::max(a.x, a.x + a.w), std::max(b.x, b.x + b.w));
    auto top = std::max(std::min(a.y, a.y + a.h), std::min(b.y, b.y + b.h));
    auto bottom = std::min(std::max(a.y, a.y + a.h), std::max(b.y, b.y + b.h));

    Rect r{};

    if (left < right && top < bottom)
    {
        r.x = left;
        r.y = top;

        float w = right - left;
        float h = bottom - top;

        r.w = (a.w < 0) ? -w : w;
        r.h = (a.h < 0) ? -h : h;
    }
    else
    {
        r.x = r.y = r.w = r.h = 0;
    }

    return r;
}

inline bool inFRect(const SDL_FRect& rect, Vector2f pos)
{
    return pos.x >= rect.x &&
        pos.x <= rect.x + rect.w &&
        pos.y >= rect.y &&
        pos.y <= rect.y + rect.h;
}

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