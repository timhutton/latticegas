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

BaseLatticeGas_drawable::BaseLatticeGas_drawable()
{
    grid_lines_colour = wxColour(100,100,100);
}

BaseLatticeGas_drawable::~BaseLatticeGas_drawable()
{
}

bool BaseLatticeGas_drawable::RequestZoomFactor(int num,int denom)
{
    // just check this wouldn't make too big an image (could only draw the bit we need to show, in future)
    {
        long int n_pixels = (this->X * num / denom)*(this->Y * num / denom);
        if(n_pixels>10e6) return false;
    }
    this->zoom_factor_num = num;
    this->zoom_factor_denom = denom;
 
    // resize the images we draw into
    this->drawing_bitmap.Create(this->X * this->zoom_factor_num / this->zoom_factor_denom, 
        this->Y * this->zoom_factor_num / this->zoom_factor_denom);
    this->gas_image.Create(this->X * this->zoom_factor_num / this->zoom_factor_denom, 
        this->Y * this->zoom_factor_num / this->zoom_factor_denom);

    // select a bitmap into the drawing buffer
    this->drawing_buffer.SelectObject(this->drawing_bitmap);

    this->need_redraw_images = true;

    return true;
}

void BaseLatticeGas_drawable::Draw(wxPaintDC& dc)
{
    this->RedrawImagesIfNeeded();
    dc.Blit(0,0,X*this->zoom_factor_num / this->zoom_factor_denom,
        Y*this->zoom_factor_num / this->zoom_factor_denom,&this->drawing_buffer,0,0);
}

void BaseLatticeGas_drawable::ResizeGrid(int x_size,int y_size)
{
    BaseLatticeGas::ResizeGrid(x_size,y_size);

    // scale down the visible grid until we it is sensible to show
    {
        // try: 10,9,...,2,1,1/2,1/3,1/4,...
        int zn=10,zd=1;
        int ms = max(X,Y);
        while((ms*zn)/zd>500)
        {
            if(zn>1) zn--;
            else zd++;
        }
        RequestZoomFactor(zn,zd);
    }
}

bool BaseLatticeGas_drawable::ZoomIn()
{
    if(this->zoom_factor_denom==1)
        return this->RequestZoomFactor(this->zoom_factor_num*2,this->zoom_factor_denom);
    else 
        return this->RequestZoomFactor(this->zoom_factor_num,this->zoom_factor_denom/2);
}

void BaseLatticeGas_drawable::ZoomOut()
{
    if(this->zoom_factor_num>1) this->RequestZoomFactor(this->zoom_factor_num/2,this->zoom_factor_denom);
    else this->RequestZoomFactor(this->zoom_factor_num,this->zoom_factor_denom*2);
}

void BaseLatticeGas_drawable::GetZoom(int &num,int &denom) const
{ 
    num = this->zoom_factor_num; 
    denom = this->zoom_factor_denom; 
}

wxColour BaseLatticeGas_drawable::GetVectorAngleColour(float x,float y)
{
    double angle = 0.5 + atan2(y,x)/(2*3.14159265358979);
    wxImage::RGBValue rgb = wxImage::HSVtoRGB(wxImage::HSVValue(angle,1.0,1.0));
    return wxColour(rgb.red,rgb.green,rgb.blue);
}

wxColour BaseLatticeGas_drawable::GetDensityColour(float density)
{
    return wxColour(255*density,255*density,255*density);
}

bool BaseLatticeGas_drawable::GetShowGas() const 
{ 
    return this->show_gas; 
}

void BaseLatticeGas_drawable::SetShowGas(bool show) 
{ 
    this->show_gas = show; 
    this->need_redraw_images = true;
}

bool BaseLatticeGas_drawable::GetShowGasColours() const 
{ 
    return this->show_gas_colours; 
}

void BaseLatticeGas_drawable::SetShowGasColours(bool show) 
{ 
    this->show_gas_colours = show; 
    this->need_redraw_images = true;
}

double BaseLatticeGas_drawable::GetLineLength() const 
{ 
    return this->line_length; 
}

void BaseLatticeGas_drawable::SetLineLength(double ll) 
{ 
    this->line_length = ll; 
    this->need_redraw_images = true;
}

bool BaseLatticeGas_drawable::GetShowFlow() const 
{ 
    return this->show_flow; 
}

void BaseLatticeGas_drawable::SetShowFlow(bool show) 
{ 
    this->show_flow = show; 
    this->need_redraw_images = true;
}

bool BaseLatticeGas_drawable::GetShowFlowColours() const 
{ 
    return this->show_flow_colours; 
}

void BaseLatticeGas_drawable::SetShowFlowColours(bool show) 
{ 
    this->show_flow_colours = show; 
    this->need_redraw_images = true;
}

bool BaseLatticeGas_drawable::GetShowGrid() const 
{ 
    return this->show_grid; 
}

void BaseLatticeGas_drawable::SetShowGrid(bool show) 
{ 
    this->show_grid = show; 
    this->need_redraw_images = true;
}

void BaseLatticeGas_drawable::ResetGridForParticlesExample()
{
    this->BaseLatticeGas::ResetGridForParticlesExample();

    this->line_length = 1;
    this->show_flow_colours = true;
	this->show_grid = true;
    this->show_flow = true;
    this->show_gas = true;
    this->show_gas_colours = true;
}

void BaseLatticeGas_drawable::ResetGridForObstacleExample()
{
    this->BaseLatticeGas::ResetGridForObstacleExample();
    this->line_length = 100;
    this->show_flow_colours = true;
    this->show_grid = true;
    this->show_flow = true;
    this->show_gas = true;
    this->show_gas_colours = false;
}

void BaseLatticeGas_drawable::ResetGridForHoleExample()
{
    this->BaseLatticeGas::ResetGridForHoleExample();
    this->line_length = 100;
    this->show_flow_colours = true;
    this->show_grid = true;
    this->show_flow = true;
    this->show_gas = true;
    this->show_gas_colours = false;
}


