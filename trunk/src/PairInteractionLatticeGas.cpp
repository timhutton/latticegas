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

#include "PairInteractionLatticeGas.h"

// STL:
#include <map>
using namespace std;

// standard lib:
#include <math.h>
#include <stdlib.h>

PairInteractionLatticeGas::PairInteractionLatticeGas()
{
    this->BOUNDARY = 5;

    // initialize the flow samples
    {
        // states: 0=empty, 1=rest particle, 2="x-mover", 3="y-mover", 4="diag-mover"
        //state samples[] = {2,4}; // density: 50%, av. horiz. speed: 1.0 
        // (the above attempt misbehaves, as the gas is already moving at the speed of sound)
        state samples[] = {2,3,4}; // density: 50%, av. horiz. speed: 0.666
        this->forward_flow_samples.assign(samples,samples+sizeof(samples)/sizeof(state));
        // (note that below we don't allow particles to move backwards, we leave the x=0 column
        //  empty when forcing the flow (see the details of PI-LGA for why))
    }

    // initialize the pairwise interaction table
    // states: 0=empty, 1=rest particle, 2="x-mover", 3="y-mover", 4="diag-mover"
    {
        // the PI-LGA interactions: (all the others are stasis transitions)
        // (here we consider the horizontal-pair case, swap states 2 and 3 for the vertical case)
        vector<pair<pair<state,state>,pair<state,state> > > pairs;
        pairs.push_back(make_pair(make_pair(3,3),make_pair(4,4)));
        pairs.push_back(make_pair(make_pair(2,4),make_pair(3,1)));
        pairs.push_back(make_pair(make_pair(2,3),make_pair(4,1)));
        pairs.push_back(make_pair(make_pair(1,1),make_pair(2,2)));
        pairs.push_back(make_pair(make_pair(0,3),make_pair(3,0)));
        pairs.push_back(make_pair(make_pair(0,1),make_pair(1,0)));
        // expand the pairs into a lookup table
        state a,b;
        unsigned int iPair;
        bool found;
        for(a=0;a<5;a++)
        {
            for(b=0;b<5;b++)
            {
                found = false;
                for(iPair=0;iPair<pairs.size() && !found;iPair++)
                {
                    pair<pair<state,state>,pair<state,state> > &pair = pairs[iPair];
                    if(pair.first.first==a && pair.first.second==b)
                    {
                        pi_table_horiz[a][b][0] = pair.second.first;
                        pi_table_horiz[a][b][1] = pair.second.second;
                        found = true;
                    }
                    else if(pair.first.second==a && pair.first.first==b)
                    {
                        pi_table_horiz[a][b][0] = pair.second.second;
                        pi_table_horiz[a][b][1] = pair.second.first;
                        found = true;
                    }
                    else if(pair.second.first==a && pair.second.second==b)
                    {
                        pi_table_horiz[a][b][0] = pair.first.first;
                        pi_table_horiz[a][b][1] = pair.first.second;
                        found = true;
                    }
                    else if(pair.second.second==a && pair.second.first==b)
                    {
                        pi_table_horiz[a][b][0] = pair.first.second;
                        pi_table_horiz[a][b][1] = pair.first.first;
                        found = true;
                    }
                }
                if(!found)
                {
                    pi_table_horiz[a][b][0]=a;
                    pi_table_horiz[a][b][1]=b;
                }
            }
        }
        for(a=0;a<5;a++)
        {
            for(b=0;b<5;b++)
            {
                pi_table_vert[swap23(a)][swap23(b)][0]=swap23(pi_table_horiz[a][b][0]);
                pi_table_vert[swap23(a)][swap23(b)][1]=swap23(pi_table_horiz[a][b][1]);
            }
        }
    }
}

