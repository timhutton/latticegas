/*
    Lattice Gas Explorer
    Copyright (C) 2008 Tim J. Hutton <tim.hutton@gmail.com>

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

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
 
#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

// STL:
#include <algorithm>
#include <vector>
#include <sstream>
#include <iomanip>
using namespace std;

// OpenMP:
//#include <omp.h> // comment this out if you don't have OpenMP (you might also need to set USE_OMP=OFF in cmake)


// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
};

typedef unsigned char state;

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
public:
    // ctor(s)
    MyFrame(const wxString& title);

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
	void OnGettingStarted(wxCommandEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnStep(wxCommandEvent& event);
    void OnStart(wxCommandEvent& event);
    void OnStop(wxCommandEvent& event);
    void OnChangeLineLength(wxCommandEvent& event);
    void OnShowFlowColours(wxCommandEvent& event);
    void OnChangeAveragingRadius(wxCommandEvent& event);
    void OnSubtractMeanVelocity(wxCommandEvent& event);

private:
    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()

	void ResetGrid(int x_size,int y_size);
    void Update();
    void RedrawImages();
    void ComputeFlow();
    void ApplyHorizontalPairwiseInteraction(state &a,state &b);
    void ApplyVerticalPairwiseInteraction(state &a,state &b);

    int X; // these must be even numbers, and divisible by flow_sample_separation
    int Y;
    vector<vector<state> > grid[2];
    vector<vector<wxRealPoint> > velocity;
    vector<vector<wxRealPoint> > averaged_velocity;
    bool have_taken_first_velocity_average;
    int current_buffer,old_buffer;
    int iterations;

    bool is_running;

    // states: 0=empty, 1=rest particle, 2="x-mover", 3="y-mover", 4="diag-mover", 5=boundary
    state pi_table_horiz[5][5][2],pi_table_vert[5][5][2]; //  maps a,b onto pi_table[a][b][0],pi_table[a][b][1]

    static const int N_SAMPLES=3;
    static const state samples[N_SAMPLES]; // the distribution we used to initialize and force flow
    static const int flow_bias[2]; // we want to force flow in one direction (and a target density of 0.5)

    wxImage density_image;
    wxBitmap drawing_image; 
    wxMemoryDC drawing_buffer;
    bool need_redraw_images,need_recompute_flow;
    float zoom_factor;
    int flow_sample_separation; // only need to compute the flow every so often (X and Y should divide by this)

    double line_length;
	bool put_obstacle;
    bool show_flow,show_flow_colours;
    int averaging_radius;
    bool force_flow;
    bool subtract_mean_velocity;

	bool save_images;
    int n_saved_images; // we output with frame number for convenience with other packages

};
// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

const state MyFrame::samples[N_SAMPLES] = {2,3,4}; 
const int MyFrame::flow_bias[2] = { 0,100 };

// IDs for the controls and the menu commands
enum
{
    // menu items
    Minimal_Quit = wxID_EXIT,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Minimal_About = wxID_ABOUT,

	ID_GETTING_STARTED,
    ID_STEP,
    ID_RUN,
    ID_STOP,
    ID_CHANGE_LINE_LENGTH,
    ID_SHOW_FLOW_COLOURS,
    ID_CHANGE_AVERAGING_RADIUS,
	ID_SUBTRACT_MEAN_VELOCITY,
};

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(Minimal_Quit,  MyFrame::OnQuit)
    EVT_MENU(Minimal_About, MyFrame::OnAbout)
	EVT_MENU(ID_GETTING_STARTED,MyFrame::OnGettingStarted)
    EVT_MENU(ID_STEP,MyFrame::OnStep)
    EVT_MENU(ID_RUN,MyFrame::OnStart)
    EVT_MENU(ID_STOP,MyFrame::OnStop)
    EVT_MENU(ID_CHANGE_LINE_LENGTH,MyFrame::OnChangeLineLength)
    EVT_MENU(ID_SHOW_FLOW_COLOURS,MyFrame::OnShowFlowColours)
    EVT_MENU(ID_CHANGE_AVERAGING_RADIUS,MyFrame::OnChangeAveragingRadius)
	EVT_MENU(ID_SUBTRACT_MEAN_VELOCITY,MyFrame::OnSubtractMeanVelocity)
    EVT_PAINT(MyFrame::OnPaint)
END_EVENT_TABLE()

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if ( !wxApp::OnInit() )
        return false;

    // create the main application window
    MyFrame *frame = new MyFrame(_T("Lattice Gas Explorer"));

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    frame->Show(true);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// vertical pair interactions are the same as horizontal ones with states 2 and 3 swapped, so we use this function
int swap23(int x) { if(x==2) return 3; else if(x==3) return 2; else return x; }

// frame constructor
MyFrame::MyFrame(const wxString& title)
       : wxFrame(NULL, wxID_ANY, title)
{

#if wxUSE_MENUS

    // create a menu bar
    wxMenuBar *menuBar = new wxMenuBar();

    // add the file menu
    {
        wxMenu *fileMenu = new wxMenu;
        fileMenu->Append(Minimal_Quit, _T("E&xit\tAlt-X"), _T("Quit this program"));
        menuBar->Append(fileMenu, _T("&File"));
    }

    // add the actions menu
    {
        wxMenu *actionsMenu = new wxMenu;
        actionsMenu->Append(ID_STEP, _T("Step\tSPACE"), _T("Advance one timestep"));
        actionsMenu->Append(ID_RUN, _T("Run\tENTER"), _T("Start running"));
        actionsMenu->Append(ID_STOP, _T("Stop\tCtrl-Enter"), _T("Stop running"));
        menuBar->Append(actionsMenu, _T("&Actions"));
    }

    // add the options menu
    {
        wxMenu *optionsMenu = new wxMenu;
        optionsMenu->Append(ID_CHANGE_LINE_LENGTH,_T("Change line length"),_T(""));
        optionsMenu->Append(ID_SHOW_FLOW_COLOURS,_T("Show flow colours"),_T(""));
        optionsMenu->Append(ID_CHANGE_AVERAGING_RADIUS,_T("Change averaging radius"),_T(""));
		optionsMenu->Append(ID_SUBTRACT_MEAN_VELOCITY,_T("Subtract mean velocity"));
        menuBar->Append(optionsMenu, _T("&Options"));
    }

    // add the help menu
    {
        wxMenu *helpMenu = new wxMenu;
        helpMenu->Append(ID_GETTING_STARTED, _T("&Getting started\tF1"), _T("Show some information"));
        helpMenu->AppendSeparator();
        helpMenu->Append(Minimal_About, _T("&About...\tAlt-F1"), _T("Show about dialog"));
        menuBar->Append(helpMenu, _T("&Help"));
    }

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);

#endif // wxUSE_MENUS

#if wxUSE_STATUSBAR
    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(2);
    SetStatusText(_T(""));
#endif // wxUSE_STATUSBAR

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

	srand((unsigned int)time(NULL));
    wxInitAllImageHandlers();
	ResetGrid(2000,1000);
}

void MyFrame::ResetGrid(int x_size,int y_size)
{
	this->X = x_size;
	this->Y = y_size;
	this->grid[0].assign(X,vector<state>(Y));
	this->grid[1].assign(X,vector<state>(Y));

	this->need_redraw_images = true;
	this->need_recompute_flow = true;
	this->subtract_mean_velocity = false;
	this->line_length = 400;
	this->show_flow_colours = true;
	this->averaging_radius = 64;
	this->iterations = 0;
	this->is_running = false;
	this->flow_sample_separation = 20;
	this->have_taken_first_velocity_average = false;
	this->n_saved_images = 1;
	this->show_flow = true;
	this->force_flow = true;
	this->put_obstacle = true;
	this->save_images = false;

    this->current_buffer=0;
    this->old_buffer=1;
    for(int x=0;x<X;x++)
    {
        for(int y=0;y<Y;y++)
        {
            if(put_obstacle && sqrt(pow(x-X/8,2.0)+pow(y-Y/2,2.0))<60)
                this->grid[current_buffer][x][y] = 5; // obstacle
            else if(force_flow)
                this->grid[current_buffer][x][y] = ((rand()%100)<flow_bias[x%2])?samples[rand()%N_SAMPLES]:0; // flow
			else
				this->grid[current_buffer][x][y] = (rand()%100==0)?samples[rand()%N_SAMPLES]:0; // sparse atoms
        }
    }

	// initialize the velocity arrays
    this->velocity.assign(X/this->flow_sample_separation,
        vector<wxRealPoint>(Y/this->flow_sample_separation,wxRealPoint(0,0)));
    this->averaged_velocity.assign(X/this->flow_sample_separation,
        vector<wxRealPoint>(Y/this->flow_sample_separation,wxRealPoint(0,0)));

	this->zoom_factor=10;
	// scale down the visible grid until we it is sensible to show
	{
		int ms = max(X,Y);
		while(ms*this->zoom_factor>1000.0)
		{
			if(this->zoom_factor>1.0) this->zoom_factor -= 1.0;
			else this->zoom_factor /= 2.0;
		}
	}
    this->drawing_image.Create(this->X * this->zoom_factor, this->Y * this->zoom_factor);
    this->density_image.Create(this->X * this->zoom_factor, this->Y * this->zoom_factor);

    // select a bitmap into the drawing buffer
    this->drawing_buffer.SelectObject(this->drawing_image);
}


// event handlers

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(_T("Lattice Gas Explorer\n\
Copyright (C) 2008 Tim J. Hutton <tim.hutton@gmail.com>\n\
http://www.sq3.org.uk\n\
\n\
This program is free software: you can redistribute it and/or modify \
it under the terms of the GNU General Public License as published by \
the Free Software Foundation, either version 3 of the License, or \
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \
GNU General Public License for more details.\
\n\
You should have received a copy of the GNU General Public License \
along with this program.  If not, see <http://www.gnu.org/licenses/>."),
                 _T("About Lattice Gas Explorer"),
                 wxOK | wxICON_INFORMATION,
                 this);
}

void MyFrame::RedrawImages()
{
    //int colour;
    int s;
    // draw the density image
    //const wxColour colours[6] =  {wxColour(0,0,0),wxColour(200,200,200),wxColour(0,255,255),
    //    wxColour(255,255,0),wxColour(255,255,255),wxColour(255,0,0)};
    const wxColour colours[6] =  {wxColour(255,255,255),wxColour(255,255,255),wxColour(255,255,255),
        wxColour(255,255,255),wxColour(255,255,255),wxColour(120,120,120)};
    for(int x=0;x<X;x++)
    {
        for(int y=0;y<Y;y++)
        {
            s = grid[current_buffer][x][y];
            //colour=(s>0)?255:0;
            //colour = ((s&NE)?63:0) + ((s&SE)?63:0) + ((s&SW)?63:0) + ((s&NW)?63:0);
            //density_image.SetRGB(x,y,colour,colour,colour);
            if(this->zoom_factor>1.0)
            {
                wxRealPoint v;
                wxColour c;
                switch(s)
                {
                    default: break;
                    case 2: if(x%2) v=wxRealPoint(1,0); else v=wxRealPoint(-1,0); break;
                    case 3: if(y%2) v=wxRealPoint(0,1); else v=wxRealPoint(0,-1); break;
                    case 4: if(x%2 && y%2) v=wxRealPoint(1,1); else if(x%2 && !(y%2)) v=wxRealPoint(1,-1);
                            else if(!(x%2) && y%2) v=wxRealPoint(-1,1); else v=wxRealPoint(-1,-1); break;
                }
                if(s==0) c=wxColour(0,0,0);
                else if(s==1) c=wxColour(200,200,200);
                else {
                    double angle = atan2(v.y,v.x)/(2*3.14159265358979);
                    wxImage::RGBValue rgb = wxImage::HSVtoRGB(wxImage::HSVValue(angle,1.0,1.0));
                    c = wxColour(rgb.red,rgb.green,rgb.blue);
                }
                for(int i=x * this->zoom_factor;i<(x+1) * this->zoom_factor;i++)
                {
                    for(int j=y*this->zoom_factor;j<(y+1)*this->zoom_factor;j++)
                    {
                        density_image.SetRGB(i,j,c.Red(),c.Green(),c.Blue());
                    }
                }
            }
            else
            {
                density_image.SetRGB(x * this->zoom_factor,y * this->zoom_factor,
                    colours[s].Red(),colours[s].Green(),colours[s].Blue());
            }
        }
    }
    this->drawing_buffer.DrawBitmap(wxBitmap(density_image),0,0);

	if(show_flow)
	{
		// draw the flow image
		if(need_recompute_flow)
			ComputeFlow();

		// draw flow vectors
		double angle;
		//dc.SetBrush(*wxWHITE_BRUSH);
		//dc.DrawRectangle(0,0,X,Y);
		if(!this->show_flow_colours)
			this->drawing_buffer.SetPen(*wxBLACK_PEN);
		for(int x=this->flow_sample_separation;x<X-this->flow_sample_separation;x+=this->flow_sample_separation)
		{
			for(int y=this->flow_sample_separation;y<Y-this->flow_sample_separation;y+=this->flow_sample_separation)
			{
				int sx=x/this->flow_sample_separation,sy=y/this->flow_sample_separation;
				wxRealPoint v = velocity[sx][sy];
				if(this->subtract_mean_velocity)
				{
					// we subtract the averaged velocity at this point, to better highlight the dynamic changes
					v.x -= averaged_velocity[sx][sy].x;
					v.y -= averaged_velocity[sx][sy].y;
				}
				angle = 0.5+atan2(v.y,v.x)/(2*3.14159265358979);
				wxImage::RGBValue rgb = wxImage::HSVtoRGB(wxImage::HSVValue(angle,1.0,1.0));
				if(this->show_flow_colours)
					this->drawing_buffer.SetPen(wxPen(wxColour(rgb.red,rgb.green,rgb.blue)));
				this->drawing_buffer.DrawLine(x * this->zoom_factor,y * this->zoom_factor,
					(x+v.x*this->line_length)*this->zoom_factor,(y+v.y*this->line_length)*this->zoom_factor);
			}
		}
	}

    this->need_redraw_images = false;
}

void MyFrame::OnPaint(wxPaintEvent& /*event*/)
{
    if(this->need_redraw_images)
        RedrawImages();

    wxPaintDC dc(this);
    dc.Blit(0,0,X*this->zoom_factor,Y*this->zoom_factor,&this->drawing_buffer,0,0);

    wxLogStatus(_T("%d iterations"),this->iterations);

    if(this->is_running)
    {
		if(this->save_images)
		{
			wxLogStatus(_T("Saving image snapshot..."));
			ostringstream oss;
			oss << setw(4) << setfill('0') << this->n_saved_images++ << ".jpg";
			this->drawing_image.SaveFile(wxString(oss.str().c_str(),wxConvUTF8),wxBITMAP_TYPE_JPEG);
		}
        static bool warm_up = false;
        if(warm_up)
        {
            wxLogStatus(_T("Performing warm up..."));
            for(int i=0;i<50;i++) 
            {
                for(int j=0;j<60;j++) 
                    this->Update();
                ComputeFlow(); // we want to find the averaged_velocity
            }
            warm_up=false;
        }
        wxLogStatus(_T("Performing gas calculations..."));
        for(int i=0;i<100;i++) 
            this->Update();
        this->Refresh(false);
    }
}

