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

#include "LatticeGasFactory.h"

#include "HPPLatticeGas.h"
#include "FHPLatticeGas.h"
#include "PairInteractionLatticeGas.h"

int LatticeGasFactory::GetNumGasTypesSupported()
{
    return 8;
}

const char* LatticeGasFactory::GetGasDescription(int type)
{
    switch(type)
    {
        case 0: return "HPP (diagonal movement)";
        case 1: return "HPP (vertical/horizontal movement)";
        case 2: return "FHP-I";
        case 3: return "FHP6";
        case 4: return "FHP-II";
        case 5: return "FHP-III";
        case 6: return "Pair-Interaction";
        case 7: return "Kagome (Boghosian et al., 2002)";
        default: return "ERROR!";
    }
}

BaseLatticeGas_drawable* LatticeGasFactory::CreateGas(int type)
{
    switch(type)
    {
        case 0: return new HPPLatticeGas(HPPLatticeGas::Diagonal); 
        case 1: return new HPPLatticeGas(HPPLatticeGas::HorizontalVertical); 
        case 2: return new FHPLatticeGas(FHPLatticeGas::FHP_I); 
        case 3: return new FHPLatticeGas(FHPLatticeGas::FHP_6); 
        case 4: return new FHPLatticeGas(FHPLatticeGas::FHP_II); 
        case 5: return new FHPLatticeGas(FHPLatticeGas::FHP_III); 
        case 6: return new PairInteractionLatticeGas(); 
        case 7: return NULL; // TODO!
        default: return NULL;
    }
}
