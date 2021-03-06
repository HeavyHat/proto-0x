////////////////////////////////////////////////////////////////////////////////////////////////////
// expr.hpp
//
//  Copyright 2012 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <type_traits>
#include <boost/proto/v5/proto.hpp>
#include "./unit_test.hpp"

namespace mpl = boost::mpl;
namespace proto = boost::proto;
using proto::_;

// Test that simple proto expressions are trivial.
// Note: expressions that store references are not, and cannot be, trivial because
// they are not default makeable.
using int_ = proto::literal<int>;
using string_ = proto::literal<std::string>;
static_assert(std::is_trivial<decltype(int_())>::value, "not trivial!");
static_assert(std::is_trivial<decltype(int_()(3.14))>::value, "not trivial!");

template<typename ExprDesc>
struct MyExpr;

struct MyDomain
  : proto::domain<MyDomain>
{
    using make_expr = proto::make_custom_expr<MyExpr<_>>;
};

template<typename ExprDesc>
struct MyExpr
  : proto::basic_expr<ExprDesc, MyDomain>
  , proto::expr_assign<MyExpr<ExprDesc>>
  , proto::expr_subscript<MyExpr<ExprDesc>>
  , proto::expr_function<MyExpr<ExprDesc>>
{
    using proto::basic_expr<ExprDesc, MyDomain>::basic_expr;
    using proto::expr_assign<MyExpr>::operator=;
};

using My = proto::custom<MyExpr<_>>;
static_assert(std::is_trivial<My::terminal<int>>::value, "not is trivial!");

void test_basic_expr()
{
    proto::basic_expr<proto::terminal(int)> bi{42};

    // Combining basic_exprs should yield more basic_exprs
    proto::basic_expr<
        proto::plus(
            proto::basic_expr<proto::terminal(int)> &
          , proto::basic_expr<proto::terminal(int)> &
        )
    > pbibi = bi + bi;
    BOOST_PROTO_IGNORE_UNUSED(pbibi);

    typedef
        proto::basic_expr<
            proto::plus(
                proto::terminal(int)
              , proto::terminal(int)
            )
        >
    pbibi_0_t;

    typedef
        proto::basic_expr<
            proto::plus(
                proto::basic_expr<proto::terminal(int)>
              , proto::basic_expr<proto::terminal(int)>
            )
        >
    pbibi_1_t;

    static_assert(std::is_same<pbibi_0_t, pbibi_1_t>::value, "not the same");
}

