#pragma once

#include "Types.h"

namespace ECS
{
	class IComponentList
	{
	public:
		virtual ~IComponentList() = default;

		virtual bool Remove(EntityID entity) = 0;

	protected:
		std::unordered_map<EntityID, uint32_t> m_ComponentMap;
	};

	class NoComponentException : public std::exception
	{
	public:
		NoComponentException()
			: exception("This entity has no component with this type attached to it.")
		{}
	};;

	template <ComponentConstraint Comp>
	class ComponentList : public IComponentList
	{
	private:
		///////////////////////////////////////////////////////////////////
		//// Internal types definition ////////////////////////////////////
		///////////////////////////////////////////////////////////////////
		struct EntityComponentPair
		{
			EntityID Entity;
			Comp Component;
		};
		using ConstIterator = typename std::vector<EntityComponentPair>::const_iterator;
	public:
		ComponentList()
		{
			m_Components.reserve(100);
			m_ComponentMap.reserve(100);
		}
		ComponentList(size_t defaultCapacity)
		{
			m_Components.reserve(size_t(defaultCapacity / 10));
			m_ComponentMap.reserve(size_t(defaultCapacity / 2));
		}
		~ComponentList() = default;

		/// <summary>
		/// Adds a component that has already been created outside of this class.
		/// </summary>
		/// <param name="entity">: ID of the entity that we want to modify</param>
		/// <param name="component">: Component to be added to the list</param>
		void Add(EntityID entity, const Comp& component)
		{
			if (m_ComponentMap.contains(entity))
				return;

			m_Components.emplace_back(entity, component);
			m_ComponentMap[entity] = (uint32_t)m_Components.size() - 1;
		}

		/// <summary>
		/// Attach a default constructed component to a given entity.
		/// </summary>
		/// <param name="entity">: ID of the entity that we want to modify</param>
		/// <returns>the component newly created component</returns>
		[[nodiscard]] Comp& Emplace(EntityID entity)
		{
			if (HasComponent(entity))
				return m_Components[m_ComponentMap[entity]].Component;

			auto component = m_Components.emplace_back(entity, Comp());
			m_ComponentMap[entity] = (uint32_t)m_Components.size() - 1;
			return m_Components.back().Component;
		}

		/// <summary>
		/// Attach a component constructed with the given arguments to an entity.
		/// </summary>
		/// <param name="entity">: ID of the entity that we want to modify</param>
		/// <param name="...args">arguments to construct the component</param>
		/// <returns>the newly created component</returns>
		template<typename... Args>
		requires IsConstructibleConstraint<Comp, Args...>
		[[nodiscard]] Comp& Emplace(EntityID entity, Args&&... args)
		{
			if (HasComponent(entity))
				return m_Components[m_ComponentMap[entity]].Component;

			m_Components.emplace_back(entity, Comp(std::forward<Args>(args)...));
			m_ComponentMap[entity] = (uint32_t)m_Components.size() - 1;
			return m_Components.back().Component;
		}

		/// <summary>
		/// Replaces the component of the given entity.
		/// </summary>
		/// <param name="entity">: ID of the entity that we want to modify</param>
		/// <param name="component">: New Component that will replace the current one</param>
		void Replace(EntityID entity, const Comp& component)
		{
			if (!HasComponent(entity))
				throw NoComponentException();

			uint32_t index = m_ComponentMap[entity];
			m_Components[index].Component = component;
		}

		/// <summary>
		/// Replaces the component of the given entity.
		/// </summary>
		/// <param name="entity">: ID of the entity that we want to modify</param>
		/// <param name="...args">: Arguments needed for the construction of the component</param>
		/// <returns>the newly created component</returns>
		template<typename... Args>
		requires IsConstructibleConstraint<Comp, Args...>
		[[nodiscard]] Comp& Replace(EntityID entity, Args&&... args)
		{
			if (!HasComponent(entity))
				throw NoComponentException();

			uint32_t index = m_ComponentMap[entity];
			m_Components[index].Component = Comp(std::forward<Args>(args)...);
			return m_Components[index].Component;
		}

		/// <summary>
		/// Replace the component attached to the entity if it exists, else add a new component to the entity.
		/// </summary>
		/// <param name="entity">: ID of the entity that we want to modify</param>
		/// <param name="component">: Component that will replace/be added the current one</param>
		void ReplaceOrAdd(EntityID entity, const Comp& component)
		{
			if (HasComponent(entity))
				Replace(entity, component);
			else
				Add(entity, component);
		}

		/// <summary>
		/// Replace the component attached to the entity if it exists, else emplace a new component to the entity.
		/// </summary>
		/// <param name="entity">: ID of the entity that we want to modify</param>
		/// <param name="...args">: Arguments needed for the construction of the new component</param>
		/// <returns>the newly attached component</returns>
		template<typename... Args>
		requires IsConstructibleConstraint<Comp, Args...>
		[[nodiscard]] Comp& ReplaceOrEmplace(EntityID entity, Args&&... args)
		{
			if (HasComponent(entity))
				Replace(entity, std::forward<Args>(args)...);
			else
				Emplace(entity, std::forward<Args>(args)...);
		}

		/// <summary>
		/// Removes a component of the given entity if it exists.
		/// </summary>
		/// <param name="entity">: ID of the entity that we want to modify</param>
		/// <returns>a boolean that's true if a component has been deleted, false otherwise.</returns>
		virtual bool Remove(EntityID entity) override final
		{
			if (!HasComponent(entity))
				return false;

			uint32_t componentIndex = m_ComponentMap[entity];
			if (componentIndex != m_Components.size() - 1)
			{
				std::swap(m_Components[componentIndex], m_Components[m_Components.size() - 1]);
				m_Components.pop_back();
				m_ComponentMap[m_Components[componentIndex].Entity] = componentIndex;
				m_ComponentMap.erase(entity);
				return true;
			}
			else
			{
				m_Components.pop_back();
				m_ComponentMap.erase(entity);
				return true;
			}
		}

		/// <summary>
		/// Returns a ref to the component attached to the entity if it exists. Throws a `NoComponentException` otherwise.
		/// </summary>
		/// <param name="entity">: ID of the entity that we want to modify</param>
		/// <returns>a component</returns>
		[[nodiscard]] Comp& GetComponent(EntityID entity)
		{
			if (!HasComponent(entity))
				throw NoComponentException();
			return m_Components[m_ComponentMap[entity]].Component;
		}

		/// <summary>
		/// Check if an entity has a component or not.
		/// </summary>
		/// <param name="entity">: ID of the entity that we want to modify</param>
		/// <returns>a boolean that's true if there is a component, false otherwise</returns>
		inline bool HasComponent(EntityID entity) { return m_ComponentMap.contains(entity); }

		/// <summary>
		/// Returns the size of the component list.
		/// </summary>
		[[nodiscard]] inline uint32_t Size() { return uint32_t(m_Components.size()); }

	public:
		///////////////////////////////////////////////////////////////////
		//// iterator + operator //////////////////////////////////////////
		///////////////////////////////////////////////////////////////////
		
		ConstIterator begin() const
		{
			return m_Components.begin();
		}

		ConstIterator end() const
		{
			return m_Components.end();
		}

		Comp& operator[](EntityID entity)
		{
			return GetComponent(entity);
		}

	private:
		std::vector<EntityComponentPair> m_Components;
	};
}