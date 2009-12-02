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

// wxWidgets
#include "wxWidgetsPreamble.h"

// local:
#include "LatticeGasFactory.h"

// STL:
#include <sstream>
using namespace std;

// OpenMP
#include <omp.h>

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

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
public:
    // ctor(s)
    MyFrame(const wxString& title);
    // dtor
    ~MyFrame();

    void OnPaint(wxPaintEvent& event);
    // file menu
    void OnQuit(wxCommandEvent& event);
    // view menu
    void OnZoomIn(wxCommandEvent& event);
    void OnZoomOut(wxCommandEvent& event);
    void OnShowGas(wxCommandEvent& event);
    void OnUpdateShowGas(wxUpdateUIEvent& event);
    void OnShowGasColours(wxCommandEvent& event);
    void OnUpdateShowGasColours(wxUpdateUIEvent& event);
    void OnShowGrid(wxCommandEvent& event);
    void OnUpdateShowGrid(wxUpdateUIEvent& event);
    void OnShowFlow(wxCommandEvent& event);
    void OnUpdateShowFlow(wxUpdateUIEvent& event);
    void OnShowFlowColours(wxCommandEvent& event);
    void OnUpdateShowFlowColours(wxUpdateUIEvent& event);
    void OnChangeLineLength(wxCommandEvent& event);
    void OnUpdateChangeLineLength(wxUpdateUIEvent& event);
    void OnChangeAveragingRadius(wxCommandEvent& event);
    void OnUpdateChangeAveragingRadius(wxUpdateUIEvent& event);
    void OnSubtractMeanVelocity(wxCommandEvent& event);
    void OnUpdateSubtractMeanVelocity(wxUpdateUIEvent& event);
    // actions menu
    void OnStep(wxCommandEvent& event);
    void OnUpdateStep(wxUpdateUIEvent& event);
    void OnRunOrStop(wxCommandEvent& event);
    void OnParticlesDemo(wxCommandEvent& event);
    void OnUpdateParticlesDemo(wxUpdateUIEvent& event);
    void OnObstacleDemo(wxCommandEvent& event);
    void OnUpdateObstacleDemo(wxUpdateUIEvent& event);
    void OnHoleDemo(wxCommandEvent& event);
    void OnUpdateHoleDemo(wxUpdateUIEvent& event);
    void OnChangeToGasTypeN(wxCommandEvent& event);
    void OnUpdateChangeToGasTypeN(wxUpdateUIEvent& event);
    void OnGetReport(wxCommandEvent& event);
    // help menu
    void OnAbout(wxCommandEvent& event);
    void OnGettingStarted(wxCommandEvent& event);

