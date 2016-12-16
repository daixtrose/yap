#ifndef BOOST_YAP_PRINT_HPP_INCLUDED
#define BOOST_YAP_PRINT_HPP_INCLUDED

#include <boost/yap/expression_fwd.hpp>

#include <boost/hana/for_each.hpp>
#include <boost/type_index.hpp>
#include <iostream>


namespace boost { namespace yap {

    namespace detail {

        inline std::ostream & print_kind (std::ostream & os, expr_kind kind)
        { return os << op_string(kind); }

        template <typename T, typename = hana::when_valid<>>
        struct printer
        {
            std::ostream & operator() (std::ostream & os, T const &)
            { return os << "<<unprintable-value>>"; }
        };

        template <typename T>
        struct printer<
            T,
            hana::when_valid<decltype(
                std::declval<std::ostream &>() << std::declval<T const &>()
            )>
        >
        {
            std::ostream & operator() (std::ostream & os, T const & x)
            { return os << x; }
        };

        template <typename T>
        inline std::ostream & print_value (std::ostream & os, T const & x)
        { return printer<T>{}(os, x); }

        template <long long I>
        inline std::ostream & print_value (std::ostream & os, hana::llong<I>)
        { return os << I << "_p"; }

        template <typename T>
        std::ostream & print_type (std::ostream & os, hana::tuple<T> const &)
        {
            os << typeindex::type_id<T>().pretty_name();
            if (std::is_const<T>{})
                os << " const";
            if (std::is_volatile<T>{})
                os << " volatile";
            if (std::is_lvalue_reference<T>{})
                os << " &";
            if (std::is_rvalue_reference<T>{})
                os << " &&";
            return os;
        }

        inline bool is_const_expr_ref (...) { return false; }
        template <typename T, template <expr_kind, class> class expr_template>
        bool is_const_expr_ref (expr_template<expr_kind::expr_ref, hana::tuple<T const *>> const &)
        { return true; }

        template <expr_kind Kind>
        struct print_impl
        {
            template <typename Expr>
            std::ostream & operator() (
                std::ostream & os,
                Expr const & expr,
                int indent,
                char const * indent_str,
                bool is_ref = false,
                bool is_const_ref = false)
            {
                for (int i = 0; i < indent; ++i) {
                    os << indent_str;
                }

                os << "expr<";
                print_kind(os, Expr::kind);
                os << ">";
                if (is_const_ref)
                    os << " const &";
                else if (is_ref)
                    os << " &";
                os << "\n";
                hana::for_each(expr.elements, [&os, indent, indent_str](auto const & element) {
                    using element_type = decltype(element);
                    constexpr expr_kind kind = detail::remove_cv_ref_t<element_type>::kind;
                    print_impl<kind>{}(os, element, indent + 1, indent_str);
                });

                return os;
            }
        };

        template <>
        struct print_impl<expr_kind::expr_ref>
        {
            template <typename Expr>
            std::ostream & operator() (
                std::ostream & os,
                Expr const & expr,
                int indent,
                char const * indent_str,
                bool is_ref = false,
                bool is_const_ref = false)
            {
                using ref_type = decltype(::boost::yap::deref(expr));
                constexpr expr_kind ref_kind = detail::remove_cv_ref_t<ref_type>::kind;
                print_impl<ref_kind>{}(
                    os,
                    ::boost::yap::deref(expr),
                    indent,
                    indent_str,
                    true,
                    is_const_expr_ref(expr)
                );
                return os;
            }
        };

        template <>
        struct print_impl<expr_kind::terminal>
        {
            template <typename Expr>
            std::ostream & operator() (
                std::ostream & os,
                Expr const & expr,
                int indent,
                char const * indent_str,
                bool is_ref = false,
                bool is_const_ref = false)
            {
                for (int i = 0; i < indent; ++i) {
                    os << indent_str;
                }

                os << "term<";
                print_type(os, expr.elements);
                os << ">[=";
                print_value(os, ::boost::yap::value(expr));
                os << "]";
                if (is_const_ref)
                    os << " const &";
                else if (is_ref)
                    os << " &";
                os << "\n";

                return os;
            }
        };

    }

    /** Prints expression \a expr to stream \a os.  Returns \a os. */
    template <typename Expr>
    std::ostream & print (std::ostream & os, Expr const & expr)
    { return detail::print_impl<Expr::kind>{}(os, expr, 0, "    "); }

} }

#endif
