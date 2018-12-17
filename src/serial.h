#pragma once

#include <stdio.h>
#include "pod.h"

template<typename T>
void Write(const T& src, FILE* dst)
{
    fwrite(&src, sizeof(T), 1)
}

template<typename T>
void Read(FILE* src, T& dst)
{
    fread(&dst, sizeof(T), 1, src);
}

template<typename T, bool POD>
void Write(const Array<T, POD>& src, FILE* dst)
{
    int32_t count = src.count();
    Write(count, dst);
    if(!POD)
    {
        for(int32_t i = 0; i < count; ++i)
        {
            Write(src[i], dst);
        }
    }
    else
    {
        fwrite(src.begin(), sizeof(T), src.count(), dst);
    }
}

template<typename T, bool POD>
void Read(FILE* src, Array<T, POD>& dst)
{
    int32_t count = 0;
    Read(src, count);
    dst.resize(count);
    if(!POD)
    {
        for(int32_t i = 0; i < count; ++i)
        {
            Read(src, dst[i]);
        }
    }
    else
    {
        fread(dst.begin(), sizeof(T), dst.count(), src);
    }
}

template<typename K, typename V, bool POD>
void Write(const Dict2<K, V, POD>& src, FILE* dst)
{
    int32_t count = src.Count();
    Write(count, dst);
    for(const auto& lane : src.m_lanes)
    {
        for(int32_t i = 0; i < lane.m_keys.count(); ++i)
        {
            Write(lane.m_keys[i], dst);
            Write(lane.m_data[i], dst);
        }
    }
}

template<typename K, typename V, bool POD>
void Read(FILE* src, Dict2<K, V, POD>& dst)
{
    int32_t count = 0;
    Read(count, dst);
    dst.Rehash(count / 8);
    for(int32_t i = 0; i < count; ++i)
    {
        K key;
        V value;
        Read(src, key);
        Read(src, value);
        dst.Insert(key, value);
    }
}
