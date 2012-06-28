////////////////////////////////////////////////////////////////////////////////////////////////////
// main.cpp
//
//  Copyright 2012 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <cstdio>
#include <utility>
#include <type_traits>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/proto/proto.hpp>
#include <boost/proto/debug.hpp>

namespace proto = boost::proto;
using proto::_;

struct foo_tag {};
struct bar_tag {};

template<typename Tag, typename Args>
struct MyExpr;

struct MyDomain
  : proto::domain<MyDomain>
{
    struct make_expr
      : proto::make_custom_expr<MyExpr, MyDomain>
    {};
};

template<typename Tag, typename Args>
struct MyExpr
  : proto::basic_expr<Tag, Args, MyDomain>
{
    BOOST_PROTO_REGULAR_TRIVIAL_CLASS(MyExpr);
    typedef proto::basic_expr<Tag, Args, MyDomain> proto_basic_expr_type;
    BOOST_PROTO_INHERIT_EXPR_CTORS(MyExpr, proto_basic_expr_type);

    BOOST_PROTO_EXTENDS_MEMBERS(
        MyExpr, MyDomain,
        ((foo_tag, foo))
        ((bar_tag, bar))
    );
};

template<typename Value>
using MyTerminal = MyExpr<proto::tag::terminal, proto::args<Value>>;

int main()
{
    MyTerminal<int> xxx{42};
    MyTerminal<int> & r = proto::child<0>(xxx.foo);
    proto::terminal<foo_tag> & e = proto::child<1>(xxx.foo);

    static_assert(std::is_lvalue_reference<decltype(proto::child<0>(xxx.foo))>::value, "");
    static_assert(std::is_rvalue_reference<decltype(proto::child<0>(MyTerminal<int>().foo))>::value, "");

    static_assert(std::is_lvalue_reference<decltype(proto::child<1>(xxx.foo))>::value, "");
    static_assert(std::is_rvalue_reference<decltype(proto::child<1>(MyTerminal<int>().foo))>::value, "");

    std::printf("this should be 42: %d\n", proto::value(xxx));
    std::printf("this should be 42: %d\n", proto::value(r));
    std::printf("&xxx == %p\n", boost::addressof(xxx));
    std::printf("&r == %p\n", boost::addressof(r));
    std::printf("\n");
    std::printf("&xxx.foo == %p\n", boost::addressof(xxx.foo.proto_args().proto_child1));
    std::printf("&e == %p\n", boost::addressof(e));

    struct G : proto::member<proto::terminal<int>, proto::terminal<foo_tag>> {};
    proto::assert_matches<G>(xxx.foo);
    MyExpr<proto::tag::member, proto::args<MyTerminal<int>, proto::terminal<foo_tag>>> tx = proto::as_transform<G>()(xxx.foo);

    void done();
    done();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void done()
{
    std::printf("Press <return> to continue...");
    std::fgetc(stdin);
}
