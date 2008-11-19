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
#include <vector>
using namespace std;

// OpenMP:
#include <omp.h> // comment this out if you don't have OpenMP (you might also need to set USE_OMP=OFF in cmake)

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
    void OnUpdateStep(wxUpdateUIEvent& event);
    void OnRunOrStop(wxCommandEvent& event);
    void OnChangeLineLength(wxCommandEvent& event);
    void OnShowFlow(wxCommandEvent& event);
    void OnUpdateShowFlow(wxUpdateUIEvent& event);
    void OnShowFlowColours(wxCommandEvent& event);
    void OnUpdateShowFlowColours(wxUpdateUIEvent& event);
    void OnChangeAveragingRadius(wxCommandEvent& event);
    void OnSubtractMeanVelocity(wxCommandEvent& event);
    void OnUpdateSubtractMeanVelocity(wxUpdateUIEvent& event);
    void OnEddiesDemo(wxCommandEvent& event);
    void OnParticlesDemo(wxCommandEvent& event);
    void OnZoomIn(wxCommandEvent& event);
    void OnZoomOut(wxCommandEvent& event);
    void OnShowCurrentStats(wxCommandEvent& event);

private:
    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()

    void UpdateGas();
    void RedrawImages();
    void ComputeFlow();
    void ApplyHorizontalPairwiseInteraction(state &a,state &b);
    void ApplyVerticalPairwiseInteraction(state &a,state &b);

    void ResizeGrid(int x_size,int y_size);
    void ResetGridForEddiesExample();
    void ResetGridForParticlesExample();
    bool RequestZoomFactor(int num,int denom); // expressed as a fraction, will fail if resulting image is too big

    int X; // these must be even numbers, and divisible by flow_sample_separation
    int Y;
    vector<vector<state> > grid[2];
    vector<vector<wxRealPoint> > velocity;
    vector<vector<wxRealPoint> > averaged_velocity;
    bool have_taken_first_velocity_average;
    int current_buffer,old_buffer;
    int iterations;

    bool is_running;
    int running_step;

    // states: 0=empty, 1=rest particle, 2="x-mover", 3="y-mover", 4="diag-mover", 5=boundary
    state pi_table_horiz[5][5][2],pi_table_vert[5][5][2]; //  maps a,b onto pi_table[a][b][0],pi_table[a][b][1]

    static const int N_SAMPLES=3;
    static const state samples[N_SAMPLES]; // the distribution we used to initialize and force flow
    static const int flow_bias[2]; // we want to force flow in one direction (and a target density of 0.5)

    wxImage density_image;
    wxBitmap drawing_image; 
    wxMemoryDC drawing_buffer;
    bool need_redraw_images,need_recompute_flow;
    int zoom_factor_num,zoom_factor_denom; // expressed as a fraction
    int flow_sample_separation; // only need to compute the flow every so often (X and Y should divide by this)

    double line_length;
    bool show_flow,show_flow_colours;
    int averaging_radius;
    bool force_flow;
    bool subtract_mean_velocity;

    bool save_images;
    int n_saved_images; // we output with frame number for convenience with other packages

    wxPoint container_momentum; // we monitor the total overall momentum, for bug checking

};

// a small helper class to set the status text temporarily (while in scope)
class SetStatusTextHelper
{
    public:
        SetStatusTextHelper(wxString s,wxFrame* frame) 
        {
            // save the current text
            this->our_frame = frame;
            this->previous_status_text = frame->GetStatusBar()->GetStatusText(iPane);
            frame->SetStatusText(s,iPane);
        }
        ~SetStatusTextHelper()
        {
            // restore the original text
            this->our_frame->SetStatusText(this->previous_status_text,iPane);
        }
    private:
        static const int iPane = 2;
        wxFrame *our_frame;
        wxString previous_status_text;
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

