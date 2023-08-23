// Copyright (c) Hrvoje "Hurubon" Žohar
// See end of file for extended copyright information, references and credits.

#ifndef PNK_CTAP_HPP
#define PNK_CTAP_HPP

// A C++ program may be provided information from the hosted environment prior
// to startup[1]. This information is referred to as "command-line arguments"
// and "command-line options". Arguments and options provide the program with
// data to transform or change its operation[2].

// EXAMPLE:
//     1) mkdir foo/example -m=rw -p
//     2) mkdir foo/example -mode=rw --parents
// "example"       is an argument specifying the path to the directory
// -m or --mode    is an option specifying which operations to allow for the
//                 directory. In this example, the modes 'r' (read) and 'w'
//                 (write) are provided.
// -p or --parents is a boolean option specifying to create all parent
//                 directories (in this case, 'foo') if they don't exist.
// Without the last option, if 'foo' didn't exist, we would get an error.

// In 1), the options are provided with a "brief" name.
// In 2), they are provided with a "wordy" name.
// Either of these can be used. By convention, brief names are prefixed with a
// hypen, while wordy names are prefixed with two hyphens.

// Generally, programs expect arguments to be provided in a certain order, while
// options can be provided in any order. Also, not all arguments and options are
// always required. Options can be boolean flags, which are set if the option is
// provided, or values which come after the option.

// PROBLEM:
// Because of this, the provided information has to be *parsed*. Generally, the
// arguments and options that can be provided to a program are known at design-
// time, so a parser for them can be created at compile-time.
// This library provides an interface to create compile-time argument parsers.

// NOTE:
// In this library, "argument" refers to both arguments and options.
// arguments are specifically called "position(al argument)s" and
// options   are specifically called "optional( argument)s".

#include <cstdlib>
#include <concepts>
#include <charconv>
#include <type_traits>
#include <string_view>

#include <pnk/static_string.hpp>
#include <pnk/type_set.hpp>

namespace pnk
{
    // See static_string.hpp for more information.
    template <
        pnk::static_string brief_name,
        pnk::static_string wordy_name,
        typename           T,
        bool               needed>
    struct argument
    {
        using value_type = T;
        using type       = T;

        auto constexpr static brief = brief_name;
        auto constexpr static wordy = wordy_name;
        auto constexpr static is_needed = needed;

        mutable bool was_parsed = false;
        mutable type value;

        [[nodiscard]]
        constexpr operator value_type() const noexcept
        {
            return value;
        }
    };

    // Functions for parsing common types from text. You may add your own. If
    // you got a scary error saying something like "call to deleted function",
    // you probably forgot to add an overload for your type.
    template <
        pnk::static_string brief,
        pnk::static_string wordy,
        typename           T,
        bool               needed>
    constexpr auto parse_from_text(
        pnk::argument<brief, wordy, T, needed>&,
        std::string_view)
    noexcept -> void = delete;

    template <
        pnk::static_string brief,
        pnk::static_string wordy,
        bool               needed>
    constexpr auto parse_from_text(
        pnk::argument<brief, wordy, bool, needed>& argument,
        std::string_view)
    noexcept -> void
    {
        argument.was_parsed = true;
        argument.value      = true;
    }

    template <
        pnk::static_string brief,
        pnk::static_string wordy,
        typename           T,
        bool               needed>
    constexpr auto parse_from_text(
        pnk::argument<brief, wordy, T, needed>& argument,
        std::string_view                        text)
    noexcept -> void
    requires (std::integral<T> or std::floating_point<T>)
    {
        auto const [pointer, _] = std::from_chars(
            text.data(),
            text.data() + text.size(),
            argument.value);
        argument.was_parsed = (pointer == text.data() + text.size());
    }

    template <
        pnk::static_string brief,
        pnk::static_string wordy,
        bool               needed>
    constexpr auto parse_from_text(
        pnk::argument<brief, wordy, std::string_view, needed>& argument,
        std::string_view                                       text)
    noexcept -> void
    {
        argument.was_parsed = true;
        argument.value      = text;
    }

    // See type_set.hpp for more information.
    // The parser stores arguments in three different sets based on their kind.
    // This makes lookup easier and more efficient. When parsing is complete,
    // these sets are merged into one and returned as a ctap_result.
    template <typename TypeSet>
    struct ctap_result
    {
    public:
        [[nodiscard]]
        constexpr ctap_result(TypeSet&& arguments) noexcept
            : m_arguments(std::move(arguments))
        {}

        template <pnk::static_string name>
        [[nodiscard]]
        auto constexpr get() const noexcept
        {
            using Argument = pnk::argument<name, name, void*, false>;

            return m_arguments.template get<Argument>().value;
        }

    private:
        TypeSet const m_arguments;
    };

