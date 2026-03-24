#pragma once
#include <wx/wx.h>

class MainFrame;

class ZPLEditorApp : public wxApp
{
public:
    bool OnInit() override;
    int  OnExit() override;

private:
    MainFrame* m_frame = nullptr;
};

wxDECLARE_APP(ZPLEditorApp);
