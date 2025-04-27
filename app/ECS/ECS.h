#ifndef ECS_ECS_H
#define ECS_ECS_H

#include <cstdint>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <typeindex>
#include <glm/glm.hpp>
#include <unordered_set>
#include <tuple>
#include <functional>
#include <algorithm>

#define DEBUGGING false

using Entity = uint32_t;
static inline Entity MaxEntities = 100'000;

struct ComponentStorageBase
{
    virtual ~ComponentStorageBase() {}
    virtual void remove(Entity entity) = 0;
};

template <typename T>
struct ComponentStorage : ComponentStorageBase
{
    std::vector<T> dense;
    std::vector<Entity> denseIndex = std::vector<Entity>(MaxEntities);
    std::vector<Entity> denseEntity;

    bool contains(Entity entity)
    {
        auto denseId = denseIndex[entity];
        return denseId < denseEntity.size() && denseEntity[denseId] == entity;
    }

    void remove(Entity entity) override
    {
        if (contains(entity))
        {
            Entity lastEntity = denseEntity.back();
            size_t lastIndex = denseEntity.size() - 1;
            size_t entityIndex = denseIndex[entity];
            std::swap(denseEntity[lastIndex], denseEntity[entityIndex]);
            std::swap(dense[lastIndex], dense[entityIndex]);
            denseIndex[lastEntity] = entityIndex;
            denseIndex[entity] = MaxEntities;
            denseEntity.pop_back();
            dense.pop_back();
            if constexpr(DEBUGGING && std::is_same<T, glm::vec2>::value)
            {
                std::cerr << "Removing " << entity << std::endl;
                std::cerr << "denseEntity: ";
                for (auto en: denseEntity)
                {
                    std::cerr << en << " ";
                }
                std::cerr << std::endl;
                std::cerr << "denseIndex: ";
                for (auto in: denseIndex)
                {
                    std::cerr << in << " ";
                }
                std::cerr << std::endl;
                std::cerr << "dense: ";
                for (auto pos: dense)
                {
                    std::cerr << pos.x << " " << pos.y << " , ";
                }
                std::cerr << std::endl;

            }
        }
    }

    T &insert(Entity entity, const T &component)
    {
        auto index = dense.size();
        dense.push_back(component);
        denseEntity.push_back(entity);
        denseIndex[entity] = index;
        if constexpr(DEBUGGING && std::is_same<T, glm::vec2>::value)
        {
            std::cerr << "Inserting " << entity << " at " << component.x << " " << component.y << std::endl;
            std::cerr << "denseEntity: ";
            for (auto en: denseEntity)
            {
                std::cerr << en << " ";
            }
            std::cerr << std::endl;
            std::cerr << "denseIndex: ";
            for (auto in: denseIndex)
            {
                std::cerr << in << " ";
            }
            std::cerr << std::endl;
            std::cerr << "dense: ";
            for (auto pos: dense)
            {
                std::cerr << pos.x << " " << pos.y << " , ";
            }
            std::cerr << std::endl;

        }
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

    const std::vector<Entity> &entities() const
    {
        return denseEntity;
    }

    std::vector<T *> get(const std::vector<Entity> &entities) const
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

    template <typename T>
    bool has(Entity entity)
    {
        return getStorage<T>()->contains(entity);
    }

    template <typename T>
    void remove(Entity entity)
    {
        return getStorage<T>()->remove(entity);
    }

    void remove(Entity entity)
    {
        for (auto& [_, storage] : m_storage)
        {
            storage->remove(entity);
        }
    }

    template <typename Component>
    std::vector<Entity> getEntities()
    {
        return getStorage<Component>()->entities();
    }

    template <typename Component, typename... OtherComponents>
        requires(sizeof...(OtherComponents) >= 1)
    std::vector<Entity> getEntities()
    {
        auto otherEntities = getEntities<OtherComponents...>();
        auto entities = getStorage<Component>()->entities();
        std::erase_if(otherEntities, [storage = getStorage<Component>()](Entity e)
                      { return !storage->contains(e); });
        return otherEntities;
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

inline Entity Registry::create()
{
    Entity entity = nextEntity++;
    return entity;
}

#endif