private:
    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()

    bool is_running;
    int running_step;

    int current_gas_type;
    BaseLatticeGas_drawable *gas;

    int current_demo;
    void LoadCurrentDemo();
};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
    Minimal_Quit = wxID_EXIT,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Minimal_About = wxID_ABOUT,

    // view menu:

    ID_ZOOM_IN = wxID_HIGHEST,
    ID_ZOOM_OUT,

    ID_SHOW_GAS,
    ID_SHOW_GAS_COLOURS,
    ID_SHOW_GRID,

    ID_SHOW_FLOW,
    ID_SHOW_FLOW_COLOURS,
    ID_CHANGE_LINE_LENGTH,
    ID_CHANGE_AVERAGING_RADIUS,
    ID_SUBTRACT_MEAN_VELOCITY,

    // actions menu:

    ID_STEP,
    ID_RUN_OR_STOP,
    
    ID_DEMO_PARTICLES,
    ID_DEMO_OBSTACLE,
    ID_DEMO_HOLE,

    ID_GAS_TYPE_0,
    ID_MAX_GAS_TYPE = ID_GAS_TYPE_0 + 100, // "should be enough for anyone"

    ID_GET_REPORT,

    // help menu:

    ID_GETTING_STARTED,
};

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_PAINT(MyFrame::OnPaint)
    // file menu:
    EVT_MENU(Minimal_Quit,  MyFrame::OnQuit)
    // view menu:
    EVT_MENU(ID_ZOOM_IN,MyFrame::OnZoomIn)
    EVT_MENU(ID_ZOOM_OUT,MyFrame::OnZoomOut)
    EVT_MENU(ID_SHOW_GAS,MyFrame::OnShowGas)
    EVT_UPDATE_UI(ID_SHOW_GAS,MyFrame::OnUpdateShowGas)
    EVT_MENU(ID_SHOW_GAS_COLOURS,MyFrame::OnShowGasColours)
    EVT_UPDATE_UI(ID_SHOW_GAS_COLOURS,MyFrame::OnUpdateShowGasColours)
    EVT_MENU(ID_SHOW_GRID,MyFrame::OnShowGrid)
    EVT_UPDATE_UI(ID_SHOW_GRID,MyFrame::OnUpdateShowGrid)
    EVT_MENU(ID_SHOW_FLOW,MyFrame::OnShowFlow)
    EVT_UPDATE_UI(ID_SHOW_FLOW,MyFrame::OnUpdateShowFlow)
    EVT_MENU(ID_SHOW_FLOW_COLOURS,MyFrame::OnShowFlowColours)
    EVT_UPDATE_UI(ID_SHOW_FLOW_COLOURS,MyFrame::OnUpdateShowFlowColours)
    EVT_MENU(ID_CHANGE_LINE_LENGTH,MyFrame::OnChangeLineLength)
    EVT_UPDATE_UI(ID_CHANGE_LINE_LENGTH,MyFrame::OnUpdateChangeLineLength)
    EVT_MENU(ID_CHANGE_AVERAGING_RADIUS,MyFrame::OnChangeAveragingRadius)
    EVT_UPDATE_UI(ID_CHANGE_AVERAGING_RADIUS,MyFrame::OnUpdateChangeAveragingRadius)
    EVT_MENU(ID_SUBTRACT_MEAN_VELOCITY,MyFrame::OnSubtractMeanVelocity)
    EVT_UPDATE_UI(ID_SUBTRACT_MEAN_VELOCITY,MyFrame::OnUpdateSubtractMeanVelocity)
    // actions menu:
    EVT_MENU(ID_STEP,MyFrame::OnStep)
    EVT_UPDATE_UI(ID_STEP,MyFrame::OnUpdateStep)
    EVT_MENU(ID_RUN_OR_STOP,MyFrame::OnRunOrStop)
    EVT_MENU(ID_DEMO_PARTICLES,MyFrame::OnParticlesDemo)
    EVT_UPDATE_UI(ID_DEMO_PARTICLES,MyFrame::OnUpdateParticlesDemo)
    EVT_MENU(ID_DEMO_OBSTACLE,MyFrame::OnObstacleDemo)
    EVT_UPDATE_UI(ID_DEMO_OBSTACLE,MyFrame::OnUpdateObstacleDemo)
    EVT_MENU(ID_DEMO_HOLE,MyFrame::OnHoleDemo)
    EVT_UPDATE_UI(ID_DEMO_HOLE,MyFrame::OnUpdateHoleDemo)
    // (no gas types here; we manually add them in the constructor)
    EVT_MENU(ID_GET_REPORT,MyFrame::OnGetReport)
    // help menu:
    EVT_MENU(ID_GETTING_STARTED,MyFrame::OnGettingStarted)
    EVT_MENU(Minimal_About, MyFrame::OnAbout)
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
        viewMenu->AppendCheckItem(ID_SHOW_GAS,_T("Show gas"),_T(""));
        viewMenu->AppendCheckItem(ID_SHOW_GAS_COLOURS,_T("Show gas colours"),_T(""));
        viewMenu->AppendCheckItem(ID_SHOW_GRID,_T("Show grid"),_T(""));
        viewMenu->AppendSeparator();
        viewMenu->AppendCheckItem(ID_SHOW_FLOW,_T("Show the flow"),_T(""));
        viewMenu->AppendCheckItem(ID_SHOW_FLOW_COLOURS,_T("Show flow colours"),_T(""));
        viewMenu->Append(ID_CHANGE_LINE_LENGTH,_T("Change flow line length..."),_T(""));
        viewMenu->Append(ID_CHANGE_AVERAGING_RADIUS,_T("Change flow averaging radius..."),_T(""));
        viewMenu->AppendCheckItem(ID_SUBTRACT_MEAN_VELOCITY,_T("Subtract mean velocity"));
        menuBar->Append(viewMenu, _T("&View"));
    }

    // add the actions menu
    {
        wxMenu *actionsMenu = new wxMenu;
        actionsMenu->Append(ID_STEP, _T("Step\tSpace"), _T("Advance one timestep"));
        actionsMenu->Append(ID_RUN_OR_STOP, _T("Run / Stop\tEnter"), _T("Start/stop running"));
        actionsMenu->AppendSeparator();
        // add a demo type submenu
        {
            wxMenu *demosMenu = new wxMenu;
            demosMenu->AppendRadioItem(ID_DEMO_PARTICLES,_T("A few gas particles colliding"));
            demosMenu->AppendRadioItem(ID_DEMO_OBSTACLE,_T("Eddies in the wake of an obstacle"));
            demosMenu->AppendRadioItem(ID_DEMO_HOLE,_T("Flow through a hole"));
            actionsMenu->AppendSubMenu(demosMenu,_T("Demo type:"));
        }
        // add a gas type submenu
        {
            wxMenu *gasTypeMenu = new wxMenu;
            if(ID_GAS_TYPE_0+LatticeGasFactory::GetNumGasTypesSupported() > ID_MAX_GAS_TYPE)
                wxMessageBox(_T("Internal error: need more gas type IDs!"));
            for(int i=0;i<LatticeGasFactory::GetNumGasTypesSupported();i++)
            {
                // add the item to the submenu and manually connect the events
                gasTypeMenu->AppendRadioItem(ID_GAS_TYPE_0+i,wxString(LatticeGasFactory::GetGasDescription(i),wxConvUTF8));
                Connect(ID_GAS_TYPE_0+i,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(MyFrame::OnChangeToGasTypeN));
                Connect(ID_GAS_TYPE_0+i,wxEVT_UPDATE_UI,wxUpdateUIEventHandler(MyFrame::OnUpdateChangeToGasTypeN));
                // (alternative to using the static event table above)
            }
            actionsMenu->AppendSubMenu(gasTypeMenu,_T("Gas type:"));
        }
        actionsMenu->Append(ID_GET_REPORT,_T("Report gas details"));
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

    srand((unsigned int)time(NULL));
    wxInitAllImageHandlers();
    
    this->SetSize(wxSize(600,600));

    this->current_gas_type = 6;
    this->gas = LatticeGasFactory::CreateGas(this->current_gas_type);
    this->current_demo = 1;
    this->LoadCurrentDemo();
}

