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

// wxWidgets:
#include "wxWidgetsPreamble.h"
#include <wx/html/htmlwin.h>
#include <wx/statline.h>

// local:
#include "LatticeGasFactory.h"

// STL:
#include <stdexcept>
#include <exception>
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
    void OnIdle(wxIdleEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    // file menu
    void OnQuit(wxCommandEvent& event);
    // view menu
    void OnZoomIn(wxCommandEvent& event);
    void OnZoomOut(wxCommandEvent& event);
    void OnPanLeft(wxCommandEvent& event);
    void OnFitToWindow(wxCommandEvent& event);
    void OnChangeRedrawStep(wxCommandEvent& event);
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
    void OnChangeToVelocityRepresentationN(wxCommandEvent& event);
    void OnUpdateVelocityRepresentationN(wxUpdateUIEvent& event);
    // actions menu
    void OnStep(wxCommandEvent& event);
    void OnUpdateStep(wxUpdateUIEvent& event);
    void OnRunOrStop(wxCommandEvent& event);
    void OnDemoN(wxCommandEvent& event);
    void OnUpdateDemoN(wxUpdateUIEvent& event);
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
    
    wxPoint offset,drag_offset;
    bool is_dragging;
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
    ID_FIT_TO_WINDOW,

    ID_SHOW_GAS,
    ID_SHOW_GAS_COLOURS,
    ID_SHOW_GRID,

    ID_SHOW_FLOW,
    ID_SHOW_FLOW_COLOURS,
    ID_CHANGE_LINE_LENGTH,
    ID_CHANGE_AVERAGING_RADIUS,

    ID_VELOCITY_REPRESENTATION_0,
    ID_MAX_VELOCITY_REPRESENTATION = ID_VELOCITY_REPRESENTATION_0 + 10,

    ID_CHANGE_REDRAW_STEP,

    // actions menu:

    ID_STEP,
    ID_RUN_OR_STOP,
    
    ID_DEMO_0,
    ID_MAX_DEMO = ID_DEMO_0 + 100,

    ID_GAS_TYPE_0,
    ID_MAX_GAS_TYPE = ID_GAS_TYPE_0 + 100,

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
    EVT_IDLE(MyFrame::OnIdle)
    EVT_LEFT_DOWN(MyFrame::OnMouseDown)
    EVT_MOTION(MyFrame::OnMouseMove)
    EVT_LEFT_UP(MyFrame::OnMouseUp)
    // file menu:
    EVT_MENU(Minimal_Quit,  MyFrame::OnQuit)
    // view menu:
    EVT_MENU(ID_ZOOM_IN,MyFrame::OnZoomIn)
    EVT_MENU(ID_ZOOM_OUT,MyFrame::OnZoomOut)
    EVT_MENU(ID_FIT_TO_WINDOW,MyFrame::OnFitToWindow)
    EVT_MENU(ID_CHANGE_REDRAW_STEP,MyFrame::OnChangeRedrawStep)
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
    // (no velocity representation options here; we manually add them in the constructor)
    // actions menu:
    EVT_MENU(ID_STEP,MyFrame::OnStep)
    EVT_UPDATE_UI(ID_STEP,MyFrame::OnUpdateStep)
    EVT_MENU(ID_RUN_OR_STOP,MyFrame::OnRunOrStop)
    // (no demos or gas types here; we manually add them in the constructor)
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
    MyFrame *frame = new MyFrame(_("Lattice Gas Explorer"));

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
        fileMenu->Append(Minimal_Quit, _("E&xit\tAlt-F4"), _("Quit this program"));
        menuBar->Append(fileMenu, _("&File"));
    }

    // add the view menu
    {
        wxMenu *viewMenu = new wxMenu;
        viewMenu->Append(ID_ZOOM_IN,_("Zoom in\t+"),_("Draw the gas at a larger scale"));
        viewMenu->Append(ID_ZOOM_OUT,_("Zoom out\t-"),_("Draw the gas at a smaller scale"));
        viewMenu->Append(ID_FIT_TO_WINDOW,_("Fit to &window\tw"),_("Scale and pan the image so it is all visible"));
        viewMenu->AppendSeparator();
        viewMenu->Append(ID_CHANGE_REDRAW_STEP,_("Change redraw step..."),_("Change how often the gas is redrawn"));
        viewMenu->AppendSeparator();
        viewMenu->AppendCheckItem(ID_SHOW_GAS,_("Show &gas\tg"),_("Turn on/off the display of the gas particles"));
        viewMenu->AppendCheckItem(ID_SHOW_GAS_COLOURS,_("Show gas colours"),_("Show either gas density or gas direction as colours"));
        viewMenu->AppendCheckItem(ID_SHOW_GRID,_("Show grid"),_("Turn on/off the lattice grid (when zoomed in enough)"));
        viewMenu->AppendSeparator();
        viewMenu->AppendCheckItem(ID_SHOW_FLOW,_("Show the &flow\tf"),_("Turn on/off the display of flowlines"));
        viewMenu->AppendCheckItem(ID_SHOW_FLOW_COLOURS,_("Show flow colours"),_("Show flowlines with colours that depict their direction, or as black"));
        viewMenu->Append(ID_CHANGE_LINE_LENGTH,_("Change flow line length..."),_("Change the length of the flowlines"));
        viewMenu->Append(ID_CHANGE_AVERAGING_RADIUS,_("Change flow averaging radius..."),_("Change the area over which the velocity is averaged"));
        viewMenu->AppendSeparator();
        // add the item to the submenu and manually connect the events
        if(ID_VELOCITY_REPRESENTATION_0+BaseLatticeGas::GetNumVelocityRepresentations() > ID_MAX_VELOCITY_REPRESENTATION)
            throw runtime_error("Internal error: need more velocity representation IDs!");
        for(int i=0;i<BaseLatticeGas::GetNumVelocityRepresentations();i++)
        {
            viewMenu->AppendRadioItem(ID_VELOCITY_REPRESENTATION_0+i,BaseLatticeGas::GetVelocityRepresentationAsString(i),_("Switch to this representation of velocity"));
            Connect(ID_VELOCITY_REPRESENTATION_0+i,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(MyFrame::OnChangeToVelocityRepresentationN));
            Connect(ID_VELOCITY_REPRESENTATION_0+i,wxEVT_UPDATE_UI,wxUpdateUIEventHandler(MyFrame::OnUpdateVelocityRepresentationN));
            // (alternative to using the static event table above)
        }
        menuBar->Append(viewMenu, _("&View"));
    }

    // add the actions menu
    {
        wxMenu *actionsMenu = new wxMenu;
        actionsMenu->Append(ID_STEP, _("Step\tSpace"), _("Advance one timestep"));
        actionsMenu->Append(ID_RUN_OR_STOP, _("Run / Stop\tEnter"), _("Start/stop running"));
        actionsMenu->AppendSeparator();
        // add a demo type submenu
        {
            wxMenu *demosMenu = new wxMenu;
            if(ID_DEMO_0+BaseLatticeGas::GetNumDemos() > ID_MAX_DEMO)
                throw runtime_error("Internal error: need more demo IDs!");
            for(int i=0;i<BaseLatticeGas::GetNumDemos();i++)
            {
                // add the item to the submenu and manually connect the events
                demosMenu->AppendRadioItem(ID_DEMO_0+i,BaseLatticeGas::GetDemoDescription(i),_("Switch to this demo"));
                Connect(ID_DEMO_0+i,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(MyFrame::OnDemoN));
                Connect(ID_DEMO_0+i,wxEVT_UPDATE_UI,wxUpdateUIEventHandler(MyFrame::OnUpdateDemoN));
                // (alternative to using the static event table above)
            }
            actionsMenu->AppendSubMenu(demosMenu,_("Demo type:"));
        }
        // add a gas type submenu
        {
            wxMenu *gasTypeMenu = new wxMenu;
            if(ID_GAS_TYPE_0+LatticeGasFactory::GetNumGasTypesSupported() > ID_MAX_GAS_TYPE)
                throw runtime_error("Internal error: need more gas type IDs!");
            for(int i=0;i<LatticeGasFactory::GetNumGasTypesSupported();i++)
            {
                // add the item to the submenu and manually connect the events
                gasTypeMenu->AppendRadioItem(ID_GAS_TYPE_0+i,LatticeGasFactory::GetGasDescription(i),_("Switch to this gas type"));
                Connect(ID_GAS_TYPE_0+i,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(MyFrame::OnChangeToGasTypeN));
                Connect(ID_GAS_TYPE_0+i,wxEVT_UPDATE_UI,wxUpdateUIEventHandler(MyFrame::OnUpdateChangeToGasTypeN));
                // (alternative to using the static event table above)
            }
            actionsMenu->AppendSubMenu(gasTypeMenu,_("Gas type:"));
        }
        actionsMenu->Append(ID_GET_REPORT,_("Report gas details"),_("Show the statistics of the current gas"));
        menuBar->Append(actionsMenu, _("&Actions"));
    }

    // add the help menu
    {
        wxMenu *helpMenu = new wxMenu;
        helpMenu->Append(ID_GETTING_STARTED, _("&Getting started\tF1"), _("Show a mini tutorial to get you started"));
        helpMenu->AppendSeparator();
        helpMenu->Append(Minimal_About, _("&About...\tAlt-F1"), _("Show information about the program"));
        menuBar->Append(helpMenu, _("&Help"));
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
    
    this->is_dragging = false;
}

