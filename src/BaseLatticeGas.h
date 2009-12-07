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

#ifndef __BASELATTICEGAS_H__
#define __BASELATTICEGAS_H__

// local:
#include "wxWidgetsPreamble.h"

// STL:
#include <vector>
#include <string>
using std::vector;
using std::string;

// (a simple replacement for wxRealPoint, to avoid dependencies)
class RealPoint {
    public:
        double x,y;
        RealPoint(double x=0.0,double y=0.0) : x(x), y(y) {}
        RealPoint(const RealPoint& r) : x(r.x), y(r.y) {}
        RealPoint operator+(const RealPoint& b) { return RealPoint(x+b.x,y+b.y); }
        RealPoint& operator+=(const RealPoint& b) { x+=b.x; y+=b.y; return *this; }
        RealPoint operator*(const double m) { return RealPoint(x*m,y*m); }
        RealPoint& operator*=(const double m) { x*=m; y*=m; return *this; }
};

// Abstract base class for all 2D lattice gas implementations. 
class BaseLatticeGas 
{
	public: // overrideables
        
        // update the gas by applying one timestep
		virtual void UpdateGas()=0;

        // what is the average particle velocity?
        RealPoint GetAverageVelocityPerParticle() const;

        // for models that force the flow at one end (typically with a sink at 
        // the other end), what is the average input flow speed of the particles?
        virtual RealPoint GetAverageInputFlowVelocityPerParticle() const =0;

        // for models that force the flow at one end (typically with a sink at 
        // the other end), what is the average number of particles per cell?
        // (we use this to size the grid appropriately)
        virtual float GetAverageInputNumParticlesPerCell() const =0;

        virtual void ResetGridForDemo(int i);
        static int GetNumDemos();
        static wxString GetDemoDescription(int i);

    public: // functions

        virtual ~BaseLatticeGas() {}

        int GetIterations() const;
        int GetAveragingRadius() const;
        void SetAveragingRadius(int ar);
        int GetVelocityRepresentation() const;
        void SetVelocityRepresentation(int i);
        static wxString GetVelocityRepresentationAsString(int i);
        static int GetNumVelocityRepresentations();
        int GetX() const;
        int GetY() const;

        // retrieve the overall number of gas particles
        int GetNumGasParticles() const;

        // if density was 100%, how many gas particles would there be?
        int GetMaxNumGasParticles() const;

    protected: // typedefs

        typedef unsigned char state;

        enum TVelocityRepresentation { Velocity_Raw, Velocity_SubtractGlobalMean, Velocity_SubtractPointMean, Velocity_LAST };
        enum TDemo { Demo_Particles, Demo_Obstacle, Demo_Hole, Demo_KelvinHelmholtz, Demo_LAST };

    protected: // overrideables
    
        // resize the grid to the specified size, leaving it empty
        virtual void ResizeGrid(int x_size,int y_size);
        
        // retrieve the number of gas particles in a particular square
        virtual int GetNumGasParticlesAt(int x,int y) const =0;

        // if the gas density were 100% in a square, how many particles would there be?
        virtual int GetMaxNumGasParticlesAt(int x,int y) const =0;

        // what is the velocity of the given location?
        virtual RealPoint GetVelocityAt(int x,int y) const =0;

        // get a text description of a state
        virtual string GetReport(state s) const =0;

        virtual void InsertRandomFlow(int x,int y)=0;
        virtual void InsertRandomBackwardFlow(int x,int y)=0;
        virtual void InsertRandomParticle(int x,int y)=0;

    protected: // functions

        // compute the average flow of the gas at regular intervals
        void ComputeFlow();

        void ResizeFlowSamples();

        void BringInside(int &x,int &y) const;

        state GetAt(int x,int y) const;
        void SetAt(int x,int y,state s);

    protected: // data

        int X;
        int Y;
        vector<vector<state> > grid[2]; // two buffers that get swapped each iteration
        int current_buffer,old_buffer;

        state BOUNDARY;

        int iterations;

        vector<state> forward_flow_samples; // we sample from this to bias the flow
        vector<state> backward_flow_samples; // (sometimes we use this too)
        int flow_sample_separation; // we compute the flow at sparse positions (X and Y should divide by this)
        vector<vector<RealPoint> > velocity; // instantaneous velocity measurement
        vector<vector<RealPoint> > averaged_velocity; // we keep a running average
        bool have_taken_first_velocity_average;
        RealPoint global_mean_velocity;

        bool force_flow; // are we forcing the flow by overwriting the leftmost column?
        
        bool need_redraw_images; // has anything changed since we last drew the images?
        bool need_recompute_flow; // has anything changed since we last computed the flow?

        // velocity computation flags
        int averaging_radius; // (usually equal to flow_sample_separation)
        TVelocityRepresentation velocity_representation;

        //wxPoint container_momentum; // we would like to monitor the total overall momentum, for bug checking
};

#endif