void MyFrame::ApplyHorizontalPairwiseInteraction(state &a,state &b)
{
    if(a==5 || b==5) return; // no interactions at boundaries

    state temp_a = this->pi_table_horiz[a][b][0];
    b = this->pi_table_horiz[a][b][1];
    a = temp_a;
}

void MyFrame::ApplyVerticalPairwiseInteraction(state &a,state &b)
{
    if(a==5 || b==5) return; // no interactions at boundaries

    state temp_a = this->pi_table_vert[a][b][0];
    b = this->pi_table_vert[a][b][1];
    a = temp_a;
}

void MyFrame::Update()
{
    current_buffer = old_buffer;
    old_buffer = 1-current_buffer;

    // -- phase 1: pairwise interactions, in x then y --

    #pragma omp parallel for
    for(int x=0;x<X;x+=2) // (we assume X is even)
        for(int y=0;y<Y;y++)
            ApplyHorizontalPairwiseInteraction(this->grid[old_buffer][x][y],this->grid[old_buffer][x+1][y]);
    #pragma omp parallel for
    for(int x=0;x<X;x++)
        for(int y=0;y<Y;y+=2) // (we assume Y is even)
            ApplyVerticalPairwiseInteraction(this->grid[old_buffer][x][y],this->grid[old_buffer][x][y+1]);

    // -- phase 2: simple transport --

    if(this->force_flow)
    {
        // the left-most column is overwritten, since we are modelling flow in an infinite tube
        for(int x=0;x<2;x++)
        {
            for(int y=0;y<Y;y++)
            {
                this->grid[current_buffer][x][y] = ((rand()%100)<flow_bias[x%2])?samples[rand()%N_SAMPLES]:0;
            }
        }
    }
    #pragma omp parallel for
    for(int x=(this->force_flow?2:0);x<X;x++)
    {
        // (need to declare these things here else omp causes problems)
        int sx,sy; // the cell coming here
        state s;
        for(int y=0;y<Y;y++)
        {
            if(this->grid[old_buffer][x][y]==5) 
                this->grid[current_buffer][x][y]=5;      // boundaries don't change
            else
            {
                if(x%2 && y%2) { sx=x-2; sy=y-2; }
                else if(x%2 && !(y%2)) { sx=x-2; sy=y+2; }
                else if(!(x%2) && y%2) { sx=x+2; sy=y-2; }
                else { sx=x+2; sy=y+2; }
                // left-right wraps around
                if(sx<0) sx+=X;
                else if(sx>=X) sx-=X;
                // top and bottom reflect (slip)
                if(sy<0) sy+=1; // (these assume Y%2==0)
                else if(sy>=Y) sy-=1;
                // retrieve the source state
                s = this->grid[old_buffer][sx][sy];
                // if coming from a boundary then take the input from the bouncing atom instead (no-slip)
                if(s==5)
                    s = this->grid[old_buffer][(x+sx)/2][(y+sy)/2];
                // copy the cell over
                this->grid[current_buffer][x][y] = s;
            }
        }
    }
    this->iterations++;
    this->need_recompute_flow = true;
    this->need_redraw_images = true;
}

