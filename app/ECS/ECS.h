#ifndef ECS_ECS_H
#define ECS_ECS_H

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <glm/glm.hpp>
#include <unordered_set>
#include <tuple>
#include <functional>

using Entity = uint32_t;
static inline Entity MaxEntities = 1'000'000;

struct ComponentStorageBase
{
};

template <typename T>
struct ComponentStorage : ComponentStorageBase
{
    std::vector<T> dense;
    std::vector<Entity> denseIndex;
    std::unordered_set<Entity> m_entities;

    ComponentStorage()
    {
        denseIndex.resize(MaxEntities);
    }

    bool contains(Entity entity)
    {
        return denseIndex[entity] != 0;
    }

    T &insert(Entity entity, const T &component)
    {
        m_entities.insert(entity);
        auto index = dense.size();
        dense.push_back(component);
        denseIndex[entity] = index;
        return dense.at(index);
    }

    T &replace(Entity entity, const T &component)
    {
        auto index = denseIndex[entity];
        dense[index] = component;
        return dense[index];
    }

    T &insert_or_replace(Entity entity, const T &component)
    {
        return contains(entity) ? replace(entity, component) : insert(entity, component);
    }

    T &get(Entity entity)
    {
        auto index = denseIndex[entity];
        return dense[index];
    }

    const std::unordered_set<Entity> &entities() const
    {
        return m_entities;
    }

    std::vector<T *> get(const std::unordered_set<Entity> &entities) const
    {
        std::vector<T *> result;
        for (auto entity : entities)
        {
            result.push_back(dense[denseIndex[entity]]);
        }
        return result;
    }
};

class Registry
{
public:
    Entity create();
    ~Registry() {
        for (auto comp : m_storage)
        {
            delete comp.second;
        }
    }

    template <typename T>
    ComponentStorage<T>* getStorage()
    {
        if (not m_storage.contains(typeid(T)))
            m_storage.insert({typeid(T), new ComponentStorage<T>});
        return static_cast<ComponentStorage<T>*>(m_storage[typeid(T)]);
    }

    template <typename T>
    T &insert_or_replace(Entity entity, const T &component)
    {
        return getStorage<T>()->insert_or_replace(entity, component);
    }
    template <typename T>
    T &insert(Entity entity, const T &component)
    {
        return getStorage<T>()->insert(entity, component);
    }
    template <typename T>
    T &replace(Entity entity, const T &component)
    {
        return getStorage<T>()->replace(entity, component);
    }
    template <typename T>
    T &get(Entity entity)
    {
        return getStorage<T>()->get(entity);
    }

    template <typename Components>
    std::unordered_set<Entity> getEntities()
    {
        return getStorage<Components>()->entities();
    }

    template <typename Component, typename... OtherComponents>
        requires(sizeof...(OtherComponents) >= 1)
    std::unordered_set<Entity> getEntities()
    {
        auto otherEntities = getEntities<OtherComponents...>();
        auto entities = getStorage<Component>()->entities();
        std::erase_if(entities, [&otherEntities](Entity e)
                      { return !otherEntities.contains(e); });
        return entities;
    }

    template <typename... Components>
    auto each()
    {
        auto entities = getEntities<Components...>();
        std::vector<decltype(std::make_tuple(*entities.begin(), std::ref(getStorage<Components>()->get(*entities.begin()))...))> result;
        for (auto entity : entities)
        {
            result.push_back(std::make_tuple(entity, std::ref(getStorage<Components>()->get(entity))...));
        }
        return result;
    }

private:
    std::unordered_map<std::type_index, ComponentStorageBase *> m_storage;
    int nextEntity = 1;
};

Entity Registry::create()
{
    Entity entity = nextEntity++;
    return entity;
}

#endif
