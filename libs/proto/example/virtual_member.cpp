//[ VirtualMember
//  Copyright 2012 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// This example demonstrates how to use BOOST_PROTO_EXTENDS_MEMBERS()
// to add "virtual" data members to expressions within a domain. For
// instance, with Phoenix you can create a lambda expression such as
//
//    if_(_1 > 0)[ std::cout << _2 ].else_[ std::cout << _3 ]
//
// In the above expression, "else_" is a so-called virtual data member
// of the expression "if_(_1 > 0)[ std::cout << _2 ]". This example
// shows how to implement the ".else_" syntax with Proto.
//
// ****WARNING****WARNING****WARNING****WARNING****WARNING****WARNING****
// * The virtual data member feature is experimental and can change at  *
// * any time. Use it at your own risk.                                 *
// **********************************************************************

#include <iostream>
#include <type_traits>
#include <boost/proto/proto.hpp>
#include <boost/proto/debug.hpp>

namespace proto = boost::proto;
using proto::_;

namespace std
{
    // For debugging purposes only. Don't do this in production code.
    std::ostream & operator<<(std::ostream & s, std::ostream &)
    {
        return s << "std::cout";
    }
}

namespace mini_lambda
{
    // An MPL IntegralConstant
    template<class N>
    struct placeholder
      : proto::tags::def<placeholder<N>>
    {
        using proto::tags::def<placeholder>::operator=;

        // So placeholder terminals can be pretty-printed with display_expr
        friend std::ostream & operator << (std::ostream & s, placeholder)
        {
            return s << "_" << N::value + 1;
        }
    };

    template<std::size_t N>
    using placeholder_c = placeholder<std::integral_constant<std::size_t, N>>;

    // Some keyword types for our lambda EDSL
    namespace keyword
    {
        struct if_ {};
        struct else_ {};
        struct do_ {};
        struct while_ {};
        struct try_ {};
        struct catch_ {};
    }

    // Forward declaration for the mini-lambda grammar
    struct grammar;
    struct eval_if_else;

    // Forward declaration for the mini-lambda expression wrapper
    template<class Tag, class Args>
    struct expression;

    // The grammar for mini-lambda expressions with transforms for
    // evaluating the lambda expression.
    struct grammar
      : proto::or_<
            // When evaluating a placeholder, use the placeholder
            // to index into the "data" parameter, which is a fusion
            // vector containing the arguments to the lambda expression.
            proto::when<
                proto::terminal<placeholder<_> >
              , proto::_env<proto::_value>()
            >
            // When evaluating if/then/else expressions of the form
            // "if_( E0 )[ E1 ].else_[ E2 ]", pass E0, E1 and E2 to
            // eval_if_else along with the "data" parameter. Note the
            // use of proto::member<> to match binary expressions like
            // "X.Y" where "Y" is a virtual data member.
          , proto::when<
                proto::subscript<
                    proto::member<
                        proto::subscript<
                            proto::function<
                                proto::terminal<keyword::if_>
                              , grammar
                            >
                          , grammar
                        >
                      , proto::terminal<keyword::else_>
                    >
                  , grammar
                >
              , eval_if_else(
                    proto::_right(proto::_left(proto::_left(proto::_left)))
                  , proto::_right(proto::_left(proto::_left))
                  , proto::_right
                  , proto::_env<>
                )
            >
          , proto::otherwise<
                proto::_eval<grammar>
            >
        >
    {};

    // A callable PolymorphicFunctionObject that evaluates
    // if/then/else expressions.
    struct eval_if_else
    {
        template<typename If, typename Then, typename Else, typename Args>
        void operator()(If const &if_, Then const &then_, Else const &else_, Args const &args) const
        {
            if(grammar()(if_, 0, args))
            {
                grammar()(then_, 0, args);
            }
            else
            {
                grammar()(else_, 0, args);
            }
        }
    };

    // Define the mini-lambda domain, in which all expressions are
    // wrapped in mini_lambda::expression.
    struct domain
      : proto::domain<domain>
    {
        struct make_expr
          : proto::make_custom_expr<expression, domain>
        {};
    };

    // Here is the mini-lambda expression wrapper. It serves two purposes:
    // 1) To define operator() overloads that evaluate the lambda expression, and
    // 2) To define virtual data members like "else_" so that we can write
    //    expressions like "if_(X)[Y].else_[Z]".
    template<class Tag, class Args>
    struct expression
      : proto::basic_expr<Tag, Args, domain>
      , proto::expr_assign<expression<Tag, Args>, domain>
      , proto::expr_subscript<expression<Tag, Args>, domain>
    {
    private:
        template<std::size_t ...I, typename ...A>
        auto eval(proto::utility::indices<I...>, A &&...a) const
        BOOST_PROTO_AUTO_RETURN(
            grammar()(*this, 0, proto::make_env(placeholder_c<I>() = std::forward<A>(a)...))
        )

    public:
        typedef proto::basic_expr<Tag, Args, domain> proto_basic_expr_type;
        BOOST_PROTO_REGULAR_TRIVIAL_CLASS(expression);
        BOOST_PROTO_INHERIT_EXPR_CTORS(expression, proto_basic_expr_type);
        using proto::expr_assign<expression, domain>::operator=;

        // Use BOOST_PROTO_EXTENDS_MEMBERS() to define "virtual"
        // data members that all expressions in the mini-lambda
        // domain will have. They can be used to create expressions
        // like "if_(x)[y].else_[z]" and "do_[y].while_(z)".
        BOOST_PROTO_EXTENDS_MEMBERS(
            expression, domain,
            ((keyword::else_,   else_))
            ((keyword::while_,  while_))
            ((keyword::catch_,  catch_))
        );

        template<typename ...A>
        auto operator()(A &&... a) const
        BOOST_PROTO_AUTO_RETURN(
            expression::eval(proto::utility::make_indices<0, sizeof...(A)>(), std::forward<A>(a)...)
        )
    };

    template<typename T>
    using var = expression<proto::tag::terminal, proto::args<T>>;

    namespace placeholders
    {
        typedef var<placeholder_c<0>> _1_t;
        typedef var<placeholder_c<1>> _2_t;
        typedef var<placeholder_c<2>> _3_t;

        // Define some placeholders
        namespace
        {
            constexpr _1_t const & _1 = proto::utility::static_const<_1_t>::value;
            constexpr _2_t const & _2 = proto::utility::static_const<_2_t>::value;
            constexpr _3_t const & _3 = proto::utility::static_const<_3_t>::value;
        }

        BOOST_PROTO_IGNORE_UNUSED(_1, _2, _3);

        // Define the if_() statement
        template<typename E>
        auto if_(E && e)
        BOOST_PROTO_AUTO_RETURN(
            proto::domains::make_expr<domain>(
                proto::tag::function()
              , keyword::if_()
              , std::forward<E>(e)
            )
        )
    }

    using placeholders::if_;
}

int main()
{
    using namespace mini_lambda::placeholders;

    // OK, we can create if/then/else lambda expressions
    // and evaluate them.
    auto fun =
        if_(_1 > 0)
        [
            std::cout << _2 << '\n'
        ]
        .else_
        [
            std::cout << _3 << '\n'
        ];

    proto::display_expr(fun);
    fun(42, "positive", "non-positive");
    fun(-42, "positive", "non-positive");

    // Even though all expressions in the mini-lambda
    // domain have members named else_, while_, and catch_,
    // they all occupy the same byte in the expression.
    static_assert(sizeof(_1) == 3, "_1 should be only 3 bytes");
}
//]