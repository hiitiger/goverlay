#pragma once

namespace Storm
{
    template <class T>
    struct identity
    {
        typedef T type;
    };

    template <class T>
    struct function_signature_impl
            : function_signature_impl<decltype(&T::operator())>
    {
    };

    template <class R, class ...A>
    struct function_signature_impl<R(A ...)>
            : identity<R(A ...)>
    {
        typedef R return_type;
    };

    template <class R, class ...A>
    struct function_signature_impl<R(*)(A ...)>
            : function_signature_impl<R(A ...)>
    {
    };

    template <class C, class R, class ...A>
    struct function_signature_impl<R(C::*)(A ...)>
            : function_signature_impl<R(A ...)>
    {
    };

    template <class C, class R, class ...A>
    struct function_signature_impl<R(C::*)(A ...) const>
            : function_signature_impl<R(A ...)>
    {
    };

    template <typename T>
    struct function_signature
            : function_signature_impl<T>
    {
    };

    template <class S>
    struct function_traits_args;

    template <class R, class ...A>
    struct function_traits_args<R(A ...)>
    {
        typedef R return_type;
        typedef std::tuple<A...> args_list;
    };

    template <int N, class T>
    struct args_traits_tuple_drop;

    template <>
    struct args_traits_tuple_drop<0, std::tuple<>>
    {
        typedef std::tuple<> type;
    };

    template <class A1, class ...A>
    struct args_traits_tuple_drop<0, std::tuple<A1, A...>>
    {
        typedef std::tuple<A1, A...> type;
    };

    template <int N, class A1, class ...A>
    struct args_traits_tuple_drop<N, std::tuple<A1, A...>> : public args_traits_tuple_drop<N - 1, std::tuple<A...>>
    {
    };

    template <class R, class A>
    struct function_traits_make_sig;

    template <class R, class ...A>
    struct function_traits_make_sig<R, std::tuple<A...>>
    {
        typedef R type(A ...);
    };

    template <class Functor>
    struct function_traits
    {
        using run_type = typename function_signature<typename std::decay<Functor>::type>::type;
        using return_type = typename function_signature<typename std::decay<Functor>::type>::return_type;
        static constexpr bool is_method = false;

    };

    template <class R, class ...A>
    struct function_traits<R(*)(A ...)>
    {
        using run_type = R(A ...);
        using return_type = R;
        static constexpr bool is_method = false;
    };

    template <class R, class C, class ...A>
    struct function_traits<R(C::*)(A ...)>
    {
        using run_type = R(C*, A ...);
        using return_type = R;
        static constexpr bool is_method = true;
    };

    template <class R, class C, class ...A>
    struct function_traits<R(C::*)(A ...) const>
    {
        using run_type = R(const C*, A ...);
        using return_type = R;
        static constexpr bool is_method = true;
    };

    template <class Fn, class ...A>
    struct funcion_traits_unbound_runtype
    {
        using fn_run_type = typename function_traits<typename std::decay<Fn>::type>::run_type;
        using args_type = typename function_traits_args<fn_run_type>::args_list;
        using unbound_args = typename args_traits_tuple_drop<sizeof...(A), args_type>::type;
        using unbound_runtype = typename function_traits_make_sig<typename function_traits_args<fn_run_type>::return_type, unbound_args>::type;
    };



    template<class T>
    using function_signature_t = typename function_signature<T>::type;

    template<class T>
    struct arg_traits
    {
        typedef const T& param_type;
    };

    template<class T>
    struct arg_traits<T&>
    {
        typedef T& param_type;
    };

    template<class T>
    struct arg_traits<const T&>
    {
        typedef const T& param_type;
    };

    template<class T>
    struct arg_traits<T&&>
    {
        typedef T&& param_type;
    };

    template<class T>
    using arg_traits_t = typename arg_traits<T> ::param_type;


}
