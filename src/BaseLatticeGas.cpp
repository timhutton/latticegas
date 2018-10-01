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

// local:
#include "BaseLatticeGas.h"

// standard library:
#include <stdlib.h>
#include <math.h>

// STL:
#include <algorithm>
#include <exception>
#include <sstream>
#include <stdexcept>
using namespace std;

void BaseLatticeGas::ResizeGrid(int x_size,int y_size)
{
    this->iterations = 0;
    this->current_buffer=0;
    this->old_buffer=1;

    this->X = x_size;
    this->Y = y_size;
    this->grid[0].assign(X,vector<state>(Y));
    this->grid[1].assign(X,vector<state>(Y));

    ResizeFlowSamples();
}

void BaseLatticeGas::ResizeFlowSamples()
{
    // initialize the velocity arrays
    this->velocity.assign(X/this->flow_sample_separation,
        vector<RealPoint >(Y/this->flow_sample_separation,RealPoint(0.0,0.0)));
    this->averaged_velocity.assign(X/this->flow_sample_separation,
        vector<RealPoint >(Y/this->flow_sample_separation,RealPoint(0.0,0.0)));
    this->have_taken_first_velocity_average = false;
    this->need_redraw_images = true;
    this->need_recompute_flow = true;
}

void BaseLatticeGas::ComputeFlow()
{
    // recompute the locally-averaged velocities
    const int R = this->averaging_radius;
    const double avF=0.95; // for a running average we take a weighted mix of the previous average and the new value

    this->global_mean_velocity = RealPoint(0.0,0.0);
    int global_n_particles=0;

    #pragma omp parallel for
    for(int x=this->flow_sample_separation;x<(X-this->flow_sample_separation);x+=this->flow_sample_separation)
    {
        for(int y=this->flow_sample_separation;y<(Y-this->flow_sample_separation);y+=this->flow_sample_separation)
        {
            RealPoint v(0,0);
            int n_counted = 0;
            for(int dx=max(0,x-R);dx<=min(X-1,x+R);dx++) // (do we need to include an even mix of the sides of the 2x2 cells?)
            {
                for(int dy=max(0,y-R);dy<=min(Y-1,y+R);dy++)
                {
                    v += GetVelocityAt(dx,dy);
                    n_counted += GetNumGasParticlesAt(dx,dy);
                }
            }
            int sx=x/this->flow_sample_separation,sy=y/this->flow_sample_separation;
            if(n_counted>0)
            {
                velocity[sx][sy] = RealPoint(v.x / n_counted, v.y / n_counted); // av. velocity per particle
                this->global_mean_velocity += v;
                global_n_particles += n_counted;
            }
            else
                velocity[sx][sy] = RealPoint(0.0,0.0);
            // compute the running point average velocity
            if(!this->have_taken_first_velocity_average)
            {
                averaged_velocity[sx][sy] = RealPoint(velocity[sx][sy].x,velocity[sx][sy].y);
            }
            else
            {
                averaged_velocity[sx][sy] = RealPoint(averaged_velocity[sx][sy].x * avF + velocity[sx][sy].x * (1.0-avF),
                    averaged_velocity[sx][sy].y * avF + velocity[sx][sy].y * (1.0-avF));
            }
        }
    }
    if(!this->have_taken_first_velocity_average)
        this->have_taken_first_velocity_average = true;

    this->global_mean_velocity *= 1.0/global_n_particles;

    this->need_recompute_flow = false;
}

int BaseLatticeGas::GetNumGasParticles() const
{
    int n_gas_particles=0;
    for(int x=0;x<X;x++)
        for(int y=0;y<Y;y++)
            n_gas_particles += GetNumGasParticlesAt(x,y);
    return n_gas_particles;
}
 
int BaseLatticeGas::GetMaxNumGasParticles() const
{
    int max_num_gas_particles=0;
    for(int x=0;x<X;x++)
        for(int y=0;y<Y;y++)
            max_num_gas_particles += GetMaxNumGasParticlesAt(x,y);
    return max_num_gas_particles;
}

int BaseLatticeGas::GetNumDemos()
{
    return Demo_LAST;
}

wxString BaseLatticeGas::GetDemoDescription(int i)
{
    switch(i)
    {
        case Demo_Particles: return _("A few gas particles colliding");
        case Demo_Obstacle: return _("Eddies in the wake of an obstacle");
        case Demo_Hole: return _("Flow through a hole");
        case Demo_KelvinHelmholtz: return _("Kelvin-Helmholtz instability");
        default: return _("ERROR!");
    }
}