void PairInteractionLatticeGas::UpdateGas()
{
    current_buffer = old_buffer;
    old_buffer = 1-current_buffer;

    vector<vector<state> > &OldBuffer = this->grid[old_buffer];
    vector<vector<state> > &NewBuffer = this->grid[current_buffer];

    // -- phase 1: pairwise interactions, in x then y --

    #pragma omp parallel for
    for(int x=0;x<X;x+=2) // (we assume X is even)
        for(int y=0;y<Y;y++)
            ApplyHorizontalPairwiseInteraction(OldBuffer[x][y],OldBuffer[x+1][y]);
    #pragma omp parallel for
    for(int x=0;x<X;x++)
        for(int y=0;y<Y;y+=2) // (we assume Y is even)
            ApplyVerticalPairwiseInteraction(OldBuffer[x][y],OldBuffer[x][y+1]);

    // -- phase 2: simple transport --

    // start with an empty grid
    #pragma omp parallel for
    for(int x=0;x<X;x++)
       std::fill(NewBuffer[x].begin(),NewBuffer[x].end(),0);

    #pragma omp parallel for
    for(int x=0;x<X;x++)
    {
        // (need to declare these things here else omp causes problems)
        int sx,sy;
        state s,s2;
        for(int y=0;y<Y;y++)
        {
            s = OldBuffer[x][y];
            if(s==BOUNDARY)
                NewBuffer[x][y]=BOUNDARY;      // boundaries don't change
            else if(s>0)
            {
                // try simple transport
                // we compute where this particle might go to: sx,sy
                if(x%2) sx=x+2; else sx=x-2;
                if(y%2) sy=y+2; else sy=y-2;
                //if(sx>=X) sx-=X; else if(sx<0) sx+=X; // left-right wraps around
                //if(sy>=Y) sy-=3; else if(sy<0) sy+=3; // top-bottom bounces (no-slip)
                BringInside(sx,sy);
                // retrieve the contents of the destination square
                s2 = OldBuffer[sx][sy];
                if(s2!=BOUNDARY)
                    NewBuffer[sx][sy]=s; // simple transport
                else {
                    // we've got some bouncing to do
                    // try the square in between
                    if(x%2) sx=x+1; else sx=x-1;
                    if(y%2) sy=y+1; else sy=y-1;
                    BringInside(sx,sy);
                    //if(sx>=X) sx-=X; else if(sx<0) sx+=X; // left-right wraps around
                    //if(sy>=Y) sy-=1; else if(sy<0) sy+=1; // top-bottom bounces (no-slip)
                    s2 = OldBuffer[sx][sy];
                    if(s2!=BOUNDARY)
                        NewBuffer[sx][sy]=s; // bounce transport
                    else
                    {
                        // try to bounce back
                        if(x%2) sx=x-1; else sx=x+1;
                        if(y%2) sy=y-1; else sy=y+1;
                        //if(sx>=X) sx-=X; else if(sx<0) sx+=X; // left-right wraps around
                        //if(sy>=Y) sy-=1; else if(sy<0) sy+=1; // top-bottom bounces (no-slip)
                        BringInside(sx,sy);
                        s2 = OldBuffer[sx][sy];
                        if(s2!=BOUNDARY)
                            NewBuffer[sx][sy]=s; // bounce back transport
                        else
                            NewBuffer[x][y]=s; // particle is trapped here!
                    }
                }
            }
        }
    }
    if(this->force_flow)
    {
        // the left-most column is overwritten, since we are modelling flow in an infinite tube
        for(int x=0;x<2;x++)
        {
            for(int y=0;y<Y;y++)
            {
                if(OldBuffer[x][y]==BOUNDARY)
                    NewBuffer[x][y] = BOUNDARY;
                else
                    NewBuffer[x][y] = (x%2)?this->forward_flow_samples[rand()%this->forward_flow_samples.size()]:0; // only flow to the right
            }
        }
    }
    this->iterations++;
    this->need_recompute_flow = true;
    this->need_redraw_images = true;
}

void PairInteractionLatticeGas::ApplyHorizontalPairwiseInteraction(state &a,state &b)
{
    if(a==BOUNDARY || b==BOUNDARY) return; // no interactions at boundaries

    state temp_a = this->pi_table_horiz[a][b][0];
    b = this->pi_table_horiz[a][b][1];
    a = temp_a;
}

void PairInteractionLatticeGas::ApplyVerticalPairwiseInteraction(state &a,state &b)
{
    if(a==BOUNDARY || b==BOUNDARY) return; // no interactions at boundaries

    state temp_a = this->pi_table_vert[a][b][0];
    b = this->pi_table_vert[a][b][1];
    a = temp_a;
}

// vertical pair interactions are the same as horizontal ones with states 2 and 3 swapped, so we use this function
int PairInteractionLatticeGas::swap23(int x) 
{ 
    if(x==2) return 3; else if(x==3) return 2; else return x; 
}

