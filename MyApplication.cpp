#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Platform/GLContext.h>


#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Shaders/VertexColor.h>


#include <wx/app.h>
#include <wx/frame.h>
#include <wx/glcanvas.h>
#include <wx/sizer.h>
#include <wx/version.h>

using namespace Magnum;

class MyApplication: public wxApp {
    public:
        bool OnInit();
};

class MainFrame: public wxFrame {
    public:
        explicit MainFrame(int argc, char** argv);
        ~MainFrame();

    private:
        void OnPaint(wxPaintEvent& event);

       void InitContext();

};

wxIMPLEMENT_APP(MyApplication);


void CreateConsole()
{
    if (!AllocConsole()) {
        // Add some error handling here.
        // You can call GetLastError() to get more info about the error.
        return;
    }

    // std::cout, std::clog, std::cerr, std::cin
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
    std::cin.clear();

    // std::wcout, std::wclog, std::wcerr, std::wcin
    HANDLE hConOut = CreateFile(_T("CONOUT$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hConIn = CreateFile(_T("CONIN$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
    SetStdHandle(STD_ERROR_HANDLE, hConOut);
    SetStdHandle(STD_INPUT_HANDLE, hConIn);
    std::wcout.clear();
    std::wclog.clear();
    std::wcerr.clear();
    std::wcin.clear();
}


bool MyApplication::OnInit() {

    CreateConsole();

    MainFrame *frame = new MainFrame{argc, argv};
    frame->Show(true);
    return true;
}

MainFrame::MainFrame(int argc, char** argv)
    : wxFrame{nullptr, wxID_ANY, "Magnum wxWidgets Application"}
{
    wxBoxSizer* bSizer;
    bSizer = new wxBoxSizer{wxVERTICAL};

    bSizer->Add(_wxGlCanvas, 1, wxALL|wxEXPAND, 5);
    SetSizer(bSizer);

    Layout();

    bSizer->Fit(this);

    Centre(wxBOTH);

}

MainFrame::~MainFrame()  = default;



