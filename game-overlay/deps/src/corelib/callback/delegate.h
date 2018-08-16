#pragma once

namespace Storm
{
    template <class R, class ...A>
    Delegate<R(A ...)> delegate(R (*function)(A ...))
    {
        return Storm::bind(function);
    }

    template <class C, class T, class R, class ...A>
    Delegate<R(A ...)> delegate(R (C::*function)(A ...), T&& object)
    {
        return Storm::bind(function, std::forward<T>(object));
    }

    template <class C, class T, class R, class ...A>
    Delegate<R(A ...)> delegate(R (C::*function)(A ...) const, T&& object)
    {
        return Storm::bind(function, std::forward<T>(object));
    }

    template <class F, typename = std::enable_if_t<!std::is_member_function_pointer<std::decay_t<F>>::value, bool>>
    Delegate<function_signature_t<std::decay_t<F>>> delegate(F&& function)
    {
        return Storm::bind(std::forward<F>(function));
    }
}
