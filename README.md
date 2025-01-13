# ECS

## Intro

This is a simple ECS (Entity Component System) implementation in c++ 20.

I'm trying to achieve an Archetype based ECS, where entities are grouped by their components so that we can take advantage of SoA & auto-vectorization.

No real performance tests have been done yet since it's a pet project and I don't want to spend too much time on this.

This ECS has some limitations:
- Has a fixed number of entities tu be assigned when you create the registry. This will probably evolve to be dynamic in the future.
- It has a fixed number of components per entity. The limit is 64 and I don't plan on changing it because it would either require for me to use sets OR dynamic bitsets. On one hand, it will be slower and on the other hand, I'd have to handle when the bitset grows further than what we want. Either way, we should know the number of components we want to use in advance.

## How to use

You can check the test.cpp file for an example on how to use this ECS but basically it goes like this:

```cpp
#include "include/ecs.h"

int main() 
{
    ecs::Registry registry(1000); // Create a registry with 1000 entities

    // Create a component
    struct Position {
        float x, y;
    };
    struct Velocity {
        float x, y;
    };

    // must be called before using the component
    // preferably at the start of the program
    ecs::Registry::RegisterComponentTypes<Position, Velocity>(); 

    // Create an entity
    auto entity = registry.CreateEntity();
    auto entity2 = registry.CreateEntity();

    Position position = {1.0f, 2.0f};
    // Add the component to the entity
    // has error checking
    registry.TryAddComponent<Position>(entity, position);
    registry.TryAddComponent<Velocity>(entity, position);

    // will throw if a component of the same type already exists
    // but returns a reference to the component
    // this reference will be invalid if the entity signature changes
    Position& p = registry.EmplaceComponent<Position>(entity2, 1.0f, 2.0f);
    p.x = 2.0f;

    // Get the component from the entity
    // also has GetComponents which returns a tuple of components
    // always returns a reference to the component
    auto& p1 = registry.GetComponent<Position>(entity);

    
    // Do something with the component
    p1.x = 3.0f;
    p1.y = 4.0f;

    // Get a view of entities that have at least these components
    // it WILL iterate faster than if you yourself try to iterate over 
    // the entities via the registry
    // prefered way of looping over entities and takes 
    // The time to create the view is O(N) where N is the number
    // of Archetypes that have at least these components.
    // Basically negligible.
    auto view = registry.GetView<Position>();

    for (auto index : view) 
    {
        // taking advantage of structured bindings to get a reference to the components
        auto[p] = view.Get(index);
        std::cout << p.x << " " << p.y << std::endl;
    }

    // add a component to the destroy list
    registry.RemoveComponent<Position>(entity);

    // add entity to a destroy list
    registry.DestroyEntity(entity);

    // Remove all entities & components that have been destroyed
    registry.Flush();

    return 0;
}
```