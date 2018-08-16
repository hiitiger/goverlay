#pragma once

namespace Storm
{
    namespace helper
    {
        namespace CHECK
        {
            struct No
            {
            };

            template <typename T, typename Arg>
            No operator==(const T&, const Arg&);

            template <typename T, typename Arg = T>
            struct EqualExists
            {
                enum { value = !std::is_same<decltype(*(T*)(nullptr) == *(Arg*)(nullptr)), No>::value };
            };
        }

        template <class Fn, bool>
        struct IsSame;

        template <class T>
        struct IsSame<T, false>
        {
            static constexpr bool check(const T&, const T&)
            {
                return false;
            }
        };

        template <class T>
        struct IsSame<T, true>
        {
            static constexpr bool check(const T& a, const T& b)
            {
                return a == b;
            }
        };
    }

    struct BinderBase
    {
        Q_LEAK_DETECTOR(BinderBase)
    public:
        BinderBase()
        {
        }

        virtual ~BinderBase()
        {
        }

        typedef void (*invoker_fun_type)();

        invoker_fun_type invoker_fun_ = nullptr;

        virtual bool isSameCallee(BinderBase* other) = 0;
    };

    template <bool is_mem_fun, class Fn, class ...LeftArgs>
    struct Binder;

    template <class Fn, class ...LeftArgs>
    struct Binder<false, Fn, LeftArgs...> : public BinderBase
    {
        Fn function_;
        std::tuple<LeftArgs...> left_args_;

        template <class F, class ...A>
        Binder(invoker_fun_type invoker, F&& function, A&&... args)
            : function_(std::forward<F>(function))
            , left_args_(std::forward<A>(args)...)
        {
            invoker_fun_ = invoker;
        }

        bool isSameCallee(BinderBase* other) override
        {
            if (Binder* otherOne = dynamic_cast<Binder*>(other))
            {
                return helper::IsSame<Fn, helper::CHECK::EqualExists<Fn>::value>::check(function_, otherOne->function_);
            }
            return false;
        }
    };

    template <class, class>
    struct MethodInvokerTraits;

    /* template<class C, class R>
     struct MethodInvokerTraits<C, R>
     {
         template<class MFn,  class ...A>
         R invoke(C&& object, MFn&& function, A&&... args)
         {
             return ((&object)->*function)(std::forward<A>(args)...);
         }
     };*/

    template <class C, class R>
    struct MethodInvokerTraits<C*, R>
    {
        template <class MFn, class ...A>
        static R invoke(C* object, MFn&& function, A&&... args)
        {
            return ((object) ->* function)(std::forward<A>(args)...);
        }
    };

    template <class C, class R>
    struct MethodInvokerTraits<std::shared_ptr<C>, R>
    {
        template <class MFn, class ...A>
        static R invoke(std::shared_ptr<C>& object, MFn&& function, A&&... args)
        {
            return ((object.get()) ->* function)(std::forward<A>(args)...);
        }
    };

    template <class C, class R>
    struct MethodInvokerTraits<std::weak_ptr<C>, R>
    {
        template <class MFn, class ...A>
        static void invoke(std::weak_ptr<C>& object, MFn&& function, A&&... args)
        {
            if (auto obj_share_ptr = object.lock())
            {
                return ((obj_share_ptr.get()) ->* function)(std::forward<A>(args)...);
            }
        }
    };

    template <class C, class R>
    struct MethodInvokerTraits<RefPtr<C>, R>
    {
        template <class MFn, class ...A>
        static R invoke(RefPtr<C>& object, MFn&& function, A&&... args)
        {
            return ((object.get()) ->* function)(std::forward<A>(args)...);
        }
    };

    template <class C, class R>
    struct MethodInvokerTraits<WeakRefPtr<C>, R>
    {
        template <class MFn, class ...A>
        static void invoke(WeakRefPtr<C>& object, MFn&& function, A&&... args)
        {
            if (auto obj_share_ptr = object.lock())
            {
                return ((obj_share_ptr.get()) ->* function)(std::forward<A>(args)...);
            }
        }
    };


