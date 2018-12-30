#pragma once

#include "array.h"

template<typename K, typename V, uint64_t width, bool POD = true, AllocBucket t_bucket = AB_Default>
struct Dict
{
    Array<K, true, t_bucket>    m_keys[width];
    Array<V, POD, t_bucket>     m_data[width];
    uint32_t                    m_count = 0u;

    inline void Insert(K key, const V& item)
    {
        uint64_t slot = key % width;
        int32_t idx = m_keys[slot].find(key);
        if(idx == -1)
        {
            m_data[slot].grow() = item;
            m_keys[slot].grow() = key;
            ++m_count;
        }
    }
    inline V* Get(K key)
    {
        uint64_t slot = key % width;
        int32_t idx = m_keys[slot].find(key);
        return idx == -1 ? nullptr : &(m_data[slot][idx]);
    }
    inline const V* Get(K key) const
    {
        uint64_t slot = key % width;
        int32_t idx = m_keys[slot].find(key);
        return idx == -1 ? nullptr : &(m_data[slot][idx]);
    }
    inline void Remove(K key)
    {
        uint64_t slot = key % width;
        int32_t idx = m_keys[slot].find(key);
        if(idx != -1)
        {
            m_data[slot].remove(idx);
            m_keys[slot].remove(idx);
            --m_count;
        }
    }
    inline uint32_t Count() const 
    {
        return m_count;
    }
};

template<typename K, typename V, uint64_t width, bool POD = true>
using TempDict = Dict<K, V, width, POD, AB_Temp>;

template<typename K, typename V, bool POD = true, AllocBucket t_bucket = AB_Default>
struct Dict2
{
    struct Lane
    {
        Array<K, true, t_bucket> m_keys;
        Array<V, POD,  t_bucket> m_data;
    };
    Array<Lane, false, t_bucket> m_lanes;
    uint32_t                     m_count = 0u;

    inline bool NeedRehash() const 
    {
        return (m_lanes.count() == 0) || (m_count / 8 > m_lanes.count());
    }
    inline uint64_t Width() const 
    {
        return (uint64_t)m_lanes.count();
    }
    inline Lane& GetLane(K key)
    {
        uint64_t slot = key % Width();
        return m_lanes[(int32_t)slot];
    }
    void Rehash(int32_t size)
    {
        Array<Lane, false, t_bucket> newLanes;
        newLanes.resize(size);
        for(Lane& lane : m_lanes)
        {
            for(int32_t i = 0; i < lane.m_keys.count(); ++i)
            {
                Insert(newLanes, lane.m_keys[i], lane.m_data[i]);
            }
        }
        m_lanes.assume(newLanes);
    }
    static inline void Insert(Array<Lane, false, t_bucket>& lanes, K key, V& item)
    {
        uint64_t slot = key % (uint64_t)lanes.count();
        Lane& lane = lanes[(int32_t)slot];
        memcpy(&lane.m_data.grow(), &item, sizeof(V));
        memset(&item, 0, sizeof(V));
    }
    inline void Insert(K key, const V& item)
    {
        if(NeedRehash())
        {
            Rehash(m_lanes.count() ? m_lanes.count() * 2 : 1);
        }

        Lane& lane = GetLane(key);
        int32_t idx = lane.m_keys.find(key);
        if(idx == -1)
        {
            lane.m_data.grow() = item;
            ++m_count;
        }
    }
    inline V& operator[](K key)
    {
        V* value = Get(key);
        if(value)
        {
            return *value;
        }
        Insert(key, {});
        return *Get(key);
    }
    inline V* Get(K key)
    {
        Lane& lane = GetLane(key);
        int32_t idx = lane.m_keys.find(key);
        return idx == -1 ? nullptr : &(lane.m_data[idx]);
    }
    inline const V* Get(K key) const
    {
        Lane& lane = GetLane(key);
        int32_t idx = lane.m_keys.find(key);
        return idx == -1 ? nullptr : &(lane.m_data[idx]);
    }
    inline void Remove(K key)
    {
        Lane& lane = GetLane(key);
        int32_t idx = lane.m_keys.find(key);
        if(idx != -1)
        {
            lane.m_data.remove(idx);
            lane.m_keys.remove(idx);
            --m_count;
        }
    }
    inline uint32_t Count() const 
    {
        return m_count;
    }
};

template<typename K, typename V, bool POD = true>
using TempDict2 = Dict2<K, V, POD, AB_Temp>;
