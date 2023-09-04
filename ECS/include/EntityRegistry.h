#pragma once

#include "Types.h"
#include "CircularBuffer.h"
#include "BaseSystem.h"
#include "ComponentList.h"

namespace ECS
{
	class EntityIDOutOfRange : public std::exception
	{
	public:
		EntityIDOutOfRange() 
			: exception("EntityID is too big.") {}
	};

	class InvalidComponentTypeException : public std::exception
	{
	public:
		InvalidComponentTypeException()
			: exception("The given component type doesn't exist as a storage!") {}
	};

	class EntityRegistry
	{
	public:
		EntityRegistry()
			: m_EntityCount(0)
			, m_MaxEntityCount(4096)
			, m_AvailableEntities(m_MaxEntityCount)
			, m_Entities()
			, m_EntityEvents(256)
		{
			Init();
		}

		EntityRegistry(uint32_t MaxEntityCount)
			: m_MaxEntityCount(MaxEntityCount)
		{
			Init();
		}

		/// <summary>
		/// Should only be done once for any types of storage.
		/// </summary>
		/// <typeparam name="...Storage">: The different component types that we want to initialize its lists.</typeparam>
		template<ComponentConstraint... Storage>
		void CreateComponentStorage()
		{
			(CreateComponentList<Storage>(), ...);
		}

		/// <summary>
		/// Creates an entity with no components attached to it.
		/// </summary>
		/// <returns>the ID of an entity</returns>
		const EntityID CreateEntity()
		{
			const EntityID entity = m_AvailableEntities.PopFront();
			m_Entities.emplace_back(entity);
			m_EntitiesSignature[entity] = SignatureEntityPair{ EntitySignature(), uint32_t(m_Entities.size()) - 1 };
			++m_EntityCount;
			return entity;
		}

		/// <summary>
		/// Deletes an entity with its components.
		/// </summary>
		/// <param name="entity">: ID of the entity to be deleted</param>
		void DeleteEntity(EntityID entity)
		{
			if (entity > m_MaxEntityCount)
				throw EntityIDOutOfRange();
			if (!m_EntitiesSignature.contains(entity))
				return;
			std::size_t count = m_EntitiesSignature[entity].Signature.count();
			uint32_t i = -1;
			while (count > 0)
			{
				if (!m_EntitiesSignature[entity].Signature[++i])
					continue;
				ComponentTypeID componentType;
				componentType.set(i);
				m_ComponentsLists[componentType]->Remove(entity);
				--count;
			}

			--m_EntityCount;
			EntityID lastEntity = m_Entities.back();
			uint32_t entityIndex = m_EntitiesSignature[entity].Index;
			m_Entities[entityIndex] = lastEntity;
			m_Entities.pop_back();
			m_EntitiesSignature[lastEntity].Index = entityIndex;
			m_EntitiesSignature.erase(entity);
			m_AvailableEntities.PushBack(entity);
		}

	public:
		///////////////////////////////////////////////////////////////////
		//// Component operations /////////////////////////////////////////
		///////////////////////////////////////////////////////////////////

		/// <summary>
		/// Deletes a component of the specified component type that has been added to the specified entity.
		/// </summary>
		/// <typeparam name="Comp">: Type of the component to be deleted.</typeparam>
		/// <param name="entity">: ID of the targeted entity</param>
		template<ComponentConstraint Comp>
		void DeleteComponent(EntityID entity)
		{
			ComponentTypeID compType = ComponentType<Comp>();
		#ifdef _DEBUG
			if (entity > m_MaxEntityCount)
				throw EntityIDOutOfRange();
			if (!m_ComponentsLists.contains(compType))
				throw InvalidComponentTypeException();
		#endif // _DEBUG

			if (m_ComponentsLists[compType]->Remove(entity))
				m_EntitiesSignature[entity].Signature &= compType.flip();
		}

		/// <summary>
		/// Adds a component that has already been created outside by you. Otherwise, use Emplace to create a new component that will be directly attached to the entity.
		/// </summary>
		/// <param name="entity">: ID of the entity that we want to modify</param>
		/// <param name="component">: Component to be added to the list</param>
		template<ComponentConstraint Comp>
		void AddComponent(EntityID entity, const Comp& component)
		{
			ComponentTypeID compType = ComponentType<Comp>();
		#ifdef _DEBUG
			if (!m_ComponentsLists.contains(compType))
				throw InvalidComponentTypeException();
		#endif // _DEBUG

			std::shared_ptr<ComponentList<Comp>>& list = (std::shared_ptr<ComponentList<Comp>>&)m_ComponentsLists[compType];
			list->Add(entity, component);
			m_EntitiesSignature[entity].Signature |= compType;
		}

		/// <summary>
		/// Attaches a component of the specified type and constructed with the given arguments to an entity. If you have already constructed the component, use add component instead.
		/// </summary>
		/// <param name="entity">: ID of the entity that we want to modify</param>
		/// <param name="...args">arguments used to construct the component</param>
		/// <returns>the newly created component</returns>
		template<ComponentConstraint Comp, IsConstructibleConstraint... Args>
		[[nodiscard]] Comp& EmplaceComponent(EntityID entity, Args&&... args)
		{
			ComponentTypeID compType = ComponentType<Comp>();
		#ifdef _DEBUG
			if (!m_ComponentsLists.contains(compType))
				throw InvalidComponentTypeException();
		#endif // _DEBUG

			std::shared_ptr<ComponentList<Comp>>& list = (std::shared_ptr<ComponentList<Comp>>&)m_ComponentsLists[compType];
			Comp& component = list->Emplace(entity, std::forward<Args>(args)...);
			m_EntitiesSignature[entity].Signature |= compType;
			return component;
		}