    template <
        typename PositionsTypeSet,
        typename OptionalsTypeSet,
        typename BooleansTypeSet>
    struct ctap
    {
    public:
        template <
            pnk::static_string name,
            typename           T,
            bool               needed = false>
        auto constexpr add_position() const noexcept
        {
            using Argument = pnk::argument<"", name, T, needed>;
            using Inserted = decltype(m_positions.template insert(Argument{}));
            
            return pnk::ctap<Inserted, OptionalsTypeSet, BooleansTypeSet>{};
        }

        template <
            pnk::static_string brief,
            pnk::static_string wordy,
            typename           T,
            bool               needed = false>
        auto constexpr add_optional() const noexcept requires (
            not std::is_same_v<T, bool>)
        {
            using Argument = pnk::argument<brief, wordy, T, needed>;
            using Inserted = decltype(m_optionals.template insert(Argument{}));
            
            return pnk::ctap<PositionsTypeSet, Inserted, BooleansTypeSet>{};
        }

        // You don't have to provide a brief name for an optional.
        template <
            pnk::static_string wordy,
            typename           T,
            bool               needed = false>
        auto constexpr add_optional() const noexcept requires (
            not std::is_same_v<T, bool>)
        {
            using Argument = pnk::argument<"", wordy, T, needed>;
            using Inserted = decltype(m_optionals.template insert(Argument{}));
            
            return pnk::ctap<PositionsTypeSet, Inserted, BooleansTypeSet>{};
        }

        // Specializations for boolean options.
        template <
            pnk::static_string brief,
            pnk::static_string wordy,
            typename           T,
            bool               needed = false>
        auto constexpr add_optional() const noexcept requires (
            std::is_same_v<T, bool>)
        {
            using Argument = pnk::argument<brief, wordy, bool, needed>;
            using Inserted = decltype(m_booleans.template insert(Argument{}));
            
            return pnk::ctap<PositionsTypeSet, OptionalsTypeSet, Inserted>{};
        }

        template <
            pnk::static_string wordy,
            typename           T,
            bool               needed = false>
        auto constexpr add_optional() const noexcept requires (
            std::is_same_v<T, bool>)
        {
            using Argument = pnk::argument<"", wordy, bool, needed>;
            using Inserted = decltype(m_booleans.template insert(Argument{}));
            
            return pnk::ctap<PositionsTypeSet, OptionalsTypeSet, Inserted>{};
        }

        [[nodiscard]]
        auto constexpr parse(
            int                const argc,
            char const* const* const argv)
        noexcept
        {
            for (auto i = argv + 1; i < argv + argc; ++i)
            {
                auto current = std::string_view(*i);

                if (current.starts_with("--"))
                    i += parse_optional<true >(current, i + 1);
                else if (current.starts_with('-'))
                    i += parse_optional<false>(current, i + 1);
                else
                    parse_position(current);
            }

            auto const disjoint_union = m_positions
                .disjoint_union(m_optionals)
                .disjoint_union(m_booleans);

            auto const index = disjoint_union.find_if([]<typename T>(T const& a)
            {
                return T::is_needed and not a.was_parsed;
            });

            if (index < 0)
            {
                return pnk::ctap_result(std::move(disjoint_union));
            }
            else
            {
                // TODO: Handle failure.
                std::exit(64);
            }
        }
    private:
        template <bool wordy>
        [[nodiscard]]
        auto constexpr parse_optional(
            std::string_view   current,
            char const* const* next_it)
        noexcept -> std::ptrdiff_t
        {
            auto const hyphen_offset = wordy + 1;
            auto const equals_index  = current.find('=');

            auto const name = equals_index != std::string_view::npos?
                current.substr(hyphen_offset, equals_index - hyphen_offset) :
                current.substr(hyphen_offset);

            auto const matches_name = [name]<typename T>(T const&) noexcept
            {
                if constexpr (wordy)
                    return T::wordy == name;
                else
                    return T::brief == name;
            };

            if (auto const index = m_booleans.find_if(matches_name);
                index != BooleansTypeSet::npos)
            {
                m_booleans.apply_at(index, [](auto& a)
                {
                    pnk::parse_from_text(a, {});
                });

                return 0;
            }

            if (auto const index = m_optionals.find_if(matches_name);
                index != OptionalsTypeSet::npos)
            {
                auto const value = equals_index != std::string_view::npos?
                    current.substr(equals_index + 1) :
                    std::string_view(*next_it);

                m_optionals.apply_at(index, [value](auto& a)
                {
                    pnk::parse_from_text(a, value);
                });

                return equals_index == std::string_view::npos;
            }
            
            // TODO: Handle failure.
            std::exit(64);
        }

