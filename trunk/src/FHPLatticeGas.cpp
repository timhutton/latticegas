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

#include "FHPLatticeGas.h"

// STL:
#include <stdexcept>
#include <exception>
#include <sstream>
using namespace std;

// simple helper function: takes n states and assembles them into a vector<state>
vector<BaseLatticeGas::state> FHPLatticeGas::Vec(int n, ...)
{
    va_list ap;
    int j;
    vector<state> v;
    va_start(ap, n); // Requires the last fixed parameter (to get the address)
    for(j=0; j<n; j++) v.push_back(va_arg(ap,int)); // (increments ap)
    // (va_arg requires the type to cast to, but gcc warns that state
    //  (currently unsigned char) gets promoted to int anyway, so we use int
    //  and let it get cast back to state on insertion into v.)
    va_end(ap);
    return v;
}

FHPLatticeGas::FHPLatticeGas(FHP_type type) : fhp_type(type)
{
    this->BOUNDARY = 128;
    
    // We're following:
    // [1] Wylie, 1990:       http://pages.cs.wisc.edu/~wylie/doc/PhD_thesis.pdf
    // [2] Arai et al., 2007: http://www.fz-juelich.de/nic-series/volume38/arai.pdf
    // (see also: [3] Wolf-Gladrow, 2000: http://epic.awi.de/Publications/Wol2000c.pdf)

    // N.B. Interestingly, [2] and [3] seem to miss out several FHP collisions,
    //  e.g. [2] has REST+NE+SE+W and NW+NE+SW+SE in different classes, likewise E+W+REST and NE+SE+W
    //  (also in [2] the last transition in Fig. 6 is misprinted (mass conservation error) and again in 
    //   Procedure 3 in Fig. 7)
    //  e.g. [3] misses NE+SE+W <-> E+W+REST

    // A "collision class" [2] is a set of states that can be swapped at will, without
    // affecting the mass or momentum of that node. For best results, a gas should be
    // "collision-saturated" - it swaps everything that can be swapped.

    // these are some possible collision classes to choose from:
    typedef vector<state> TStates;
    // first four from Fig. 1.6 in [1], for FHP6
    TStates pair_head_on = Vec(3, E+W, NE+SW, NW+SE); // i) "linear"
    TStates symmetric_3 = Vec(2, E+NW+SW, W+NE+SE); // ii) "triple"
    TStates two_plus_spectator[6] = { // iii) "lambda"
        Vec(2, E+SW+NE, E+SE+NW), Vec(2, SE+E+W, SE+NE+SW), Vec(2, SW+E+W, SW+NW+SE),
        Vec(2, W+NW+SE, W+NE+SW), Vec(2, NW+E+W, NW+NE+SW), Vec(2, NE+E+W, NE+NW+SE) };
    TStates four_particles = Vec(3, NE+NW+SE+SW, E+W+SE+NW, E+W+NE+SW); // iv) "dual-linear"
    // next ones from Fig. 1.9 in [1], for FHP7
    TStates pair_head_on_plus_rest = // ii) and iii) "triple" and "linear+rest"
        Vec(5, E+W+REST, NE+SW+REST, NW+SE+REST, E+NW+SW, W+NE+SE);
    TStates pair_head_on_plus_rest_no_triple = // iii) "linear+rest" (used in FHP-II)
        Vec(3, E+W+REST, NE+SW+REST, NW+SE+REST);
    TStates one_plus_rest[6] = { // iv) and v) "fundamental+rest" and "jay"
        Vec(2, E+REST, SE+NE), Vec(2, NE+REST, E+NW), Vec(2, NW+REST, W+NE),
        Vec(2, W+REST, NW+SW), Vec(2, SW+REST, W+SE), Vec(2, SE+REST, SW+E) };
    TStates two_plus_spectator_including_rest[6] = { // vi) and (vii) "lambda" and "jay+rest"
        Vec(3, E+SW+NE, E+SE+NW, NE+SE+REST), Vec(3, SE+E+W, SE+NE+SW, E+SW+REST), 
        Vec(3, SW+E+W, SW+NW+SE, W+SE+REST),  Vec(3, W+NW+SE, W+NE+SW, NW+SW+REST), 
        Vec(3, NW+E+W, NW+NE+SW, NE+W+REST),  Vec(3, NE+E+W, NE+NW+SE, NW+E+REST) };
    TStates four_particles_including_rest_no_momentum = // viii) and ix) "dual-linear" and "dual-triple + rest"
        Vec(5, NE+NW+SE+SW, E+W+SE+NW, E+W+NE+SW, E+NW+SW+REST, W+NE+SE+REST );
    TStates symmetric_3_plus_rest = Vec(2, E+NW+SW+REST, W+NE+SE+REST); // "dual-triple + rest" (used in FHP-II)
    TStates four_particles_plus_rest = Vec(3, NE+NW+SE+SW+REST, E+W+SE+NW+REST, E+W+NE+SW+REST); // x) "dual-linear + rest"
    TStates five_particles_including_rest_momentum_one[6] = { // xi) and xii) "dual-fundamental" and "dual-jay + rest"
        Vec(2, NE+NW+W+SW+SE, E+W+NW+SW+REST), Vec(2, E+NE+NW+W+SW, NW+SE+NE+W+REST),
        Vec(2, SE+E+NE+NW+W, SW+NE+E+NW+REST), Vec(2, SW+SE+E+NE+NW, W+E+NE+SE+REST),
        Vec(2, W+SW+SE+E+NE, NW+SE+E+SW+REST), Vec(2, NW+W+SW+SE+E, NE+SW+W+SE+REST) };
    TStates two_plus_spectator_plus_rest[6] = { // xiii) and xiv) "dual-lambda + rest" and "dual-jay"
        Vec(3, E+SW+NE+REST, E+SE+NW+REST, NE+SE+E+W), Vec(3, SE+E+W+REST, SE+NE+SW+REST, E+SW+SE+NW), 
        Vec(3, SW+E+W+REST, SW+NW+SE+REST, W+SE+SW+NE),  Vec(3, W+NW+SE+REST, W+NE+SW+REST, NW+SW+W+E), 
        Vec(3, NW+E+W+REST, NW+NE+SW+REST, NE+W+NW+SE),  Vec(3, NE+E+W+REST, NE+NW+SE+REST, NW+E+NE+SW) };
    // now select which of these collision classes we're going to use:
    switch(type)
    {
        case FHP_I:
            this->collision_classes.push_back(pair_head_on);
            this->collision_classes.push_back(symmetric_3);
            break;
        case FHP_II:
            this->collision_classes.push_back(pair_head_on);
            this->collision_classes.push_back(symmetric_3);
            this->collision_classes.push_back(pair_head_on_plus_rest_no_triple);
            this->collision_classes.push_back(symmetric_3_plus_rest);
            for(int i=0;i<6;i++)
                this->collision_classes.push_back(one_plus_rest[i]);
            break;
        case FHP_III: // FHP7, collision-saturated
            this->collision_classes.push_back(pair_head_on);
            this->collision_classes.push_back(pair_head_on_plus_rest);
            for(int i=0;i<6;i++)
            {
                this->collision_classes.push_back(one_plus_rest[i]);
                this->collision_classes.push_back(two_plus_spectator_including_rest[i]);
            }
            this->collision_classes.push_back(four_particles_including_rest_no_momentum);
            this->collision_classes.push_back(four_particles_plus_rest);
            for(int i=0;i<6;i++)
            {
                this->collision_classes.push_back(five_particles_including_rest_momentum_one[i]);
                this->collision_classes.push_back(two_plus_spectator_plus_rest[i]);
            }
            break;
        case FHP_6: // FHP6, collision-saturated
            this->collision_classes.push_back(pair_head_on);
            this->collision_classes.push_back(symmetric_3);
            for(int i=0;i<6;i++) 
            {
                this->collision_classes.push_back(two_plus_spectator[i]);
            }
            this->collision_classes.push_back(four_particles);
            break;
    }
    // optional debug checks:
    if(true)
    {
        int n_cases_swapped = 0;
        for(vector<TStates>::iterator it=this->collision_classes.begin();it!=this->collision_classes.end();it++)
            n_cases_swapped += it->size();
        // should be: FHP-I: 5 of 64, FHP6: 20 of 64, FHP-II: 22 of 128, FHP-III: 76 of 128
        try 
        {
            // check that all these collision cases maintain momentum and mass
            VerifyCollisionMap();
            // check that these collision cases are complete: any other swaps possible?
            if(this->fhp_type==FHP_III || this->fhp_type==FHP_6) // (only these expected to be complete)
                VerifyIsCollisionSaturated();
        }
        catch(runtime_error e)
        {
            wxASSERT_MSG(false,e.what());
            throw e;
        }
    }
    InitializeCollisionMap();

    // create the flow_samples array
    //state samples[] = { NE, SE, E, NE, SE, E, NE, SE, E, NE, SE, E, NW, SW, W }; 
    state samples[] = { NE, SE, E, W, E};
    this->flow_samples.assign(samples,samples+sizeof(samples)/sizeof(state));
}

