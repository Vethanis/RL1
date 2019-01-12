#pragma once

#include "array.h"

template<typename K, typename V, uint64_t width, AllocBucket t_bucket = AB_Default>
struct Dict
{
    typedef Array2<K, V, t_bucket> Lane;

    Lane     m_lanes[width];
    int32_t  m_count = 0u;

    inline Lane& GetLane(K key)
    {
        return m_lanes[key % width];
    }
    inline const Lane& GetLane(K key) const
    {
        return m_lanes[key % width];
    }
    inline void Insert(K key, const V& item)
    {
        Lane& lane = GetLane(key);
        int32_t idx = lane.findA(key);
        if(idx == -1)
        {
            lane.grow();
            lane.backA() = key;
            lane.backB() = item;
            ++m_count;
        }
    }
    inline V* Get(K key)
    {
        Lane& lane = GetLane(key);
        int32_t idx = lane.findA(key);
        return idx == -1 ? nullptr : &(lane.getB(idx));
    }
    inline const V* Get(K key) const
    {
        const Lane& lane = GetLane(key);
        int32_t idx = lane.findA(key);
        return idx == -1 ? nullptr : &(lane.getB(idx));
    }
    inline void Remove(K key)
    {
        Lane& lane = GetLane(key);
        int32_t idx = lane[slot].findA(key);
        if(idx != -1)
        {
            lane.remove(idx);
            --m_count;
        }
    }
    inline void Clear()
    {
        for(Lane& lane : m_lanes)
        {
            lane.clear();
        }
    }
    inline void Reset()
    {
        for(Lane& lane : m_lanes)
        {
            lane.reset();
        }
    }
    inline int32_t Count() const 
    {
        return m_count;
    }
};

template<typename K, typename V, AllocBucket t_bucket = AB_Default>
struct Dict2
{
    typedef Array2<K, V, t_bucket> Lane;

    Array<Lane, false, t_bucket> m_lanes;
    int32_t                      m_count = 0u;

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
    inline const Lane& GetLane(K key) const
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
            K* keys = lane.beginA();
            V* data = lane.beginB();
            for(int32_t i = 0; i < lane.count(); ++i)
            {
                Insert(newLanes, keys[i], data[i]);
            }
        }
        m_lanes.assume(newLanes);
    }
    static inline void Insert(Array<Lane, false, t_bucket>& lanes, K key, V& item)
    {
        uint64_t slot = key % (uint64_t)lanes.count();
        Lane& lane = lanes[(int32_t)slot];
        lane.grow();
        lane.backA() = key;
        Assume(lane.backB(), item);
    }
    inline void Insert(K key, const V& item)
    {
        if(NeedRehash())
        {
            Rehash(m_lanes.count() ? m_lanes.count() * 2 : 1);
        }

        Lane& lane = GetLane(key);
        int32_t idx = lane.findA(key);
        if(idx == -1)
        {
            lane.grow();
            lane.backA() = key;
            lane.backB() = item;
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
        int32_t idx = lane.findA(key);
        return idx == -1 ? nullptr : &(lane.getB(idx));
    }
    inline const V* Get(K key) const
    {
        const Lane& lane = GetLane(key);
        int32_t idx = lane.findA(key);
        return idx == -1 ? nullptr : &(lane.getB(idx));
    }
    inline void Remove(K key)
    {
        Lane& lane = GetLane(key);
        int32_t idx = lane.findA(key);
        if(idx != -1)
        {
            lane.remove(idx);
            --m_count;
        }
    }
    inline int32_t Count() const 
    {
        return m_count;
    }
};

template<typename K, typename V, uint64_t width>
using TempDict = Dict<K, V, width, AB_Temp>;

template<typename K, typename V>
using TempDict2 = Dict2<K, V, AB_Temp>;
