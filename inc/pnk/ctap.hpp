#ifndef PNK_CTAP_HPP
#define PNK_CTAP_HPP

#include <iterator>
#include <concepts>
#include <charconv>
#include <type_traits>
#include <vector>
#include <string_view>

#include <pnk/static_string.hpp>
#include <pnk/type_set.hpp>

namespace pnk
{
    template <
        pnk::static_string brief_,
        pnk::static_string wordy_,
        typename           T>
    struct argument
    {
        static constexpr auto brief = brief_;
        static constexpr auto wordy = wordy_;

        using type = T;
        mutable type value;
        mutable bool parsed = false;
    };

    // HACK: For some reason, using argument<"", "", T> directly gives an error.
    template <typename T>
    struct empty_argument : pnk::argument<"", "", T> {};

    template <
        typename           T,
        pnk::static_string brief,
        pnk::static_string wordy>
    constexpr auto parse_from_string(
        pnk::argument<brief, wordy, T>& argument,
        std::string_view                string)
    noexcept -> void = delete;

    template <
        pnk::static_string brief,
        pnk::static_string wordy>
    constexpr auto parse_from_string(
        pnk::argument<brief, wordy, bool>& argument,
        std::string_view)
    noexcept -> void
    {
        argument.value  = true;
        argument.parsed = true;
    }

    template <
        typename           T,
        pnk::static_string brief,
        pnk::static_string wordy>
    constexpr auto parse_from_string(
        pnk::argument<brief, wordy, T>& argument,
        std::string_view                string)
    noexcept -> void
    requires (std::integral<T> or std::floating_point<T>)
    {
        auto [pointer, _] = std::from_chars(
            string.begin(),
            string.end(),
            argument.value);
        argument.parsed = pointer == string.end();
    }

    template <
        pnk::static_string brief,
        pnk::static_string wordy>
    constexpr auto parse_from_string(
        pnk::argument<brief, wordy, std::string_view>& argument,
        std::string_view                               string)
    noexcept -> void
    {
        argument.value  = string;
        argument.parsed = true;
    }

    template <
        typename           T,
        pnk::static_string brief,
        pnk::static_string wordy>
    constexpr auto parse_from_string(
        pnk::argument<brief, wordy, std::vector<T>>& argument,
        std::string_view                             string)
    noexcept -> void
    {
        // HACK: [1]
        auto subargument = pnk::empty_argument<T>{};
        parse_from_string(subargument, string);

        argument.value.push_back(subargument.value);
    }

    template <typename TypeSet>
    struct ctap
    {
    private:
        TypeSet arguments;

        template <bool wordy>
        constexpr auto parse_optional(
            std::string_view curr,
            char**           next)
        noexcept -> int
        {
            constexpr auto hyphenation_offset = wordy? 2 : 1;
            auto const equal = curr.find('=');
            auto const label = equal != std::string_view::npos?
                curr.substr(hyphenation_offset, equal - hyphenation_offset) :
                curr.substr(hyphenation_offset);

            // Flags are named and store bool.
            auto const is_flag = [label]<typename T>(T const&)
                requires (std::same_as<typename T::type, bool>)
            {
                if constexpr (wordy)
                    return T::wordy == label;
                else
                    return T::brief == label;
            };

            if (auto const index = arguments.find_if(is_flag);
                index != TypeSet::npos)
            {
                arguments.apply(index, [](auto& x)
                {
                    parse_from_string(x, {});
                });
                return 0;
            }

            // Options are named.
            auto const is_optional = [label]<typename T>(T const&)
                requires (not std::same_as<typename T::type, bool>)
            {
                if constexpr (wordy)
                    return T::wordy == label;
                else
                    return T::brief == label;
            };

            if (auto const index = arguments.find_if(is_optional);
                index != TypeSet::npos)
            {
                auto const value = equal != std::string_view::npos?
                    curr.substr(equal + 1) :
                    *next;

                arguments.apply(index, [value](auto& x)
                {
                    parse_from_string(x, value);
                });

                // If no equals sign was found, skip next token.
                return equal == std::string_view::npos;
            }

            // TODO: Handle this better.
            std::exit(69);
        }

        constexpr auto parse_position(std::string_view value) noexcept -> int
        {
            auto const is_position = []<typename T>(T const& arg)
            {
                return not arg.parsed;
            };

            if (auto const index = arguments.find_if(is_position);
                index != TypeSet::npos)
            {
                arguments.apply(index, [value](auto& x)
                {
                    parse_from_string(x, value);
                });
                return 0;
            }

            // TODO: Handle this  better.
            std::exit(69);
        }

    public:
        // FIXME: Not very happy with this either.
        constexpr auto
        parse(
            int    argc,
            char** argv)
        noexcept -> void
        {
            for (auto it = std::next(argv); it < std::next(argv, argc); ++it)
            {
                std::string_view current = *it;

                if ( current.starts_with("--") )
                    it += parse_optional<true >(current, std::next(it));
                else if ( current.starts_with('-') )
                    it += parse_optional<false>(current, std::next(it));
                else
                    parse_position(current);
            }
        }

        template <
            pnk::static_string brief,
            pnk::static_string wordy,
            typename           T>
        constexpr auto add_optional() const noexcept -> decltype(auto)
        {
            using Argument = argument<brief, wordy, T>;
            auto new_arguments = arguments.template insert(Argument{});
            return pnk::ctap<decltype(new_arguments)>{};
        }

        template <
            pnk::static_string wordy,
            typename           T>
        constexpr auto add_optional() const noexcept -> decltype(auto)
        {
            using Argument = argument<"", wordy, T>;
            auto new_arguments = arguments.template insert(Argument{});
            return pnk::ctap<decltype(new_arguments)>{};
        }

        template <
            pnk::static_string label,
            typename           T>
        constexpr auto add_position() const noexcept -> decltype(auto)
        {
            using Argument = pnk::argument<"", label, T>;
            auto new_arguments = arguments.template insert(Argument{});
            return pnk::ctap<decltype(new_arguments)>{};
        }

        template <pnk::static_string name>
        [[nodiscard]]
        constexpr auto get() const noexcept -> decltype(auto)
        {
            using Argument = pnk::argument<name, name, void*>;
            return arguments.template get<Argument>().value;
        }
    };

    struct ctap_builder
    {
    private:
        template <
            typename Lhs,
            typename Rhs>
        struct comparator : std::bool_constant<
            Lhs::wordy == Rhs::wordy or
            (not Rhs::brief.empty() and Lhs::wordy == Rhs::brief) or
            (not Lhs::brief.empty() and Lhs::brief == Rhs::wordy) or
            (not Lhs::brief.empty() and Lhs::brief == Rhs::brief)>
        {};

    public:
        // TODO: Add help messages and whatnot.
        template <
            pnk::static_string brief,
            pnk::static_string wordy,
            typename           T>
        constexpr auto add_optional() const noexcept
        {
            using Argument = pnk::argument<brief, wordy, T>;
            return pnk::ctap<pnk::type_set<comparator, Argument>>{};
        }

        template <
            pnk::static_string wordy,
            typename           T>
        constexpr auto add_optional() const noexcept
        {
            using Argument = pnk::argument<"", wordy, T>;
            return pnk::ctap<pnk::type_set<comparator, Argument>>{};
        }

        template <
            pnk::static_string label,
            typename           T>
        constexpr auto add_position() const noexcept
        {
            using Argument = pnk::argument<"", label, T>;
            return pnk::ctap<pnk::type_set<comparator, Argument>>{};
        }
    };
} // namespace pnk

#endif // PNK_CTAP_HPP
