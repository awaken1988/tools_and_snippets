#include <iostream>
#include <vector>
#include <list>
#include <memory>

namespace NsExchangeType
{
    template<typename T>
    struct indexable_info {};

    template<typename T>
    struct indexable_info<std::vector<T>> {
        using list_type  = std::list<T>;
        using value_type = T;

        template<typename V> using tmplt_list_type = std::list<V>;
    };

    template<typename T>
    struct indexable_info<std::list<T>> {
        using list_type  = std::vector<T>;
        using value_type = T;
        template<typename V> using tmplt_list_type = std::vector<V>;
    };

    template<typename T>
    struct indexable {
        using tt  = indexable_info<T>::list_type;

        using ttt_inner = indexable_info<T>::value_type;
        using ttt       = typename indexable_info<T>:: template tmplt_list_type<ttt_inner>;
    };

    void example() {
        using result_type_0 = NsExchangeType::indexable<std::vector<double>>::tt;
        using result_type_1 = NsExchangeType::indexable<std::vector<double>>::tt;
        static_assert(std::is_same<result_type_0,std::list<double>>::value, "bla");
        static_assert(std::is_same<result_type_1,std::list<double>>::value, "bla");
    }
}

namespace NsExchangeTypeGeneric
{
    template<typename T>
    struct indexable_info {};

    template<template<typename> typename C, typename V>
    struct indexable_info<C<V>> {
        using list_type  = C<V>;
        using value_type = V;

        template<typename X> using tmplt_list_type = C<X>;
    };

    template<typename T>
    struct indexable {
        using value_type  = indexable_info<T>::value_type;
        using list_type   = std::list<value_type>;
        using vector_type = std::vector<value_type>;
        using shptr_type  = std::shared_ptr<value_type>;
    };

    void example() {
        using test_type = std::vector<double>;
        using result_type_0 = indexable<test_type>::list_type;
        using result_type_1 = indexable<test_type>::value_type;
        static_assert(std::is_same<result_type_0,std::list<double>>::value, "bla");
        static_assert(std::is_same<result_type_1,double>::value, "bladd");
    }
   
}




int main()
{
    NsExchangeType::example();
    NsExchangeTypeGeneric::example();

    return 0;
}