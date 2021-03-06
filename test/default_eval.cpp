#define BOOST_YAP_CONVERSION_OPERATOR_TEMPLATE 1
#include <boost/yap/expression.hpp>

#include <gtest/gtest.h>


template <typename T>
using term = boost::yap::terminal<boost::yap::expression, T>;

template <typename T>
using ref = boost::yap::expression_ref<boost::yap::expression, T>;

namespace yap = boost::yap;
namespace bh = boost::hana;


TEST(default_eval, default_eval)
{
    term<double> unity{1.0};
    int i_ = 42;
    term<int &&> i{std::move(i_)};
    yap::expression<
        yap::expr_kind::minus,
        bh::tuple<
            ref<term<double> &>,
            term<int &&>
        >
    > expr = unity - std::move(i);
    yap::expression<
        yap::expr_kind::plus,
        bh::tuple<
            ref<term<double> &>,
            yap::expression<
                yap::expr_kind::minus,
                bh::tuple<
                    ref<term<double> &>,
                    term<int &&>
                >
            >
        >
    > unevaluated_expr_1 = unity + std::move(expr);

    yap::expression<
        yap::expr_kind::plus,
        bh::tuple<
            ref<term<double> &>,
            ref<term<double> &>
        >
    > unevaluated_expr_2 = unity + unity;

    term<double> const const_unity{1.0};
    yap::expression<
        yap::expr_kind::plus,
        bh::tuple<
            ref<term<double> &>,
            ref<term<double> const &>
        >
    > unevaluated_expr_3 = unity + const_unity;

    {
        double result = unity;
        EXPECT_EQ(result, 1);
    }

    {
        double result = expr;
        EXPECT_EQ(result, -41);
    }

    {
        double result = unevaluated_expr_1;
        EXPECT_EQ(result, -40);
    }

    {
        double result = unevaluated_expr_2;
        EXPECT_EQ(result, 2);
    }

    {
        double result = unevaluated_expr_3;
        EXPECT_EQ(result, 2);
    }

    {
        double result = evaluate(unity, 5, 6, 7);
        EXPECT_EQ(result, 1);
    }

    {
        double result = evaluate(expr);
        EXPECT_EQ(result, -41);
    }

    {
        double result = evaluate(unevaluated_expr_1, std::string("15"));
        EXPECT_EQ(result, -40);
    }

    {
        double result = evaluate(unevaluated_expr_2, std::string("15"));
        EXPECT_EQ(result, 2);
    }

    {
        double result = evaluate(unevaluated_expr_3, std::string("15"));
        EXPECT_EQ(result, 2);
    }
}