void MyFrame::LoadCurrentDemo()
{
    switch(this->current_demo)
    {
        case 0: 
            this->gas->ResetGridForParticlesExample(); 
            this->running_step = 1;
            break;
        case 1: 
            this->gas->ResetGridForObstacleExample(); 
            this->running_step = 10;
            break;
        case 2: 
            this->gas->ResetGridForHoleExample(); 
            this->running_step = 10;
            break;
    }
    this->is_running = false;
    this->Refresh(true);
}

MyFrame::~MyFrame()
{
   delete this->gas;
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
Copyright (C) 2008-9 Tim J. Hutton <tim.hutton@gmail.com>\nhttp://www.sq3.org.uk\n\n\
Project homepage: http://code.google.com/p/latticegas\n\
\n\
This program is free software: you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation, either version 3 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program.  If not, see <http://www.gnu.org/licenses/>."),
                 _T("About Lattice Gas Explorer"),
                 wxOK | wxICON_INFORMATION,
                 this);
}

void MyFrame::OnPaint(wxPaintEvent& /*event*/)
{
    wxBusyCursor busy;

    SetStatusText(_T("Redrawing images..."),2);
    // BUG: this message doesn't show up on linux, only on Windows

    wxPaintDC dc(this);
    this->gas->Draw(dc);

    SetStatusText(wxString::Format(_T("%d iterations"),this->gas->GetIterations()),0);

    // display the current zoom setting
    {
        int num,denom;
        this->gas->GetZoom(num,denom);
        if(denom==1)
            SetStatusText(wxString::Format(_T("Zoom: %d"),num),1);
        else
            SetStatusText(wxString::Format(_T("Zoom: 1/%d"),denom),1);
    }

    if(this->is_running)
    {
        // cause some computation to happen
        SetStatusText(_T("Performing gas calculations..."),2);
        for(int i=0;i<this->running_step;i++) 
            this->gas->UpdateGas();
        // trigger a repaint
        this->Refresh(false);
    }
    else
        SetStatusText(_T("Stopped - press Enter to start/stop."),2);
}

