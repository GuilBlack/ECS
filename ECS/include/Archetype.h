#pragma once
#include "Types.h"

namespace ecs
{

struct IComponentStorage
{
    virtual ~IComponentStorage() = default;
};

template<ComponentConstraint Comp>
struct ComponentStorage : public IComponentStorage
{
    std::vector<Comp> Components;
};

class Archetype
{
public:
    Archetype() = default;
    ~Archetype() = default;

    void AddEntity(EntityID entity)
    {
        m_Entities.push_back(entity);
        m_EntityIndexMap[entity] = (uint32_t)m_Entities.size() - 1;
    }

    // Remove an entity from this archetype
    void RemoveEntity(EntityID entity)
    {
        if (!m_EntityIndexMap.contains(entity))
            return;

        uint32_t index = m_EntityIndexMap[entity];
        uint32_t lastIndex = (uint32_t)m_Entities.size() - 1;

        // Swap and pop to maintain contiguous storage
        if (index != lastIndex)
        {
            std::swap(m_Entities[index], m_Entities[lastIndex]);
            m_EntityIndexMap[m_Entities[index]] = index;
        }

        m_Entities.pop_back();
        m_EntityIndexMap.erase(entity);
    }

    // more for testing than anything else
    [[nodiscard]] bool HasEntity(EntityID entity) const
    {
        return m_EntityIndexMap.contains(entity);
    }

    [[nodiscard]] uint32_t GetEntityIndex(EntityID entity) const
    {
        assert(m_EntityIndexMap.contains(entity) && "This archetype doesn't contain this entity");
        return m_EntityIndexMap.at(entity);
    }

    [[nodiscard]] const std::vector<EntityID>& GetEntities() const
    {
        return m_Entities;
    }

    [[nodiscard]] uint32_t GetEntityCount() const
    {
        return (uint32_t)m_Entities.size();
    }

    template<ComponentConstraint Comp, typename... Args>
    Comp& EmplaceComponent(EntityID entity, Args&&... args)
    {
        assert(m_EntityIndexMap.contains(entity) && "This archetype doesn't contain this entity");

        auto& compStorage = GetComponentStorage<Comp>();
        compStorage.Components.emplace_back(std::forward<Args>(args)...);
        return compStorage.Components.back();
    }

    template<ComponentConstraint Comp>
    void AddComponent(EntityID entity, const Comp& comp)
    {
        assert(m_EntityIndexMap.contains(entity) && "This archetype doesn't contain this entity");

        auto& compStorage = GetComponentStorage<Comp>();
        compStorage.Components.emplace_back(comp);
    }

    template<ComponentConstraint ...Comps>
    void AddComponents(EntityID entity, Comps... comps)
    {
        (AddComponent<Comps>(entity, comps), ...);
    }

    template<ComponentConstraint Comp>
    bool RemoveComponent(EntityID entity)
    {
        auto type = GetComponentTypeIndex<Comp>();
        if (!m_EntityIndexMap.contains(entity))
            return false;
        if (!m_ComponentStorages.contains(type))
            return false;

        auto& compStorate = *static_cast<ComponentStorage<Comp>*>(m_ComponentStorages[type].get());
        auto index = m_EntityIndexMap[entity];
        auto lastIndex = m_Entities.size() - 1;

        if (index != lastIndex)
        {
            std::swap(compStorate.Components[index], compStorate.Components[lastIndex]);
        }
        compStorate.Components.pop_back();
        return true;
    }

    template<ComponentConstraint ...Comps>
    void RemoveComponents(EntityID entity)
    {
        (RemoveComponent<Comps>(entity), ...);
    }

    template<ComponentConstraint Comp>
    [[nodiscard]] Comp& GetComponent(EntityID entity)
    {
        auto type = GetComponentTypeIndex<Comp>();
        assert(m_ComponentStorages.contains(type) && "Component storage doesn't exist.");
        auto& compStorage = *static_cast<ComponentStorage<Comp>*>(m_ComponentStorages[type].get());
        auto index = m_EntityIndexMap[entity];
        return compStorage.Components[index];
    }

    template<ComponentConstraint... Comps>
    [[nodiscard]] inline std::tuple<Comps&...> GetComponents(EntityID entity)
    {
        return { GetComponent<Comps>(entity)... };
    }

    template<ComponentConstraint Comp>
    [[nodiscard]] ComponentStorage<Comp>& GetComponentStorage()
    {
        auto type = GetComponentTypeIndex<Comp>();
        assert(m_ComponentStorages.contains(type) && "Component storage doesn't exist.");
        return *static_cast<ComponentStorage<Comp>*>(m_ComponentStorages[type].get());
    }

    template<ComponentConstraint Comp>
    void CreateComponentStorage()
    {
        auto type = GetComponentTypeIndex<Comp>();
        if (m_ComponentStorages.contains(type))
            return;
        m_ComponentStorages[type] = std::make_unique<ComponentStorage<Comp>>();
    }

    template<ComponentConstraint ...Comps>
    void CreateComponentStorages()
    {
        (CreateComponentStorage<Comps>(), ...);
    }

private:
    std::vector<EntityID> m_Entities;
    std::unordered_map<EntityID, uint32_t> m_EntityIndexMap;
    std::unordered_map<ComponentTypeIndex, std::unique_ptr<IComponentStorage>> m_ComponentStorages;

    friend class EntityRegistry;
};
}
