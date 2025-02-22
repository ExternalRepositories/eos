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

#ifndef EOS_GUARD_SRC_UTILS_EXPRESSION_OBSERVABLE_HH
#define EOS_GUARD_SRC_UTILS_EXPRESSION_OBSERVABLE_HH 1

#include <eos/observable-impl.hh>
#include <eos/utils/expression-fwd.hh>
#include <eos/utils/expression-parser-impl.hh>
#include <eos/utils/wrapped_forward_iterator-impl.hh>


namespace eos
{
    using eos::exp::Expression;

    class ExpressionObservable :
        public Observable
    {
        private:
            QualifiedName _name;

            Parameters _parameters;

            Kinematics _kinematics;

            Options _options;

            Expression _expression;

        public:
            ExpressionObservable(const QualifiedName & name,
                    const Parameters & parameters,
                    const Kinematics & kinematics,
                    const Options & options,
                    const Expression & expression);

            ~ExpressionObservable()
            {
            }

            virtual double evaluate() const;
            virtual ObservablePtr clone() const;
            virtual ObservablePtr clone(const Parameters & parameters) const;

            virtual const QualifiedName & name() const
            {
                return _name;
            }

            virtual Parameters parameters()
            {
                return _parameters;
            };

            virtual Kinematics kinematics()
            {
                return _kinematics;
            };

            virtual Options options()
            {
                return _options;
            }
    };

    class ExpressionObservableEntry :
        public ObservableEntry
    {
        private:
            QualifiedName _name;

            std::string _latex;

            const Unit & _unit;

            const Expression _expression;

            std::vector<std::string> _kinematics_names;

            Options _forced_options;

        public:
            ExpressionObservableEntry(const QualifiedName & name, const std::string & latex,
                    const Unit & unit,
                    const Expression & expression,
                    const Options & forced_options);

            ~ExpressionObservableEntry()
            {
            }

            virtual ObservableEntry::KinematicVariableIterator begin_kinematic_variables() const;
            virtual ObservableEntry::KinematicVariableIterator end_kinematic_variables() const;
            virtual ObservablePtr make(const Parameters & parameters, const Kinematics & kinematics, const Options & options) const;
            virtual std::ostream & insert(std::ostream & os) const;

            virtual const QualifiedName & name() const
            {
                return _name;
            }

            virtual const std::string & latex() const
            {
                return _latex;
            }

            virtual const Unit & unit() const
            {
                return _unit;
            }
    };
}

#endif
