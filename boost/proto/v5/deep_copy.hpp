////////////////////////////////////////////////////////////////////////////////////////////////////
// deep_copy.hpp
// Replace all nodes stored by reference with nodes stored by value.
//
//  Copyright 2012 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_V5_DEEP_COPY_HPP_INCLUDED
#define BOOST_PROTO_V5_DEEP_COPY_HPP_INCLUDED

#include <boost/proto/v5/proto_fwd.hpp>
#include <boost/proto/v5/matches.hpp>
#include <boost/proto/v5/make_expr.hpp>
#include <boost/proto/v5/action/basic_action.hpp>
#include <boost/proto/v5/action/switch.hpp>
#include <boost/proto/v5/action/passthru.hpp>

namespace boost
{
    namespace proto
    {
        inline namespace v5
        {
            namespace detail
            {
                struct _deep_copy_cases
                {
                    template<typename Tag, bool IsTerminal = Tag::proto_is_terminal_type::value>
                    struct case_
                      : as_action_<proto::v5::passthru(proto::v5::_deep_copy...)>
                    {};

                    template<typename Tag>
                    struct case_<Tag, true>
                      : proto::v5::basic_action<case_<Tag, true>>
                    {
                        template<typename E, typename... Rest>
                        constexpr auto operator()(E && e, Rest &&...) const
                        BOOST_PROTO_AUTO_RETURN(
                            typename result_of::domain_of<E>::type::make_expr()(
                                proto::v5::tag_of(static_cast<E &&>(e))
                              , utility::by_val()(proto::v5::value(static_cast<E &&>(e)))
                            )
                        )
                    };
                };
            }

            ////////////////////////////////////////////////////////////////////////////////////////
            // _deep_copy
            // A BasicAction that replaces all nodes stored by reference with
            // nodes stored by value.
            struct _deep_copy
              : detail::as_action_<switch_(detail::_deep_copy_cases)>
            {};

            ////////////////////////////////////////////////////////////////////////////////////////
            // deep_copy
            // Replaces all nodes stored by reference with nodes stored by value.
            template<typename E>
            constexpr auto deep_copy(E && e)
            BOOST_PROTO_AUTO_RETURN(
                proto::v5::_deep_copy()(static_cast<E &&>(e))
            )

            namespace functional
            {
                ////////////////////////////////////////////////////////////////////////////////////
                // deep_copy
                // A UnaryPolymorphicFunction that replaces all nodes stored by reference
                // with nodes stored by value.
                struct deep_copy
                {
                    template<typename E>
                    constexpr auto operator()(E && e) const
                    BOOST_PROTO_AUTO_RETURN(
                        proto::v5::_deep_copy()(static_cast<E &&>(e))
                    )
                };
            }
        }
    }
}

#endif