RealPoint PairInteractionLatticeGas::GetAverageInputFlowVelocityPerParticle() const
{
    // here we force the flow by overwriting two columns: x=0 and x=1
    // but leave the left one (x=0) empty, so particles only move to the right (2),
    // up/down (3) or diagonally up and right (4)
    float flow_speed = 0.0f;
    int n_counted = 0;
    for(int i=0;i<(int)this->forward_flow_samples.size();i++)
    {
        state s = this->forward_flow_samples[i];
        // states: 0=empty, 1=rest particle, 2="x-mover", 3="y-mover", 4="diag-mover"
        if(s==2 || s==4)
            flow_speed += 1.0f;
    }
    return RealPoint(flow_speed / this->forward_flow_samples.size(),0.0);
}

float PairInteractionLatticeGas::GetAverageInputNumParticlesPerCell() const
{
    // here we force the flow by overwriting two columns: x=0 and x=1
    // but leave the left one (x=0) empty, so particles only move to the right (2),
    // up/down (3) or diagonally up and right (4)
    int n_counted = 0;
    for(int i=0;i<(int)this->forward_flow_samples.size();i++)
    {
        state s = this->forward_flow_samples[i];
        // states: 0=empty, 1=rest particle, 2="x-mover", 3="y-mover", 4="diag-mover"
        if(s>0)
            n_counted++;
    }
    return (n_counted/2.0) / this->forward_flow_samples.size(); // because we leave one column empty
}

int PairInteractionLatticeGas::GetNumGasParticlesAt(int x,int y) const
{
    state s = this->grid[current_buffer][x][y];
    if(s>=1 && s<=4) return 1;
    else return 0;
}

int PairInteractionLatticeGas::GetMaxNumGasParticlesAt(int x,int y) const
{
    state s = this->grid[current_buffer][x][y];
    if(s==BOUNDARY) return 0;
    else return 1;
}

RealPoint PairInteractionLatticeGas::GetVelocityAt(int x,int y) const
{
    state s = grid[current_buffer][x][y];
    switch(s)
    {
        default: return RealPoint(0,0);
        case 2: if(x%2) return RealPoint(1,0); else return RealPoint(-1,0);
        case 3: if(y%2) return RealPoint(0,1); else return RealPoint(0,-1);
        case 4: if(x%2 && y%2) return RealPoint(1,1); else if(x%2 && !(y%2)) return RealPoint(1,-1);
                else if(!(x%2) && y%2) return RealPoint(-1,1); else return RealPoint(-1,-1);
    }
}

wxColour PairInteractionLatticeGas::GetColour(int x,int y) const
{
    state s = grid[current_buffer][x][y];
    wxColour c;
    if(s==0) c=wxColour(0,0,0);
    else if(s==1) c=wxColour(200,200,200);
    else if(s==BOUNDARY) c=wxColour(120,120,120);
    else {
        if(this->show_gas_colours)
        {
            RealPoint v = GetVelocityAt(x,y);
            c = GetVectorAngleColour(v.x,v.y);
        }
        else
        {
            float density = GetNumGasParticlesAt(x,y) / (float)GetMaxNumGasParticlesAt(x,y);
            c = GetDensityColour(density);
        }
    }
    return c;
}

void PairInteractionLatticeGas::InsertRandomParticle(int x,int y)
{
    this->grid[current_buffer][x][y] = ((rand()%100)==0)?(rand()%5):0; // sparse atoms
}

void PairInteractionLatticeGas::InsertRandomFlow(int x, int y)
{
    this->grid[current_buffer][x][y] = (x%2)?this->forward_flow_samples[rand()%this->forward_flow_samples.size()]:0; // flow
}

void PairInteractionLatticeGas::InsertRandomBackwardFlow(int x, int y)
{
    this->grid[current_buffer][x][y] = (1-(x%2))?this->forward_flow_samples[rand()%this->forward_flow_samples.size()]:0; // flow
}

string PairInteractionLatticeGas::GetReport(state s) const
{
    return "TODO"; // TODO
}