    template<class T, size_t sz>
    struct BinderObjectCheck
    {
        static constexpr bool check(T& t, T & other)
        {
            using CP = std::remove_reference_t<decltype(std::get<0>(t))>;
            using C = std::remove_reference_t<decltype(std::addressof(*(CP)0))>;
            return helper::IsSame<C, helper::CHECK::EqualExists<C>::value>::check(std::addressof(* std::get<0>(t)), std::addressof(*std::get<0>(other)));
        }
    };

    template<class T>
    struct BinderObjectCheck<T, 0>
    {
        static constexpr bool check(T& , T & )
        {
            return false;
        }
    };

    template <class MFn, class ...LeftArgs>
    struct Binder<true, MFn, LeftArgs...> : public BinderBase
    {
        MFn function_;
        std::tuple<LeftArgs...> left_args_;

        template <class F, class ...A>
        Binder(invoker_fun_type invoker, F&& function, A&&... args)
            : function_(std::forward<F>(function))
            , left_args_(std::forward<A>(args)...)
        {
            invoker_fun_ = invoker;
        }

        bool isSameCallee(BinderBase* other) override
        {
            if (Binder* otherOne = dynamic_cast<Binder*>(other))
            {
                static constexpr size_t num_bound_args = std::tuple_size<decltype(left_args_)>::value;

                return helper::IsSame<MFn, helper::CHECK::EqualExists<MFn>::value>::check(
                    function_, otherOne->function_)
                    && BinderObjectCheck<std::tuple<LeftArgs...>, num_bound_args>::check(left_args_, otherOne->left_args_);
            }
            return false;
        }

    };

    template <bool is_mem_fun, class B, class Fn>
    struct Invoker;

    template <class B, class R, class ...A>
    struct Invoker<false, B, R(A ...)>
    {
        static R invoke(BinderBase* binderBase, A ... args)
        {
            B* binder = static_cast<B*>(binderBase);
            static constexpr size_t num_bound_args = std::tuple_size<decltype(binder->left_args_)>::value;
            return apply(binder->function_, binder->left_args_, std::make_index_sequence<num_bound_args>{},
                         std::forward<A>(args)...);
        }

    private:
        template <class Fn, class LeftArgs, std::size_t... Is>
        static R apply(Fn&& function, LeftArgs&& left_args, std::index_sequence<Is...>, A ... args)
        {
            (void)left_args;
            return function(std::get<Is>(std::forward<LeftArgs>(left_args))..., std::forward<A>(args)...);
        }
    };

    template <class B, class R, class ...A>
    struct Invoker<true, B, R(A ...)>
    {
        static R invoke(BinderBase* binderBase, A ... args)
        {
            B* binder = static_cast<B*>(binderBase);

            static constexpr size_t num_bound_args = std::tuple_size<decltype(binder->left_args_)>::value;

            return apply(binder->function_, binder->left_args_, std::make_index_sequence<num_bound_args>{},
                         std::forward<A>(args)...);
        }

    private:
        template <class MFn, class LeftArgs, std::size_t... Is>
        static R apply(MFn&& function, LeftArgs&& left_args, std::index_sequence<Is...>, A ... args)
        {
            (void)left_args;
            return apply_(std::forward<MFn>(function), std::get<Is>(std::forward<LeftArgs>(left_args))..., std::forward<A>(args)...);
        }

        template<class MFn, class C, class ...Args>
        static R apply_(MFn&& function, C&& object, Args&&... args)
        {
            return MethodInvokerTraits<std::remove_reference_t<C>, R>::invoke(object, std::forward<MFn>(function), std::forward<Args>(args)...);
        }
    };

    template <class Fn, class R, class ...A, typename = std::enable_if_t<std::is_same<function_signature_t<std::decay_t<Fn>>, R(A ...)>::value, bool>>
    BinderBase* bindFunctor0_(Fn&& functor)
    {
        using UnboundRunType = R(A ...);
        using Binder = Binder<false, std::decay_t<Fn>>;
        using Invoker = Invoker<false, Binder, UnboundRunType>;
        using invoker_fun_type = BinderBase::invoker_fun_type;
        return static_cast<BinderBase*>(new Binder(reinterpret_cast<invoker_fun_type>(&Invoker::invoke), std::forward<Fn>(functor)));
    }

