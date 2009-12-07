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

#ifndef __HEXGRIDLATTICEGAS__
#define __HEXGRIDLATTICEGAS__

#include "BaseLatticeGas_drawable.h"

class HexGridLatticeGas : public BaseLatticeGas_drawable
{
    public: // functions

        HexGridLatticeGas();
        
        void RedrawImagesIfNeeded();

    protected:

        RealPoint GetVelocity(state s) const;

    protected: // data

        static const int N_DIRS = 6; // how many directions do the particles travel in?
        float DIR[N_DIRS][2]; // what direction is each travelling in? (clockwise from East)
        static int opposite_dir(int s) { return (s+N_DIRS/2)%N_DIRS; }

        // We store the hex grid with every alternate row indented, so it maps nicely onto
        // a rectangular area. The only complication is that to access the neighbours you
        // need to know which row you're on.
        vector<vector<vector<int> > > NBORS; // [y%2][6][x,y]
};

#endif
