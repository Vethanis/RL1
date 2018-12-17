#pragma once

#include "array.h"
#include "dict.h"

template<typename T>
constexpr bool IsPOD(const T& x)
{
  return true;
}

template<typename T, bool POD>
constexpr bool IsPOD(const Array<T, POD>& x)
{
  return false;
}

template<typename K, typename V, bool POD>
constexpr bool IsPOD(const Dict<K, V, POD>& x)
{
  return false;
}
