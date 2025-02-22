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

#include <eos/utils/expression-fwd.hh>
#include <eos/utils/expression-observable.hh>
#include <eos/utils/expression-parser-impl.hh>
#include <eos/utils/expression-visitors.cc>

#include <set>

namespace eos
{
    using eos::exp::Expression;

    ExpressionObservable::ExpressionObservable(const QualifiedName & name,
            const Parameters & parameters,
            const Kinematics & kinematics,
            const Options & options,
            const Expression & expression) :
        _name(name),
        _parameters(parameters),
        _kinematics(kinematics),
        _options(options)
    {
        if (expression.empty())
        {
            throw InternalError("Empty expression encountered in ExpressionObservable!");
        }

        exp::ExpressionMaker maker(parameters, kinematics, options, this);
        _expression = expression.accept_returning<Expression>(maker);
    }

    double
    ExpressionObservable::evaluate() const
    {
        if (_expression.empty())
        {
            throw InternalError("Empty expression encountered in ExpressionObservable::evaluate!");
        }

        exp::ExpressionEvaluator evaluator;

        return _expression.accept_returning<double>(evaluator);
    }


    ObservablePtr
    ExpressionObservable::clone() const
    {
        auto parameters = _parameters.clone();
        auto kinematics = _kinematics.clone();

        exp::ExpressionCloner cloner(parameters, kinematics, _options);

        return ObservablePtr(new ExpressionObservable(_name, parameters, kinematics, _options, _expression.accept_returning<Expression>(cloner)));
    }

    ObservablePtr
    ExpressionObservable::clone(const Parameters & parameters) const
    {
        auto kinematics = _kinematics.clone();

        exp::ExpressionCloner cloner(parameters, kinematics, _options);

        return ObservablePtr(new ExpressionObservable(_name, parameters, kinematics, _options, _expression.accept_returning<Expression>(cloner)));
    }


    ExpressionObservableEntry::ExpressionObservableEntry(const QualifiedName & name, const std::string & latex,
            const Unit & unit,
            const Expression & expression,
            const Options & forced_options) :
        _name(name),
        _latex(latex),
        _unit(unit),
        _expression(expression),
        _forced_options(forced_options)
    {
        if (_expression.empty())
        {
            throw InternalError("Empty expression encountered in ExpressionObservableEntry!");
        }

        // Read kinematic variables
        exp::ExpressionKinematicReader kinematic_reader;

        const std::set<std::string> & kinematic_set = expression.accept_returning<std::set<std::string>>(kinematic_reader);

        _kinematics_names.assign(kinematic_set.begin(), kinematic_set.end());
    }

    ObservableEntry::KinematicVariableIterator
    ExpressionObservableEntry::begin_kinematic_variables() const
    {
        return _kinematics_names.data();
    }

    ObservableEntry::KinematicVariableIterator
    ExpressionObservableEntry::end_kinematic_variables() const
    {
        return _kinematics_names.data() + _kinematics_names.size();
    }

    ObservablePtr
    ExpressionObservableEntry::make(const Parameters & parameters, const Kinematics & kinematics, const Options & options) const
    {
        if (_expression.empty())
        {
            throw InternalError("Empty expression encountered in ExpressionObservableEntry::make!");
        }

        return ObservablePtr(new ExpressionObservable(_name, parameters, kinematics, options + _forced_options, _expression));
    }

    std::ostream &
    ExpressionObservableEntry::insert(std::ostream & os) const
    {
        os << "    type: expression observable" << std::endl;

        return os;
    }

}
