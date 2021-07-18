#ifndef MYOPENGLFRAME_H
#define MYOPENGLFRAME_H

//(*Headers(MyOpenGLFrame)
#include <wx/frame.h>
class wxBoxSizer;
class wxButton;
class wxGLCanvas;
class wxTextCtrl;
//*)

#include "GLPanel.h"


// for reading the ini file
#include <wx/filename.h>
#include <wx/fileconf.h>
#include <wx/stdpaths.h>


class MyOpenGLFrame: public wxFrame
{
	public:

		MyOpenGLFrame(wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~MyOpenGLFrame();

		//(*Declarations(MyOpenGLFrame)
		GLPanel* GLCanvas1;
		wxButton* Button1;
		wxButton* Button2;
		wxTextCtrl* TextCtrl1;
		//*)


	protected:

		//(*Identifiers(MyOpenGLFrame)
		static const long ID_GLCANVAS1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON1;
		static const long ID_TEXTCTRL1;
		//*)

	private:

		//(*Handlers(MyOpenGLFrame)

		//*)

		DECLARE_EVENT_TABLE()
};

#endif