    ID_GETTING_STARTED = wxID_HIGHEST,
    ID_STEP,
    ID_RUN_OR_STOP,
    ID_CHANGE_LINE_LENGTH,
    ID_SHOW_FLOW,
    ID_SHOW_FLOW_COLOURS,
    ID_CHANGE_AVERAGING_RADIUS,
    ID_SUBTRACT_MEAN_VELOCITY,
    ID_DEMO_EDDIES,
    ID_DEMO_PARTICLES,
    ID_ZOOM_IN,
    ID_ZOOM_OUT,
    ID_SHOW_CURRENT_STATS,
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
    EVT_UPDATE_UI(ID_STEP,MyFrame::OnUpdateStep)
    EVT_MENU(ID_RUN_OR_STOP,MyFrame::OnRunOrStop)
    EVT_MENU(ID_CHANGE_LINE_LENGTH,MyFrame::OnChangeLineLength)
    EVT_MENU(ID_SHOW_FLOW,MyFrame::OnShowFlow)
    EVT_UPDATE_UI(ID_SHOW_FLOW,MyFrame::OnUpdateShowFlow)
    EVT_MENU(ID_SHOW_FLOW_COLOURS,MyFrame::OnShowFlowColours)
    EVT_UPDATE_UI(ID_SHOW_FLOW_COLOURS,MyFrame::OnUpdateShowFlowColours)
    EVT_MENU(ID_CHANGE_AVERAGING_RADIUS,MyFrame::OnChangeAveragingRadius)
    EVT_MENU(ID_SUBTRACT_MEAN_VELOCITY,MyFrame::OnSubtractMeanVelocity)
    EVT_UPDATE_UI(ID_SUBTRACT_MEAN_VELOCITY,MyFrame::OnUpdateSubtractMeanVelocity)
    EVT_MENU(ID_DEMO_EDDIES,MyFrame::OnEddiesDemo)
    EVT_MENU(ID_DEMO_PARTICLES,MyFrame::OnParticlesDemo)
    EVT_PAINT(MyFrame::OnPaint)
    EVT_MENU(ID_ZOOM_IN,MyFrame::OnZoomIn)
    EVT_MENU(ID_ZOOM_OUT,MyFrame::OnZoomOut)
    EVT_MENU(ID_SHOW_CURRENT_STATS,MyFrame::OnShowCurrentStats)
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
        fileMenu->Append(Minimal_Quit, _T("E&xit\tAlt-F4"), _T("Quit this program"));
        menuBar->Append(fileMenu, _T("&File"));
    }

    // add the view menu
    {
        wxMenu *viewMenu = new wxMenu;
        viewMenu->Append(ID_ZOOM_IN,_T("Zoom in\t+"));
        viewMenu->Append(ID_ZOOM_OUT,_T("Zoom out\t-"));
        viewMenu->AppendSeparator();
        viewMenu->AppendCheckItem(ID_SHOW_FLOW,_T("Show the flow"),_T(""));
        viewMenu->Append(ID_CHANGE_LINE_LENGTH,_T("Change line length..."),_T(""));
        viewMenu->AppendCheckItem(ID_SHOW_FLOW_COLOURS,_T("Show flow colours"),_T(""));
        viewMenu->Append(ID_CHANGE_AVERAGING_RADIUS,_T("Change averaging radius..."),_T(""));
        viewMenu->AppendCheckItem(ID_SUBTRACT_MEAN_VELOCITY,_T("Subtract mean velocity"));
        viewMenu->AppendSeparator();
        viewMenu->Append(ID_SHOW_CURRENT_STATS,_T("Show statistics for this instant\tF2"));
        menuBar->Append(viewMenu, _T("&View"));
    }

    // add the actions menu
    {
        wxMenu *actionsMenu = new wxMenu;
        actionsMenu->Append(ID_STEP, _T("Step\tSpace"), _T("Advance one timestep"));
        actionsMenu->Append(ID_RUN_OR_STOP, _T("Run / Stop\tEnter"), _T("Start/stop running"));
        actionsMenu->AppendSeparator();
        // add a demos submenu
        {
            wxMenu *demosMenu = new wxMenu;
            demosMenu->Append(ID_DEMO_EDDIES,_T("Eddies in the wake of a cylinder"));
            demosMenu->Append(ID_DEMO_PARTICLES,_T("A few gas particles colliding"));
            actionsMenu->AppendSubMenu(demosMenu,_T("Reset with an inbuilt demo:"));
        }
        menuBar->Append(actionsMenu, _T("&Actions"));
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
    CreateStatusBar(3);
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
    ResetGridForEddiesExample();
}

