/*
 * Copyright (c) 2021 Méril Reboud
 *
 * This file is part of the EOS project. EOS is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * EOS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <test/test.hh>
#include <eos/observable.hh>
#include <eos/utils/options.hh>

using namespace test;
using namespace eos;

class ObservableTest :
    public TestCase
{
    public:
        ObservableTest() :
            TestCase("observable_test")
        {
        }

        virtual void run() const
        {
            /* Test insertion of a new observable */
            {
                auto observables = Observables();

                TEST_CHECK_THROWS(
                    ParsingError,
                    observables.insert("mass::ratio", "m_r",  Options(), "<<mass::mu>> /* <<mass::tau>>")
                );

                observables.insert("mass::ratio", "m_r",  Options(), "<<mass::mu>> / <<mass::tau>>");

                Parameters p = Parameters::Defaults();
                p["mass::mu"]     =  0.105658;
                p["mass::tau"]    =  1.77682;
                Kinematics k;
                Options o;

                TEST_CHECK(observables["mass::ratio"]->make(p, k, o));
                TEST_CHECK_NEARLY_EQUAL(observables["mass::ratio"]->make(p, k, o)->evaluate(), 0.059464662, 10e-5);

                auto observable = Observable::make("mass::ratio", p, k, o);
                TEST_CHECK_NEARLY_EQUAL(observable->evaluate(), 0.059464662, 10e-5);
            }
        }

} observable_test;
