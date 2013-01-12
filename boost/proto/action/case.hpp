////////////////////////////////////////////////////////////////////////////////////////////////////
// case.hpp
// Associate a custom basic_action with a grammar rule
//
//  Copyright 2012 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_ACTION_CASE_HPP_INCLUDED
#define BOOST_PROTO_ACTION_CASE_HPP_INCLUDED

#include <boost/proto/proto_fwd.hpp>
#include <boost/proto/action/action.hpp>
#include <boost/proto/action/block.hpp>

namespace boost
{
    namespace proto
    {
        namespace extension
        {
            ////////////////////////////////////////////////////////////////////////////////////////////
            // case_
            template<typename Grammar, typename ...Action>
            struct action_impl<case_(Grammar, Action...)>
              : detail::_block<Action...>
            {};

            template<typename Grammar, typename Action>
            struct action_impl<case_(Grammar, Action)>
              : action<Action>
            {};

            // These two are defined in pass.hpp
            template<typename Tag, typename ...ActiveGrammars>
            struct action_impl<case_(Tag(ActiveGrammars...), pass)>;

            template<typename Tag, typename ...ActiveGrammars>
            struct action_impl<case_(Tag(ActiveGrammars......), pass)>;
        }
    }
}

#endif
