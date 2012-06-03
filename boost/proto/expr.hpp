///////////////////////////////////////////////////////////////////////////////
// expr.hpp
//
// Copyright 2012 Eric Niebler.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_EXPR_HPP_INCLUDED
#define BOOST_PROTO_EXPR_HPP_INCLUDED

#include <utility>
#include <boost/mpl/bool.hpp>
#include <boost/proto/proto_fwd.hpp>
#include <boost/proto/args.hpp>
#include <boost/proto/domain.hpp>
#include <boost/proto/tags.hpp>

namespace boost
{
    namespace proto
    {
        namespace detail
        {
            struct any { template<typename T> constexpr any(T const &) noexcept; };

            ////////////////////////////////////////////////////////////////////////////////////////
            // dependent_static_cast
            template<typename T, typename U, typename ...V>
            inline constexpr T dependent_static_cast(U &&u, V &&...)
            {
                return static_cast<T>(static_cast<U &&>(u));
            }

            ////////////////////////////////////////////////////////////////////////////////////////
            // is_expr
            template<typename ...T>
            std::true_type is_expr(args<T...> const &);

            std::false_type is_expr(any);

            ////////////////////////////////////////////////////////////////////////////////////////
            // is_same_expr
            template<typename Expr>
            std::true_type is_same_expr(Expr const &);

            template<typename Expr>
            std::false_type is_same_expr(any);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        // is_expr
        template<typename T>
        struct is_expr
          : decltype(detail::is_expr(std::declval<T>()))
        {};

        ////////////////////////////////////////////////////////////////////////////////////////////
        // is_terminal
        template<typename Expr>
        struct is_terminal
          : Expr::proto_is_terminal
        {};

        #define BOOST_PROTO_IS_SAME_EXPR(EXPR, T)                                                   \
            decltype(boost::proto::detail::is_same_expr<EXPR>(std::declval<T>()))::value

        #define BOOST_PROTO_DEPENDENT_STATIC_CAST(TO, FROM, ...)                                    \
            boost::proto::detail::dependent_static_cast<TO>(FROM, __VA_ARGS__)

        ////////////////////////////////////////////////////////////////////////////////////
        // constructors TODO: add noexcept clauses
        // This will no longer be needed once clang implements inheriting constructors
        #define BOOST_PROTO_INHERIT_EXPR_CTORS(EXPR, BASE)                                          \
            using typename BASE::proto_tag_type;                                                    \
            using typename BASE::proto_args_type;                                                   \
                                                                                                    \
            constexpr EXPR(proto_tag_type tag, proto_args_type args)                                \
              : BASE(static_cast<proto_tag_type &&>(tag), static_cast<proto_args_type &&>(args))    \
            {}                                                                                      \
                                                                                                    \
            constexpr explicit EXPR(proto_args_type args)                                           \
              : EXPR(proto_tag_type(), static_cast<proto_args_type &&>(args))                       \
            {}                                                                                      \
                                                                                                    \
            template<typename A                                                                     \
              , BOOST_PROTO_ENABLE_IF(                                                              \
                    proto_args_type::proto_size::value == 1 && !BOOST_PROTO_IS_SAME_EXPR(EXPR &, A) \
                )                                                                                   \
            >                                                                                       \
            constexpr EXPR(proto_tag_type tag, A &&a)                                               \
              : BASE(static_cast<proto_tag_type &&>(tag), static_cast<A &&>(a))                     \
            {}                                                                                      \
                                                                                                    \
            template<typename A                                                                     \
              , BOOST_PROTO_ENABLE_IF(                                                              \
                    proto_args_type::proto_size::value == 1 && !BOOST_PROTO_IS_SAME_EXPR(EXPR &, A) \
                )                                                                                   \
            >                                                                                       \
            explicit constexpr EXPR(A &&a)                                                          \
              : EXPR(proto_tag_type(), static_cast<A &&>(a))                                        \
            {}                                                                                      \
                                                                                                    \
            template<typename A, typename B, typename ...C                                          \
              , BOOST_PROTO_ENABLE_IF(proto_args_type::proto_size::value == sizeof...(C) + 2)       \
            >                                                                                       \
            constexpr EXPR(proto_tag_type tag, A &&a, B &&b, C &&... c)                             \
              : BASE(                                                                               \
                    static_cast<proto_tag_type &&>(tag)                                             \
                  , static_cast<A &&>(a), static_cast<B &&>(b), static_cast<C &&>(c)...             \
                )                                                                                   \
            {}                                                                                      \
                                                                                                    \
            template<typename A, typename B, typename ...C                                          \
              , BOOST_PROTO_ENABLE_IF(proto_args_type::proto_size::value == sizeof...(C) + 2)>      \
            constexpr EXPR(A &&a, B &&b, C &&... c)                                                 \
              : EXPR(                                                                               \
                    proto_tag_type()                                                                \
                  , static_cast<A &&>(a), static_cast<B &&>(b), static_cast<C &&>(c)...             \
                )                                                                                   \
            {}                                                                                      \
                                                                                                    \
            typedef int boost_proto_inherited_ctors ## __LINE__                                     \
            /**/

