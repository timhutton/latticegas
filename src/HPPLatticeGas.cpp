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

#include "HPPLatticeGas.h"

// STL:
#include <map>
#include <sstream>
using namespace std;

// standard lib:
#include <math.h>
#include <stdlib.h>

// class member constants
HPPLatticeGas::HPPLatticeGas(HPP_type type) : hpp_type(type)
{
    /* We must have density at 50% else the gas doesn't diffuse correctly: collisions 
       must happen as often as possible. And we want the flow to be fast (is this right?) 
       because of the limited Reynold's number, but with enough randomness else the gas 
       misbehaves.
    */
    const int DIAG_DIRS[N_DIRS][2] = {{1,-1},{1,1},{-1,1},{-1,-1}}; // 1=NE, 2=SE, 4=SW, 8=NW
    const int HV_DIRS[N_DIRS][2] = {{0,-1},{1,0},{0,1},{-1,0}}; // 1=N,2=E,4=S,8=W
    switch(type)
    {
        case Diagonal:
            {
                memcpy(DIR,DIAG_DIRS,sizeof(DIR));
                const int NE=1,SE=2,SW=4,NW=8;
                state forward_samples[] = { NE, SE, NE+SE, NE+SE+SW, NE+SE+NW }; // density: 50%, av. horiz. speed = 0.6
                this->forward_flow_samples.assign(forward_samples,forward_samples+sizeof(forward_samples)/sizeof(state));
                state backward_samples[] = { NW, SW, NW+SW, NW+SW+SE, NW+SW+NE }; // density: 50%, av. horiz. speed = 0.6
                this->backward_flow_samples.assign(backward_samples,backward_samples+sizeof(backward_samples)/sizeof(state));
            }
            break;
        case HorizontalVertical:
            {
                memcpy(DIR,HV_DIRS,sizeof(DIR));
                const int N=1,E=2,S=4,W=8;
                state forward_samples[] = { E, N+E+S, N+E, E+S }; // density: 50%, av. horiz. speed = 0.5
                this->forward_flow_samples.assign(forward_samples,forward_samples+sizeof(forward_samples)/sizeof(state));
                state backward_samples[] = { W, N+W+S, N+W, W+S }; // density: 50%, av. horiz. speed = 0.5
                this->backward_flow_samples.assign(backward_samples,backward_samples+sizeof(backward_samples)/sizeof(state));
            }
            break;
    }

    this->BOUNDARY = 1<<N_DIRS; // state 16 is the boundary
}

void HPPLatticeGas::UpdateGas()
{
    current_buffer = old_buffer;
    old_buffer = 1-current_buffer;

    #pragma omp parallel for
    for(int x=0;x<X;x++)
    {
        for(int y=0;y<Y;y++)
        {
            if(force_flow && x==0)
            {
                // the left-most column gets overwritten randomly, since we are simulating
                // an infinite tube filled with moving gas
                state s = this->grid[old_buffer][0][y];
                if(s!=BOUNDARY)
                    s = this->forward_flow_samples[rand()%this->forward_flow_samples.size()];
                this->grid[current_buffer][0][y]=s;
            }
            else
            {
                state c = this->grid[old_buffer][x][y];
                state new_state = c;
                if(c!=BOUNDARY)
                {
                    new_state = 0;
                    for(int dir=0;dir<N_DIRS;dir++)
                    {
                        // look for an inbound particle travelling in this direction
                        state nbor = this->grid[old_buffer][(x+DIR[opposite_dir(dir)][0]+X)%X][(y+DIR[opposite_dir(dir)][1]+Y)%Y];
                        if((nbor&(1<<dir)) || (nbor==BOUNDARY && (c&(1<<opposite_dir(dir)))))
                            new_state |= 1<<dir;
                    }
                }
                new_state = PermuteMaintainingMomentum(new_state);
                this->grid[current_buffer][x][y] = new_state;
            }
        }
    }

    this->iterations++;
    this->need_recompute_flow = true;
    this->need_redraw_images = true;
}