void MyFrame::OnStep(wxCommandEvent& /*event*/)
{
    this->gas->UpdateGas();
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
    double old_line_length = this->gas->GetLineLength(),new_line_length;
    wxString ret = wxGetTextFromUser(_T("Enter the line length:"),_T("Line lengths"),
        wxString::Format(_T("%.0f"),old_line_length));
    if(ret.IsEmpty()) return; // user cancelled
    ret.ToDouble(&new_line_length);
    if(new_line_length!=old_line_length) // (double comparison...)
    {
        this->gas->SetLineLength(new_line_length);
        this->Refresh(false);
    }
}

void MyFrame::OnUpdateChangeLineLength(wxUpdateUIEvent& event)
{
    event.Enable(this->gas->GetShowFlow());
}

void MyFrame::OnShowFlowColours(wxCommandEvent& /*event*/)
{
    this->gas->SetShowFlowColours(!this->gas->GetShowFlowColours());
    this->Refresh(false);
}

void MyFrame::OnUpdateShowFlowColours(wxUpdateUIEvent& event)
{
    event.Enable(this->gas->GetShowFlow());
    event.Check(this->gas->GetShowFlowColours());
}

void MyFrame::OnShowGrid(wxCommandEvent& /*event*/)
{
    this->gas->SetShowGrid(!this->gas->GetShowGrid());
    this->Refresh(false);
}

void MyFrame::OnUpdateShowGrid(wxUpdateUIEvent& event)
{
    event.Enable(this->gas->GetShowGas());
    event.Check(this->gas->GetShowGrid());
}

void MyFrame::OnSubtractMeanVelocity(wxCommandEvent& /*event*/)
{
    this->gas->SetSubtractMeanVelocity(!this->gas->GetSubtractMeanVelocity());
    this->Refresh(false);
}

void MyFrame::OnUpdateSubtractMeanVelocity(wxUpdateUIEvent& event)
{
    event.Enable(this->gas->GetShowFlow());
    event.Check(this->gas->GetSubtractMeanVelocity());
}

void MyFrame::OnChangeAveragingRadius(wxCommandEvent& /*event*/)
{
    long int ar = this->gas->GetAveragingRadius(),new_ar;
    bool redo = false;
    do {
        wxString ret = wxGetTextFromUser(_T("Enter the averaging radius:"),_T("Averaging radius"),
            wxString::Format(_T("%d"),ar));
        if(ret.IsEmpty()) return; // user cancelled
        ret.ToLong(&new_ar);
        redo = new_ar<1;
        if(redo)
            wxMessageBox(_T("Value must be greater than 0."));
    } while(redo);
    this->gas->SetAveragingRadius(new_ar);
    this->Refresh(false);
}

void MyFrame::OnUpdateChangeAveragingRadius(wxUpdateUIEvent& event)
{
    event.Enable(this->gas->GetShowFlow());
}

void MyFrame::OnGettingStarted(wxCommandEvent& /*event*/)
{
    wxMessageBox(_T("Set the simulation running (press 'Enter' to start and stop). The fluid flows around the obstacle.\n\n\
After a few thousand timesteps the flow has become unstable, with alternating vortices being shed. \
Turn on 'subtract mean velocity' and increase the line length to 300 to better see them.\n\n\
Change the view settings (View menu) to visualize the flow in different ways.\n\n\
Try the other demos and gases (Actions menu) to see more.\n\n\
This is an implementation of lattice gas cellular automata. Search the web for \
more information.\n\n\
For more advanced changes, explore the source code."),_T("Getting started"),wxOK | wxICON_INFORMATION,
                 this);
}