        namespace exprs
        {
            ////////////////////////////////////////////////////////////////////////////////////////
            // struct expr_assign
            template<typename Expr, typename Domain>
            struct expr_assign
            {
                BOOST_PROTO_REGULAR_TRIVIAL_CLASS(expr_assign);

                ////////////////////////////////////////////////////////////////////////////////////
                // operator=
                template<typename U, BOOST_PROTO_ENABLE_IF(!BOOST_PROTO_IS_SAME_EXPR(expr_assign, U))>
                auto operator=(U && u) &
                BOOST_PROTO_RETURN(
                    boost::proto::domains::make_expr<Domain>(
                        boost::proto::tag::assign()
                      , BOOST_PROTO_DEPENDENT_STATIC_CAST(Expr &, *this, u)
                      , static_cast<U &&>(u)
                    )
                )

                template<typename U, BOOST_PROTO_ENABLE_IF(!BOOST_PROTO_IS_SAME_EXPR(expr_assign, U))>
                auto operator=(U && u) const &
                BOOST_PROTO_RETURN(
                    boost::proto::domains::make_expr<Domain>(
                        boost::proto::tag::assign()
                      , BOOST_PROTO_DEPENDENT_STATIC_CAST(Expr const &, *this, u)
                      , static_cast<U &&>(u)
                    )
                )

                template<typename U, BOOST_PROTO_ENABLE_IF(!BOOST_PROTO_IS_SAME_EXPR(expr_assign, U))>
                auto operator=(U && u) &&
                BOOST_PROTO_RETURN(
                    boost::proto::domains::make_expr<Domain>(
                        boost::proto::tag::assign()
                      , BOOST_PROTO_DEPENDENT_STATIC_CAST(Expr &&, static_cast<expr_assign &&>(*this), u)
                      , static_cast<U &&>(u)
                    )
                )

                template<typename U, BOOST_PROTO_ENABLE_IF(!BOOST_PROTO_IS_SAME_EXPR(expr_assign, U))>
                void operator=(U && u) const && = delete;
            };

            ////////////////////////////////////////////////////////////////////////////////////////
            // struct expr_subscript
            template<typename Expr, typename Domain>
            struct expr_subscript
            {
                BOOST_PROTO_REGULAR_TRIVIAL_CLASS(expr_subscript);

                ////////////////////////////////////////////////////////////////////////////////////
                // operator[]
                template<typename U>
                auto operator[](U && u) &
                BOOST_PROTO_RETURN(
                    boost::proto::domains::make_expr<Domain>(
                        boost::proto::tag::subscript()
                      , BOOST_PROTO_DEPENDENT_STATIC_CAST(Expr &, *this, u)
                      , static_cast<U &&>(u)
                    )
                )

                template<typename U>
                auto operator[](U && u) const &
                BOOST_PROTO_RETURN(
                    boost::proto::domains::make_expr<Domain>(
                        boost::proto::tag::subscript()
                      , BOOST_PROTO_DEPENDENT_STATIC_CAST(Expr const &, *this, u)
                      , static_cast<U &&>(u)
                    )
                )

                template<typename U>
                auto operator[](U && u) &&
                BOOST_PROTO_RETURN(
                    boost::proto::domains::make_expr<Domain>(
                        boost::proto::tag::subscript()
                      , BOOST_PROTO_DEPENDENT_STATIC_CAST(Expr &&, static_cast<expr_subscript &&>(*this), u)
                      , static_cast<U &&>(u)
                    )
                )

                template<typename U>
                void operator[](U && u) const && = delete;
            };

            ////////////////////////////////////////////////////////////////////////////////////////
            // struct expr_function
            template<typename Expr, typename Domain>
            struct expr_function
            {
                BOOST_PROTO_REGULAR_TRIVIAL_CLASS(expr_function);

                ////////////////////////////////////////////////////////////////////////////////////
                // operator()
                template<typename ...U>
                auto operator()(U &&... u) &
                BOOST_PROTO_RETURN(
                    boost::proto::domains::make_expr<Domain>(
                        boost::proto::tag::function()
                      , BOOST_PROTO_DEPENDENT_STATIC_CAST(Expr &, *this, u...)
                      , static_cast<U &&>(u)...
                    )
                )

                template<typename ...U>
                auto operator()(U &&... u) const &
                BOOST_PROTO_RETURN(
                    boost::proto::domains::make_expr<Domain>(
                        boost::proto::tag::function()
                      , BOOST_PROTO_DEPENDENT_STATIC_CAST(Expr const &, *this, u...)
                      , static_cast<U &&>(u)...
                    )
                )

