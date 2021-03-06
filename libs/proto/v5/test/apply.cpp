////////////////////////////////////////////////////////////////////////////////////////////////////
// apply.cpp
//
//  Copyright 2012 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <utility>
#include <boost/proto/v5/proto.hpp>
#include "./unit_test.hpp"

namespace proto = boost::proto;
using proto::_;

template<int I>
struct fN
  : proto::env_tag<fN<I>>
{
    BOOST_PROTO_REGULAR_TRIVIAL_CLASS(fN);
    using proto::env_tag<fN>::operator=;
};

namespace
{
    constexpr auto const & _f0 = proto::utility::static_const<fN<0>>::value;
    constexpr auto const & _f1 = proto::utility::static_const<fN<1>>::value;
}

struct eval_unpack
  : proto::def<
        proto::apply(
            proto::get_env(fN<0>())
          , proto::apply(proto::get_env(fN<1>()), proto::pack(_))...
        )
    >
{};

template<typename E, typename F0, typename F1>
auto unpack(E && e, F0 && f0, F1 && f1)
BOOST_PROTO_AUTO_RETURN(
    eval_unpack()(
        std::forward<E>(e)
      , (_f0 = std::forward<F0>(f0), _f1 = std::forward<F1>(f1))
    )
)

struct square
{
    template<typename T>
    T operator()(T t) const
    {
        return t * t;
    }
};

struct sum
{
    template<typename T>
    T operator()(T t) const
    {
        return t;
    }

    template<typename T, typename ...U>
    T operator()(T t, U ...u) const
    {
        return t + (*this)(u...);
    }
};

void test_apply()
{
    proto::literal<int> i{0};

    // 0^2 + 1^2 + 2^2 + 3^2 = 0+1+4+9 = 14
    proto::def<square(proto::_value)> square_;
    int sum_of_squares = unpack(i(1,2,3), sum(), square_);
    BOOST_CHECK_EQUAL(sum_of_squares, 14);
}

using namespace boost::unit_test;
////////////////////////////////////////////////////////////////////////////////////////////////////
// init_unit_test_suite
//
test_suite* init_unit_test_suite( int argc, char* argv[] )
{
    test_suite *test = BOOST_TEST_SUITE("tests for proto::apply");

    test->add(BOOST_TEST_CASE(&test_apply));

    return test;
}