void FHPLatticeGas::InsertRandomFlow(int x,int y)
{
    this->grid[current_buffer][x][y] = this->flow_samples[rand()%this->flow_samples.size()];
}

void FHPLatticeGas::InsertRandomParticle(int x,int y)
{
    this->grid[current_buffer][x][y] = ((rand()%50)==0)?(1<<(rand()%N_DIRS)):0;
}

void FHPLatticeGas::UpdateGas()
{
    this->RandomizeCollisionMap(); 

    current_buffer = old_buffer;
    old_buffer = 1-current_buffer;

    const vector<vector<state> > &OldBuffer = this->grid[old_buffer];
    vector<vector<state> > &NewBuffer = this->grid[current_buffer];

    int from_x = force_flow?1:0;

    #pragma omp parallel for
    for(int y=0;y<Y;y++)
    {
        const vector<vector<int> > &nbors = NBORS[y%2]; // alternate rows are indented (see HexGridLatticeGas)
        state new_state,nbor;
        int dir,opposite_dir;
        for(int x=from_x;x<X;x++)
        {
            const state& c = OldBuffer[x][y];
            if(c==BOUNDARY) continue;
            new_state = c & REST;
            for(dir=0,opposite_dir=N_DIRS/2;dir<N_DIRS;dir++,opposite_dir=(opposite_dir+1)%N_DIRS) 
                // (this is the innermost loop: optimize here!)
            {
                nbor = OldBuffer[(x+nbors[opposite_dir][0]+X)%X][(y+nbors[opposite_dir][1]+Y)%Y];
                if(nbor!=BOUNDARY)
                {
                    // accept an inbound particle travelling in this direction, if there is one
                    new_state |= nbor&(1<<dir);
                }
                else if(c&(1<<opposite_dir))
                {
                    // or if the neighbor is a boundary then reverse one of our own particles
                    new_state |= 1<<dir;
                }
            }
            // apply the collisions remapping, and store the new value
            NewBuffer[x][y] = this->collision_map[new_state];
        }
    }

    if(force_flow)
    {
        state s;
        for(int y=0;y<Y;y++)
        {
            // the left-most column gets overwritten randomly, since we are simulating
            // an infinite tube filled with moving gas
            s = OldBuffer[0][y];
            if(s==BOUNDARY) continue;
            s = this->flow_samples[rand()%this->flow_samples.size()];
            NewBuffer[0][y]=s;
        }
    }


    this->iterations++;
    this->need_recompute_flow = true;
    this->need_redraw_images = true;
}