                template<typename ...U>
                auto operator()(U &&... u) &&
                BOOST_PROTO_RETURN(
                    boost::proto::domains::make_expr<Domain>(
                        boost::proto::tag::function()
                      , BOOST_PROTO_DEPENDENT_STATIC_CAST(Expr &&, static_cast<expr_function &&>(*this), u...)
                      , static_cast<U &&>(u)...
                    )
                )

                template<typename ...U>
                void operator()(U &&... u) const && = delete;
            };

            ////////////////////////////////////////////////////////////////////////////////////////
            // struct basic_expr
            template<typename Tag, typename Args, typename Domain>
            struct basic_expr
              : Tag, Args
            {
                ////////////////////////////////////////////////////////////////////////////////////
                // Check constraints
                static_assert(
                    !Tag::proto_is_terminal::value || Args::proto_size::value <= 1
                  , "terminals can have only 1 value"
                );

                BOOST_PROTO_REGULAR_TRIVIAL_CLASS(basic_expr);

                ////////////////////////////////////////////////////////////////////////////////////
                // typedefs
                typedef Tag     proto_tag_type;
                typedef Args    proto_args_type;
                typedef Domain  proto_domain_type;
                typedef
                    std::integral_constant<
                        std::size_t
                      , Tag::proto_is_terminal::value ? 0 : Args::proto_size::value
                    >
                proto_arity;

                // TODO: add noexcept to ctors

                ////////////////////////////////////////////////////////////////////////////////////
                // constructors
                constexpr basic_expr(Tag tag, Args args)
                  : Tag(static_cast<Tag &&>(tag))
                  , Args(static_cast<Args &&>(args))
                {}

                explicit constexpr basic_expr(Args args)
                  : basic_expr(Tag(), static_cast<Args &&>(args))
                {}

                template<typename A
                  , BOOST_PROTO_ENABLE_IF(
                        Args::proto_size::value == 1 && !BOOST_PROTO_IS_SAME_EXPR(basic_expr &, A)
                    )
                >
                constexpr basic_expr(Tag tag, A &&a)
                  : Tag(static_cast<Tag &&>(tag))
                  , Args(static_cast<A &&>(a))
                {}

                template<
                    typename A
                  , BOOST_PROTO_ENABLE_IF(
                        Args::proto_size::value == 1 && !BOOST_PROTO_IS_SAME_EXPR(basic_expr &, A)
                    )
                >
                explicit constexpr basic_expr(A &&a)
                  : basic_expr(Tag(), static_cast<A &&>(a))
                {}

                template<typename A, typename B, typename ...C
                  , BOOST_PROTO_ENABLE_IF(sizeof...(C) + 2 == Args::proto_size::value)
                >
                constexpr basic_expr(Tag tag, A &&a, B &&b, C &&... c)
                  : Tag(static_cast<Tag &&>(tag))
                  , Args(static_cast<A &&>(a), static_cast<B &&>(b), static_cast<C &&>(c)...)
                {}

                template<typename A, typename B, typename ...C
                  , BOOST_PROTO_ENABLE_IF(sizeof...(C) + 2 == Args::proto_size::value)
                >
                constexpr basic_expr(A &&a, B &&b, C &&... c)
                  : basic_expr(
                        Tag()
                      , static_cast<A &&>(a), static_cast<B &&>(b), static_cast<C &&>(c)...
                    )
                {}

                ////////////////////////////////////////////////////////////////////////////////////
                // accessors
                Tag proto_tag() const
                {
                    return *this;
                }

                Args & proto_args() &
                {
                    return *this;
                }

                Args const & proto_args() const &
                {
                    return *this;
                }

                Args && proto_args() &&
                {
                    return static_cast<Args &&>(*this);
                }

                Domain proto_domain()
                {
                    return Domain();
                }
            };

            ////////////////////////////////////////////////////////////////////////////////////////
            // struct expr
            template<typename Tag, typename Args, typename Domain>
            struct expr
              : basic_expr<Tag, Args, Domain>
              , expr_assign<expr<Tag, Args, Domain>, Domain>
              , expr_subscript<expr<Tag, Args, Domain>, Domain>
              , expr_function<expr<Tag, Args, Domain>, Domain>
            {
                BOOST_PROTO_REGULAR_TRIVIAL_CLASS(expr);

                ////////////////////////////////////////////////////////////////////////////////////
                // constructors
                typedef basic_expr<Tag, Args, Domain> proto_base_expr;
                BOOST_PROTO_INHERIT_EXPR_CTORS(expr, proto_base_expr);

                ////////////////////////////////////////////////////////////////////////////////////
                // operator=
                using expr_assign<expr, Domain>::operator=;
            };
        }
    }
}

#endif
