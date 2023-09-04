#pragma once

#include "Types.h"

namespace ECS
{
	class BaseSystem
	{
	public:
		BaseSystem() = default;
		virtual ~BaseSystem() = default;
		
		virtual void OnAttach() {}
		virtual void OnUpdate() {}
		virtual void OnDestroy() {}

		virtual void AddEntity(EntityID entity) {}

		virtual void RemoveEntity(EntityID entity) {}

		template<ComponentConstraint T>
		inline void AddComponentSignature() { m_AcceptableEntitySignature |= ComponentType<T>(); }

		inline const EntitySignature& GetAcceptableSignature() const { return m_AcceptableEntitySignature; }

	protected:
		friend class EntityRegistry;
		EntitySignature m_AcceptableEntitySignature;
	};
}