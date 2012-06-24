////////////////////////////////////////////////////////////////////////////////////////////////////
// lambda.cpp
//
//  Copyright 2012 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <cstdio>
#include <type_traits>
#include <boost/mpl/size_t.hpp>
#include <boost/proto/proto.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;
using proto::_;

template<typename T>
struct placeholder
  : proto::tags::def<placeholder<T>>
{
    using proto::tags::def<placeholder<T>>::operator=;
};

template<std::size_t I>
using placeholder_c = placeholder<mpl::size_t<I>>;

struct lambda_eval
  : proto::or_<
        proto::when< proto::terminal<placeholder<_>>,   proto::_env<proto::_value>()>
      , proto::when< proto::terminal<_>,                proto::_value>
      , proto::when< _,                                 proto::_eval<lambda_eval>>
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

template<typename Tag, typename Args>
struct lambda_expr;

struct lambda_domain
  : proto::domain<lambda_domain>
{
    struct make_expr
      : proto::make_custom_expr<lambda_expr, lambda_domain>
    {};
};

template<typename Tag, typename Args>
struct lambda_expr
  : proto::basic_expr<Tag, Args, lambda_domain>
  , proto::expr_assign<lambda_expr<Tag, Args>, lambda_domain>
  , proto::expr_subscript<lambda_expr<Tag, Args>, lambda_domain>
{
    BOOST_PROTO_REGULAR_TRIVIAL_CLASS(lambda_expr);

    using proto::expr_assign<lambda_expr, lambda_domain>::operator=;
    //using proto::basic_expr<Tag, Args, lambda_domain>::basic_expr;
    typedef proto::basic_expr<Tag, Args, lambda_domain> proto_basic_expr;
    BOOST_PROTO_INHERIT_EXPR_CTORS(lambda_expr, proto_basic_expr);

    template<typename ...T>
    auto operator()(T &&... t) const
    BOOST_PROTO_AUTO_RETURN(
        lambda_eval_(proto::utility::make_indices<0, sizeof...(T)>(), *this, std::forward<T>(t)...)
    )
};

template<typename T>
using lambda_var = lambda_expr<proto::tag::terminal, proto::args<T>>;

typedef lambda_var<placeholder_c<0>> _1_type;
typedef lambda_var<placeholder_c<1>> _2_type;
static_assert(std::is_trivial<_1_type>::value, "_1 is not trivial");

namespace
{
    constexpr _1_type const & _1 = proto::utility::static_const<_1_type>::value;
    constexpr _2_type const & _2 = proto::utility::static_const<_2_type>::value;
}

int main()
{
    std::printf("*** \n");
    std::printf("*** This program demonstrates how to build a lambda library with Proto.\n");
    std::printf("*** \n");

    // Create a lambda
    auto fun = _1 + 42 * _2;

    // Call the lambda
    int i = fun(8, 2);

    // print the result
    std::printf("The lambda '_1 + 42 * _2' yields '%d' when called with 8 and 2.\n", i);
}