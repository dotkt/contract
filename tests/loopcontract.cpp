
// Copyright Alexei Zakharov, 2013.
// Copyright niXman (i dot nixman dog gmail dot com) 2016.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <contract/contract.hpp>

#include "contract_error.hpp"

#include <boost/test/unit_test.hpp>

void loop_invariant_success() {
    for (int i = 0; i != 10; ++i)
    {
        CONTRACT(loop)
        {
            INVARIANT(i >= 0);
            INVARIANT(i < 10);
        };
    }
}

void loop_invariant_failure() {
    for (int i = 0; i != 10; ++i)
    {
        CONTRACT(loop)
        {
            INVARIANT((i & 1) == 0);
        };
    }
}

void ignore_pre_postcondition() {
    for (int i = 0; i != 10; ++i)
    {
        CONTRACT(loop)
        {
            PRECONDITION((i & 1) == 0);
            POSTCONDITION((i & 1) != 0);
        };
    }
}

BOOST_AUTO_TEST_CASE(loop_invariant) {
    test::contract_handler_frame cframe;

    // expect loop invariant to pass
    BOOST_CHECK_NO_THROW(loop_invariant_success());

    // expect loop invariant to fail
    BOOST_CHECK_THROW(loop_invariant_failure(), test::contract_error);

    // skip precondition and postcondition inside loop contract
    BOOST_CHECK_NO_THROW(ignore_pre_postcondition());
}
