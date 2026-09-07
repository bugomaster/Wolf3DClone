#pragma once
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

inline bool wentPast(Vector2f vel, Vector2f pos, Vector2f targetPos)
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
