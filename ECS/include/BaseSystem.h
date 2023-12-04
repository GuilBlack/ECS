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

        virtual void AddEntity(EntityID entity)
        {
            m_Entities.insert(entity);
        }

        virtual void RemoveEntity(EntityID entity)
        {
            m_Entities.erase(entity);
        }

        template<ComponentConstraint T>
        inline void AddComponentSignature() { m_AcceptableEntitySignature |= ComponentType<T>(); }

        template<ComponentConstraint... Comps>
        inline void AddComponentSignatures() { m_AcceptableEntitySignature |= (ComponentType<Comps>() | ...); }

        inline const EntitySignature& GetAcceptableSignature() const { return m_AcceptableEntitySignature; }

    protected:
        friend class EntityRegistry;
        EntitySignature m_AcceptableEntitySignature;
        std::unordered_set<EntityID> m_Entities;
    };
}