RealPoint FHPLatticeGas::GetAverageInputFlowVelocityPerParticle() const
{
    RealPoint flow(0,0);
    int n_particles_counted = 0;
    for(int i=0;i<(int)this->flow_samples.size();i++)
    {
        state s = this->flow_samples[i];
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
        if(s&REST) n_particles_counted++;
    }
    return RealPoint(flow.x / n_particles_counted,flow.y / n_particles_counted);
}

float FHPLatticeGas::GetAverageInputNumParticlesPerCell() const
{
    int n_particles_counted = 0;
    for(int i=0;i<(int)this->flow_samples.size();i++)
    {
        state s = this->flow_samples[i];
        for(int i=0;i<7;i++)
        {
            if(s&(1<<i))
            {
                n_particles_counted++;
                // (the complication here is that we allow up to four particles
                //  per square, and we want the average particle speed, not the
                //  average speed in each square)
            }
        }
    }
    return n_particles_counted / (float)this->flow_samples.size();
}

int FHPLatticeGas::GetNumGasParticlesAt(int x, int y) const
{
    state s = this->grid[current_buffer][x][y];
    int n_gas_particles = 0;
    for(int i=0;i<7;i++)
        if(s & (1<<i))
            n_gas_particles++;
    return n_gas_particles;
}

int FHPLatticeGas::GetMaxNumGasParticlesAt(int x, int y) const
{
    state s = this->grid[current_buffer][x][y];
    if(s==BOUNDARY) return 0;
    else {
        if(this->fhp_type==FHP_II || this->fhp_type==FHP_III)
            return 7; // (these gases include a rest particle)
        else
            return 6;
    }
}

RealPoint FHPLatticeGas::GetVelocityAt(int x, int y) const
{
    return GetVelocity(grid[current_buffer][x][y]);
}

