//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2010  Sebastian Mach (*1983)
// * mail: phresnel/at/gmail/dot/com
// * http://phresnel.org
// * http://picogen.org
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef ARRAY0_HH_INCLUDED_20100309
#define ARRAY0_HH_INCLUDED_20100309

namespace kallisto {
        //-- KALLISTO_ARRAY_IMPLEMENT_ASSIGNMENT ------------------------------
        #define KALLISTO_ARRAY_IMPLEMENT_ASSIGNMENT(name,op)                  \
                                                                              \
        template<typename T_, unsigned int N_, typename RULES_, typename REP_>\
        typename traits::enable_if<                                           \
                has_member_##name##_a<RULES_>,                                \
                array<T_,N_,RULES_,REP_> &                                    \
        >::type                                                               \
        operator op (array<T_,N_,RULES_,REP_> const &rhs) {                   \
                KALLISTO_ARRAY_FOREACHELEM(expr_rep[i] op rhs[i]);            \
                return *this;                                                 \
        }                                                                     \
                                                                              \
        template<typename T_>                                                 \
        typename traits::enable_if_c<                                         \
                has_member_##name##_s<RULES>::value                           \
                  && traits::same_type<T_,T>::value,                          \
                array &                                                       \
        >::type                                                               \
        operator op (T_ rhs) {                                                \
                KALLISTO_ARRAY_FOREACHELEM(expr_rep[i] op rhs);               \
                return *this;                                                 \
        }                                                                     \
                                                                              \
        template<typename T_, unsigned int N_, typename RULES_, typename REP_,\
                                                               typename REP2_>\
        typename traits::enable_if<                                           \
                has_member_##name##_a<RULES_>,                                \
                array<T_,N_,RULES_,REP_> &                                    \
        >::type                                                               \
        operator op (array<T_,N_,RULES_,REP2_> const &rhs) {                  \
                KALLISTO_ARRAY_FOREACHELEM(expr_rep[i] op rhs[i]);            \
                return *this;                                                 \
        }
        //---------------------------------------------------------------------

        template <typename T, unsigned int N, typename RULES=r_std<T>, typename REP=array_rep<T,N> >
        class array {
        public:
                enum { size_ = N };
                unsigned int size() const { return N; }

                enum noinit_ {noinit};
                array (noinit_) {}

                explicit array (T val = 0) {
                        for (unsigned int i=0; i<N; ++i)
                                expr_rep[i] = val;
                }

                array (REP const &expr_rep)
                : expr_rep (expr_rep)
                {}

                // It is near-impossible (de-factor-impossible?) to make
                // the copy-assignment operator enable-/disable-able.
                // 0) Every tweakable operator of array<> is made tweakable
                //    by means of traits::enable_if, which requires to postpone
                //    their definitions out of array<>, e.g. by making
                //    them non-members, or by templatizing them.
                // 1) operator= cannot be postponed per the holy law
                // 2) but we can templatize it
                // 3) but if we 2), then the compiler will generate
                //    a non-templatized copy-assignment, which will always
                //    be prefered over any member-templates
                // 4) we can write a private copy-assignment, but lookup
                //    will still find the private version, and compilation
                //    will fail for non-friends.
                // 5) a solution would be to forbid the rule-of-three, but I am
                //    not aware of any method that does not increase the
                //    conceptual size of array<> (const member, reference
                //    member)
                // 6) another solution would expose macros to client code(kthx)
                // 7) apart from 0)-6), would it make (much) sense at all
                //    to forbid copy-assignment? use cases anyone? so for now,
                //    let us just allow operator=(array), and forget about my
                //    hours of effort to make it tweakable :( ... :)
                // I hereby present, copy-assignment:
                void operator = (array const &rhs) {
                        KALLISTO_ARRAY_FOREACHELEM(expr_rep[i] = rhs[i]);
                        //return *this;
                }
                // ... as for 7), and because clients can still forbid any
                // other operation, we also don't make expr-assignment
                // tweakable.
                template<typename REP2>
                void
                operator = (array<T,N,RULES,REP2> const &rhs) {
                        KALLISTO_ARRAY_FOREACHELEM(expr_rep[i] = rhs[i]);
                        //return *this;
                }

                // But this should be tweakable: array = scalar
                template<typename T_>
                typename traits::enable_if_c<
                        has_member_assign_s<RULES>::value
                          && traits::same_type<T_,T>::value,
                        array &
                >::type
                operator = (T_ rhs) {
                        KALLISTO_ARRAY_FOREACHELEM(expr_rep[i] = rhs);
                        return *this;
                }

                KALLISTO_ARRAY_IMPLEMENT_ASSIGNMENT(augmented_plus, +=)
                KALLISTO_ARRAY_IMPLEMENT_ASSIGNMENT(augmented_minus, -=)
                KALLISTO_ARRAY_IMPLEMENT_ASSIGNMENT(augmented_multiplies,*=)
                KALLISTO_ARRAY_IMPLEMENT_ASSIGNMENT(augmented_divides,/=)
                KALLISTO_ARRAY_IMPLEMENT_ASSIGNMENT(augmented_mod,%=)
                KALLISTO_ARRAY_IMPLEMENT_ASSIGNMENT(augmented_shift_left,<<=)
                KALLISTO_ARRAY_IMPLEMENT_ASSIGNMENT(augmented_shift_right,>>=)
                KALLISTO_ARRAY_IMPLEMENT_ASSIGNMENT(augmented_bitwise_and,&=)
                KALLISTO_ARRAY_IMPLEMENT_ASSIGNMENT(augmented_bitwise_or,^=)
                KALLISTO_ARRAY_IMPLEMENT_ASSIGNMENT(augmented_bitwise_xor,|=)
                #undef KALLISTO_ARRAY_IMPLEMENT_ASSIGNMENT

                T operator [] (unsigned int i) const {
                        return expr_rep[i];
                }
                T & operator [] (unsigned int i) {
                        return expr_rep[i];
                }

                REP const &rep () const {
                        return expr_rep;
                }
                REP &rep () {
                        return expr_rep;
                }

        private:

                REP expr_rep;

                template <typename F_, typename T_, unsigned int N_, typename RULES_, typename REP_>
                friend inline void _array_invoke(array<T_,N_,RULES_,REP_> &arr);

                template <template <typename> class F_,
                        typename T_, unsigned int N_, typename RULES_, typename REP_
                >
                friend inline T_ _array_reduce(array<T_,N_,RULES_,REP_> const &arr);
        };
}


namespace kallisto {
        // Implement pre-/post- inc-/decrement. Prolly a bit too waste
        // (the basic idea was that array<> only needs to make a single friend)
        template <typename F, typename T, unsigned int N, typename RULES, typename REP>
        inline void _array_invoke(array<T,N,RULES,REP> &arr) {
                KALLISTO_ARRAY_FRIEND_FOREACHELEM(F::apply(arr.expr_rep[i]),N);
        }

        template <template <typename> class F,
                typename T, unsigned int N, typename RULES, typename REP
        >
        inline T _array_reduce(array<T,N,RULES,REP> const &arr) {
                F<T> ret;
                KALLISTO_ARRAY_FRIEND_FOREACHELEM(ret.apply(arr.expr_rep[i]),N);
                return ret.result();
        }
}


#endif // ARRAY0_HH_INCLUDED_20100309