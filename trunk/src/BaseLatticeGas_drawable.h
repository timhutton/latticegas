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

#ifndef __BASELATTICEGAS_DRAWABLE__
#define __BASELATTICEGAS_DRAWABLE__

// local:
#include "BaseLatticeGas.h"

// wxWidgets:
#include "wxWidgetsPreamble.h"

// STL:
#include <algorithm>
using std::min;
using std::max;

// Abstract base class for an entity that can be zoomed and redrawn (using wxWidgets)
class BaseLatticeGas_drawable : public BaseLatticeGas
{
    public: // functions

        BaseLatticeGas_drawable();
        virtual ~BaseLatticeGas_drawable();

        bool ZoomIn();
        void ZoomOut();
        void GetZoom(int &num,int &denom) const;
        bool RequestZoomFactor(int num,int denom);

        virtual void RedrawImagesIfNeeded()=0;
        void Draw(wxPaintDC& dc);
        
        bool GetShowGas() const;
        void SetShowGas(bool show);
        bool GetShowGasColours() const;
        void SetShowGasColours(bool show);
        double GetLineLength() const;
        void SetLineLength(double ll);
        bool GetShowFlow() const;
        void SetShowFlow(bool show);
        bool GetShowFlowColours() const;
        void SetShowFlowColours(bool show);
        bool GetShowGrid() const;
        void SetShowGrid(bool show);

        void ResetGridForObstacleExample(); // override
        void ResetGridForHoleExample(); // override
        void ResetGridForParticlesExample(); // override

    protected: // functions

        void ResizeGrid(int x_size,int y_size); // override

        static wxColour GetVectorAngleColour(float x,float y);
        static wxColour GetDensityColour(float density);
        virtual wxColour GetColour(int x,int y) const =0;

    protected: // data

        wxImage gas_image;
        wxBitmap drawing_bitmap; 
        wxMemoryDC drawing_buffer;

        int zoom_factor_num,zoom_factor_denom; // zoom is expressed as a rational
        
        // some flags for visual things, used in subclasses when output is graphical
        double line_length;
        bool show_gas,show_gas_colours,show_grid,show_flow,show_flow_colours;
        
        wxColour grid_lines_colour;
};

#endif