wxColour FHPLatticeGas::GetColour(int x, int y) const
{
    state s = this->grid[current_buffer][x][y];
    wxColour c;
    if(s==0) c=wxColour(0,0,0);
    else if(s==BOUNDARY) c=wxColour(120,120,120);
    else {
        if(this->show_gas_colours)
        {
            if(s==REST) 
                c=wxColour(200,200,200);
            else{
                RealPoint v = GetVelocity(s);
                c = GetVectorAngleColour(v.x,v.y);
            }
        }
        else
        {
            float density = GetNumGasParticlesAt(x,y) / (float)GetMaxNumGasParticlesAt(x,y);
            c = GetDensityColour(density);
        }
    }
    return c;
}

void FHPLatticeGas::InitializeCollisionMap()
{
    for(state s=0;s<=BOUNDARY;s++)
        this->collision_map[s] = s; // default: no change
    for(int iClass=0;iClass<(int)this->collision_classes.size();iClass++)
    {
        for(int iCollision=0;iCollision<(int)this->collision_classes[iClass].size();iCollision++)
        {
            int input = this->collision_classes[iClass][iCollision];
            int output = this->collision_classes[iClass][(iCollision+1)%this->collision_classes[iClass].size()];
            // (each collision state becomes the next one in its class)
            this->collision_map[input] = output;
        }
    }
}

void FHPLatticeGas::RandomizeCollisionMap()
{
    for(int iClass=0;iClass<(int)this->collision_classes.size();iClass++)
    {
        int nCollisions = this->collision_classes[iClass].size();
        if(nCollisions>2) // no need to randomize outcome of collision classes with just two entries
        {
            int move = 1 + (rand()%(nCollisions-1)); // each i will become i+move mod n
            for(int iCollision=0;iCollision<nCollisions;iCollision++)
            {
                int input = this->collision_classes[iClass][iCollision];
                int output = this->collision_classes[iClass][(iCollision+move)%nCollisions];
                this->collision_map[input] = output;
            }
        }
    }
}

// each transition must maintain mass and momentum
void FHPLatticeGas::VerifyCollisionMap()
{
    state max_state;
    switch(this->fhp_type)
    {
        case FHP_II: 
        case FHP_III: 
            max_state=128; // including a rest particle
            break;
        case FHP_I:
        case FHP_6: 
            max_state=64; // no rest particle
            break;
    }
    vector<bool> checked(max_state,false);
    for(int iClass=0;iClass<(int)this->collision_classes.size();iClass++)
    {
        state s1 = this->collision_classes[iClass][0];
        if(s1<0 || s1>=max_state)
        {
            ostringstream oss;
            oss << "Range error: " << GetReport(s1);
            throw runtime_error(oss.str());
        }
        // also verify that each state appears in the collision cases at most once
        if(checked[s1])
        {
            ostringstream oss;
            oss << "Duplication error: " << GetReport(s1);
            throw runtime_error(oss.str());
        }
        checked[s1]=true;
        for(int iCollision=1;iCollision<(int)this->collision_classes[iClass].size();iCollision++)
        {
            state s2 = this->collision_classes[iClass][iCollision];
            if(s2<0 || s2>=max_state)
            {
                ostringstream oss;
                oss << "Range error: " << GetReport(s2);
                throw runtime_error(oss.str());
            }
            if(checked[s2])
            {
                ostringstream oss;
                oss << "Duplication error: " << GetReport(s2);
                throw runtime_error(oss.str());
            }
            checked[s2]=true;
            RealPoint v1,v2;
            int mass1,mass2;
            // find the mass and momentum of s1 and s2
            mass1 = (s1 & REST)?1:0;
            mass2 = (s2 & REST)?1:0;
            for(int dir=0;dir<N_DIRS;dir++)
            {
                if(s1 & (1<<dir))
                {
                    v1 += RealPoint(DIR[dir][0],DIR[dir][1]);
                    mass1++;
                }
                if(s2 & (1<<dir))
                {
                    v2 += RealPoint(DIR[dir][0],DIR[dir][1]);
                    mass2++;
                }
            }    
            double d = fabs(v1.x-v2.x)+fabs(v1.y-v2.y);
            if(mass1!=mass2)
            {
                ostringstream oss;
                oss << "Mass conservation error: " << GetReport(s1) << "->" << GetReport(s2);
                throw runtime_error(oss.str());
            }
            else if(d>1E-08)
            {
                ostringstream oss;
                oss << "Momentum conservation error: " << GetReport(s1) << "->" << GetReport(s2);
                throw runtime_error(oss.str());
            }
        }
    }
}