void MyFrame::ResizeGrid(int x_size,int y_size)
{
    this->need_redraw_images = true;
    this->need_recompute_flow = true;
    this->iterations = 0;
    this->is_running = false;
    this->have_taken_first_velocity_average = false;
    this->n_saved_images = 1;
    this->current_buffer=0;
    this->old_buffer=1;
    this->container_momentum = wxPoint(0,0);

    this->X = x_size;
    this->Y = y_size;
    this->grid[0].assign(X,vector<state>(Y));
    this->grid[1].assign(X,vector<state>(Y));

    // initialize the velocity arrays
    this->velocity.assign(X/this->flow_sample_separation,
        vector<wxRealPoint>(Y/this->flow_sample_separation,wxRealPoint(0,0)));
    this->averaged_velocity.assign(X/this->flow_sample_separation,
        vector<wxRealPoint>(Y/this->flow_sample_separation,wxRealPoint(0,0)));

    // scale down the visible grid until we it is sensible to show
    {
        // try: 10,9,...,2,1,1/2,1/3,1/4,...
        int zn=10,zd=1;
        int ms = max(X,Y);
        while((ms*zn)/zd>1000)
        {
            if(zn>1) zn--;
            else zd++;
        }
        RequestZoomFactor(zn,zd);
    }
}

bool MyFrame::RequestZoomFactor(int num,int denom)
{
    // just check this wouldn't make too big an image (could only draw the bit we need to show, in future)
    {
        long int n_pixels = (this->X * num / denom)*(this->Y * num / denom);
        if(n_pixels>10e6) return false;
    }
    this->zoom_factor_num = num;
    this->zoom_factor_denom = denom;

    // resize the images we draw into
    this->drawing_image.Create(this->X * this->zoom_factor_num / this->zoom_factor_denom, 
        this->Y * this->zoom_factor_num / this->zoom_factor_denom);
    this->density_image.Create(this->X * this->zoom_factor_num / this->zoom_factor_denom, 
        this->Y * this->zoom_factor_num / this->zoom_factor_denom);

    // select a bitmap into the drawing buffer
    this->drawing_buffer.SelectObject(this->drawing_image);

    if(this->zoom_factor_denom>1)
        SetStatusText(wxString::Format(_T("Zoom: 1/%d"),this->zoom_factor_denom),1);
    else
        SetStatusText(wxString::Format(_T("Zoom: %d"),this->zoom_factor_num),1);

    return true;
}

void MyFrame::ResetGridForEddiesExample()
{
    this->subtract_mean_velocity = false;
    this->line_length = 400;
    this->show_flow_colours = true;
    this->averaging_radius = 32;
    this->flow_sample_separation = 20;
    this->show_flow = true;
    this->force_flow = true;
    this->save_images = false;
    this->running_step = 50;

    this->ResizeGrid(2000,1000);

    for(int x=0;x<X;x++)
    {
        for(int y=0;y<Y;y++)
        {
            if(sqrt(pow(x-X/8,2.0)+pow(y-Y/2,2.0))<100)
                this->grid[current_buffer][x][y] = 5; // obstacle
            else
                this->grid[current_buffer][x][y] = ((rand()%100)<flow_bias[x%2])?samples[rand()%N_SAMPLES]:0; // flow
        }
    }
}

