#include "App.h"
#include "AppConfig.h"
#include "MainFrame.h"
#include "I18n.h"
#include <wx/config.h>

wxIMPLEMENT_APP(ZPLEditorApp);

bool ZPLEditorApp::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    // Load persisted preferences
    AppConfig::Get() = AppConfig::Load();

    // Restore language selection from config
    {
        wxConfig cfg("ZPLEditor");
        wxString code = cfg.Read("/Language", "en");
        I18n::SetLanguage(code == "sr" ? AppLang::Serbian : AppLang::English);
    }

    m_frame = new MainFrame(nullptr);
    m_frame->Show(true);
    return true;
}

int ZPLEditorApp::OnExit()
{
    AppConfig::Get().Save();
    return wxApp::OnExit();
}
