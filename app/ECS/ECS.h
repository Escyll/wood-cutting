#ifndef ECS_ECS_H
#define ECS_ECS_H

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <unordered_set>

using Entity = uint32_t;
static inline Entity MaxEntities = 1'000'000;

struct ComponentStorageBase {};

template<typename T>
struct ComponentStorage : ComponentStorageBase
{
    std::vector<T> dense;
    std::vector<Entity> denseIndex{MaxEntities};
    std::unordered_set<Entity> m_entities;

    bool contains(Entity entity)
    {
        return denseIndex[entity] != 0;
    }

    T& insert(Entity entity, const T& component)
    {
        m_entities.insert(entity);
        auto index = dense.size();
        dense.push_back(component);
        denseIndex[entity] = index;
        return dense[index];
    }

    T& replace(Entity entity, const T& component)
    {
        auto index = denseIndex[entity];
        dense[index] = component;
        return dense[index];
    }

    T& insert_or_replace(Entity entity, const T& component)
    {
        return contains(entity) ? replace(entity, component) : insert(entity, component);
    }

    T& get(Entity entity)
    {
        auto index = denseIndex[entity];
        return dense[index];
    }

    const std::unordered_set<Entity>& entities() const
    {
        return m_entities;
    }
};

class Registry
{
public:
    Entity create() const;
    
    template<typename T>
    ComponentStorage<T>& getStorage()
    {
        if (not m_storage.contains(typeid(T)))
            m_storage.insert({typeid(T), new ComponentStorage<T>});
        return static_cast<ComponentStorage<T>&>(*m_storage[typeid(T)]);
    }

    template<typename T>
    T& insert_or_replace(Entity entity, const T& component)
    {
        return getStorage<T>().insert_or_replace(entity, component);
    }
    template<typename T>
    T& insert(Entity entity, const T& component)
    {
        return getStorage<T>().insert(entity, component);
    }
    template<typename T>
    T& replace(Entity entity, const T& component)
    {
        return getStorage<T>().replace(entity, component);
    }
    template<typename T>
    T& get(Entity entity)
    {
        return getStorage<T>().get(entity);
    }

    template<typename Component>
    std::unordered_set<Entity> all()
    {
       return getStorage<Component>().entities();
    }

    template<typename Component, typename... OtherComponents> requires (sizeof...(OtherComponents) >= 1)
    std::unordered_set<Entity> all()
    {
        auto otherEntities = all<OtherComponents...>();
        auto entities = all<Component>();
        for (auto entity : otherEntities)
        {
            entities.insert(entity);
        }
        return entities;
    }
        

private:
    std::unordered_map<std::type_index, ComponentStorageBase*> m_storage;
};

Entity Registry::create() const
{
    static Entity nextEntity = 1;
    Entity entity = nextEntity++;
    return entity;
}

#endif