void MyFrame::ComputeFlow()
{
    wxLogStatus(_T("Computing average flow patterns..."));
    // recompute the locally-averaged velocities
    const int R = this->averaging_radius;
    const double avF=0.95; // for a running average we take a weighted mix of the previous average and the new value
    #pragma omp parallel for
    for(int x=this->flow_sample_separation;x<(X-this->flow_sample_separation);x+=this->flow_sample_separation)
    {
        for(int y=this->flow_sample_separation;y<(Y-this->flow_sample_separation);y+=this->flow_sample_separation)
        {
            wxRealPoint v(0,0);
            int n_counted = 0;
            for(int dx=max(0,x-R);dx<=min(X-1,x+R+1);dx++)
            {
                for(int dy=max(0,y-R);dy<=min(Y-1,y+R+1);dy++)
                {
                    state s = grid[current_buffer][dx][dy];
                    switch(s)
                    {
                        default: break;
                        case 2: if(dx%2) v=v+wxRealPoint(1,0); else v=v+wxRealPoint(-1,0); break;
                        case 3: if(dy%2) v=v+wxRealPoint(0,1); else v=v+wxRealPoint(0,-1); break;
                        case 4: if(dx%2 && dy%2) v=v+wxRealPoint(1,1); else if(dx%2 && !(dy%2)) v=v+wxRealPoint(1,-1);
                                else if(!(dx%2) && dy%2) v=v+wxRealPoint(-1,1); else v=v+wxRealPoint(-1,-1); break;
                    }
                    n_counted++;
                }
            }
            int sx=x/this->flow_sample_separation,sy=y/this->flow_sample_separation;
            velocity[sx][sy] = wxRealPoint(v.x / n_counted, v.y / n_counted);
            // take a running average of the velocity too
            if(!this->have_taken_first_velocity_average)
            {
                averaged_velocity[sx][sy] = wxRealPoint(velocity[sx][sy].x,velocity[sx][sy].y);
            }
            else
            {
                averaged_velocity[sx][sy] = wxRealPoint(averaged_velocity[sx][sy].x * avF + velocity[sx][sy].x * (1.0-avF),
                    averaged_velocity[sx][sy].y * avF + velocity[sx][sy].y * (1.0-avF));
            }
        }
    }
    if(!this->have_taken_first_velocity_average)
        this->have_taken_first_velocity_average = true;
    this->need_recompute_flow = false;
}

