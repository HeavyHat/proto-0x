////////////////////////////////////////////////////////////////////////////////////////////////////
// eval.hpp
// Evaluate a proto expression according to the rules of C++
//
//  Copyright 2012 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_V5_ACTION_EVAL_HPP_INCLUDED
#define BOOST_PROTO_V5_ACTION_EVAL_HPP_INCLUDED

//#include <memory> for std::addressof
#include <utility>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>
#include <boost/proto/v5/proto_fwd.hpp>
#include <boost/proto/v5/children.hpp>
#include <boost/proto/v5/matches.hpp>
#include <boost/proto/v5/utility.hpp>
#include <boost/proto/v5/action/basic_action.hpp>
#include <boost/proto/v5/action/case.hpp>
#include <boost/proto/v5/grammar/switch.hpp>
#include <boost/proto/v5/grammar/case.hpp>
#include <boost/proto/v5/grammar/not.hpp>

namespace boost
{
    namespace proto
    {
        inline namespace v5
        {
            namespace detail
            {
                template<typename Tag>
                struct _op
                {
                    using type = _op;

                    template<typename ...T>
                    constexpr utility::any operator()(T &&...) const noexcept
                    {
                        static_assert(
                            utility::never<Tag, T...>::value
                          , "proto::eval doesn't know how to evaluate this expression!"
                        );
                        return utility::any();
                    }
                };

                template<>
                struct _op<terminal>
                {
                    using type = utility::identity;
                };

                ////////////////////////////////////////////////////////////////////////////////////
                // _eval_unknown
                struct _eval_unknown
                  : basic_action<_eval_unknown>
                {
                    template<typename Expr, typename ...Rest>
                    constexpr utility::any operator()(Expr && expr, Rest &&...) const noexcept
                    {
                        static_assert(
                            utility::never<Expr>::value
                          , "proto::_eval doesn't know how to evaluate this expression!"
                        );
                        return utility::any();
                    }
                };

                ////////////////////////////////////////////////////////////////////////////////////
                // _eval_case
                template<typename ActiveGrammar, typename Tag>
                struct _eval_case
                  : def<case_(not_(matches_(_)), _eval_unknown)>
                {};

                template<typename ActiveGrammar>
                struct _eval_case<ActiveGrammar, terminal>
                  : def<case_(terminal(_), _value)>
                {};

                template<typename Tag, typename Action>
                struct _op_unpack
                  : basic_action<_op_unpack<Tag, Action>>
                {
                    template<std::size_t ...I, typename Expr, typename ...Rest>
                    constexpr auto impl(utility::indices<I...>, Expr && expr, Rest &&... rest) const
                    BOOST_PROTO_AUTO_RETURN(
                        BOOST_PROTO_TRY_CALL(typename _op<Tag>::type())(
                            call_action_<Action>()(
                                proto::v5::child<I>(static_cast<Expr &&>(expr))
                              , static_cast<Rest &&>(rest)...
                            )...
                        )
                    )

                    template<typename Expr, typename ...Rest>
                    constexpr auto operator()(Expr && expr, Rest &&... rest) const
                    BOOST_PROTO_AUTO_RETURN(
                        this->impl(
                            utility::make_indices<result_of::arity_of<Expr>::value>()
                          , static_cast<Expr &&>(expr)
                          , static_cast<Rest &&>(rest)...
                        )
                    )
                };

                template<typename ActiveGrammar>
                struct _op_unpack<terminal, ActiveGrammar>
                  : _value
                {};

                // Must respect short-circuit evaluation
                template<typename ActiveGrammar>
                struct _op_unpack<logical_or, ActiveGrammar>
                {
                    template<typename Expr, typename ...Rest>
                    constexpr auto operator()(Expr && expr, Rest &&... rest) const
                    BOOST_PROTO_AUTO_RETURN(
                        call_action_<ActiveGrammar>()(
                            proto::v5::child<0>(static_cast<Expr &&>(expr))
                          , static_cast<Rest &&>(rest)...
                        )
                     || call_action_<ActiveGrammar>()(
                            proto::v5::child<1>(static_cast<Expr &&>(expr))
                          , static_cast<Rest &&>(rest)...
                        )
                    )
                };

                // Must respect short-circuit evaluation
                template<typename ActiveGrammar>
                struct _op_unpack<logical_and, ActiveGrammar>
                {
                    template<typename Expr, typename ...Rest>
                    constexpr auto operator()(Expr && expr, Rest &&... rest) const
                    BOOST_PROTO_AUTO_RETURN(
                        call_action_<ActiveGrammar>()(
                            proto::v5::child<0>(static_cast<Expr &&>(expr))
                          , static_cast<Rest &&>(rest)...
                        )
                     && call_action_<ActiveGrammar>()(
                            proto::v5::child<1>(static_cast<Expr &&>(expr))
                          , static_cast<Rest &&>(rest)...
                        )
                    )
                };

                // Must respect short-circuit evaluation
                template<typename ActiveGrammar>
                struct _op_unpack<if_else_, ActiveGrammar>
                {
                    template<typename Expr, typename ...Rest>
                    constexpr auto operator()(Expr && expr, Rest &&... rest) const
                    BOOST_PROTO_AUTO_RETURN(
                        call_action_<ActiveGrammar>()(
                            proto::v5::child<0>(static_cast<Expr &&>(expr))
                          , static_cast<Rest &&>(rest)...
                        )
                      ? call_action_<ActiveGrammar>()(
                            proto::v5::child<1>(static_cast<Expr &&>(expr))
                          , static_cast<Rest &&>(rest)...
                        )
                      : call_action_<ActiveGrammar>()(
                            proto::v5::child<2>(static_cast<Expr &&>(expr))
                          , static_cast<Rest &&>(rest)...
                        )
                    )
                };

            #define BOOST_PROTO_UNARY_EVAL(TAG)                                                     \
                template<>                                                                          \
                struct _op<TAG>                                                                     \
                {                                                                                   \
                    using type = TAG;                                                               \
                };                                                                                  \
                                                                                                    \
                template<typename ActiveGrammar>                                                    \
                struct _eval_case<ActiveGrammar, TAG>                                               \
                  : def<case_(TAG(ActiveGrammar), _op_unpack<TAG, ActiveGrammar>)>                  \
                {};                                                                                 \
                /**/

            #define BOOST_PROTO_BINARY_EVAL(TAG)                                                    \
                template<>                                                                          \
                struct _op<TAG>                                                                     \
                {                                                                                   \
                    using type = TAG;                                                               \
                };                                                                                  \
                                                                                                    \
                template<typename ActiveGrammar>                                                    \
                struct _eval_case<ActiveGrammar, TAG>                                               \
                  : def<case_(TAG(ActiveGrammar, ActiveGrammar), _op_unpack<TAG, ActiveGrammar>)>   \
                {};                                                                                 \
                /**/

                BOOST_PROTO_UNARY_EVAL(unary_plus)
                BOOST_PROTO_UNARY_EVAL(negate)
                BOOST_PROTO_UNARY_EVAL(dereference)
                BOOST_PROTO_UNARY_EVAL(complement)
                BOOST_PROTO_UNARY_EVAL(address_of)
                BOOST_PROTO_UNARY_EVAL(logical_not)
                BOOST_PROTO_UNARY_EVAL(pre_inc)
                BOOST_PROTO_UNARY_EVAL(pre_dec)
                BOOST_PROTO_UNARY_EVAL(post_inc)
                BOOST_PROTO_UNARY_EVAL(post_dec)

                BOOST_PROTO_BINARY_EVAL(shift_left)
                BOOST_PROTO_BINARY_EVAL(shift_right)
                BOOST_PROTO_BINARY_EVAL(multiplies)
                BOOST_PROTO_BINARY_EVAL(divides)
                BOOST_PROTO_BINARY_EVAL(modulus)
                BOOST_PROTO_BINARY_EVAL(plus)
                BOOST_PROTO_BINARY_EVAL(minus)
                BOOST_PROTO_BINARY_EVAL(less)
                BOOST_PROTO_BINARY_EVAL(greater)
                BOOST_PROTO_BINARY_EVAL(less_equal)
                BOOST_PROTO_BINARY_EVAL(greater_equal)
                BOOST_PROTO_BINARY_EVAL(equal_to)
                BOOST_PROTO_BINARY_EVAL(not_equal_to)
                BOOST_PROTO_BINARY_EVAL(logical_or)
                BOOST_PROTO_BINARY_EVAL(logical_and)
                BOOST_PROTO_BINARY_EVAL(bitwise_and)
                BOOST_PROTO_BINARY_EVAL(bitwise_or)
                BOOST_PROTO_BINARY_EVAL(bitwise_xor)
                BOOST_PROTO_BINARY_EVAL(comma)
                BOOST_PROTO_BINARY_EVAL(subscript)
                BOOST_PROTO_BINARY_EVAL(mem_ptr)
                //BOOST_PROTO_BINARY_EVAL(member)

                BOOST_PROTO_BINARY_EVAL(assign)
                BOOST_PROTO_BINARY_EVAL(shift_left_assign)
                BOOST_PROTO_BINARY_EVAL(shift_right_assign)
                BOOST_PROTO_BINARY_EVAL(multiplies_assign)
                BOOST_PROTO_BINARY_EVAL(divides_assign)
                BOOST_PROTO_BINARY_EVAL(modulus_assign)
                BOOST_PROTO_BINARY_EVAL(plus_assign)
                BOOST_PROTO_BINARY_EVAL(minus_assign)
                BOOST_PROTO_BINARY_EVAL(bitwise_and_assign)
                BOOST_PROTO_BINARY_EVAL(bitwise_or_assign)
                BOOST_PROTO_BINARY_EVAL(bitwise_xor_assign)

            #undef BOOST_PROTO_UNARY_EVAL
            #undef BOOST_PROTO_BINARY_EVAL

                template<>
                struct _op<if_else_>
                {
                    using type = if_else_;
                };

                template<>
                struct _op<function>
                {
                    using type = function;
                };

                template<typename ActiveGrammar>
                struct _eval_case<ActiveGrammar, if_else_>
                  : def<case_(if_else_(ActiveGrammar, ActiveGrammar, ActiveGrammar),
                                  _op_unpack<if_else_, ActiveGrammar>)>
                {};

                template<typename ActiveGrammar>
                struct _eval_case<ActiveGrammar, function>
                  : def<case_(function(ActiveGrammar...), _op_unpack<function, ActiveGrammar>)>
                {};

                ////////////////////////////////////////////////////////////////////////////////////
                // _eval_cases
                template<typename ActiveGrammar>
                struct _eval_cases
                {
                    template<typename Tag>
                    using case_ = _eval_case<ActiveGrammar, Tag>;
                };

                ////////////////////////////////////////////////////////////////////////////////////
                // _eval_
                template<typename ActiveGrammar = _eval>
                struct _eval_
                  : detail::as_action_<switch_(detail::_eval_cases<ActiveGrammar>)>
                {};

                // Loopy indirection that allows proto::_eval_<> to be
                // used without specifying an ActiveGrammar argument.
                struct _eval
                  : _eval_<>
                {};
            }

            ////////////////////////////////////////////////////////////////////////////////////////
            // _eval
            struct _eval
              : detail::_eval_<>
            {};

            ////////////////////////////////////////////////////////////////////////////////////////
            // eval_with
            struct eval_with
            {};

            namespace extension
            {
                ////////////////////////////////////////////////////////////////////////////////////
                // e.g. eval_with(X)
                template<typename ActiveGrammar>
                struct action_impl<eval_with(ActiveGrammar)>
                  : detail::_eval_<ActiveGrammar>
                {};
            }
        }
    }
}

#endif