void test_expr()
{
    int_ p(42);
    int_ const pc(42);
    constexpr int_ i_(42);

    // Quick test for big expression nodes (>10 children)
    using ints_ = proto::expr<proto::function(int_, int_, int_, int_, int_, int_, int_, int_, int_, int_, int_, int_, int_, int_)>;
    ints_ is(p,p,p,p,p,p,p,p,p,p,p,p,p,p);
    p = proto::child<13>(is);

    // quick sanity check for as_expr
    auto a1 = proto::as_expr(42);
    auto a2 = proto::as_expr(p);
    static_assert(std::is_same<decltype(a1), decltype(a2)>::value, "not the same!");

    // quick sanity test for proto::value
    int i = proto::value(p);
    BOOST_PROTO_IGNORE_UNUSED(i);

    // sanity test for stored_value and stored_child (used by as_expr when building nodes)
    proto::expr<proto::function(int_&, int_, int_, int_, proto::terminal(char const (&)[6]))> x = p(1, 2, 3, "hello");
    static_assert(std::is_same<decltype(proto::value(proto::child<4>(x))), char const (&)[6]>::value, "not the same!");

    // verify that expression nodes are Regular types.
    auto y0 = (p=p);
    auto y1 = (p=pc);
    static_assert(std::is_same<decltype(y0)::proto_tag_type, proto::terminal>::value, "");
    static_assert(std::is_same<decltype(y1)::proto_tag_type, proto::terminal>::value, "");

    // Verify that overloaded assignment builds an assign expression node.
    auto y2 = (p='c');
    static_assert(std::is_same<decltype(y2)::proto_tag_type, proto::assign>::value, "");

    // verify that args accessor on rvalue expression is itself an rvalue
    static_assert(std::is_rvalue_reference<decltype(proto::exprs::access::proto_args(int_()))>::value, "isn't an rvalue reference!");

    // verify that expression nodes are no larger than they need to be.
    static_assert(sizeof(proto::literal<int>) == sizeof(int), "sizeof(proto::literal<int>) != sizeof(int)");
    static_assert(sizeof(proto::expr<proto::function()>) == 1, "size of empty expr is not 1");

    // This should fail to compile:
    //using cint_ = int_ const;
    //cint_(42)[p];

    constexpr My::terminal<int> iii_(42);
    auto jjj_ = iii_[42];

    // Test proto_equal_to
    BOOST_CHECK(proto::exprs::access::proto_equal_to(jjj_, iii_[42]));
    BOOST_CHECK(!proto::exprs::access::proto_equal_to(jjj_, iii_[43]));

    // Test convertibility to bool
    BOOST_CHECK(proto::make_expr<MyDomain>(proto::equal_to(), jjj_, iii_[42]));
    BOOST_CHECK(!proto::make_expr<MyDomain>(proto::equal_to(), jjj_, iii_[43]));
    BOOST_CHECK(!proto::make_expr<MyDomain>(proto::not_equal_to(), jjj_, iii_[42]));
    BOOST_CHECK(proto::make_expr<MyDomain>(proto::not_equal_to(), jjj_, iii_[43]));

    static_assert(std::is_convertible<proto::exprs::equal_to<int_, int_>, bool>::value, "not convertible to bool");
    static_assert(std::is_convertible<proto::exprs::not_equal_to<int_, int_>, bool>::value, "not convertible to bool");
    static_assert(!std::is_convertible<proto::exprs::equal_to<int_, proto::literal<void *>>, bool>::value, "convertible to bool");
    static_assert(!std::is_convertible<proto::exprs::not_equal_to<int_, proto::literal<void *>>, bool>::value, "convertible to bool");

    bool b0 = (int_(42) + 42) == (42 + int_(42));
    BOOST_CHECK(b0);
    bool b1 = (int_(42) + int_(42)) == (int_(42) + int_(43));
    BOOST_CHECK(!b1);

    // test for nothrow operations
    using int_ne_int = proto::exprs::not_equal_to<int_, int_>;
    int_ne_int inei(int_(42), int_(42));
    constexpr int_ne_int cinei(int_(42), int_(42));
    static_assert(noexcept(int_ne_int()), "not noexcept default constructor");
    static_assert(noexcept(int_ne_int(cinei)), "not noexcept copy constructor");
    static_assert(noexcept(int_ne_int(int_ne_int())), "not noexcept move constructor");
    static_assert(noexcept(inei = cinei), "not noexcept copy assign");
    static_assert(noexcept(inei = int_ne_int()), "not noexcept move assign");

    // Verify that the constexpr ctors can be called even when the
    // contained objects have non-constexpr constructors
    string_ str("hellohellohellohellohellohellohellohello!");
    BOOST_PROTO_IGNORE_UNUSED(str);

    // Check that nested expressions are in the right domain:
    static_assert(
        std::is_same<
            MyExpr<proto::negate(proto::terminal(int))>::proto_children_type::proto_child_type0
          , MyExpr<proto::terminal(int)>
        >::value
      , ""
    );
    MyExpr<proto::negate(proto::terminal(int))> outer;
    MyExpr<proto::terminal(int)> inner = proto::child<0>(outer);
    BOOST_PROTO_IGNORE_UNUSED(inner);
}

using namespace boost::unit_test;
////////////////////////////////////////////////////////////////////////////////////////////////////
// init_unit_test_suite
//
test_suite* init_unit_test_suite( int argc, char* argv[] )
{
    test_suite *test = BOOST_TEST_SUITE("basic tests for the expr data structure");

    test->add(BOOST_TEST_CASE(&test_basic_expr));
    test->add(BOOST_TEST_CASE(&test_expr));

    return test;
}