void BaseLatticeGas::ResetGridForDemo(int i)
{
    switch(i)
    {
        case Demo_Particles: // a few particles in a box
            {
                this->velocity_representation = Velocity_Raw;
                this->averaging_radius = 0;
                this->flow_sample_separation = 1;
                this->force_flow = false;

                this->ResizeGrid(40,30);

                for(int x=0;x<X;x++)
                {
                    for(int y=0;y<Y;y++)
                    {
                        if(x<=1 || x>=X-2 || y<=1 || y>=Y-2)
                            SetAt(x,y,BOUNDARY);
                        else
                            InsertRandomParticle(x,y);
                    }
                }
            }
            break;
        case Demo_Obstacle: // a wind tunnel with a plate obstacle
            {
                this->velocity_representation = Velocity_Raw;
                this->averaging_radius = 20;
                this->flow_sample_separation = 20;
                this->force_flow = true;
                
                const bool large_scale = false;

                int target_n_particles = 200000;
                
                if(large_scale)
                {
                    target_n_particles = 10000000;
                }
                
                float n_cells_needed = target_n_particles / this->GetAverageInputNumParticlesPerCell();
                // we want a rectangle of the right ratio
                float ratio = 2.0f;
                int height = (int)ceil(sqrt(n_cells_needed/ratio));
                // make height divisible by 2
                height -= height%2;
                int width = (int)ceil(height*ratio);
                width -= width%2;

                this->ResizeGrid(width,height);
                
                int barrier_height = Y/4;

                if(large_scale)
                {
                    barrier_height = 530;
                }

                for(int x=0;x<X;x++)
                {
                    for(int y=0;y<Y;y++)
                    {
                        if(y==0 || y==Y-1 || (abs(x-X/8)<2 && abs(y-Y/2)<(barrier_height/2)))//sqrt(pow(x-X/8,2.0)+pow(y-Y/2,2.0))<Y/5)
                            SetAt(x,y,BOUNDARY);
                        else
                            InsertRandomFlow(x,y);
                    }
                }
            }
            break;
        case Demo_Hole: // a wind tunnel with a plate with a hole
            {
                this->velocity_representation = Velocity_Raw;
                this->averaging_radius = 20;
                this->flow_sample_separation = 20;
                this->force_flow = true;

                const int target_n_particles = 200000;
                float n_cells_needed = target_n_particles / this->GetAverageInputNumParticlesPerCell();
                // we want a rectangle of the right ratio
                float ratio = 2.0f;
                int height = (int)ceil(sqrt(n_cells_needed/ratio));
                // make height divisible by 2
                height -= height%2;
                int width = (int)ceil(height*ratio);
                width -= width%2;

                this->ResizeGrid(width,height);

                for(int x=0;x<X;x++)
                    for(int y=0;y<Y;y++)
                        if(y==0 || y==Y-1 || (abs(x-X/8)<2 && abs(y-Y/2)>Y/4))
                            SetAt(x,y,BOUNDARY);
                        else
                            InsertRandomFlow(x,y);
            }
            break;
        case Demo_KelvinHelmholtz: // Kelvin-Helmholtz instability
            {
                this->velocity_representation = Velocity_Raw;
                this->averaging_radius = 20;
                this->flow_sample_separation = 20;
                this->force_flow = false;

                const int target_n_particles = 1000000;
                float n_cells_needed = target_n_particles / this->GetAverageInputNumParticlesPerCell();
                // we want a rectangle of the right ratio
                float ratio = 1.0f;
                int height = (int)ceil(sqrt(n_cells_needed/ratio));
                // make height divisible by 2
                height -= height%2;
                int width = (int)ceil(height*ratio);
                width -= width%2;

                this->ResizeGrid(width,height);

                int barrier_height = min(250,Y/4);

                for(int x=0;x<X;x++)
                {
                    for(int y=0;y<Y;y++)
                    {
                        if(y==0 || y==Y-1)
                            SetAt(x,y,BOUNDARY);
                        else if(y<Y/2)
                            InsertRandomFlow(x,y);
                        else
                            InsertRandomBackwardFlow(x,y);
                    }
                }
            }
            break;
        default: 
            throw runtime_error("Demo range error!");
    }
}

int BaseLatticeGas::GetIterations() const 
{ 
    return this->iterations; 
}

int BaseLatticeGas::GetAveragingRadius() const 
{ 
    return this->averaging_radius; 
}

void BaseLatticeGas::SetAveragingRadius(int ar) 
{ 
    this->averaging_radius = ar; 
    this->flow_sample_separation = ar;
    ResizeFlowSamples();
}

int BaseLatticeGas::GetVelocityRepresentation() const 
{ 
    return this->velocity_representation; 
}

void BaseLatticeGas::SetVelocityRepresentation(int rep) 
{ 
    switch(rep)
    {
        case Velocity_Raw: this->velocity_representation = Velocity_Raw; break;
        case Velocity_SubtractGlobalMean: this->velocity_representation = Velocity_SubtractGlobalMean; break;
        case Velocity_SubtractPointMean: this->velocity_representation = Velocity_SubtractPointMean; break;
        default: throw runtime_error("Velocity representation out of range!");
    }
    this->have_taken_first_velocity_average = false;
    this->need_recompute_flow = true;
    this->need_redraw_images = true;
}

int BaseLatticeGas::GetNumVelocityRepresentations()
{
    return Velocity_LAST;
}

wxString BaseLatticeGas::GetVelocityRepresentationAsString(int i)
{
    switch(i)
    {
        case Velocity_Raw: return _("Raw velocity");
        case Velocity_SubtractGlobalMean: return _("Subtract global mean velocity");
        case Velocity_SubtractPointMean: return _("Subtract time-averaged local mean velocity");
        default: return _("ERROR!");
    }
}

int BaseLatticeGas::GetX() const 
{ 
    return this->X; 
}

int BaseLatticeGas::GetY() const 
{ 
    return this->Y; 
}

void BaseLatticeGas::BringInside(int &x,int &y) const
{
    while(x<0) x+=X;
    while(x>=X) x-=X;
    while(y<0) y+=Y;
    while(y>=Y) y-=Y;
}

BaseLatticeGas::state BaseLatticeGas::GetAt(int x,int y) const
{
    return this->grid[current_buffer][x][y];
}

void BaseLatticeGas::SetAt(int x,int y,state s)
{
    this->grid[current_buffer][x][y] = s;
    this->grid[old_buffer][x][y] = s;
    // (by setting both buffers we don't need to copy over boundary cells)
}

RealPoint BaseLatticeGas::GetAverageVelocityPerParticle() const
{
    return this->global_mean_velocity;
}
