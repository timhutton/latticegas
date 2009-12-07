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

enum { GasType_HPP_diag, GasType_HPP_ortho, GasType_FHP_I, GasType_FHP_6,
    GasType_FHP_II, GasType_FHP_III, GasType_PI, GasType_Kagome, GasType_LAST };

int LatticeGasFactory::GetNumGasTypesSupported()
{
    return GasType_LAST;
}

wxString LatticeGasFactory::GetGasDescription(int type)
{
    switch(type)
    {
        case GasType_HPP_diag: return _("HPP (diagonal movement)");
        case GasType_HPP_ortho: return _("HPP (vertical/horizontal movement)");
        case GasType_FHP_I: return _("FHP-I");
        case GasType_FHP_6: return _("FHP6");
        case GasType_FHP_II: return _("FHP-II");
        case GasType_FHP_III: return _("FHP-III");
        case GasType_PI: return _("Pair-Interaction");
        case GasType_Kagome: return _("Kagome (Boghosian et al., 2002)");
        default: return _("ERROR!");
    }
}

BaseLatticeGas_drawable* LatticeGasFactory::CreateGas(int type)
{
    switch(type)
    {
        case GasType_HPP_diag: return new HPPLatticeGas(HPPLatticeGas::Diagonal); 
        case GasType_HPP_ortho: return new HPPLatticeGas(HPPLatticeGas::HorizontalVertical); 
        case GasType_FHP_I: return new FHPLatticeGas(FHPLatticeGas::FHP_I); 
        case GasType_FHP_6: return new FHPLatticeGas(FHPLatticeGas::FHP_6); 
        case GasType_FHP_II: return new FHPLatticeGas(FHPLatticeGas::FHP_II); 
        case GasType_FHP_III: return new FHPLatticeGas(FHPLatticeGas::FHP_III); 
        case GasType_PI: return new PairInteractionLatticeGas(); 
        case GasType_Kagome: return NULL; // TODO!
        default: return NULL;
    }
}
