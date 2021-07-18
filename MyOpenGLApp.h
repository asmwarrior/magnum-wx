#ifndef RADAR_GUI_APP_H
#define RADAR_GUI_APP_H

#include <wx/app.h>

class MyApp : public wxApp
{
    public:
        virtual bool OnInit();
};

wxDECLARE_APP(MyApp);

#endif // RADAR_GUI_APP_H