// we only need to override this to draw a more widely spaced grid, should split this up a bit
void PairInteractionLatticeGas::RedrawImagesIfNeeded()
{
    if(!this->need_redraw_images) return;

    if(this->show_gas)
    {
        if(this->zoom_factor_denom==1) // ie. zoomed in enough to see cells
        {
            for(int x=0;x<X;x++)
            {
                for(int y=0;y<Y;y++)
                {
                    wxColour c = GetColour(x,y);
                    // draw a filled rect (quickest this way)
                    for(int i=x * this->zoom_factor_num / this->zoom_factor_denom;i<(x+1) * this->zoom_factor_num / this->zoom_factor_denom;i++)
                    {
                        for(int j=y*this->zoom_factor_num / this->zoom_factor_denom;j<(y+1)*this->zoom_factor_num / this->zoom_factor_denom;j++)
                        {
						    // draw grid if zoomed in enough
						    if(this->show_grid && this->zoom_factor_num>=4 && 
							    ((x%2==0 && i==x * this->zoom_factor_num / this->zoom_factor_denom) ||
							    (y%2==0 && j==y * this->zoom_factor_num / this->zoom_factor_denom) ))
								    gas_image.SetRGB(i,j,grid_lines_colour.Red(),grid_lines_colour.Green(),grid_lines_colour.Blue());
						    else
							    gas_image.SetRGB(i,j,c.Red(),c.Green(),c.Blue());
                        }
                    }
                }
            }
        }
        else // zoomed out beyond 1 pixel: show the density of the gas
        {
            // for each pixel:
            for(int px=0;px<this->drawing_bitmap.GetWidth();px++)
            {
                for(int py=0;py<this->drawing_bitmap.GetHeight();py++)
                {
                    int x = px * this->zoom_factor_denom;
                    int y = py * this->zoom_factor_denom;
                    // find the average colour for the cells in this region
                    int r=0,g=0,b=0,n=0;
                    for(int i=x;i<min(x+this->zoom_factor_denom,X-1);i++)
                    {
                        for(int j=y;j<min(y+this->zoom_factor_denom,Y-1);j++)
                        {
                            wxColour c = GetColour(i,j);
                            r+=c.Red();
                            g+=c.Green();
                            b+=c.Blue();
                            n++;
                        }
                    }
                    wxColour c = wxColour(r/n,g/n,b/n);
                    gas_image.SetRGB(px,py,c.Red(),c.Green(),c.Blue());
                }
            }
        }
        this->drawing_buffer.DrawBitmap(wxBitmap(gas_image),0,0);
    }
    else // show_gas==false
    {
        this->drawing_buffer.SetBackground(*wxWHITE_BRUSH);
        this->drawing_buffer.Clear();
    }

    if(show_flow)
    {
        // draw the flow image
        if(need_recompute_flow)
            ComputeFlow();

        // draw flow vectors
        if(!this->show_flow_colours)
            this->drawing_buffer.SetPen(*wxBLACK_PEN);
        for(int x=this->flow_sample_separation;x<X-this->flow_sample_separation;x+=this->flow_sample_separation)
        {
            for(int y=this->flow_sample_separation;y<Y-this->flow_sample_separation;y+=this->flow_sample_separation)
            {
                int sx=x/this->flow_sample_separation,sy=y/this->flow_sample_separation;
                RealPoint v(velocity[sx][sy]);
                if(this->velocity_representation == Velocity_SubtractGlobalMean)
                {
                    // we subtract the averaged velocity at this point, to better highlight the dynamic changes
                    v.x -= global_mean_velocity.x;
                    v.y -= global_mean_velocity.y;
                }
                else if(this->velocity_representation == Velocity_SubtractPointMean)
                {
                    // we subtract the averaged velocity at this point, to better highlight the dynamic changes
                    v.x -= averaged_velocity[sx][sy].x;
                    v.y -= averaged_velocity[sx][sy].y;
                }
                if(this->show_flow_colours)
                    this->drawing_buffer.SetPen(wxPen(GetVectorAngleColour(v.x,v.y)));
                this->drawing_buffer.DrawLine((x+0.5) * this->zoom_factor_num / this->zoom_factor_denom,
                    (y+0.5) * this->zoom_factor_num / this->zoom_factor_denom,
                    (x+0.5+v.x*this->line_length)*this->zoom_factor_num / this->zoom_factor_denom,
                    (y+0.5+v.y*this->line_length)*this->zoom_factor_num / this->zoom_factor_denom);
            }
        }
    }

    /* we can save images to make an animation but this is currently dormant
    if(this->save_images)
    {
        SetStatusTextHelper sth(_("Saving image snapshot..."),this);
        this->drawing_bitmap.SaveFile(wxString::Format(_("%04d.jpg"),this->n_saved_images++),wxBITMAP_TYPE_JPEG);
    }*/

    this->need_redraw_images = false;
}

