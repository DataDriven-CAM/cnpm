#pragma once

#include <memory>
#include <iterator>
#include <type_traits>
#include <concepts>
#include <ranges>
#include <functional>

namespace sylvanmats::npm{

    template <class T>
    concept Container = requires(T a, const T b){
        requires std::regular<T>;
        requires std::swappable<T>;
        requires std::destructible<typename T::value_type>;
        requires std::same_as<typename T::reference, typename T::value_type &>;
        requires std::same_as<typename T::const_reference, const typename T::value_type &>;
        requires std::forward_iterator<typename T::iterator>;
        requires std::forward_iterator<typename T::const_iterator>;
        requires std::signed_integral<typename T::difference_type>;
        requires std::same_as<typename T::difference_type, typename std::iterator_traits<typename T::iterator>::difference_type>;
        requires std::same_as<typename T::difference_type, typename std::iterator_traits<typename T::const_iterator>::difference_type>;
        { a.begin() } -> std::convertible_to<typename T::iterator>;
        { a.end() } -> std::convertible_to<typename T::iterator>;
        { b.begin() } -> std::convertible_to<typename T::const_iterator>;
        { b.end() } -> std::convertible_to<typename T::const_iterator>;
        { a.cbegin() } -> std::convertible_to<typename T::const_iterator>;
        { a.cend() } -> std::convertible_to<typename T::const_iterator>;
        { a.size() } -> std::convertible_to<typename T::size_type>;
        { a.max_size() } -> std::convertible_to<typename T::size_type>;
        { a.empty() } -> std::same_as<bool>;
    };
    
    template <class T>
    concept DefaultConstructible = requires(T a){
        requires std::is_default_constructible_v<T>;
        requires std::default_initializable<T>;
    };
    
    template <class T>
    concept CopyAssignable = requires(T a, const T b){
        requires std::is_copy_assignable_v<T>;
    };
    
    template <class T>
    concept AllocatorAwareContainer = requires(T a, const T b){
        requires std::destructible<typename T::value_type>;
    };
    
    template<typename K, typename V, typename P = std::equal_to<K>,typename A = std::allocator<std::tuple<const K, V>>>
    requires Container<K> && AllocatorAwareContainer<K>
    class unordered_map{
        
        public:
        virtual ~unordered_map() = default;
    };
    
    static_assert(DefaultConstructible<unordered_map<std::string, std::string>>);
    static_assert(CopyAssignable<unordered_map<std::string, std::string>>);
    //requires std::is_default_constructible<unordered_map>;
}
