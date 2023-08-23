// Copyright (c) Hrvoje "Hurubon" Žohar
// See end of file for extended copyright information.

#ifndef PNK_TYPE_SET_HPP
#define PNK_TYPE_SET_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace
{
    template <std::size_t i>
    using constant = std::integral_constant<decltype(i), i>;

    template <
        template <
            typename,
            typename>
        typename    Comparator,
        typename... Arguments>
    struct contains_duplicates : std::false_type
    {};

    template <
        template <
            typename,
            typename>
        typename    Comparator,
        typename    Head,
        typename... Tail>
    struct contains_duplicates<Comparator, Head, Tail...> : std::disjunction<
        std::disjunction<Comparator<Head, Tail>...>,
        contains_duplicates<Comparator, Tail...>>
    {};
}

namespace pnk
{
    namespace detail
    {
        template <
            std::size_t current,
            template <
                typename,
                typename>
            typename    Comparator,
            typename    ToFind,
            typename... Arguments>
        struct index_of : constant<static_cast<std::size_t>(-1)>
        {};

        template <
            std::size_t current,
            template <
                typename,
                typename>
            typename    Comparator,
            typename    ToFind,
            typename    Head,
            typename... Tail>
        struct index_of<current, Comparator, ToFind, Head, Tail...>
            : std::conditional_t<
                Comparator<ToFind, Head>::value,
                constant<current>,
                index_of<current + 1, Comparator, ToFind, Tail...>>
        {};
    } // namespace detail

    template <
        template <
            typename,
            typename>
        typename    Comparator,
        typename... Arguments>
    struct type_set
    {
    public:
        // All type_sets are friends with eachother.
        template <
            template <
                typename,
                typename>
            typename,
            typename...>
        friend struct pnk::type_set;

        [[nodiscard]]
        constexpr type_set() noexcept = default;
        [[nodiscard]]
        constexpr type_set(std::tuple<Arguments...> const& data) noexcept
            : m_data{ data }
        {
            static_assert(
                not contains_duplicates<Comparator, Arguments...>(),
                "Cannot create a set with duplicate keys.");
        }
        [[nodiscard]]
        constexpr type_set(std::tuple<Arguments...>&& data) noexcept
            : m_data{ data }
        {
            static_assert(
                not contains_duplicates<Comparator, Arguments...>(),
                "Cannot create a set with duplicate keys.");
        }

        template <typename Predicate>
        [[nodiscard]]
        auto constexpr find_if(
            Predicate&& predicate)
        const noexcept -> std::size_t
        {
            auto const loop = [&]<std::size_t i>(auto&& self, constant<i>)
            {
                if constexpr (i == sizeof...(Arguments))
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
        auto constexpr apply_at(
            std::size_t index,
            Function&&  function)
        noexcept -> void
        {
            auto const loop = [&]<std::size_t i>(auto&& self, constant<i>)
            {
                if constexpr (i == sizeof...(Arguments))
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
                            return;
                        }
                    }

                    self(self, constant<i + 1>{});
                }
            };

            loop(loop, constant<0>{});
        }

        template <typename Function>
        auto constexpr apply_at(
            std::size_t index,
            Function&&  function)
        const noexcept -> void
        {
            auto const loop = [&]<std::size_t i>(auto&& self, constant<i>)
            {
                if constexpr (i == sizeof...(Arguments))
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
                            return;
                        }
                    }

                    self(self, constant<i + 1>{});
                }
            };

            loop(loop, constant<0>{});
        }

        template <typename... Others>
        [[nodiscard]]
        auto constexpr disjoint_union(
            pnk::type_set<Comparator, Others...> const& other)
        const noexcept -> pnk::type_set<Comparator, Arguments..., Others...>
        {
            return std::tuple_cat(m_data, other.m_data);
        }

        template <typename Argument>
        [[nodiscard]]
        auto constexpr insert(
            Argument&& a)
        const & noexcept -> pnk::type_set<
            Comparator,
            Arguments...,
            std::remove_cvref_t<Argument>>
        {
            using Key = std::remove_cvref_t<Argument>;
            return std::tuple_cat(
                m_data, std::tuple<Key>(std::forward<Argument>(a)));
        }

        template <typename Argument>
        [[nodiscard]]
        auto constexpr insert(
            Argument&& a)
        const && noexcept -> pnk::type_set<
            Comparator,
            Arguments...,
            std::remove_cvref_t<Argument>>
        {
            using Key = std::remove_cvref_t<Argument>;
            return std::tuple_cat(
                std::move(m_data), std::tuple<Key>(std::forward<Argument>(a)));
        }

        template <typename Key>
        [[nodiscard]]
        auto constexpr get() const noexcept
        {
            auto constexpr i = index_of<Key>();
            static_assert(i != npos, "Cannot find key.");
            return std::get<i>(m_data);
        }

    auto constexpr static npos = static_cast<std::size_t>(-1);

    private:
        template <typename ToFind>
        [[nodiscard]]
        auto consteval static index_of() noexcept -> std::size_t
        {
            return detail::index_of<0, Comparator, ToFind, Arguments...>::value;
        }

        std::tuple<Arguments...> m_data;
    }; // struct type_set
} // namespace pnk

#endif // PNK_TYPE_SET_HPP

// MIT License
// Copyright (c) Hrvoje "Hurubon" Žohar
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
