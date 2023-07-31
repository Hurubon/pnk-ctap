#ifndef PNK_STATIC_STRING_HPP
#define PNK_STATIC_STRING_HPP

#include <cstddef>
#include <iterator>
#include <string_view>

namespace pnk
{
    template <std::size_t n>
    struct static_string
    {
        using value_type             = char;
        using size_type              = std::size_t;
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

        value_type m_data[n];

        constexpr static_string(value_type const (&s)[n])
        {
            std::ranges::copy(s, s + n, m_data);
        }

        // Iterators
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