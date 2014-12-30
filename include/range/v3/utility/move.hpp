/// \file
// Range v3 library
//
//  Copyright Eric Niebler 2013-2014
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3
//

#ifndef RANGES_V3_UTILITY_MOVE_HPP
#define RANGES_V3_UTILITY_MOVE_HPP

#include <utility>
#include <type_traits>
#include <range/v3/range_fwd.hpp>
#include <range/v3/utility/meta.hpp>
#include <range/v3/utility/associated_types.hpp>

namespace ranges
{
    inline namespace v3
    {
        namespace aux
        {
            /// \ingroup group-utility
            struct move_fn
            {
                template<typename T,
                    typename U = meta::eval<std::remove_reference<T>>>
                U && operator()(T && t) const noexcept
                {
                    return static_cast<U &&>(t);
                }
            };

            /// \ingroup group-utility
            /// \sa `move_fn`
            constexpr move_fn move{};

            /// \ingroup group-utility
            /// \sa `move_fn`
            template<typename T>
            meta::eval<std::remove_reference<T>> && operator|(T && t, move_fn move) noexcept
            {
                return move(t);
            }
        }

        /// \cond
        namespace adl_move_detail
        {
            // Default indirect_move overload.
            template<typename I,
                typename R = decltype(*std::declval<I>()),
                typename U = meta::eval<std::remove_reference<R>>>
            meta::if_<std::is_reference<R>, U &&, detail::decay_t<U>>
            indirect_move(I const &, meta::id_t<R> && ref)
                noexcept(std::is_reference<R>::value ||
                    std::is_nothrow_constructible<detail::decay_t<U>, U &&>::value)
            {
                return aux::move(ref);
            }

            template<typename I>
            auto indirect_move(I const &i)
                noexcept(noexcept(indirect_move(i, *i))) ->
                decltype(indirect_move(i, *i))
            {
                return indirect_move(i, *i);
            }

            struct indirect_move_fn
            {
                template<typename I>
                auto operator()(I const &i) const
                    noexcept(noexcept(indirect_move(i))) ->
                    decltype(indirect_move(i))
                {
                    return indirect_move(i);
                }
                template<typename I>
                auto operator()(I const &i, decltype(*i) && ref) const
                    noexcept(noexcept(indirect_move(i, (decltype(*i) &&) ref))) ->
                    decltype(indirect_move(i, (decltype(*i) &&) ref))
                {
                    return indirect_move(i, (decltype(*i) &&) ref);
                }
            };
        }
        /// \endcond

        constexpr adl_move_detail::indirect_move_fn indirect_move {};

        template<typename I, typename O>
        struct is_indirectly_movable
          : meta::and_<
                std::is_constructible<
                    meta::eval<value_type<I>>,
                    decltype(indirect_move(std::declval<I>()))>,
                std::is_assignable<
                    decltype(*std::declval<O>()),
                    decltype(indirect_move(std::declval<I>()))>>
        {};

        template<typename I, typename O>
        struct is_nothrow_indirectly_movable
          : meta::and_<
                std::is_nothrow_constructible<
                    meta::eval<value_type<I>>,
                    decltype(indirect_move(std::declval<I>()))>,
                std::is_nothrow_assignable<
                    decltype(*std::declval<O>()),
                    decltype(indirect_move(std::declval<I>()))>>
        {};
    }
}

#endif
