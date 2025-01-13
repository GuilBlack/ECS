#pragma once
#ifdef _DEBUG
#undef _ITERATOR_DEBUG_LEVEL 0
#endif
#include <type_traits>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <array>
#include <array>
#ifdef _DEBUG
#define _ITERATOR_DEBUG_LEVEL 2
#endif
#include <concepts>
#include <memory>
#include <cassert>
#include <bitset>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <atomic>


#if defined(__clang__)
#define ECS_FORCE_INLINE __attribute__((always_inline))
#elif defined(__GNUC__) || defined(__GNUG__)
#define ECS_FORCE_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
#define ECS_FORCE_INLINE __forceinline
#else
#define ECS_FORCE_INLINE inline
#endif

namespace ecs
{
using EntityID = uint32_t;
using SystemTypeID = uint32_t;
constexpr uint32_t MAX_COMPONENTS = 64;
constexpr EntityID INVALID_ENTITY_ID = std::numeric_limits<EntityID>::max();
using ComponentTypeID = std::bitset<MAX_COMPONENTS>;
using EntitySignature = std::bitset<MAX_COMPONENTS>;
using ComponentTypeIndex = uint32_t;

class BaseSystem;
class EntityRegistry;

template<typename Base, typename Derived>
concept DerivedFromConstraint = std::is_base_of_v<Base, Derived> && !std::is_same_v<Base, Derived>;

template<typename T>
concept ComponentConstraint = std::is_default_constructible_v<T> && !std::is_empty_v<T> && (std::is_move_assignable_v<T> || std::is_copy_assignable_v<T>);

template<typename T, typename... Args>
concept IsConstructibleConstraint = std::is_constructible_v<T, Args...>;

template<typename T>
concept SystemConstraint = DerivedFromConstraint<BaseSystem, T>&& std::is_default_constructible_v<T>;

inline static const ComponentTypeIndex CreateComponentTypeIndex()
{
    static std::atomic<uint32_t> typeCounter{ 0 };
    return typeCounter++;
}

template<ComponentConstraint T>
inline ComponentTypeIndex GetComponentTypeIndex()
{
    static ComponentTypeIndex typeIndex = CreateComponentTypeIndex();
    return typeIndex;
}

template<ComponentConstraint T>
inline const ComponentTypeID ComponentType()
{
    static const ComponentTypeID compId = ComponentTypeID((uint64_t)1 << GetComponentTypeIndex<T>());
    return compId;
}
}