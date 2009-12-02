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

#include "HexGridLatticeGas.h"

class FHPLatticeGas : public HexGridLatticeGas
{
    public:

        enum FHP_type { FHP_I, FHP_II, FHP_III, FHP_6 };

        FHPLatticeGas(FHP_type type);

        void UpdateGas(); // override

        RealPoint GetAverageInputFlowVelocityPerParticle() const; // override
        float GetAverageInputNumParticlesPerCell() const; // override

    protected: // functions

        void ResetGridForParticlesExample(); // override

        int GetNumGasParticlesAt(int x,int y) const; // override
        int GetMaxNumGasParticlesAt(int x,int y) const; // override

        RealPoint GetVelocityAt(int x,int y) const; // override
        wxColour GetColour(int x,int y) const; // override
        string GetReport(state s) const; // override

        void InsertRandomFlow(int x,int y); // override
        void InsertRandomParticle(int x,int y); // override

        void InitializeCollisionMap();
        void RandomizeCollisionMap();

        // a helper function to assemble vectors from values
        vector<state> Vec(int n, ...);

        // an internal check that we haven't typed something in wrongly
        void VerifyCollisionMap();

        // an internal check that a gas is collision-saturated
        void VerifyIsCollisionSaturated();

    protected: // data

        const FHP_type fhp_type;

        static const state E=1, SE=2, SW=4, W=8, NW=16, NE=32, REST=64;

        vector< vector<state> > collision_classes;
        state collision_map[129];
};

