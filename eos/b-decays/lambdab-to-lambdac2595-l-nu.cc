/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2017 Elena Graverini
 * Copyright (c) 2017-2018 Danny van Dyk
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

#include <eos/form-factors/form-factors.hh>
#include <eos/form-factors/baryonic.hh>
#include <eos/b-decays/lambdab-to-lambdac2595-l-nu.hh>
#include <eos/utils/integrate.hh>
#include <eos/utils/kinematic.hh>
#include <eos/utils/model.hh>
#include <eos/utils/power_of.hh>
#include <eos/utils/private_implementation_pattern-impl.hh>
#include <eos/utils/save.hh>

namespace eos
{
    template <>
    struct Implementation<LambdaBToLambdaC2595LeptonNeutrino>
    {
        std::shared_ptr<Model> model;

        std::shared_ptr<FormFactors<OneHalfPlusToOneHalfMinus>> form_factors;

        Parameters parameters;

        UsedParameter m_LambdaB;

        UsedParameter tau_LambdaB;

        UsedParameter m_LambdaC2595;

        UsedParameter m_l;

        UsedParameter g_fermi;

        UsedParameter hbar;

        Implementation(const Parameters & p, const Options & o, ParameterUser & u) :
            model(Model::make(o.get("model", "SM"), p, o)),
            parameters(p),
            m_LambdaB(p["mass::Lambda_b"], u),
            tau_LambdaB(p["life_time::Lambda_b"], u),
            m_LambdaC2595(p["mass::Lambda_c(2595)"], u),
            m_l(p["mass::" + o.get("l", "mu")], u),
            g_fermi(p["G_Fermi"], u),
            hbar(p["QM::hbar"], u)
        {
            form_factors = FormFactorFactory<OneHalfPlusToOneHalfMinus>::create("Lambda_b->Lambda_c(2595)@" + o.get("form-factors","HQET"), p);

            if (! form_factors.get())
                throw InternalError("Form factors not found!");

            u.uses(*form_factors);
            u.uses(*model);
        }

        double F12T(const double & s) const { return form_factors->f_time_v(s); };
        double F120(const double & s) const { return form_factors->f_long_v(s); };
        double F12P(const double & s) const { return form_factors->f_perp_v(s); };
        double G12T(const double & s) const { return form_factors->f_time_a(s); };
        double G120(const double & s) const { return form_factors->f_long_a(s); };
        double G12P(const double & s) const { return form_factors->f_perp_a(s); };
        double s_plus(const double & s) const { return pow((m_LambdaB + m_LambdaC2595), 2) - s; };
        double s_minus(const double & s) const { return pow((m_LambdaB - m_LambdaC2595), 2) - s; };

        // [BBGIOvD] parametrization for the differential decay width
        double a_l(const double & s) const
        {
            double val = pow(F12T(s), 2.) * (pow(m_l, 2.) / s) * pow((m_LambdaB - m_LambdaC2595), 2.);
            val += (pow(F120(s), 2.) * pow((m_LambdaB + m_LambdaC2595), 2.) + (pow(F12P(s), 2.)) * (pow(m_l, 2.) + s));
            val += pow(G12T(s), 2.) * (pow(m_l, 2.) / s) * pow((m_LambdaB + m_LambdaC2595), 2.);
            val += (pow(G120(s), 2.) * pow((m_LambdaB - m_LambdaC2595), 2.) + (pow(G12P(s), 2.)) * (pow(m_l, 2.) + s));
            return val / 2.0;
        }

        double b_l(const double & s) const
        {
            double val = 2. * (F12T(s) * F120(s) + G12T(s) * G120(s)) * pow(m_l, 2.) / s;
            val *= (pow(m_LambdaB, 2.) - pow(m_LambdaC2595, 2.));
            val += (-4.) * s * (F12P(s) * G12P(s));
            return val / 2.0;
        }

        double c_l(const double & s) const
        {
            double val = pow(F120(s), 2.) * pow(m_LambdaB + m_LambdaC2595, 2.);
            val += (-1.) * s * pow(F12P(s), 2.);
            val += pow(G120(s), 2.) * pow(m_LambdaB - m_LambdaC2595, 2.);
            val += (-1.) * s * pow(G12P(s), 2.);
            val *= (-1.) * (1. - pow(m_l, 2.) / s);
            return val / 2.0;
        }

        double gamma_0(const double & s) const
        {
            double val = pow(g_fermi, 2.) * sqrt(s_plus(s) * s_minus(s));
            val *= m_LambdaB * m_LambdaC2595;
            val *= (1. / (96. * pow(M_PI * m_LambdaB, 3.)));
            val *= pow((1. - pow(m_l, 2.) / s), 2.);
            return val;
        }

        inline double lambda(const double & s) const
        {
            return s_plus(s) * s_minus(s);
        }

        // normalized to V_cb = 1
        double normalized_differential_decay_width(const double & s) const
        {
            if ((s < m_l * m_l) || (lambda(s) < 0.0))
            {
                return 0.0;
            }

            return 2. * gamma_0(s) * (a_l(s) + c_l(s) / 3.);
        }

