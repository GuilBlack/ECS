#pragma once
#include <type_traits>
#include <unordered_set>
#include <concepts>
#include <unordered_map>
#include <queue>
#include <memory>
#include <vector>
#include <cassert>
#include <string>
#include <bitset>
#include <utility>
#include <algorithm>
#include <stdexcept>

namespace ECS
{
	const uint32_t MAX_COMPONENTS = 64;
	using EntityID = uint32_t;
	using SystemTypeID = uint32_t;
	using ComponentTypeID = std::bitset<MAX_COMPONENTS>;
	using EntitySignature = std::bitset<MAX_COMPONENTS>;

	class BaseSystem;
	class EntityRegistry;

	template<typename Base, typename Derived>
	concept DerivedFromConstraint = std::is_base_of_v<Base, Derived> && !std::is_same_v<Base, Derived>;

	template<typename T>
	concept ComponentConstraint = std::is_default_constructible_v<T> && !std::is_empty_v<T>;

	template<typename T, typename... Args>
	concept IsConstructibleConstraint = std::is_constructible_v<T, Args...>;

	template<typename T>
	concept SystemConstraint = DerivedFromConstraint<BaseSystem, T>;

	inline static const ComponentTypeID GetComponentTypeID()
	{
		static uint32_t typeCounter = 0;
		ComponentTypeID type;
		type.set(typeCounter++);
		return type;
	}

	inline static const SystemTypeID GetSystemTypeID()
	{
		static SystemTypeID type = 0;
		return type++;
	}

	template<ComponentConstraint T>
	inline const ComponentTypeID ComponentType()
	{
		static const ComponentTypeID compType = GetComponentTypeID();
		return compType;
	}

	template<SystemConstraint T>
	inline const SystemTypeID SystemType()
	{
		static const SystemTypeID systemType = GetSystemTypeID();
		return systemType;
	}
}