
// Copyright Alexei Zakharov, 2013.
// Copyright niXman (i dot nixman dog gmail dot com) 2016.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <contract/contract.hpp>

#include "contract_error.hpp"

#include <boost/test/unit_test.hpp>

namespace {

class throwing_ctor_t {};

class base_account
{
public:
    base_account(double rate)
        : interest_rate_(rate)
    {
        CONTRACT(ctor) {};
    }

    ~base_account()
    {
        interest_rate_ = -1;
    }

private:
    CONTRACT(class)
    {
        INVARIANT(interest_rate_ >= 0);
    };

protected:
    // derived class can change the state
    // and base class contract should catch violation
    double interest_rate_;
};

class account : public base_account
{
public:
    account(int bal, double rate)
        : base_account(rate)
        , balance_(-1)
    {
        CONTRACT(ctor) {};
        balance_ = bal;  // the class contract is checked on constructor exit
    }

    account(throwing_ctor_t)
        : base_account(1)
        , balance_(-1)
    {
        CONTRACT(ctor) {};
        throw test::non_contract_error{};
    }

    ~account() noexcept(false)  // only needed so the contract check can throw
                                // an exception; if the contract check aborts
                                // the destructor can and should be noexcept
    {
        CONTRACT(dtor) {};
        balance_ = -1;  // the class contract is checked on destructor entry
    }

    int balance() const
    {
        CONTRACT(mfun) {};
        return balance_;
    }

    void balance(int bal)
    {
        CONTRACT(mfun) {};
        balance_ = bal;  // the class contract is checked both on method
                         // entry and exit
    }

    double interest_rate() const
    {
        CONTRACT(mfun) {};
        return interest_rate_;
    }

    void interest_rate(double rate)
    {
        CONTRACT(mfun) {};
        interest_rate_ = rate;
    }

private:
    CONTRACT(derived)(base_account) { INVARIANT(balance_ > 0, "invariant"); };

private:
    int balance_;
};


class base_no_invariant {};
class base_no_invariant2 {};

// Derived class contract with a base class that doesn't have a class invariant
// contract.
class derived_with_invariant : public base_no_invariant
{
public:
    derived_with_invariant()
    {
        CONTRACT(ctor) {};
    }

private:
    CONTRACT(derived)(base_no_invariant)
    {
        INVARIANT(true);
    };
};


class base_with_invariant
{
private:
    CONTRACT(class)
    {
        INVARIANT(true);
    };
};

// Derived class with more than one base class.
class derived_with_many_bases : public derived_with_invariant
                              , public base_with_invariant
                              , public base_no_invariant2
{
private:
    CONTRACT(derived)(derived_with_invariant,
                      base_with_invariant,
                      base_no_invariant2)
    {
        INVARIANT(true);
    };
};

} // anon namespace

BOOST_AUTO_TEST_CASE(derived_contract_with_many_bases) {
    BOOST_CHECK_NO_THROW(derived_with_many_bases());
}

BOOST_AUTO_TEST_CASE(derived_contract_base_without_contract) {
    BOOST_CHECK_NO_THROW(derived_with_invariant());
}

BOOST_AUTO_TEST_CASE(derived_contract_in_ctor_dtor) {
    test::contract_handler_frame cframe;

    // expect class invariant to pass
    BOOST_CHECK_NO_THROW(account(10, 0.05));

    // expect class invariant to fail
    BOOST_CHECK_THROW(account(-2, 0.05), test::contract_error);
    BOOST_CHECK_THROW(account(10, -0.05), test::contract_error);

    // skip class invariant if constructor throws
    BOOST_CHECK_THROW(account(throwing_ctor_t{}), test::non_contract_error);
}

BOOST_AUTO_TEST_CASE(derived_contract_in_method) {
    test::contract_handler_frame cframe;

    bool caught_exception = false;

    try {
        // expect class invariant to pass
        account acc{10, 0.05};
        BOOST_CHECK_NO_THROW(acc.interest_rate(0.1));

        // once the invariant is violated, all methods start to throw
        // the contract error
        BOOST_CHECK_THROW(acc.interest_rate(-0.1), test::contract_error);
        BOOST_CHECK_THROW(acc.interest_rate(), test::contract_error);

        // verify that the contract is checked on both entry/exit of a method,
        // and the class invariant can't be restored once it's broken
        BOOST_CHECK_THROW(acc.interest_rate(0.07), test::contract_error);
        BOOST_CHECK_THROW(acc.interest_rate(), test::contract_error);

        // verify that the destructor also throws on contract violation
    } catch (test::contract_error & e) {
        caught_exception = true;
        BOOST_CHECK(e.type() == contract::type::invariant);
    } catch (...) {
        caught_exception = true;
        BOOST_FAIL("expected to catch test::contract_error");
    }

    BOOST_CHECK(caught_exception);
}