        double normalized_differential_forward_backward_asymmetry(const double & s) const
        {
            if ((s < m_l * m_l) || (lambda(s) < 0.0))
            {
                return 0.0;
            }

            // in order to obtain the q^2-integrated A_FB later on, we require
            // this to be normalized to Gamma_0.
            return gamma_0(s) * b_l(s);
        }

        double normalized_double_differential_decay_width(const double & s, const double & z) const
        {
            if ((s < m_l * m_l) || (lambda(s) < 0.0))
            {
                return 0.0;
            }

            return gamma_0(s) * (a_l(s) + b_l(s) * z + c_l(s) * pow(z, 2.));
        }

        double differential_decay_width(const double & s) const
        {
            return normalized_differential_decay_width(s) * std::norm(model->ckm_cb());
        }

        double double_differential_decay_width(const double & s, const double & theta_l) const
        {
            return normalized_double_differential_decay_width(s, theta_l) * std::norm(model->ckm_cb());
        }

        double differential_branching_ratio(const double & s) const
        {
            return differential_decay_width(s) * tau_LambdaB / hbar;
        }

        double double_differential_branching_ratio(const double & s, const double & theta_l) const
        {
            return double_differential_decay_width(s, theta_l) * tau_LambdaB / hbar;
        }

        double integrated_branching_ratio(const double & s_min, const double & s_max) const
        {
            std::function<double (const double &)> f = std::bind(&Implementation<LambdaBToLambdaC2595LeptonNeutrino>::differential_branching_ratio,
                    *this, std::placeholders::_1);

            return integrate<GSL::QAGS>(f, s_min, s_max);
        }

        double integrated_forward_backward_asymmetry(const double & s_min, const double & s_max) const
        {
            std::function<double (const double &)> numerator   = std::bind(&Implementation<LambdaBToLambdaC2595LeptonNeutrino>::normalized_differential_forward_backward_asymmetry,
                    *this, std::placeholders::_1);
            std::function<double (const double &)> denominator = std::bind(&Implementation<LambdaBToLambdaC2595LeptonNeutrino>::normalized_differential_decay_width,
                    *this, std::placeholders::_1);

            const double inum   = integrate<GSL::QAGS>(numerator,   s_min, s_max);
            const double idenom = integrate<GSL::QAGS>(denominator, s_min, s_max);

            return inum / idenom;
        }
    };

    LambdaBToLambdaC2595LeptonNeutrino::LambdaBToLambdaC2595LeptonNeutrino(const Parameters & parameters, const Options & options) :
        PrivateImplementationPattern<LambdaBToLambdaC2595LeptonNeutrino>(new Implementation<LambdaBToLambdaC2595LeptonNeutrino>(parameters, options, *this))
    {
    }

    LambdaBToLambdaC2595LeptonNeutrino::~LambdaBToLambdaC2595LeptonNeutrino()
    {
    }

    double
    LambdaBToLambdaC2595LeptonNeutrino::a_l(const double & s) const
    {
        return _imp->a_l(s);
    }

    double
    LambdaBToLambdaC2595LeptonNeutrino::b_l(const double & s) const
    {
        return _imp->b_l(s);
    }

    double
    LambdaBToLambdaC2595LeptonNeutrino::c_l(const double & s) const
    {
        return _imp->c_l(s);
    }

    double
    LambdaBToLambdaC2595LeptonNeutrino::differential_branching_ratio(const double & s) const
    {
        return _imp->differential_branching_ratio(s);
    }

    double
    LambdaBToLambdaC2595LeptonNeutrino::double_differential_branching_ratio(const double & s, const double & theta_l) const
    {
        return _imp->double_differential_branching_ratio(s, theta_l);
    }

    double
    LambdaBToLambdaC2595LeptonNeutrino::integrated_branching_ratio(const double & s_min, const double & s_max) const
    {
        return _imp->integrated_branching_ratio(s_min, s_max);
    }

    double
    LambdaBToLambdaC2595LeptonNeutrino::integrated_forward_backward_asymmetry(const double & s_min, const double & s_max) const
    {
        return _imp->integrated_forward_backward_asymmetry(s_min, s_max);
    }

    double
    LambdaBToLambdaC2595LeptonNeutrino::normalized_integrated_branching_ratio(const double & s_min, const double & s_max) const
    {
        const double abs_s_min = pow(_imp->m_l, 2);
        const double abs_s_max = pow(_imp->m_LambdaB - _imp->m_LambdaC2595, 2);

        return _imp->integrated_branching_ratio(s_min, s_max) / _imp->integrated_branching_ratio(abs_s_min, abs_s_max);
    }

    const std::string
    LambdaBToLambdaC2595LeptonNeutrino::description = "\
The decay Lambda_b -> Lambda_c(2595) l nu, where l=e,mu,tau is a lepton.";

    const std::string
    LambdaBToLambdaC2595LeptonNeutrino::kinematics_description_s = "\
The invariant mass of the l-nubar pair in GeV^2.";

    const std::string
    LambdaBToLambdaC2595LeptonNeutrino::kinematics_description_c_theta_l = "\
The cosine of the helicity angle between the direction of flight of the muon and of the Lambda_c(2595) in the l-nubar rest frame.";

    const std::set<ReferenceName>
    LambdaBToLambdaC2595LeptonNeutrino::references
    {
    };
}