void MyFrame::ResetGridForParticlesExample()
{
    this->subtract_mean_velocity = false;
    this->line_length = 1;
    this->show_flow_colours = true;
    this->averaging_radius = 0;
    this->flow_sample_separation = 1;
    this->show_flow = true;
    this->force_flow = false;
    this->save_images = false;
    this->running_step = 1;

    this->ResizeGrid(70,50);

    for(int x=0;x<X;x++)
    {
        for(int y=0;y<Y;y++)
        {
            if(sqrt(pow(x-X/8,2.0)+pow(y-Y/2,2.0))<4)
                this->grid[current_buffer][x][y] = 5; // obstacle
            else
                this->grid[current_buffer][x][y] = ((rand()%200)==0)?(rand()%5):0; // sparse atoms
        }
    }
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
Copyright (C) 2008 Tim J. Hutton <tim.hutton@gmail.com> - http://www.sq3.org.uk\n\n\
Project homepage: http://code.google.com/p/latticegas\n\
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
    SetStatusTextHelper sth(_T("Redrawing images..."),this);

    for(int x=0;x<X;x++)
    {
        for(int y=0;y<Y;y++)
        {
            state s = grid[current_buffer][x][y];
            if(this->zoom_factor_denom==1)
            {
                // when the cells themselves are big enough to see, colour them by their direction
                wxColour c;
                if(s==0) c=wxColour(0,0,0);
                else if(s==1) c=wxColour(200,200,200);
                else if(s==5) c=wxColour(120,120,120);
                else {
                    wxRealPoint v;
                    switch(s)
                    {
                        default: break;
                        case 2: if(x%2) v=wxRealPoint(1,0); else v=wxRealPoint(-1,0); break;
                        case 3: if(y%2) v=wxRealPoint(0,1); else v=wxRealPoint(0,-1); break;
                        case 4: if(x%2 && y%2) v=wxRealPoint(1,1); else if(x%2 && !(y%2)) v=wxRealPoint(1,-1);
                                else if(!(x%2) && y%2) v=wxRealPoint(-1,1); else v=wxRealPoint(-1,-1); break;
                    }
                    double angle = 0.5 + atan2(v.y,v.x)/(2*3.14159265358979);
                    wxImage::RGBValue rgb = wxImage::HSVtoRGB(wxImage::HSVValue(angle,1.0,1.0));
                    c = wxColour(rgb.red,rgb.green,rgb.blue);
                }
                for(int i=x * this->zoom_factor_num / this->zoom_factor_denom;i<(x+1) * this->zoom_factor_num / this->zoom_factor_denom;i++)
                {
                    for(int j=y*this->zoom_factor_num / this->zoom_factor_denom;j<(y+1)*this->zoom_factor_num / this->zoom_factor_denom;j++)
                    {
                        // we check bounds in case something has changed during drawing
                        if(i>=0&&i<density_image.GetWidth()&&j>=0&&j<density_image.GetHeight())
                            density_image.SetRGB(i,j,c.Red(),c.Green(),c.Blue());
                    }
                }
            }
            else
            {
                // when zoomed out beyond 1 pixel per cell, just draw the obstacles
                wxColour c = (s==5)?wxColour(120,120,120):wxColour(255,255,255);
                int i = x * this->zoom_factor_num / this->zoom_factor_denom;
                int j = y * this->zoom_factor_num / this->zoom_factor_denom;
                // we check bounds in case something has changed during drawing
                if(i>=0&&i<density_image.GetWidth()&&j>=0&&j<density_image.GetHeight())
                    density_image.SetRGB(i,j,c.Red(),c.Green(),c.Blue());
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
                angle = 0.5 + atan2(v.y,v.x)/(2*3.14159265358979);
                wxImage::RGBValue rgb = wxImage::HSVtoRGB(wxImage::HSVValue(angle,1.0,1.0));
                if(this->show_flow_colours)
                    this->drawing_buffer.SetPen(wxPen(wxColour(rgb.red,rgb.green,rgb.blue)));
                this->drawing_buffer.DrawLine((x+0.5) * this->zoom_factor_num / this->zoom_factor_denom,
                    (y+0.5) * this->zoom_factor_num / this->zoom_factor_denom,
                    (x+0.5+v.x*this->line_length)*this->zoom_factor_num / this->zoom_factor_denom,
                    (y+0.5+v.y*this->line_length)*this->zoom_factor_num / this->zoom_factor_denom);
            }
        }
    }

    this->need_redraw_images = false;
}