RealPoint HPPLatticeGas::GetVelocityAt(int x,int y) const
{
    state s = this->grid[current_buffer][x][y];
    RealPoint v;
    for(int dir=0;dir<N_DIRS;dir++)
    {
        if(s&(1<<dir))
            v += RealPoint(DIR[dir][0],DIR[dir][1]);
    }
    return v;
}

BaseLatticeGas::state HPPLatticeGas::PermuteMaintainingMomentum(state c)
{
    // there are only two states that can be swapped in HPP: 
    // NE+SW <--> NW+SE for diagonal model (NE=1,SE=2,SW=4,NW=8)
    // N+S <--> E+W for horiz-vert model (N=1,E=2,S=4,W=8)
    if(c==5) return 10;
    else if(c==10) return 5;
    return c;
}

RealPoint HPPLatticeGas::GetAverageInputFlowVelocityPerParticle() const
{
    RealPoint flow(0,0);
    int n_particles_counted = 0;
    for(int i=0;i<(int)this->forward_flow_samples.size();i++)
    {
        state s = this->forward_flow_samples[i];
        for(int dir=0;dir<N_DIRS;dir++)
        {
            if(s&(1<<dir))
            {
                flow += RealPoint(DIR[dir][0],DIR[dir][1]);
                n_particles_counted++;
                // (the complication here is that we allow up to four particles
                //  per square, and we want the average particle speed, not the
                //  average speed in each square)
            }
        }
    }
    return RealPoint(flow.x / n_particles_counted,flow.y / n_particles_counted);
}

float HPPLatticeGas::GetAverageInputNumParticlesPerCell() const
{
    int n_particles_counted = 0;
    for(int i=0;i<(int)this->forward_flow_samples.size();i++)
    {
        state s = this->forward_flow_samples[i];
        for(int dir=0;dir<N_DIRS;dir++)
        {
            if(s&(1<<dir))
            {
                n_particles_counted++;
                // (the complication here is that we allow up to four particles
                //  per square, and we want the average particle speed, not the
                //  average speed in each square)
            }
        }
    }
    return n_particles_counted / (float)this->forward_flow_samples.size();
}

int HPPLatticeGas::GetNumGasParticlesAt(int x,int y) const
{
    state s = this->grid[current_buffer][x][y];
    int n_gas_particles = 0;
    for(int dir=0;dir<N_DIRS;dir++)
        if(s & (1<<dir))
            n_gas_particles++;
    return n_gas_particles;
}

int HPPLatticeGas::GetMaxNumGasParticlesAt(int x,int y) const
{
    state s = this->grid[current_buffer][x][y];
    if(s==BOUNDARY) return 0;
    else return 4;
}

wxColour HPPLatticeGas::GetColour(int x, int y) const
{
    state s = grid[current_buffer][x][y];
    wxColour c;
    if(s==0) c=wxColour(0,0,0);
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

void HPPLatticeGas::InsertRandomParticle(int x,int y)
{
    this->grid[current_buffer][x][y] = ((rand()%50)==0)?(1<<(rand()%4)):0;
}

void HPPLatticeGas::InsertRandomFlow(int x, int y)
{
    this->grid[current_buffer][x][y] = this->forward_flow_samples[rand()%this->forward_flow_samples.size()];
}

void HPPLatticeGas::InsertRandomBackwardFlow(int x, int y)
{
    this->grid[current_buffer][x][y] = this->backward_flow_samples[rand()%this->backward_flow_samples.size()];
}

string HPPLatticeGas::GetReport(state s) const
{
    ostringstream oss;
    bool first=true;
    const string dir_labels_diag[N_DIRS]={"NE","SE","SW","NW"};
    const string dir_labels_hv[N_DIRS]={"N","E","S","W"};
    for(int dir=0;dir<N_DIRS;dir++)
    {
        if(s&(1<<dir))
        {
            if(!first) oss << "+"; 
            first=false;
            if(this->hpp_type==Diagonal)
                oss << dir_labels_diag[dir];
            else
                oss << dir_labels_hv[dir];
        }
    }
    return oss.str();
}
