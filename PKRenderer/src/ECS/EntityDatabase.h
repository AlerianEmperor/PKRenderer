#pragma once
#include "PrecompiledHeader.h"
#include "Utilities/BufferView.h"
#include "Utilities/Ref.h"
#include "Core/Services/Log.h"
#include "Core/Services/IService.h"

namespace PK::ECS
{
    enum class ENTITY_GROUPS
    {
        INVALID = 0,
        INACTIVE = 1,
        ACTIVE = 2,
        FREE = 3
    };

    struct EGID
    {
        public:
            constexpr uint32_t entityID() const { return (uint32_t)(m_GID & 0xFFFFFFFF); }
            constexpr uint32_t groupID() const { return (uint32_t)(m_GID >> 32); }
            constexpr EGID() : m_GID(0) {}
            constexpr EGID(const EGID& other) : m_GID(other.m_GID) {}
            constexpr EGID(uint64_t identifier) : m_GID(identifier) {}
            constexpr EGID(uint32_t entityID, uint32_t groupID) : m_GID((uint64_t)groupID << 32 | ((uint64_t)(uint32_t)entityID & 0xFFFFFFFF)) {}
            constexpr bool IsValid() const { return m_GID > 0; }
            constexpr bool operator ==(const EGID& obj2) const { return m_GID == obj2.m_GID; }
            constexpr bool operator !=(const EGID& obj2) const { return m_GID != obj2.m_GID; }
            constexpr bool operator <(const EGID& obj2) const { return m_GID < obj2.m_GID; }
            constexpr bool operator >(const EGID& obj2) const { return m_GID > obj2.m_GID; }

        private:
            uint64_t m_GID;
    };

    constexpr static const EGID EGIDDefault = EGID(1);
    constexpr static const EGID EGIDInvalid = EGID(0);

    struct IImplementer
    {
        virtual ~IImplementer() = default;
    };

    struct IEntityView
    {
        EGID GID;
        virtual ~IEntityView() = default;
    };
    
    constexpr static const uint32_t PK_ECS_BUCKET_SIZE = 32000;

    struct ImplementerBucket
    {
        void (*destructor)(void* value);
        void* data;

        ~ImplementerBucket() { destructor(data); }
    };

    struct ImplementerContainer
    {
        size_t count = 0;
        std::vector<Utilities::Scope<ImplementerBucket>> buckets;
    };

    struct EntityViewsCollection
    {
        std::map<uint32_t, size_t> indices;
        std::vector<uint8_t> buffer;
    };

    struct ViewCollectionKey
    {
        std::type_index type;
        uint32_t group;
        
        inline bool operator < (const ViewCollectionKey& r) const noexcept
        {
            return (type < r.type) || ((type == r.type) && (group < r.group));
        }
    };


    class EntityDatabase : public Core::Services::IService
    {
        public:
            constexpr uint32_t ReserveEntityId() { return ++m_idCounter; }
            inline EGID ReserveEntityId(uint32_t groupId) { return EGID(ReserveEntityId(), groupId); }

            template<typename T>
            T* ReserveImplementer()
            {
                static_assert(std::is_base_of<IImplementer, T>::value, "Template argument type does not derive from IImplementer!");

                auto type = std::type_index(typeid(T));
                auto& container = m_implementerBuckets[type];

                uint64_t elementsPerBucket = PK_ECS_BUCKET_SIZE / sizeof(T);
                uint64_t bucketIndex = container.count / elementsPerBucket;
                uint64_t subIndex = container.count - bucketIndex * elementsPerBucket;
                ++container.count;

                if (container.buckets.size() <= bucketIndex)
                {
                    auto newBucket = new ImplementerBucket();
                    newBucket->data = new T[elementsPerBucket];
                    newBucket->destructor = [](void* v) { delete[] reinterpret_cast<T*>(v); };
                    container.buckets.push_back(Utilities::Scope<ImplementerBucket>(newBucket));
                }

                return reinterpret_cast<T*>(container.buckets.at(bucketIndex).get()->data) + subIndex;
            }

            template<typename TView>
            TView* ReserveEntityView(const EGID& egid)
            {
                static_assert(std::is_base_of<IEntityView, TView>::value, "Template argument type does not derive from IEntityView!");
                PK_THROW_ASSERT(egid.IsValid(), "Trying to acquire resources for an invalid egid!");

                auto& views = m_entityViews[{ std::type_index(typeid(TView)), egid.groupID() }];
                auto offset = views.buffer.size();
                views.buffer.resize(offset + sizeof(TView));
                views.indices[egid.entityID()] = offset;
                auto* element = reinterpret_cast<TView*>(views.buffer.data() + offset);
                element->GID = egid;
                return element;
            }

            template<typename TImpl, typename TView, typename ...M>
            TView* ReserveEntityView(TImpl* implementer, const EGID& egid, M TView::* ...params)
            {
                auto* view = ReserveEntityView<TView>(egid);
                static_assert((... && std::is_assignable<decltype(view->*params), TImpl*>::value), "Components are not present in implementer");
                ((view->*params = static_cast<M>(implementer)), ...);
                return view;
            }

            template<typename TView>
            const Utilities::BufferView<TView> Query(const uint32_t group)
            {
                static_assert(std::is_base_of<IEntityView, TView>::value, "Template argument type does not derive from IEntityView!");
                PK_THROW_ASSERT(group, "Trying to acquire resources for an invalid egid!");

                auto& views = m_entityViews[{ std::type_index(typeid(TView)), group }];
                auto count = views.buffer.size() / sizeof(TView);
                return { reinterpret_cast<TView*>(views.buffer.data()), count };
            }

            template<typename TView>
            TView* Query(const EGID& egid)
            {
                static_assert(std::is_base_of<IEntityView, TView>::value, "Template argument type does not derive from IEntityView!");
                PK_THROW_ASSERT(egid.IsValid(), "Trying to acquire resources for an invalid egid!");

                auto& views = m_entityViews.at({ std::type_index(typeid(TView)), egid.groupID() });
                auto offset = views.indices.at(egid.entityID());
                return reinterpret_cast<TView*>(views.buffer.data() + offset);
            }

        private:
            std::map<ViewCollectionKey, EntityViewsCollection> m_entityViews;
            std::map<std::type_index, ImplementerContainer> m_implementerBuckets;
            uint32_t m_idCounter = 0;
    };
}