void MyFrame::OnStep(wxCommandEvent& /*event*/)
{
    this->Update();
    this->Refresh(false);
}

void MyFrame::OnStart(wxCommandEvent& /*event*/)
{
    this->is_running = true;
    this->Refresh(false);
}

void MyFrame::OnStop(wxCommandEvent& /*event*/)
{
    this->is_running = false;
    this->Refresh(false);
}

void MyFrame::OnChangeLineLength(wxCommandEvent& /*event*/)
{
    ostringstream oss;
    oss << this->line_length;
    wxGetTextFromUser(_T("Enter the line length:"),_T("Line lengths"),
        wxString(oss.str().c_str(),wxConvUTF8)).ToDouble(&this->line_length);
    this->need_redraw_images = true;
    this->Refresh(false);
}

void MyFrame::OnShowFlowColours(wxCommandEvent& /*event*/)
{
    this->show_flow_colours = !this->show_flow_colours;
    this->need_redraw_images = true;
    this->Refresh(false);
}

void MyFrame::OnSubtractMeanVelocity(wxCommandEvent& /*event*/)
{
    this->subtract_mean_velocity = !this->subtract_mean_velocity;
    this->need_redraw_images = true;
    this->Refresh(false);
}

void MyFrame::OnChangeAveragingRadius(wxCommandEvent& /*event*/)
{
    ostringstream oss;
    oss << this->averaging_radius;
    long int ar;
    wxGetTextFromUser(_T("Enter the averaging radius:"),_T("Averaging radius"),
        wxString(oss.str().c_str(),wxConvUTF8)).ToLong(&ar);
    this->averaging_radius = ar;
    this->need_recompute_flow = this->need_redraw_images = true;
    this->Refresh(false);
}

void MyFrame::OnGettingStarted(wxCommandEvent& /*event*/)
{
	wxMessageBox(_T("Set the simulation running (Actions menu : Run). The fluid flows around the obstacle.\n\n\
After a few thousand timesteps the flow has become unstable, with alternating vortices being shed. \
Turn on 'subtract mean velocity' and increase the line length to 4000 to better see them.\n\n\
This is an implementation of the pair-interaction lattice gas cellular automata. Search the web for \
more information on lattice gases.\n\n\
For more advanced changes, explore the source code."),_T("Getting started"),wxOK | wxICON_INFORMATION,
                 this);
}