        auto constexpr parse_position(std::string_view current) noexcept -> void
        {
            auto const wasnt_parsed = [](auto&& a) constexpr noexcept
            {
                return not a.was_parsed;
            };

            if (auto const index = m_positions.find_if(wasnt_parsed);
                index != PositionsTypeSet::npos)
            {
                m_positions.apply_at(index, [current](auto& a)
                {
                    pnk::parse_from_text(a, current);
                });
            }
            else
            {
                // TODO: Handle failure.
                std::exit(64);
            }
        }
        
        PositionsTypeSet m_positions;
        OptionalsTypeSet m_optionals;
        BooleansTypeSet  m_booleans;
    }; // struct ctap

    // You might've noticed that all add_* functions return a parser. This is
    // because adding an argument defines a new distinct parser type with three
    // type template parameters, namely the sets containing all the arguments.

    // The names of these types get very long and it's completely unfeasible to
    // spell them out. That's okay, because we can chain calls to add_* on
    // previous parser objects and capture the final result in an auto variable.

    // But initially, we don't have a previous parser object to call add_* on.
    // Having to spell out even this first parser object's type would be less
    // than ideal because it's complex and reveals more implementation details
    // than necessary. Instead, we can use this builder struct, which also has
    // all the same add_* functions, which create a parser with the first arg.
    struct ctap_builder
    {
    public:
        template <
            pnk::static_string name,
            typename           T,
            bool               needed = false>
        constexpr auto add_position() const noexcept
        {
            using Argument = pnk::argument<"", name, T, needed>;
            return pnk::ctap<
                pnk::type_set<default_argument_comparator, Argument>,
                pnk::type_set<default_argument_comparator>,
                pnk::type_set<default_argument_comparator>>{};
        }

        template <
            pnk::static_string brief,
            pnk::static_string wordy,
            typename           T,
            bool               needed = false>
        constexpr auto add_optional() const noexcept requires (
            not std::is_same_v<T, bool>)
        {
            using Argument = pnk::argument<brief, wordy, T, needed>;
            return pnk::ctap<
                pnk::type_set<default_argument_comparator>,
                pnk::type_set<default_argument_comparator, Argument>,
                pnk::type_set<default_argument_comparator>>{};
        }

        template <
            pnk::static_string wordy,
            typename           T,
            bool               needed = false>
        constexpr auto add_optional() const noexcept requires (
            not std::is_same_v<T, bool>)
        {
            using Argument = pnk::argument<"", wordy, T, needed>;
            return pnk::ctap<
                pnk::type_set<default_argument_comparator>,
                pnk::type_set<default_argument_comparator, Argument>,
                pnk::type_set<default_argument_comparator>>{};
        }

        template <
            pnk::static_string brief,
            pnk::static_string wordy,
            typename           T,
            bool               needed = false>
        constexpr auto add_optional() const noexcept requires (
            std::is_same_v<T, bool>)
        {
            using Argument = pnk::argument<brief, wordy, bool, needed>;
            return pnk::ctap<
                pnk::type_set<default_argument_comparator>,
                pnk::type_set<default_argument_comparator>,
                pnk::type_set<default_argument_comparator, Argument>>{};
        }

        template <
            pnk::static_string wordy,
            typename           T,
            bool               needed = false>
        constexpr auto add_optional() const noexcept requires (
            std::is_same_v<T, bool>)
        {
            using Argument = pnk::argument<"", wordy, bool, needed>;
            return pnk::ctap<
                pnk::type_set<default_argument_comparator>,
                pnk::type_set<default_argument_comparator>,
                pnk::type_set<default_argument_comparator, Argument>>{};
        }

    private:
        template <
            typename Lhs,
            typename Rhs>
        struct default_argument_comparator : std::bool_constant<
            Lhs::wordy == Rhs::wordy or
            (not Rhs::brief.empty() and Lhs::wordy == Rhs::brief) or
            (not Lhs::brief.empty() and Lhs::brief == Rhs::wordy) or
            (not Lhs::brief.empty() and Lhs::brief == Rhs::brief)>
        {};
    }; // struct ctap_builder;
} // namespace pnk

#endif // PNK_CTAP_HPP

// Thanks to everyone in Together C & C++ who helped me make this, especially:
// wreien    https://github.com/wreien
// eisenwave https://github.com/Eisenwave
// karnkaul  https://github.com/karnkaul

// You can see our discussion thread here:
// https://discordapp.com/channels/331718482485837825/1134950122208370708

// Inspired by/based on:
// wreien   - cstring-parse: https://github.com/wreien/cstring-parse
// karnkaul - clap:          https://github.com/karnkaul/clap
// jarro    - cxxopts:       https://github.com/jarro2783/cxxopts
//            argparse:      https://docs.python.org/3/library/argparse.html

// References:
// [1] https://open-std.org/JTC1/SC22/WG14/www/docs/n3096.pdf#paragraph.5.1.2.2.1.2
// [2] https://en.wikipedia.org/wiki/Command-line_interface#Arguments

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
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
