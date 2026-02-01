#pragma once
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace kairo::ds
{

/// @brief A bidirectional map (bimap) between keys of type K and values of type
/// V Allows lookup in both directions
template <typename K, typename V>
class Bimap
{
    std::unordered_map<K, V> forward;
    std::unordered_map<V, K> backward;

  public:
    void insert(const K &k, const V &v)
    {
        if (forward.count(k) || backward.count(v))
            throw std::logic_error("Duplicate key or value");

        forward[k] = v;
        backward[v] = k;
    }

    std::optional<const V &> value(const K &k) const
    {
        auto it = forward.find(k);
        if (it == forward.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    std::optional<const K &> key(const V &v) const
    {
        auto it = backward.find(v);
        if (it == backward.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    bool contains_key(const K &k) const { return forward.count(k) > 0; }

    bool contains_value(const V &v) const { return backward.count(v) > 0; }

    std::vector<std::pair<K, V>> items() const
    {
        std::vector<std::pair<K, V>> result;
        result.reserve(forward.size());
        for (const auto &kv : forward)
        {
            result.push_back(kv);
        }
        return result;
    }

    void erase_by_key(const K &k)
    {
        auto it = forward.find(k);
        if (it != forward.end())
        {
            V v = it->second;
            forward.erase(it);
            backward.erase(v);
        }
    }

    void erase_by_value(const V &v)
    {
        auto it = backward.find(v);
        if (it != backward.end())
        {
            K k = it->second;
            backward.erase(it);
            forward.erase(k);
        }
    }

    size_t size() const { return forward.size(); }

    void clear()
    {
        forward.clear();
        backward.clear();
    }

    bool empty() const { return forward.empty(); }

    std::vector<K> get_keys() const
    {
        std::vector<K> result;
        result.reserve(forward.size());
        for (const auto &kv : forward)
        {
            result.push_back(kv.first);
        }
        return result;
    }

    std::vector<V> get_values() const
    {
        std::vector<V> result;
        result.reserve(backward.size());
        for (const auto &kv : backward)
        {
            result.push_back(kv.first);
        }
        return result;
    }
};
} // namespace kairo::ds