void MyFrame::OnPaint(wxPaintEvent& event)
{
    if(this->need_redraw_images)
        RedrawImages();

    wxPaintDC dc(this);
    dc.Blit(0,0,X*this->zoom_factor_num / this->zoom_factor_denom,
        Y*this->zoom_factor_num / this->zoom_factor_denom,&this->drawing_buffer,0,0);

    SetStatusText(wxString::Format(_T("%d iterations"),this->iterations),0);

    if(this->is_running)
    {
        if(this->save_images)
        {
            SetStatusTextHelper sth(_T("Saving image snapshot..."),this);
            this->drawing_image.SaveFile(wxString::Format(_T("%04d.jpg"),this->n_saved_images++),wxBITMAP_TYPE_JPEG);
        }
        static bool warm_up = false;
        if(warm_up)
        {
            SetStatusTextHelper sth(_T("Performing warm up..."),this);
            for(int i=0;i<50;i++) 
            {
                for(int j=0;j<60;j++) 
                    this->UpdateGas();
                ComputeFlow(); // we want to find the averaged_velocity
            }
            warm_up=false;
        }
        {
            SetStatusTextHelper sth(_T("Performing gas calculations..."),this);
            for(int i=0;i<this->running_step && this->is_running;i++) 
            {
                this->UpdateGas();
                wxYield(); // allow the user to interrupt
            }
        } // (without these scope limits, the screen doesn't update on linux)
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

void MyFrame::UpdateGas()
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

    // (this is a poor implementation, should be a lot more efficient)

    // start with an empty grid
    for(int x=0;x<X;x++)
        for(int y=0;y<Y;y++)
            this->grid[current_buffer][x][y]=0;

    #pragma omp parallel for
    for(int x=0;x<X;x++)
    {
        // (need to declare these things here else omp causes problems)
        int sx,sy;
        state s,s2;
        for(int y=0;y<Y;y++)
        {
            s = this->grid[old_buffer][x][y];
            if(s==5) 
                this->grid[current_buffer][x][y]=5;      // boundaries don't change
            else if(s>0)
            {
                // try simple transport
                if(x%2) sx=x+2; else sx=x-2;
                if(y%2) sy=y+2; else sy=y-2;
                if(sx>=X) sx-=X; else if(sx<0) sx+=X; // left-right wraps around
                if(sy>=Y) sy-=3; else if(sy<0) sy+=3; // top-bottom bounces (no-slip)
                s2 = this->grid[old_buffer][sx][sy];
                if(s2!=5)
                    this->grid[current_buffer][sx][sy]=s; // simple transport
                else {
                    // try the square in between
                    if(x%2) sx=x+1; else sx=x-1;
                    if(y%2) sy=y+1; else sy=y-1;
                    if(sx>=X) sx-=X; else if(sx<0) sx+=X; // left-right wraps around
                    if(sy>=Y) sy-=1; else if(sy<0) sy+=1; // top-bottom bounces (no-slip)
                    s2 = this->grid[old_buffer][sx][sy];
                    if(s2!=5)
                        this->grid[current_buffer][sx][sy]=s; // bounce transport
                    else
                    {
                        // try to bounce back
                        if(x%2) sx=x-1; else sx=x+1;
                        if(y%2) sy=y-1; else sy=y+1;
                        if(sx>=X) sx-=X; else if(sx<0) sx+=X; // left-right wraps around
                        if(sy>=Y) sy-=1; else if(sy<0) sy+=1; // top-bottom bounces (no-slip)
                        s2 = this->grid[old_buffer][sx][sy];
                        if(s2!=5)
                            this->grid[current_buffer][sx][sy]=s; // bounce back transport
                        else
                            this->grid[current_buffer][x][y]=s; // particles is trapped here!
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
                if(this->grid[old_buffer][x][y]==5)
                    this->grid[current_buffer][x][y] = 5;
                else
                    this->grid[current_buffer][x][y] = ((rand()%100)<flow_bias[x%2])?samples[rand()%N_SAMPLES]:0;
            }
        }
    }
    this->iterations++;
    this->need_recompute_flow = true;
    this->need_redraw_images = true;
}

void MyFrame::ComputeFlow()
{
    SetStatusTextHelper sth(_T("Computing average flow patterns..."),this);
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
            for(int dx=max(0,x-R);dx<=min(X-1,x+R);dx++) // (do we need to include an even mix of the sides of the 2x2 cells?)
            {
                for(int dy=max(0,y-R);dy<=min(Y-1,y+R);dy++)
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
    this->UpdateGas();
    this->Refresh(false);
}

void MyFrame::OnUpdateStep(wxUpdateUIEvent& event)
{
    event.Enable(!this->is_running);
}

void MyFrame::OnRunOrStop(wxCommandEvent& /*event*/)
{
    this->is_running = !this->is_running;
    this->Refresh(false);
}

void MyFrame::OnChangeLineLength(wxCommandEvent& /*event*/)
{
    wxGetTextFromUser(_T("Enter the line length:"),_T("Line lengths"),
        wxString::Format(_T("%.0f"),this->line_length)).ToDouble(&this->line_length);
    this->need_redraw_images = true;
    this->Refresh(false);
}

void MyFrame::OnShowFlowColours(wxCommandEvent& /*event*/)
{
    this->show_flow_colours = !this->show_flow_colours;
    this->need_redraw_images = true;
    this->Refresh(false);
}

void MyFrame::OnUpdateShowFlowColours(wxUpdateUIEvent& event)
{
    event.Check(this->show_flow_colours);
}

void MyFrame::OnSubtractMeanVelocity(wxCommandEvent& /*event*/)
{
    this->subtract_mean_velocity = !this->subtract_mean_velocity;
    this->need_redraw_images = true;
    this->Refresh(false);
}

void MyFrame::OnUpdateSubtractMeanVelocity(wxUpdateUIEvent& event)
{
    event.Check(this->subtract_mean_velocity);
}

void MyFrame::OnChangeAveragingRadius(wxCommandEvent& /*event*/)
{
    long int ar;
    wxGetTextFromUser(_T("Enter the averaging radius:"),_T("Averaging radius"),
        wxString::Format(_T("%d"),this->averaging_radius)).ToLong(&ar);
    this->averaging_radius = ar;
    this->need_recompute_flow = this->need_redraw_images = true;
    this->Refresh(false);
}

void MyFrame::OnGettingStarted(wxCommandEvent& /*event*/)
{
    wxMessageBox(_T("Set the simulation running (press 'Enter' to start and stop). The fluid flows around the obstacle.\n\n\
After a few thousand timesteps the flow has become unstable, with alternating vortices being shed. \
Turn on 'subtract mean velocity' and increase the line length to 2000 to better see them.\n\n\
Try the other demos (Actions menu) to see more.\n\n\
This is an implementation of the pair-interaction lattice gas cellular automata. Search the web for \
more information on lattice gases.\n\n\
For more advanced changes, explore the source code."),_T("Getting started"),wxOK | wxICON_INFORMATION,
                 this);
}

void MyFrame::OnEddiesDemo(wxCommandEvent& /*event*/)
{
    this->ResetGridForEddiesExample();
    this->Refresh(true);
}

void MyFrame::OnParticlesDemo(wxCommandEvent& /*event*/)
{
    this->ResetGridForParticlesExample();
    this->Refresh(true);
}

void MyFrame::OnZoomIn(wxCommandEvent& /*event*/)
{
    bool ret;
    if(this->zoom_factor_denom==1)
        ret = this->RequestZoomFactor(this->zoom_factor_num+1,this->zoom_factor_denom);
    else 
        ret = this->RequestZoomFactor(this->zoom_factor_num,this->zoom_factor_denom-1);
    if(!ret)
    {
        wxMessageBox(_T("Resulting image too large, can't zoom in any further."));
        return;
    }
    this->need_redraw_images = true;
    this->Refresh(true);
}

void MyFrame::OnZoomOut(wxCommandEvent& /*event*/)
{
    if(this->zoom_factor_num>1) this->RequestZoomFactor(this->zoom_factor_num-1,this->zoom_factor_denom);
    else this->RequestZoomFactor(this->zoom_factor_num,this->zoom_factor_denom+1);
    this->need_redraw_images = true;
    this->Refresh(true);
}

void MyFrame::OnShowFlow(wxCommandEvent& /*event*/)
{
    this->show_flow = !this->show_flow;
    this->need_redraw_images = true;
    this->Refresh(true);
}

void MyFrame::OnUpdateShowFlow(wxUpdateUIEvent& event)
{
    event.Check(this->show_flow);
}

void MyFrame::OnShowCurrentStats(wxCommandEvent& /*event*/)
{
    int n_boundary_cells=0,n_gas_particles=0;
    int x_momentum=0,y_momentum=0;
    for(int x=0;x<X;x++)
    {
        for(int y=0;y<Y;y++)
        {
            state s = this->grid[current_buffer][x][y];
            if(s==5) n_boundary_cells++;
            else if(s>0)
            {
                n_gas_particles++;
                if((s==2||s==4) && (x%2)==1) x_momentum++;
                else if((s==2||s==4) && (x%2)==0) x_momentum--;
                else if((s==3||s==4) && (y%2)==1) y_momentum++;
                else if((s==3||s==4) && (y%2)==0) y_momentum--;
            }
        }
    }
    wxMessageBox(wxString::Format(_T("Size: (%d,%d)\nBoundary cells: %d\nGas particles: %d\nTotal gas momentum: (%d,%d)"),
        X,Y,n_boundary_cells,n_gas_particles,x_momentum,y_momentum));
}
