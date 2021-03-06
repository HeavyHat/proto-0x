////////////////////////////////////////////////////////////////////////////////////////////////////
// fold.hpp
// Contains definition of the _fold<> and _reverse_fold<> actions.
//
//  Copyright 2012 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_V5_ACTION_FOLD_HPP_INCLUDED
#define BOOST_PROTO_V5_ACTION_FOLD_HPP_INCLUDED

#include <boost/fusion/include/equal_to.hpp>
#include <boost/fusion/include/begin.hpp>
#include <boost/fusion/include/end.hpp>
#include <boost/fusion/include/next.hpp>
#include <boost/fusion/include/prior.hpp>
#include <boost/proto/v5/proto_fwd.hpp>
#include <boost/proto/v5/fusion.hpp>
#include <boost/proto/v5/action/basic_action.hpp>
#include <boost/proto/v5/action/env.hpp>

namespace boost
{
    namespace proto
    {
        inline namespace v5
        {
            namespace detail
            {
                ////////////////////////////////////////////////////////////////////////////////////
                // fold_2_
                template<typename Fun>
                struct fold_2_
                {
                    template<typename Cur, typename End, typename Env, typename State, typename ...Rest
                      , typename Impl = fold_2_<Fun>
                      , BOOST_PROTO_ENABLE_IF(!(fusion::result_of::equal_to<Cur, End>::value))>
                    constexpr auto operator()(Cur const &cur, End const &end, Env &&env, State const &state, Rest &&...rest) const
                    BOOST_PROTO_AUTO_RETURN(
                        BOOST_PROTO_TRY_CALL(Impl())(
                            fusion::next(cur)
                          , end
                          , static_cast<Env &&>(env)
                          , call_action_<Fun>()(
                                fusion::deref(cur)
                              , static_cast<Env &&>(env)
                              , state
                              , static_cast<Rest &&>(rest)...
                            )
                          , static_cast<Rest &&>(rest)...
                        )
                    )

                    template<typename Cur, typename End, typename Env, typename State, typename ...Rest
                      , BOOST_PROTO_ENABLE_IF((fusion::result_of::equal_to<Cur, End>::value))>
                    constexpr State operator()(Cur const &, End const &, Env &&, State const &state, Rest &&...) const
                    {
                        return state;
                    }
                };

                ////////////////////////////////////////////////////////////////////////////////////
                // fold_1_
                // TODO: Optimize when Sequence == children<T...>
                template<typename Fun>
                struct fold_1_
                {
                    template<typename Sequence, typename Env, typename State0, typename ...Rest>
                    constexpr auto operator()(Sequence && seq, Env &&env, State0 const &state0, Rest &&... rest) const
                    BOOST_PROTO_AUTO_RETURN(
                        BOOST_PROTO_TRY_CALL(fold_2_<Fun>())(
                            fusion::begin(static_cast<Sequence &&>(seq))
                          , fusion::end(static_cast<Sequence &&>(seq))
                          , static_cast<Env &&>(env)
                          , state0
                          , static_cast<Rest &&>(rest)...
                        )
                    )
                };

                ////////////////////////////////////////////////////////////////////////////////////
                // reverse_fold_2_
                template<typename Fun>
                struct reverse_fold_2_
                {
                    template<typename Cur, typename End, typename Env, typename State, typename ...Rest
                      , typename Impl = reverse_fold_2_<Fun>
                      , BOOST_PROTO_ENABLE_IF(!(fusion::result_of::equal_to<Cur, End>::value))>
                    constexpr auto operator()(Cur const &cur, End const &end, Env &&env, State const &state, Rest &&...rest) const
                    BOOST_PROTO_AUTO_RETURN(
                        BOOST_PROTO_TRY_CALL(Impl())(
                            cur
                          , fusion::prior(end)
                          , static_cast<Env &&>(env)
                          , call_action_<Fun>()(
                                fusion::deref(fusion::prior(end))
                              , static_cast<Env &&>(env)
                              , state
                              , static_cast<Rest &&>(rest)...
                            )
                          , static_cast<Rest &&>(rest)...
                        )
                    )

                    template<typename Cur, typename End, typename Env, typename State, typename ...Rest
                      , BOOST_PROTO_ENABLE_IF((fusion::result_of::equal_to<Cur, End>::value))>
                    constexpr State operator()(Cur const &, End const &, Env &&, State const &state, Rest &&...) const
                    {
                        return state;
                    }
                };

                ////////////////////////////////////////////////////////////////////////////////////
                // reverse_fold_1_
                // TODO: Optimize when Sequence == children<T...>
                template<typename Fun>
                struct reverse_fold_1_
                {
                    template<typename Sequence, typename Env, typename State0, typename ...Rest>
                    constexpr auto operator()(Sequence && seq, Env &&env, State0 const &state0, Rest &&... rest) const
                    BOOST_PROTO_AUTO_RETURN(
                        BOOST_PROTO_TRY_CALL(reverse_fold_2_<Fun>())(
                            fusion::begin(static_cast<Sequence &&>(seq))
                          , fusion::end(static_cast<Sequence &&>(seq))
                          , static_cast<Env &&>(env)
                          , state0
                          , static_cast<Rest &&>(rest)...
                        )
                    )
                };

                ////////////////////////////////////////////////////////////////////////////////////
                // _fold
                // \brief A BasicAction that invokes the <tt>fusion::_fold\<\></tt>
                // action to accumulate
                template<typename Sequence, typename State0, typename Fun>
                struct _fold
                  : basic_action<_fold<Sequence, State0, Fun>>
                {
                    template<typename E>
                    constexpr auto operator()(E && e) const
                    BOOST_PROTO_AUTO_RETURN(
                        BOOST_PROTO_TRY_CALL(fold_1_<Fun>())(
                            call_action_<Sequence>()(static_cast<E &&>(e))
                          , empty_env()
                          , call_action_<State0>()(static_cast<E &&>(e))
                        )
                    )

                    template<typename E, typename D>
                    constexpr auto operator()(E && e, D && d) const
                    BOOST_PROTO_AUTO_RETURN(
                        BOOST_PROTO_TRY_CALL(fold_1_<Fun>())(
                            call_action_<Sequence>()(static_cast<E &&>(e), static_cast<D &&>(d))
                          , static_cast<D &&>(d)
                          , call_action_<State0>()(static_cast<E &&>(e), static_cast<D &&>(d))
                        )
                    )

                    template<typename E, typename D, typename S, typename ...Rest>
                    constexpr auto operator()(E && e, D && d, S && s, Rest &&... rest) const
                    BOOST_PROTO_AUTO_RETURN(
                        BOOST_PROTO_TRY_CALL(fold_1_<Fun>())(
                            call_action_<Sequence>()(static_cast<E &&>(e), static_cast<D &&>(d), static_cast<S &&>(s), static_cast<Rest &&>(rest)...)
                          , static_cast<D &&>(d)
                          , call_action_<State0>()(static_cast<E &&>(e), static_cast<D &&>(d), static_cast<S &&>(s), static_cast<Rest &&>(rest)...)
                          , static_cast<Rest &&>(rest)...
                        )
                    )
                };

                ////////////////////////////////////////////////////////////////////////////////////
                // _reverse_fold
                // A BasicAction that invokes the <tt>fusion::fold\<\></tt>
                // action to accumulate
                template<typename Sequence, typename State0, typename Fun>
                struct _reverse_fold
                  : basic_action<_reverse_fold<Sequence, State0, Fun>>
                {
                    template<typename E>
                    constexpr auto operator()(E && e) const
                    BOOST_PROTO_AUTO_RETURN(
                        BOOST_PROTO_TRY_CALL(reverse_fold_1_<Fun>())(
                            call_action_<Sequence>()(static_cast<E &&>(e))
                          , empty_env()
                          , call_action_<State0>()(static_cast<E &&>(e))
                        )
                    )

                    template<typename E, typename D>
                    constexpr auto operator()(E && e, D && d) const
                    BOOST_PROTO_AUTO_RETURN(
                        BOOST_PROTO_TRY_CALL(reverse_fold_1_<Fun>())(
                            call_action_<Sequence>()(static_cast<E &&>(e), static_cast<D &&>(d))
                          , static_cast<D &&>(d)
                          , call_action_<State0>()(static_cast<E &&>(e), static_cast<D &&>(d))
                        )
                    )

                    template<typename E, typename D, typename S, typename ...Rest>
                    constexpr auto operator()(E && e, D && d, S && s, Rest &&... rest) const
                    BOOST_PROTO_AUTO_RETURN(
                        BOOST_PROTO_TRY_CALL(reverse_fold_1_<Fun>())(
                            call_action_<Sequence>()(static_cast<E &&>(e), static_cast<D &&>(d), static_cast<S &&>(s), static_cast<Rest &&>(rest)...)
                          , static_cast<D &&>(d)
                          , call_action_<State0>()(static_cast<E &&>(e), static_cast<D &&>(d), static_cast<S &&>(s), static_cast<Rest &&>(rest)...)
                          , static_cast<Rest &&>(rest)...
                        )
                    )
                };
            }

            struct fold
            {};

            struct reverse_fold
            {};

            namespace extension
            {
                template<typename Seq, typename State0, typename Fun>
                struct action_impl<fold(Seq, State0, Fun)>
                  : detail::_fold<Seq, State0, Fun>
                {};

                template<typename Seq, typename State0, typename Fun>
                struct action_impl<reverse_fold(Seq, State0, Fun)>
                  : detail::_reverse_fold<Seq, State0, Fun>
                {};
            }
        }
    }
}

#endif
