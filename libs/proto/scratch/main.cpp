////////////////////////////////////////////////////////////////////////////////////////////////////
// main.cpp
//
//  Copyright 2012 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <vector>
#include <boost/proto/proto.hpp>
#include <boost/proto/debug.hpp>
namespace mpl = boost::mpl;
namespace proto = boost::proto;
using proto::_;

namespace linear_algebra
{
    // A trait that returns true only for std::vector
    template<typename T>
    struct is_std_vector
      : mpl::false_
    {};

    template<typename T, typename A>
    struct is_std_vector<std::vector<T, A> >
      : mpl::true_
    {};

    // A type used as a domain for linear algebra expressions
    struct linear_algebra_domain
      : proto::domain<linear_algebra_domain>
    {};

    // Define all the operator overloads for combining std::vectors
    BOOST_PROTO_DEFINE_OPERATORS(is_std_vector, linear_algebra_domain)

    // Take any expression and turn each node
    // into a subscript expression, using the
    // state as the RHS.
    struct Distribute
      : proto::action<
            proto::match(
                proto::case_(
                    proto::terminal(_)
                  , proto::subscript(_, proto::_state)
                )
              , proto::case_(
                    proto::plus(Distribute, Distribute)
                  , proto::pass
                )
            )
        >
    {};

    struct Optimize
      : proto::action<
            proto::match(
                proto::case_(
                    proto::subscript(Distribute, proto::terminal(_))
                  , Distribute(proto::_left, proto::_right)
                )
              , proto::case_(
                    proto::plus(Optimize, Optimize)
                  , proto::pass
                )
              , proto::case_(
                    proto::terminal(_)
                  , proto::pass
                )
            )
        >
    {};
}

static constexpr int celems = 4;
static constexpr int value[celems] = {1,2,3,4};
std::vector<int> A(value, value+celems), B(A);

int main()
{
    using namespace linear_algebra;
    proto::_eval<> eval;
    int result = eval(Optimize()((A + B)[3]));
    proto::assert_matches<Optimize>((A + B)[3]);
    proto::assert_matches_not<Optimize>((A - B)[3]);
    BOOST_CHECK_EQUAL(8, result);
}


/*
#include <cstdio>
#include <utility>
#include <type_traits>
#include <boost/proto/proto.hpp>
#include <boost/proto/debug.hpp>

namespace proto = boost::proto;
using proto::_;

template<typename T>
struct placeholder
  : proto::tags::env_var_tag<placeholder<T>>
{
    BOOST_PROTO_REGULAR_TRIVIAL_CLASS(placeholder);
    using proto::tags::env_var_tag<placeholder<T>>::operator=;

    // So placeholder terminals can be pretty-printed with display_expr
    friend std::ostream & operator << (std::ostream & s, placeholder<T>)
    {
        return s << "_" << T::value + 1;
    }
};

template<std::size_t I>
using placeholder_c = placeholder<std::integral_constant<std::size_t, I>>;

struct lambda_eval
  : proto::action<
        proto::match(
            proto::case_( proto::terminal(placeholder<_>),
                proto::apply(proto::construct(proto::_env_var<proto::_value>()))
            )
          , proto::case_( proto::terminal(_),
                proto::_value
            )
          , proto::case_( _,
                proto::_eval<lambda_eval>
            )
        )
    >
{};

template<std::size_t ...I, typename E, typename ...T>
inline auto lambda_eval_(proto::utility::indices<I...>, E && e, T &&... t)
BOOST_PROTO_AUTO_RETURN(
    lambda_eval()(
        std::forward<E>(e)
      , 0
      , proto::make_env(placeholder_c<I>() = std::forward<T>(t)...)
    )
)

template<typename ExprDesc>
struct lambda_expr;

struct lambda_domain
  : proto::domain<lambda_domain>
{
    struct make_expr
      : proto::make_custom_expr<lambda_expr>
    {};
};

template<typename ExprDesc>
struct lambda_expr
  : proto::basic_expr<ExprDesc, lambda_domain>
  , proto::expr_assign<lambda_expr<ExprDesc>, lambda_domain>
  , proto::expr_subscript<lambda_expr<ExprDesc>, lambda_domain>
{
    BOOST_PROTO_REGULAR_TRIVIAL_CLASS(lambda_expr);

    using proto::expr_assign<lambda_expr, lambda_domain>::operator=;
    //using proto::basic_expr<ExprDesc, lambda_domain>::basic_expr;
    typedef proto::basic_expr<ExprDesc, lambda_domain> proto_basic_expr_type;
    BOOST_PROTO_INHERIT_EXPR_CTORS(lambda_expr, proto_basic_expr_type);

    template<typename ...T>
    auto operator()(T &&... t) const
    BOOST_PROTO_AUTO_RETURN(
        lambda_eval_(proto::utility::make_indices<sizeof...(T)>(), *this, std::forward<T>(t)...)
    )
};

template<typename T>
using lambda_var = proto::custom<lambda_expr>::terminal<T>;

namespace
{
    constexpr auto const & _1 = proto::utility::static_const<lambda_var<placeholder_c<0>>>::value;
    constexpr auto const & _2 = proto::utility::static_const<lambda_var<placeholder_c<1>>>::value;
    constexpr auto const & _3 = proto::utility::static_const<lambda_var<placeholder_c<2>>>::value;
}

static_assert(std::is_trivial<lambda_var<placeholder_c<0>>>::value, "_1 should be trivial");
BOOST_PROTO_IGNORE_UNUSED(_1, _2, _3);

int main()
{
    std::printf("*** \n");
    std::printf("*** This program demonstrates how to build a lambda library with Proto.\n");
    std::printf("*** \n");

    // Create a lambda
    auto fun = _1 + 42 * _2;

    // pretty-print the expression
    proto::display_expr(fun);

    // Call the lambda
    int i = fun(8, 2);

    // print the result
    std::printf("The lambda '_1 + 42 * _2' yields '%d' when called with 8 and 2.\n", i);

    void done();
    done();
}

void done()
{
    char ch{};
    std::cout << "CTRL+D to end...";
    std::cin >> ch;
}
//*/