		/// <summary>
		/// Replaces the component of the specified type of the given entity.
		/// </summary>
		/// <param name="entity">: ID of the entity that we want to modify</param>
		/// <param name="component">: New Component that will replace the current one</param>
		template<ComponentConstraint Comp>
		void ReplaceComponent(EntityID entity, const Comp& component)
		{
		#ifdef _DEBUG
			if (!m_ComponentsLists.contains(ComponentType<Comp>()))
				throw InvalidComponentTypeException();
		#endif // _DEBUG

			std::shared_ptr<ComponentList<Comp>>& list = (std::shared_ptr<ComponentList<Comp>>&)m_ComponentsLists[ComponentType<Comp>()];
			list->Replace(entity, component);
		}

		/// <summary>
		/// Replaces the component of the specified type of the given entity.
		/// </summary>
		/// <param name="entity">: ID of the entity that we want to modify</param>
		/// <param name="...args">: Arguments needed for the construction of the component</param>
		/// <returns>the newly created component</returns>
		template<ComponentConstraint Comp, IsConstructibleConstraint... Args>
		[[nodiscard]] Comp& ReplaceComponent(EntityID entity, Args... args)
		{
			ComponentTypeID compType = ComponentType<Comp>();
		#ifdef _DEBUG
			if (!m_ComponentsLists.contains(compType))
				throw InvalidComponentTypeException();
		#endif

			std::shared_ptr<ComponentList<Comp>>& list = (std::shared_ptr<ComponentList<Comp>>&)m_ComponentsLists[compType];
			Comp& comp = list->Replace(entity, std::forward<Args>(args)...);
			return comp;
		}

		/// <summary>
		/// Checks if the given entity has a component of the specified type.
		/// </summary>
		/// <typeparam name="Comp">: Type of the component that we want to check</typeparam>
		/// <param name="entity">: ID of the entity that we want to modify</param>
		/// <returns>a boolean that's true if the component has been found, false otherwise</returns>
		template<ComponentConstraint Comp>
		inline bool HasComponent(EntityID entity)
		{
		#ifdef _DEBUG
			if (!m_ComponentsLists.contains(ComponentType<Comp>()))
				throw InvalidComponentTypeException();
		#endif // _DEBUG

			std::shared_ptr<ComponentList<Comp>>& list = (std::shared_ptr<ComponentList<Comp>>&)m_ComponentsLists[ComponentType<Comp>()];
			return list->HasComponent(entity);
		}

		/// <summary>
		/// Checks if the given entity has components of the specified types.
		/// </summary>
		/// <typeparam name="...Comps">: Types of the components that we want to check</typeparam>
		/// <param name="entity">: ID of the entity that we want to modify</param>
		/// <returns>a boolean that's true if the components have been found, false otherwise</returns>
		template<ComponentConstraint... Comps>
		inline bool HasComponents(EntityID entity)
		{
			return (HasComponent<Comps>(entity) && ...);
		}

		template<ComponentConstraint Comp>
		inline Comp& GetComponent(EntityID entity)
		{
		#ifdef _DEBUG
			if (!m_ComponentsLists.contains(ComponentType<Comp>()))
				throw InvalidComponentTypeException();
		#endif // DEBUG

			std::shared_ptr<ComponentList<Comp>>& list = (std::shared_ptr<ComponentList<Comp>>&)m_ComponentsLists[ComponentType<Comp>()];
			return list->GetComponent(entity);
		}

		template<ComponentConstraint... Comps>
		inline std::tuple<Comps&...> GetComponents(EntityID entity)
		{
			return { GetComponent<Comps>(entity)... };
		}

		template<ComponentConstraint Comp>
		const std::shared_ptr<const ComponentList<Comp>>& View() const
		{
		#ifdef _DEBUG
			if (!m_ComponentsLists.contains(ComponentType<Comp>()))
				throw InvalidComponentTypeException();
		#endif // DEBUG
			return (std::shared_ptr<const ComponentList<Comp>>&)m_ComponentsLists.at(ComponentType<Comp>());
		}

	private:
		uint32_t m_EntityCount = 0;
		uint32_t m_MaxEntityCount = 5000;

		struct SignatureEntityPair
		{
			EntitySignature Signature;
			uint32_t Index = -1;
		};

		enum class EntityEventType
		{
			Created,
			Deleted,
			ComponentAdded,
			ComponentRemoved
		};

		struct EntityEvent
		{
			EntityEventType Type;
			EntityID Entity;
			ComponentTypeID ComponentType;
		};

		CircularBuffer<EntityID> m_AvailableEntities;
		std::vector<EntityID> m_Entities;
		CircularBuffer<EntityEvent> m_EntityEvents;
		std::unordered_map<EntityID, SignatureEntityPair> m_EntitiesSignature;
		std::unordered_map<SystemTypeID, std::shared_ptr<BaseSystem>> m_Systems;
		std::unordered_map<ComponentTypeID, std::shared_ptr<IComponentList>> m_ComponentsLists;

	private:

		/// <summary>
		/// Initialization helper
		/// </summary>
		void Init()
		{
			m_EntitiesSignature.reserve(m_MaxEntityCount);
			m_Entities.reserve(m_MaxEntityCount);
			for (EntityID i = 0; i < m_MaxEntityCount; ++i)
			{
				m_AvailableEntities.PushBack(i);
			}
		}
		
		/// <summary>
		/// Helper to create component list also known as `component storage` to the user.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		template<ComponentConstraint Comp>
		inline void CreateComponentList()
		{
			ComponentTypeID compType = ComponentType<Comp>();
			if (m_ComponentsLists.contains(compType))
				return;
			m_ComponentsLists[compType].reset(new ComponentList<Comp>(m_MaxEntityCount / 10));
		}
	};
}