#include "MyOpenGLFrame.h"
#include "MyOpenGLApp.h"

#include <algorithm>    // std::find

#include "GLPanel.h"


//(*InternalHeaders(MyOpenGLFrame)
#include <wx/button.h>
#include <wx/glcanvas.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(MyOpenGLFrame)
const long MyOpenGLFrame::ID_GLCANVAS1 = wxNewId();
const long MyOpenGLFrame::ID_BUTTON2 = wxNewId();
const long MyOpenGLFrame::ID_BUTTON1 = wxNewId();
const long MyOpenGLFrame::ID_TEXTCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(MyOpenGLFrame,wxFrame)
	//(*EventTable(MyOpenGLFrame)
	//*)
END_EVENT_TABLE()

MyOpenGLFrame::MyOpenGLFrame(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(MyOpenGLFrame)
	wxBoxSizer* BoxSizer1;
	wxBoxSizer* BoxSizer2;

	Create(parent, id, _("My App"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("id"));
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	int GLCanvasAttributes_1[] = {
		WX_GL_RGBA,
		WX_GL_DOUBLEBUFFER,
		WX_GL_DEPTH_SIZE,      16,
		WX_GL_STENCIL_SIZE,    0,
		0, 0 };
	#if wxCHECK_VERSION(3,0,0)
		GLCanvas1 = new GLPanel(this, ID_GLCANVAS1, GLCanvasAttributes_1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GLCANVAS1"));
	#else
		GLCanvas1 = new GLPanel(this, ID_GLCANVAS1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GLCANVAS1"), GLCanvasAttributes_1);
	#endif // wxCHECK_VERSION
	GLCanvas1->SetMinSize(wxSize(400,400));
	BoxSizer1->Add(GLCanvas1, 2, wxALL|wxEXPAND, 5);
	BoxSizer2 = new wxBoxSizer(wxVERTICAL);
	Button1 = new wxButton(this, ID_BUTTON2, _("TestButton1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	BoxSizer2->Add(Button1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button2 = new wxButton(this, ID_BUTTON1, _("TestButton2"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	BoxSizer2->Add(Button2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1 = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_RICH, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	BoxSizer2->Add(TextCtrl1, 1, wxALL|wxEXPAND, 5);
	BoxSizer1->Add(BoxSizer2, 1, wxALL|wxEXPAND, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->SetSizeHints(this);
	//*)

}

MyOpenGLFrame::~MyOpenGLFrame()
{
	//(*Destroy(MyOpenGLFrame)
	//*)
}


