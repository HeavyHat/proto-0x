////////////////////////////////////////////////////////////////////////////////////////////////////
// main.cpp
//
//  Copyright 2012 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
//#include <string>
//
//#define RETURN(...) -> \
//  decltype(__VA_ARGS__) { return __VA_ARGS__; }
//
//template<typename Sig>
//struct SUBSTITUTION_FAILURE;
//
//template<typename Fun, typename...Args>
//struct SUBSTITUTION_FAILURE<Fun(Args...)> {
//    virtual void what(Args&&...args) {
//        Fun()(std::forward<Args>(args)...);
//    }
//};
//
//template<typename Fun>
//struct try_call {
//  template<typename...Args>
//  auto operator()(Args&&...args) const
//    RETURN( Fun()( std::forward<Args>(args)... ) )
//
//  template<typename...Args>
//  SUBSTITUTION_FAILURE<Fun(Args...)>
//  operator()(Args&&...args) const volatile;
//};
//
//struct S0
//{
//  template<typename T>
//  auto operator()(T t) const RETURN( t + 1 )
//};
//
//struct S1
//{
//  template<typename T>
//  auto operator()(T t) const RETURN( try_call<S0>()(t) )
//};
//
//struct S2
//{
//  template<typename T>
//  auto operator()(T t) const RETURN( try_call<S1>()(t) )
//};
//
//struct S3
//{
//  template<typename T>
//  auto operator()(T t) const RETURN( try_call<S2>()(t) )
//};
//
//struct foo
//{};
//
//int main()
//{
//    //int i = S3()(32);
//    auto x = S3()(std::string("huh?"));
//}

#include <map>
#include <boost/proto/v5/proto.hpp>
#include <boost/proto/v5/debug.hpp>
namespace proto = boost::proto;
using namespace proto;

struct map_list_of_ 
{};

struct MapListOf : def<
  match(
    case_(  terminal(map_list_of_),
      void()
    ),
    case_(  function(MapListOf, terminal(_), terminal(_)),
      MapListOf(_child0),
      assign(subscript(_data, _value(_child1)), _value(_child2))
    )
  )
>{};

template<typename ExprDesc>
struct map_list_of_expr;

struct map_list_of_domain
  : domain<map_list_of_domain, MapListOf>
{
  using make_expr = make_custom_expr<map_list_of_expr>;
};

template<typename ExprDesc>
struct map_list_of_expr
  : expr<map_list_of_expr<ExprDesc>, map_list_of_domain>
{
  using expr<map_list_of_expr, map_list_of_domain>::expr;

  template<class K, class V, class C, class A>
  operator std::map<K,V,C,A>() const
  {
    BOOST_PROTO_ASSERT_MATCHES(*this, MapListOf);
    std::map<K,V,C,A> m;
    MapListOf()( *this, data = m );
    return m;
  }
};

constexpr map_list_of_expr<terminal(map_list_of_)> map_list_of {};
//constexpr auto map_list_of = as_expr<map_list_of_domain>(map_list_of_{});

constexpr auto omg = map_list_of(1,2)(2,3)(3,4)(42,5)(5,6);

enum class srsly
{
    wtf = value(child<1>(child<0>(omg)))
};

static_assert((int)srsly::wtf == 42, "");

int main()
{
  std::map<int, int> next = map_list_of(1,2)(2,3)(3,4)(4,5)(5,6);

  void done();
  done();
}

/*
#include <cstdio>
#include <utility>
#include <typeinfo>
#include <type_traits>
#include <memory>
#include <cstdlib>
#include <boost/proto/v5/proto.hpp>
#include <boost/proto/v5/debug.hpp>

namespace proto = boost::proto;
using proto::_;

template<typename T>
struct placeholder
  : proto::env_var_tag<placeholder<T>>
{
    using proto::env_var_tag<placeholder<T>>::operator=;
};

static_assert(std::is_trivial<placeholder<std::integral_constant<std::size_t, 0>>>::value, "");

// So placeholder terminals can be pretty-printed with display_expr
template<typename T>
std::ostream & operator << (std::ostream & s, placeholder<T>)
{
    return s << "_" << T::value + 1;
}

template<std::size_t I>
using placeholder_c = placeholder<std::integral_constant<std::size_t, I>>;

struct lambda_eval
  : proto::def<
        proto::match(
            proto::case_( proto::terminal(placeholder<_>),
                proto::apply(proto::make(proto::_env_var<proto::_value>()))
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
      , proto::make_env(placeholder_c<I>() = std::forward<T>(t)...)
    )
)

template<typename ExprDesc>
struct lambda_expr;

struct lambda_domain
  : proto::domain<lambda_domain>
{
    using make_expr = proto::make_custom_expr<lambda_expr>;
    using store_child = proto::utility::by_ref;
};

template<typename ExprDesc>
struct lambda_expr
  : proto::basic_expr<ExprDesc, lambda_domain>
  , proto::expr_assign<lambda_expr<ExprDesc>>
  , proto::expr_subscript<lambda_expr<ExprDesc>>
{
    using proto::basic_expr<ExprDesc, lambda_domain>::basic_expr;
    using proto::expr_assign<lambda_expr>::operator=;

    template<typename ...T>
    auto operator()(T &&... t) const
    BOOST_PROTO_AUTO_RETURN(
        lambda_eval_(proto::utility::make_indices<sizeof...(T)>(), *this, std::forward<T>(t)...)
    )
};

template<typename T>
using lambda_var = lambda_expr<proto::terminal(T)>;

namespace
{
    constexpr auto const & _1 = proto::utility::static_const<lambda_var<placeholder_c<0>>>::value;
    constexpr auto const & _2 = proto::utility::static_const<lambda_var<placeholder_c<1>>>::value;
    constexpr auto const & _3 = proto::utility::static_const<lambda_var<placeholder_c<2>>>::value;
}

static_assert(std::is_trivial<lambda_var<placeholder_c<0>>>::value, "_1 should be trivial");
BOOST_PROTO_IGNORE_UNUSED(_1, _2, _3);

using make_minus = proto::functional::make_expr<proto::minus>;

struct Invert
  : proto::def<
        proto::match(
            proto::case_(   proto::terminal(_),
                            proto::_                            )
          , proto::case_(   proto::plus(_,_),
                            proto::minus(Invert(proto::_left),
                                         Invert(proto::_right)) )
          , proto::case_(   _(Invert...),
                            proto::pass                         )
        )
    >
{};

int main()
{
    std::printf("*** \n");
    std::printf("*** This program demonstrates how to build a lambda library with Proto.\n");
    std::printf("*** \n");

    std::cout << "expr:\n"
              << proto::pretty_expr(_1 + 42 * _2, 4)
              << std::endl;

    // Create a lambda
    auto fun = proto::deep_copy(_1 + 42 * _2);

    std::cout << "deep-copied expr:\n"
              << proto::pretty_expr(fun, 4)
              << std::endl;

    std::cout << "Invert'ed expr:\n"
              << proto::pretty_expr(Invert()(fun), 4)
              << std::endl;

    // call the lambda
    int i = fun(8, 2);

    // print the result
    std::printf("the lambda '_1 + 42 * _2' yields '%d' when called with 8 and 2.\n", i);

    void done();
    done();
}
//*/

void done()
{
    char ch{};
    std::cout << "CTRL+D to end...";
    std::cin >> ch;
}