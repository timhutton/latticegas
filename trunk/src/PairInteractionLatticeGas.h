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

class PairInteractionLatticeGas : public SquareGridLatticeGas
{
    public:
  
        PairInteractionLatticeGas();
        
        void UpdateGas(); // override

        RealPoint GetAverageInputFlowVelocityPerParticle() const; // override

        float GetAverageInputNumParticlesPerCell() const; // override

    protected: // functions

        wxColour GetColour(int x,int y) const; // override
        RealPoint GetVelocityAt(int x,int y) const; // override
        string GetReport(state s) const; // override

        void ApplyHorizontalPairwiseInteraction(state &a,state &b);
        void ApplyVerticalPairwiseInteraction(state &a,state &b);

        static int swap23(int x);

        int GetNumGasParticlesAt(int x,int y) const; // override
        int GetMaxNumGasParticlesAt(int x,int y) const; // override

        void InsertRandomFlow(int x,int y); // override
        void InsertRandomBackwardFlow(int x,int y); // override
        void InsertRandomParticle(int x,int y); // override
        
        void RedrawImagesIfNeeded(); // override

    protected: // data

        // states: 0=empty, 1=rest particle, 2="x-mover", 3="y-mover", 4="diag-mover", 5=boundary
        state pi_table_horiz[5][5][2],pi_table_vert[5][5][2]; //  maps a,b onto pi_table[a][b][0],pi_table[a][b][1]

};

