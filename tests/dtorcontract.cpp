
// Copyright Alexei Zakharov, 2013.
// Copyright niXman (i dot nixman dog gmail dot com) 2016.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <contract/contract.hpp>

#include "contract_error.hpp"

#include <boost/test/unit_test.hpp>

namespace
{

class account
{
public:
    account(int bal)
        : balance_(bal)
    {}

    ~account() noexcept(false)
    {
        CONTRACT(dtor)
        {
            PRECONDITION(balance_ > 0);
            POSTCONDITION(balance_ < 0);
        };

        balance_ -= 100;
    }

private:
    int balance_;
};

}

BOOST_AUTO_TEST_CASE(dtor_contract)
{
    test::contract_handler_frame cframe;

    // expect contract to pass
    BOOST_CHECK_NO_THROW(account(10));

    // expect precondition to fail
    test::check_throw_on_contract_violation([] { account(0); },
                                            contract::type::precondition);

    // expect postcondition to fail
    test::check_throw_on_contract_violation([] { account(200); },
                                            contract::type::postcondition);
}
