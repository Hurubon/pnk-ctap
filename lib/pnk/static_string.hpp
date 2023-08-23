// Copyright (c) Hrvoje "Hurubon" Žohar
// See end of file for extended copyright information.

#ifndef PNK_STATIC_STRING_HPP
#define PNK_STATIC_STRING_HPP

// PROBLEM:
// As of August 2023, there is no built-in way to pass a string as a non-type
// template parameter. This utility library provides a type for passing strings
// as NTTP's.

#include <cstddef>
#include <iterator>
#include <algorithm>
#include <string_view>

namespace pnk
{
    // NOTE:
    // This interface currently only provides the minimal operations needed for
    // pnk::ctap. It will be expanded as it becomes used in other projects. 
    template <std::size_t n>
    struct static_string
    {
        // TODO: Generalize to any character type.
        using value_type             = char;
        using size_type              = std::size_t;
        using ssize_type             = std::ptrdiff_t;
        using difference_type        = std::ptrdiff_t;
        using traits_type            = std::char_traits<value_type>;
        using pointer                = value_type*;
        using const_pointer          = value_type const*;
        using reference              = value_type&;
        using const_reference        = value_type const&;
        using iterator               = value_type*;
        using const_iterator         = value_type const*;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using view_type              = std::basic_string_view<value_type>;

        constexpr static_string(value_type const (&s)[n])
        {
            std::copy(s, s + n, m_data);
        }

        // Iterator interface.
        [[nodiscard]] constexpr auto begin  ()       noexcept -> iterator               { return m_data;                   }
        [[nodiscard]] constexpr auto begin  () const noexcept -> const_iterator         { return m_data;                   }
        [[nodiscard]] constexpr auto cbegin () const noexcept -> const_iterator         { return m_data;                   }
        [[nodiscard]] constexpr auto end    ()       noexcept -> iterator               { return std::next(m_data, n);     }
        [[nodiscard]] constexpr auto end    () const noexcept -> const_iterator         { return std::next(m_data, n);     }
        [[nodiscard]] constexpr auto cend   () const noexcept -> const_iterator         { return std::next(m_data, n);     }
        [[nodiscard]] constexpr auto rbegin ()       noexcept -> reverse_iterator       { return std::next(m_data, n - 1); }
        [[nodiscard]] constexpr auto rbegin () const noexcept -> const_reverse_iterator { return std::next(m_data, n - 1); }
        [[nodiscard]] constexpr auto crbegin() const noexcept -> const_reverse_iterator { return std::next(m_data, n - 1); }
        [[nodiscard]] constexpr auto rend   ()       noexcept -> reverse_iterator       { return std::prev(m_data);        }
        [[nodiscard]] constexpr auto rend   () const noexcept -> const_reverse_iterator { return std::prev(m_data);        }
        [[nodiscard]] constexpr auto crend  () const noexcept -> const_reverse_iterator { return std::prev(m_data);        }

        [[nodiscard]]
        consteval auto empty() const noexcept -> bool
        {
            return n == 1;
        }

        [[nodiscard]]
        constexpr operator view_type() const noexcept
        {
            return view_type(m_data, n - 1);
        }

        friend constexpr auto operator<=>(
            static_string const&,
            static_string const&)
        noexcept = default;

        value_type m_data[n];
    };

    template <
        std::size_t n,
        std::size_t m>
    [[nodiscard]]
    constexpr auto operator==(
        static_string<n> const&,
        static_string<m> const&)
    noexcept -> bool
    {
        return false;
    }
} // namespace pnk

#endif // PNK_STATIC_STRING_HPP

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
