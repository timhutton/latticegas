/*
    Lattice Gas Explorer
    Copyright (C) 2008-2009 Tim J. Hutton <tim.hutton@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SquareGridLatticeGas.h"

class HPPLatticeGas : public SquareGridLatticeGas
{
    public: // typedefs

        enum HPP_type { Diagonal, HorizontalVertical };

    public: // functions

        HPPLatticeGas(HPP_type type);

        void UpdateGas(); // override

        RealPoint GetAverageInputFlowVelocityPerParticle() const; // override
        float GetAverageInputNumParticlesPerCell() const; // override

    protected: // functions

        static state PermuteMaintainingMomentum(state c);

        int GetNumGasParticlesAt(int x,int y) const; // override
        int GetMaxNumGasParticlesAt(int x,int y) const; // override

        RealPoint GetVelocityAt(int x,int y) const; // override
        wxColour GetColour(int x,int y) const; // override
        string GetReport(state s) const; // override

        void InsertRandomFlow(int x,int y); // override
        void InsertRandomBackwardFlow(int x,int y); // override
        void InsertRandomParticle(int x,int y); // override

    protected: // data

        HPP_type hpp_type;

        static const int N_DIRS = 4; // how many directions do the particles travel in?
        int DIR[N_DIRS][2]; // what direction is each travelling in?
        static state opposite_dir(state s) { return (s+N_DIRS/2)%N_DIRS; }
};
