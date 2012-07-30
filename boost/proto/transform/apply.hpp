///////////////////////////////////////////////////////////////////////////////
// apply.hpp
// Treat the first argument as a callable and the others as arguments to the
// callable
//
//  Copyright 2012 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_TRANSFORM_APPLY_HPP_INCLUDED
#define BOOST_PROTO_TRANSFORM_APPLY_HPP_INCLUDED

#include <boost/proto/proto_fwd.hpp>
#include <boost/proto/transform/base.hpp>

namespace boost
{
    namespace proto
    {
        template<typename Fun, typename ...Tfxs, int I>
        struct as_transform<apply(Fun, Tfxs...), I>
          : transform<as_transform<apply(Fun, Tfxs...), I>>
        {
            template<typename ...Args, typename X = decltype(as_transform<Fun>()(std::declval<Args>()...))>
            auto operator()(Args &&... args) const
            BOOST_PROTO_AUTO_RETURN(
                detail::call_1_<is_transform<X>::value, Tfxs...>()(
                    as_transform<Fun>()(static_cast<Args &&>(args)...)
                  , static_cast<Args &&>(args)...
                )
            )
        };
    }
}

#endif