    template <class R, class ...A>
    BinderBase* bindStatic0_(R (*function)(A ...))
    {
        using Fn = R(*)(A ...);
        using UnboundRunType = R(A ...);
        using Binder = Binder<false, std::decay_t<Fn>>;
        using Invoker = Invoker<false, Binder, UnboundRunType>;
        using invoker_fun_type = BinderBase::invoker_fun_type;
        return static_cast<BinderBase*>(new Binder(reinterpret_cast<invoker_fun_type>(&Invoker::invoke), function));
    }

    class CBChecker
    {
        Q_LEAK_DETECTOR(CBChecker);
    public:
        CBChecker()
        {
        }

        virtual ~CBChecker()
        {
        }
    };

    template <class Fn>
    class Callback;

    template <class R, class ...A>
    class Callback<R(A ...)> : public CBChecker
    {
        std::shared_ptr<BinderBase> binder_;
        typedef R (*invoker_type)(BinderBase*, A ...);

    public:
        Callback() = default;

        explicit Callback(std::shared_ptr<BinderBase>&& binder)
            : binder_(std::move(binder))
        {
        }

        Callback(const Callback& other) = default;

        Callback(Callback&& other) = default;

        Callback& operator=(const Callback& other) = default;

        Callback& operator=(Callback&& other) = default;

        template <class Fn, typename = std::enable_if_t<!std::is_same<std::remove_cv_t<std::remove_reference_t<Fn>>, Callback>::value, bool>>
        Callback(Fn&& functor)
        {
            binder_.reset(bindFunctor0_<Fn, R, A...>(std::forward<Fn>(functor)));
        }

        Callback(R (*function)(A ...))
        {
            binder_.reset(bindStatic0_<R, A...>(function));
        }

        bool operator==(const Callback& other) const
        {
            return binder_ == other.binder_;
        }

        bool operator !=(const Callback& other) const
        {
            return !operator==(other);
        }

        R operator()(A ... args) const
        {
            DAssert(!isEmpty());
            invoker_type invoker = reinterpret_cast<invoker_type>(binder_->invoker_fun_);
            return invoker(binder_.get(), std::forward<A>(args)...);
        }

        bool isEmpty() const
        {
            return binder_.get() == nullptr;
        }

        bool isSameCallee(const Callback& other) const
        {
            if (binder_)
            {
                return binder_->isSameCallee(other.binder_.get());
            }
            return false;
        }
    };

    template <class Fn, class ...A>
    Callback<typename funcion_traits_unbound_runtype<Fn, A...>::unbound_runtype> bind(Fn&& function, A&&... args)
    {
        static constexpr bool is_method = std::is_member_function_pointer<std::decay_t<Fn>>::value;
        using Binder = Binder<is_method, std::decay_t<Fn>, std::decay_t<A>...>;
        using UnboundRunType = typename funcion_traits_unbound_runtype<Fn, A...>::unbound_runtype;
        using Invoker = Invoker<is_method, Binder, UnboundRunType>;
        using Callback = Callback<UnboundRunType>;
        using invoker_fun_type = BinderBase::invoker_fun_type;
        return Callback(std::shared_ptr<BinderBase>(std::make_shared<Binder>(reinterpret_cast<invoker_fun_type>(&Invoker::invoke),
                                                            std::forward<Fn>(function), std::forward<A>(args)...)));
    }

    template <typename Fun>
    struct is_fun_ptr
            : std::integral_constant<bool, std::is_pointer<Fun>::value && std::is_function<std::remove_pointer_t<Fun>>::value>
    {
    };

    //just to make it clear
    //enable move only lambda!!
    template <class Fn, class ...A>
    Callback<typename funcion_traits_unbound_runtype<Fn>::unbound_runtype> lambda(Fn&& function)
    {
        static_assert(!is_fun_ptr<std::decay_t<Fn>>::value, "use lambda");
        static_assert(!std::is_member_function_pointer<std::decay_t<Fn>>::value, "use lambda");
        using Binder = Binder<false, std::decay_t<Fn>>;
        using UnboundRunType = typename funcion_traits_unbound_runtype<Fn>::unbound_runtype;
        using Invoker = Invoker<false, Binder, UnboundRunType>;
        using Callback = Callback<UnboundRunType>;
        using invoker_fun_type = BinderBase::invoker_fun_type;
        return Callback(std::shared_ptr<BinderBase>(std::make_shared<Binder>(reinterpret_cast<invoker_fun_type>(&Invoker::invoke),
                                                            std::forward<Fn>(function))));
    }
}