void FHPLatticeGas::VerifyIsCollisionSaturated()
{
    state max_state;
    switch(this->fhp_type)
    {
        default:
        case FHP_III: max_state=128; break; // including a rest particle
        case FHP_6: max_state=64; break; // no rest particle
        // (no other gas types are expected to be collision-saturated)
    }
    // for each collision class: is there any other state that should be in that class?
    vector<bool> checked(max_state,false);
    for(int iClass=0;iClass<(int)this->collision_classes.size();iClass++)
    {
        for(int iCollision=1;iCollision<(int)this->collision_classes[iClass].size();iCollision++)
        {
            state s1 = this->collision_classes[iClass][0];
            checked[s1]=true;
            RealPoint v1;
            int mass1;
            // find the mass and momentum of s1
            mass1 = (s1 & REST)?1:0;
            for(int dir=0;dir<N_DIRS;dir++)
            {
                if(s1 & (1<<dir))
                {
                    v1 += RealPoint(DIR[dir][0],DIR[dir][1]);
                    mass1++;
                }
            }    
            // now check every state not in class, to see if it has the same mass and momentum
            for(state s2=0;s2<max_state;s2++)
            {
                if(find(this->collision_classes[iClass].begin(),this->collision_classes[iClass].end(),s2)!=this->collision_classes[iClass].end())
                {
                    checked[s2]=true;
                    continue; // this state already in the class
                }
                RealPoint v2;
                int mass2 = (s2 & REST)?1:0;
                for(int dir=0;dir<N_DIRS;dir++)
                {
                    if(s2 & (1<<dir))
                    {
                        v2 += RealPoint(DIR[dir][0],DIR[dir][1]);
                        mass2++;
                    }
                }    
                double d = fabs(v1.x-v2.x)+fabs(v1.y-v2.y);
                if(mass1==mass2 && d<1E-08)
                {
                    ostringstream oss;
                    oss << "Missing case: " << GetReport(s1) << "<-->" << GetReport(s2);
                    throw runtime_error(oss.str());
                }
            }
        }
    }
    // for every state not in a collision class: is there any other state with equal mass and momentum?
    for(state s1=0;s1<max_state;s1++)
    {
        if(checked[s1]) continue;
        // find the mass and momentum of s1
        RealPoint v1;
        int mass1 = (s1 & REST)?1:0;
        for(int dir=0;dir<N_DIRS;dir++)
        {
            if(s1 & (1<<dir))
            {
                v1 += RealPoint(DIR[dir][0],DIR[dir][1]);
                mass1++;
            }
        }    
        for(state s2=0;s2<max_state;s2++)
        {
            if(s2==s1 || checked[s2]) continue;
            // find the mass and momentum of s2
            RealPoint v2;
            int mass2 = (s2 & REST)?1:0;
            for(int dir=0;dir<N_DIRS;dir++)
            {
                if(s2 & (1<<dir))
                {
                    v2 += RealPoint(DIR[dir][0],DIR[dir][1]);
                    mass2++;
                }
            }    
            double d = fabs(v1.x-v2.x)+fabs(v1.y-v2.y);
            if(mass1==mass2 && d<1E-08)
            {
                ostringstream oss;
                oss << "Missing case: " << GetReport(s1) << "<->" << GetReport(s2);
                throw runtime_error(oss.str());
            }
        }
    }
}

void FHPLatticeGas::ResetGridForParticlesExample()
{
    BaseLatticeGas_drawable::ResetGridForParticlesExample();
    /*ResizeGrid(40,30);

    // two particles colliding head on
    SetAt(3,4,SW);
    SetAt(2,6,NE);
    SetAt(2,5,REST);
    //SetAt(2,10,E);
    //SetAt(4,10,W);
    //SetAt(2,12,SE);
    //SetAt(3,14,NW);
    // two particles collide with spectator
   // SetAt(
   */
}

string FHPLatticeGas::GetReport(state s) const
{
    ostringstream oss;
    bool first=true;
    const string dir_labels[N_DIRS]={"E","SE","SW","W","NW","NE"};
    for(int dir=0;dir<N_DIRS;dir++)
    {
        if(s&(1<<dir))
        {
            if(!first) oss << "+";
            first=false;
            oss << dir_labels[dir];
        }
    }
    if(s&REST)
    {
        if(!first) oss << "+";
        first=false;
        oss << "REST";
    }
    return oss.str();
}