void MyFrame::OnParticlesDemo(wxCommandEvent& /*event*/)
{
    this->current_demo = 0;
    this->LoadCurrentDemo();
}

void MyFrame::OnUpdateParticlesDemo(wxUpdateUIEvent& event)
{
    event.Check(this->current_demo == 0);
}

void MyFrame::OnObstacleDemo(wxCommandEvent& /*event*/)
{
    this->current_demo = 1;
    this->LoadCurrentDemo();
}

void MyFrame::OnUpdateObstacleDemo(wxUpdateUIEvent& event)
{
    event.Check(this->current_demo == 1);
}

void MyFrame::OnHoleDemo(wxCommandEvent& /*event*/)
{
    this->current_demo = 2;
    this->LoadCurrentDemo();
}

void MyFrame::OnUpdateHoleDemo(wxUpdateUIEvent& event)
{
    event.Check(this->current_demo == 2);
}

void MyFrame::OnZoomIn(wxCommandEvent& /*event*/)
{
    if(this->gas->ZoomIn())
        this->Refresh(true);
    else
        wxMessageBox(_T("Resulting image too large, can't zoom in any further."));
}

void MyFrame::OnZoomOut(wxCommandEvent& /*event*/)
{
    this->gas->ZoomOut();
    this->Refresh(true);
}

void MyFrame::OnShowFlow(wxCommandEvent& /*event*/)
{
    this->gas->SetShowFlow(!this->gas->GetShowFlow());
    this->Refresh(true);
}

void MyFrame::OnUpdateShowFlow(wxUpdateUIEvent& event)
{
    event.Check(this->gas->GetShowFlow());
}

void MyFrame::OnChangeToGasTypeN(wxCommandEvent& event)
{
    int new_ID = event.GetId() - ID_GAS_TYPE_0;
    BaseLatticeGas_drawable* new_gas = LatticeGasFactory::CreateGas(new_ID);
    if(new_gas==NULL) 
    {
        wxMessageBox(_T("Gas not yet supported, sorry!"));
        return;
    }
    delete this->gas;
    this->current_gas_type = new_ID;
    this->gas = new_gas;
    this->LoadCurrentDemo();
}

void MyFrame::OnUpdateChangeToGasTypeN(wxUpdateUIEvent& event)
{
    bool is_current = (this->current_gas_type == event.GetId() - ID_GAS_TYPE_0);
    event.Check(is_current);
}

void MyFrame::OnGetReport(wxCommandEvent& event)
{
    RealPoint v = this->gas->GetAverageInputFlowVelocityPerParticle();
    ostringstream oss;
    oss << "Gas type: " << LatticeGasFactory::GetGasDescription(this->current_gas_type) << "\n";
    oss << "Size: " << this->gas->GetX() << "x" << this->gas->GetY() << "\n";
    oss << "Number of gas particles: " << this->gas->GetNumGasParticles() << "\n";
    oss.precision(1);
    oss.setf(ios_base::fixed,ios_base::floatfield);
    oss << "Density: " << 100.0f*this->gas->GetNumGasParticles()/this->gas->GetMaxNumGasParticles() << "%\n";
    oss.precision(3);
    oss << "Average input flow per particle: " << v.x << "," << v.y << "\n";
    wxMessageBox(wxString(oss.str().c_str(),wxConvUTF8));
}

void MyFrame::OnShowGas(wxCommandEvent& event)
{
    this->gas->SetShowGas(!this->gas->GetShowGas());
    this->Refresh(true);
}

void MyFrame::OnUpdateShowGas(wxUpdateUIEvent& event)
{
    event.Check(this->gas->GetShowGas());
}

void MyFrame::OnShowGasColours(wxCommandEvent& event)
{
    this->gas->SetShowGasColours(!this->gas->GetShowGasColours());
    this->Refresh(true);
}

void MyFrame::OnUpdateShowGasColours(wxUpdateUIEvent& event)
{
    event.Enable(this->gas->GetShowGas());
    event.Check(this->gas->GetShowGasColours());
}