void MyFrame::LoadCurrentDemo()
{
    this->gas->ResetGridForDemo(this->current_demo);
    this->offset = wxPoint(0,0);
    this->gas->RequestBestFitZoomFactor(this->GetClientSize().GetWidth(),this->GetClientSize().GetHeight());
    if(this->current_demo==0) this->running_step = 1;
    else this->running_step = 10;
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
    wxString text = _("<html><body><table><tr><td>\
\
<b>Lattice Gas Explorer</b>\
\
<p>Copyright (C) 2008-9 Tim J. Hutton<br>\
<a href=\"mailto:tim.hutton@gmail.com\">tim.hutton@gmail.com</a><br>\
<a href=\"http://www.sq3.org.uk\">http://www.sq3.org.uk</a></p>\
\
<p>Project homepage: <a href=\"http://code.google.com/p/latticegas\">http://code.google.com/p/latticegas</a></p>\
\
<p>This program is free software: you can redistribute it and/or modify \
it under the terms of the GNU General Public License as published by \
the Free Software Foundation, either version 3 of the License, or \
(at your option) any later version.</p>\
\
<p>This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \
GNU General Public License for more details.</p>\
\
<p>You should have received a copy of the GNU General Public License \
along with this program.  If not, see \
<a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>.</p>\
</td></tr></table></body></html>");

    wxBoxSizer *topsizer;
    wxHtmlWindow *html;
    wxDialog dlg(this, wxID_ANY, wxString(_("About")));

    topsizer = new wxBoxSizer(wxVERTICAL);

    html = new wxHtmlWindow(&dlg, wxID_ANY, wxDefaultPosition, wxSize(380, 160), wxHW_SCROLLBAR_NEVER);
    html -> SetBorders(0);
    html -> SetPage(text);
    html -> SetSize(html -> GetInternalRepresentation() -> GetWidth(),
                    html -> GetInternalRepresentation() -> GetHeight());

    topsizer -> Add(html, 1, wxALL, 10);

#if wxUSE_STATLINE
    topsizer -> Add(new wxStaticLine(&dlg, wxID_ANY), 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
#endif // wxUSE_STATLINE

    wxButton *bu1 = new wxButton(&dlg, wxID_OK, _("OK"));
    bu1 -> SetDefault();

    topsizer -> Add(bu1, 0, wxALL | wxALIGN_RIGHT, 15);

    dlg.SetSizer(topsizer);
    topsizer -> Fit(&dlg);

    dlg.ShowModal();
}

void MyFrame::OnPaint(wxPaintEvent& /*event*/)
{
    {
        SetStatusText(_("Redrawing images..."),2);
        // BUG: this message doesn't show up on linux, only on Windows

        wxPaintDC dc(this);
        this->gas->Draw(dc,this->offset.x,this->offset.y);
    }

    SetStatusText(wxString::Format(_("%d iterations"),this->gas->GetIterations()),0);

    // display the current zoom setting
    {
        int num,denom;
        this->gas->GetZoom(num,denom);
        if(denom==1)
            SetStatusText(wxString::Format(_("Zoom: %d"),num),1);
        else
            SetStatusText(wxString::Format(_("Zoom: 1/%d"),denom),1);
    }

    if(!this->is_running)
    {
        SetStatusText(_("Stopped - press Enter to start/stop."),2);
    }
    else
    {
        // cause some computation to happen
        SetStatusText(_("Performing gas calculations..."),2);
    }
}

void MyFrame::OnIdle(wxIdleEvent& event)
{
    if(this->is_running)
    {
        this->gas->UpdateGas();
        // trigger this event again (after processing other user interactions)
        event.RequestMore();
        // paint if needed
        if(this->gas->GetIterations()%running_step==0)
            this->Refresh(false);
    }
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
    wxString ret = wxGetTextFromUser(_("Enter the line length:"),_("Line lengths"),
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

void MyFrame::OnChangeToVelocityRepresentationN(wxCommandEvent& event)
{
    this->gas->SetVelocityRepresentation(event.GetId()-ID_VELOCITY_REPRESENTATION_0);
    this->Refresh(false);
}

void MyFrame::OnUpdateVelocityRepresentationN(wxUpdateUIEvent& event)
{
    event.Enable(this->gas->GetShowFlow());
    event.Check(event.GetId()-ID_VELOCITY_REPRESENTATION_0 == this->gas->GetVelocityRepresentation());
}

void MyFrame::OnChangeAveragingRadius(wxCommandEvent& /*event*/)
{
    long int ar = this->gas->GetAveragingRadius(),new_ar;
    bool redo = false;
    do {
        wxString ret = wxGetTextFromUser(_("Enter the averaging radius:"),_("Averaging radius"),
            wxString::Format(_T("%d"),ar));
        if(ret.IsEmpty()) return; // user cancelled
        ret.ToLong(&new_ar);
        redo = new_ar<1;
        if(redo)
            wxMessageBox(_("Value must be greater than 0."));
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
    wxString text = _("<html><body><table><tr><td>\
<b>Getting started:</b>\
<p><b>1.</b> Set the simulation running (press 'Enter' to start and stop). The fluid flows around the obstacle \
and soon forms two vortices, as the fluid is pulled back into the low pressure area. Turn off the gas density \
display (View menu | Show gas) to better see the flowlines.\
</p>\
<p><b>2.</b> After a few thousand timesteps the flow usually becomes unstable, with alternating vortices being shed. \
Select 'Subtract time-averaged point mean velocity' (View menu) and increase the line length to 300 to better see them.\
</p>\
<p>Change the view settings (View menu) to visualize the flow in different ways.\
</p>\
<p>Try the other demos and gases (Actions menu) to see more.\
</p>\
<p>This is an implementation of lattice gas cellular automata. Search the web for \
more information.\
</p>\
<p>For more advanced changes, explore the source code.</p></td></tr></table></body></html>");

    wxBoxSizer *topsizer;
    wxHtmlWindow *html;
    wxDialog *dlg = new wxDialog(this, wxID_ANY, wxString(_("Tutorial")));

    topsizer = new wxBoxSizer(wxVERTICAL);

    html = new wxHtmlWindow(dlg, wxID_ANY, wxDefaultPosition, wxSize(380, 160), wxHW_SCROLLBAR_NEVER);
    html -> SetBorders(0);
    html -> SetPage(text);
    html -> SetSize(html -> GetInternalRepresentation() -> GetWidth(),
                    html -> GetInternalRepresentation() -> GetHeight());

    topsizer -> Add(html, 1, wxALL, 10);

#if wxUSE_STATLINE
    topsizer -> Add(new wxStaticLine(dlg, wxID_ANY), 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
#endif // wxUSE_STATLINE

    wxButton *bu1 = new wxButton(dlg, wxID_OK, _("OK"));
    bu1 -> SetDefault();

    topsizer -> Add(bu1, 0, wxALL | wxALIGN_RIGHT, 15);

    dlg->SetSizer(topsizer);
    topsizer -> Fit(dlg);

    dlg->Show();
}

void MyFrame::OnDemoN(wxCommandEvent& event)
{
    this->current_demo = event.GetId()-ID_DEMO_0;
    this->LoadCurrentDemo();
}

void MyFrame::OnUpdateDemoN(wxUpdateUIEvent& event)
{
    event.Check(this->current_demo == event.GetId()-ID_DEMO_0);
}

void MyFrame::OnZoomIn(wxCommandEvent& /*event*/)
{
    if(this->gas->ZoomIn())
        this->Refresh(true);
    else
        wxMessageBox(_("Resulting image too large, can't zoom in any further."));
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
        wxMessageBox(_("Gas not yet supported, sorry!"));
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
    wxString oss;
    oss << _("Gas type: ") << LatticeGasFactory::GetGasDescription(this->current_gas_type);
    oss << _T("\n");
    oss << _("Size: ") << this->gas->GetX() << _T("x") << this->gas->GetY() << _T("\n");
    oss << _("Number of gas particles: ") << this->gas->GetNumGasParticles() << _T("\n");
    oss << _("Density: ") << 100.0f*this->gas->GetNumGasParticles()/this->gas->GetMaxNumGasParticles() << _T("%\n");
    RealPoint v = this->gas->GetAverageInputFlowVelocityPerParticle();
    oss << _("Average input flow per particle: ") << v.x << _T(",") << v.y << _T("\n");
    RealPoint av = this->gas->GetAverageVelocityPerParticle();
    oss << _("Average velocity: ") << av.x << _T(",") << av.y << _T("\n");
    wxMessageBox(oss);
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

void MyFrame::OnChangeRedrawStep(wxCommandEvent &event)
{
    long int rs = this->running_step,new_rs;
    bool redo = false;
    do {
        wxString ret = wxGetTextFromUser(_(
"While running, the gas is repainted every N steps.\n\n\
Lower values update more frequently but slow down the overall speed.\n\n\
Enter the new redraw step:"),_("Redraw step"),
            wxString::Format(_T("%d"),rs));
        if(ret.IsEmpty()) return; // user cancelled
        ret.ToLong(&new_rs);
        redo = new_rs<1;
        if(redo)
            wxMessageBox(_("Value must be greater than 0."));
    } while(redo);
    this->running_step = new_rs;
    this->Refresh(false);
}

void MyFrame::OnFitToWindow(wxCommandEvent& event)
{
    this->offset = wxPoint(0,0);
    this->gas->RequestBestFitZoomFactor(this->GetClientSize().GetWidth(),this->GetClientSize().GetHeight());
    this->Refresh(false);
}

void MyFrame::OnMouseDown(wxMouseEvent& event)
{
    if(!this->is_dragging)
    {
        this->is_dragging = true;
        this->drag_offset = event.GetPosition();
    }
}

void MyFrame::OnMouseMove(wxMouseEvent& event)
{
    if(this->is_dragging)
    {
        this->offset += event.GetPosition() - this->drag_offset;
        this->drag_offset = event.GetPosition();
        this->Refresh(false);
    }
}

void MyFrame::OnMouseUp(wxMouseEvent& event)
{
    if(this->is_dragging)
        this->is_dragging = false;
}

