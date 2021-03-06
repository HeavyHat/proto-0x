////////////////////////////////////////////////////////////////////////////////////////////////////
// bug2407.hpp
//
//  Copyright 2008 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <boost/proto/v5/proto.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;
using proto::_;

struct g
  : proto::def<
        proto::match(
            proto::terminal(int)
          , proto::plus(g, g)
        )
    >
{};

struct d
  : proto::domain<d, g>
{};

template<class E>
struct e
  : proto::basic_expr<e<E>, d>
{
    static_assert(proto::matches<proto::basic_expr<E>, g>(), "");
    using proto::basic_expr<e, d>::basic_expr;
};

e<proto::terminal(int)> i;

template<class E>
std::ostream &operator<<(std::ostream &sout, e<E> const &x)
{
    return sout;
}

int main()
{
    std::cout << (i+i);
}
