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

#include "BaseLatticeGas_drawable.h"

class LatticeGasFactory
{
	public:

		static BaseLatticeGas_drawable* CreateGas(int type);

        static const char* GetGasDescription(int type);

        static int GetNumGasTypesSupported();

    private:

      // not implemented:
      LatticeGasFactory();
      LatticeGasFactory(const LatticeGasFactory& c);
      LatticeGasFactory& operator=(const LatticeGasFactory& c);
};