#ifndef PNK_TYPE_SET_HPP
#define PNK_TYPE_SET_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace
{
    template <std::size_t i>
    using constant = std::integral_constant<decltype(i), i>;
}

namespace pnk
{
    namespace detail
    {
        template <
            template <typename, typename> typename,
            typename...>
        struct has_duplicates : std::false_type {};

        template <
            template <typename, typename> typename Compare,
            typename                               Head,
            typename...                            Tail>
        struct has_duplicates<Compare, Head, Tail...> : std::disjunction<
            std::disjunction<Compare<Head, Tail>...>,
            has_duplicates  <Compare, Tail...      >>
        {};

        template <
            std::size_t,
            template <typename, typename> typename,
            typename,
            typename...>
        struct index_of : constant<static_cast<std::size_t>(-1)> {};

        template <
            std::size_t                            current,
            template <typename, typename> typename Compare,
            typename                               ToFind,
            typename                               Head,
            typename...                            Tail>
        struct index_of<current, Compare, ToFind, Head, Tail...>
            : std::conditional_t<
                Compare<ToFind, Head>::value,
                constant<current>,
                index_of<current + 1, Compare, ToFind, Tail...>>
        {};
    }

    template <
        template <typename, typename> typename Compare,
        typename...                            Args>
    struct type_set
    {
    private:
        std::tuple<Args...> m_data;

        [[nodiscard]]
        static consteval auto has_duplicates() noexcept -> bool
        {
            return detail::has_duplicates<Compare, Args...>::value;
        }

        template <typename ToFind>
        [[nodiscard]]
        static consteval auto index_of() noexcept -> std::size_t
        {
            return detail::index_of<0, Compare, ToFind, Args...>::value;
        }

    public:
        constexpr type_set() noexcept = default;
        constexpr type_set(std::tuple<Args...> const& data) noexcept
            : m_data{ data }
        {
            static_assert(
                not has_duplicates(),
                "Cannot create a set with duplicate keys.");
        }
        constexpr type_set(std::tuple<Args...>&& data) noexcept
            : m_data{ data }
        {
            static_assert(
                not has_duplicates(),
                "Cannot create a set with duplicate keys.");
        }

        template <typename Predicate>
        constexpr auto find_if(
            Predicate&& predicate)
        const noexcept -> std::size_t
        {
            auto const loop = [&]<std::size_t i>(auto&& self, constant<i>)
            {
                if constexpr (i == sizeof...(Args))
                {
                    return npos;
                }
                else
                {
                    using T = std::tuple_element_t<i, decltype(m_data)>;
                    if constexpr (std::regular_invocable<Predicate, T const&>)
                    {
                        if (std::invoke(predicate, std::get<i>(m_data)))
                        {
                            return i;
                        }
                    }

                    return self(self, constant<i + 1>{});
                }
            };

            return loop(loop, constant<0>{});
        }

        template <typename Function>
        constexpr auto apply(
            std::size_t index,
            Function&&  function)
        noexcept -> void
        {
            auto const loop = [&, index]<std::size_t i>(auto&& self, constant<i>)
            {
                if constexpr (i == sizeof...(Args))
                {
                    return;
                }
                else
                {
                    using T = std::tuple_element_t<i, decltype(m_data)>;
                    if constexpr (std::invocable<Function, T&>)
                    {
                        if (i == index)
                        {
                            std::invoke(function, std::get<i>(m_data));
                        }
                    }

                    self(self, constant<i + 1>{});
                }
            };

            loop(loop, constant<0>{});
        }

        template <typename Function>
        constexpr auto apply(
            std::size_t index,
            Function&&  function)
        const noexcept -> void
        {
            auto const loop = [&, index]<std::size_t i>(auto&& self, constant<i>)
            {
                if constexpr (i == sizeof...(Args))
                {
                    return;
                }
                else
                {
                    using T = std::tuple_element_t<i, decltype(m_data)>;
                    if constexpr (std::regular_invocable<Function, T const&>)
                    {
                        if (i == index)
                        {
                            std::invoke(function, std::get<i>(m_data));
                        }
                    }

                    self(self, constant<i + 1>{});
                }
            };

            loop(loop, constant<0>{});
        }


        template <typename... Others>
        constexpr auto merge(
            type_set<Compare, Others...> const& other)
        const noexcept -> type_set<Compare, Args..., Others...>
        {
            return std::tuple_cat(m_data, other.m_data);
        }

        template <typename T>
        constexpr auto insert(
            T&& t)
        const& noexcept -> type_set<Compare, Args..., std::remove_cvref_t<T>>
        {
            using Key = std::remove_cvref_t<T>;
            return std::tuple_cat(
                m_data,
                std::tuple<Key>(std::forward<T>(t)));
        }

        template <typename T>
        constexpr auto insert(
            T&& t)
        && noexcept -> type_set<Compare, Args..., std::remove_cvref_t<T>>
        {
            using Key = std::remove_cvref_t<T>;
            return std::tuple_cat(
                std::move(m_data),
                std::tuple<Key>(std::forward<T>(t)));
        }

        template <typename Key>
        constexpr auto get() const noexcept -> decltype(auto)
        {
            constexpr auto i = index_of<Key>();
            static_assert(i != npos, "Cannot find key.");
            if constexpr (i != npos)
                return std::get<i>(m_data);
        }

        // All type_sets are friends with eachother.
        template <template <typename, typename> typename, typename...>
        friend struct type_set;

        static constexpr std::size_t npos = static_cast<std::size_t>(-1);
    };
} // namespace pnk

#endif // PNK_TYPE_SET_HPP
