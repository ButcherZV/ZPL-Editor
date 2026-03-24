#include "I18n.h"
#include <array>
#include <cassert>

// ─────────────────────────────────────────────────────────────────────────────
// Current language (default: English)
// ─────────────────────────────────────────────────────────────────────────────
static AppLang s_lang = AppLang::English;

// ─────────────────────────────────────────────────────────────────────────────
// Raw string tables (wchar_t* – safe at static-init time)
// Order MUST match StrId enum exactly.
// ─────────────────────────────────────────────────────────────────────────────
static const wchar_t* const kEn[] =
{
    /* APP_TITLE            */ L"ZPL Label Designer",
    /* TITLE_NEW_LABEL      */ L"ZPL Label Designer \u2014 New Label",

    /* MENU_FILE            */ L"&File",
    /* MENU_NEW             */ L"&New Label\tCtrl+N",
    /* MENU_OPEN            */ L"&Open...\tCtrl+O",
    /* MENU_RECENT_FILES    */ L"Recent &Files",
    /* MENU_SAVE            */ L"&Save\tCtrl+S",
    /* MENU_SAVE_AS         */ L"Save &As...\tCtrl+Shift+S",
    /* MENU_PRINT           */ L"&Print (Raw ZPL)...\tCtrl+P",
    /* MENU_EXIT            */ L"E&xit\tAlt+F4",

    /* MENU_EDIT            */ L"&Edit",
    /* MENU_UNDO            */ L"&Undo\tCtrl+Z",
    /* MENU_REDO            */ L"&Redo\tCtrl+Y",
    /* MENU_DUPLICATE       */ L"D&uplicate Element\tCtrl+D",
    /* MENU_ALIGN           */ L"&Align to Label",
    /* MENU_ALIGN_LEFT      */ L"Align &Left",
    /* MENU_ALIGN_RIGHT     */ L"Align &Right",
    /* MENU_ALIGN_TOP       */ L"Align &Top",
    /* MENU_ALIGN_BOTTOM    */ L"Align &Bottom",
    /* MENU_ALIGN_CENTERH   */ L"Center &Horizontally",
    /* MENU_ALIGN_CENTERV   */ L"Center &Vertically",
    /* MENU_OPTIONS         */ L"&Options...",

    /* MENU_VIEW            */ L"&View",
    /* MENU_ZOOM_IN         */ L"Zoom &In\tCtrl++",
    /* MENU_ZOOM_OUT        */ L"Zoom &Out\tCtrl+-",
    /* MENU_ZOOM_FIT        */ L"Zoom to &Fit\tCtrl+0",
    /* MENU_SNAP_GRID       */ L"&Snap to Grid\tG",
    /* MENU_SHOW_ZPL        */ L"Show &ZPL Code\tCtrl+Shift+Z",
    /* MENU_LANGUAGE        */ L"&Language",
    /* STR_LANG_ENGLISH     */ L"English",
    /* STR_LANG_SERBIAN     */ L"Serbian (Srpski)",

    /* PANEL_CANVAS         */ L"Canvas",
    /* PANEL_TOOLBOX        */ L"Toolbox",
    /* PANEL_PROPERTIES     */ L"Properties",
    /* PANEL_ZPL_CODE       */ L"ZPL Code",

    /* STATUS_READY         */ L"Ready",
    /* STATUS_NO_LABEL      */ L"No label open",
    /* STATUS_SNAP_OFF      */ L"Snap: OFF",
    /* STATUS_SNAP_ON       */ L" | Snap: %ddots",
    /* STATUS_LABEL_INFO    */ L"Label: %d x %d dots @ %d DPI",
    /* STATUS_ZOOM          */ L"Zoom: %d%%%s",
    /* STATUS_PRINT_SENT    */ L"Sent %zu bytes to %s",

    /* TOOL_SELECT          */ L"Select",
    /* TOOL_TEXT            */ L"Text",
    /* TOOL_BARCODE         */ L"Barcode",
    /* TOOL_BOX             */ L"Box",
    /* TOOL_IMAGE           */ L"Image",
    /* TOOLTIP_SELECT       */ L"Select / Move  [V or Esc]",
    /* TOOLTIP_TEXT         */ L"Place Text element  [T]",
    /* TOOLTIP_BARCODE      */ L"Place Barcode element  [B]",
    /* TOOLTIP_BOX          */ L"Place Box / Shape element  [R]",
    /* TOOLTIP_IMAGE        */ L"Place Image element  [I]",

    /* PROP_PANEL_TITLE     */ L"Properties",
    /* PROP_POS_SIZE        */ L"Position / Size",
    /* PROP_X               */ L"X (dots)",
    /* PROP_Y               */ L"Y (dots)",
    /* PROP_WIDTH           */ L"Width (dots)",
    /* PROP_HEIGHT          */ L"Height (dots)",
    /* PROP_TEXT_ELEM       */ L"Text Element",
    /* PROP_CONTENT         */ L"Content",
    /* PROP_FONT_SIZE       */ L"Font Size",
    /* PROP_ROTATION        */ L"Rotation",
    /* PROP_BARCODE_ELEM    */ L"Barcode Element",
    /* PROP_BARCODE_TYPE    */ L"Barcode Type",
    /* PROP_DATA            */ L"Data",
    /* PROP_BAR_HEIGHT      */ L"Bar Height",
    /* PROP_SHOW_TEXT       */ L"Show Text",
    /* PROP_CHECK_DIGIT     */ L"Check Digit",
    /* PROP_BOX_ELEM        */ L"Box Element",
    /* PROP_SHAPE           */ L"Shape",
    /* PROP_SHAPE_RECT      */ L"Rectangle",
    /* PROP_SHAPE_CIRCLE    */ L"Circle",
    /* PROP_SHAPE_DIAG_BK   */ L"Diagonal \\",
    /* PROP_SHAPE_DIAG_FW   */ L"Diagonal /",
    /* PROP_SHAPE_ELLIPSE   */ L"Ellipse",
    /* PROP_BORDER_THICK    */ L"Border Thickness",
    /* PROP_BORDER_COLOR    */ L"Border Colour",
    /* PROP_FILL_COLOR      */ L"Fill Colour",
    /* PROP_FILLED          */ L"Filled",
    /* PROP_IMAGE_ELEM      */ L"Image Element",
    /* PROP_FILE_PATH       */ L"File Path",
    /* PROP_FONT_WIDTH      */ L"Font Width (dots)",
    /* PROP_FONT_PATH       */ L"Font (path)",
    /* PROP_BOLD            */ L"Bold",
    /* PROP_ITALIC          */ L"Italic",
    /* PROP_FIELD_BLOCK     */ L"Field Block",
    /* PROP_FB_WIDTH        */ L"Block Width (dots)",
    /* PROP_FB_MAX_LINES    */ L"Max Lines",
    /* PROP_FB_JUSTIFY      */ L"Justification",

    /* NEWLABEL_TITLE       */ L"New Label",
    /* NEWLABEL_PRINTER_RES */ L"Printer / Resolution",
    /* NEWLABEL_PRINTER     */ L"Printer:",
    /* NEWLABEL_DPI         */ L"Resolution:",
    /* NEWLABEL_MANUAL      */ L"(manual - enter DPI below)",
    /* NEWLABEL_LABEL_SIZE  */ L"Label Size",
    /* NEWLABEL_UNITS       */ L"Units:",
    /* NEWLABEL_MM          */ L"mm",
    /* NEWLABEL_INCHES      */ L"inches",
    /* NEWLABEL_DOTS        */ L"dots",
    /* NEWLABEL_WIDTH       */ L"Width:",
    /* NEWLABEL_HEIGHT      */ L"Height:",
    /* NEWLABEL_MARGINS     */ L"Margins",
    /* NEWLABEL_TOP         */ L"Top:",
    /* NEWLABEL_BOTTOM      */ L"Bottom:",
    /* NEWLABEL_LEFT        */ L"Left:",
    /* NEWLABEL_RIGHT       */ L"Right:",
    /* NEWLABEL_ORIENTATION */ L"Orientation",
    /* NEWLABEL_PORTRAIT    */ L"Portrait",
    /* NEWLABEL_LANDSCAPE   */ L"Landscape",
    /* NEWLABEL_LAYOUT      */ L"Multi-Label Layout",
    /* NEWLABEL_LABELS_PER_ROW */ L"Labels per row:",
    /* NEWLABEL_LPR_TOOLTIP */ L"Number of label columns printed side-by-side across the media.",
    /* NEWLABEL_PREVIEW     */ L"Preview",
    /* NEWLABEL_INVALID_SIZE*/ L"Invalid Size",
    /* NEWLABEL_INVALID_MSG */ L"Width and height must be greater than zero.",

    /* BTN_CREATE           */ L"Create Label",
    /* BTN_CANCEL           */ L"Cancel",
    /* BTN_OK               */ L"OK",

    /* DLG_OPEN_TITLE       */ L"Open ZPL File",
    /* DLG_SAVE_TITLE       */ L"Save ZPL File",
    /* DLG_CHOOSE_IMAGE     */ L"Choose Image",
    /* FILE_FILTER_ZPL      */ L"ZPL files (*.zpl)|*.zpl|All files (*.*)|*.*",
    /* FILE_FILTER_IMAGE    */ L"Image files (*.png;*.jpg;*.bmp;*.jpeg)|*.png;*.jpg;*.bmp;*.jpeg|All files (*.*)|*.*",

    /* ERR_OPEN_FILE        */ L"Cannot open file: ",
    /* ERR_PARSE            */ L"Failed to parse ZPL file:\n",
    /* ERR_PARSE_TITLE      */ L"Parse Error",
    /* ERR_WRITE            */ L"Cannot write file: ",
    /* ERR_TITLE            */ L"Error",
    /* ERR_FILE_NOT_FOUND   */ L"File not found:\n",
    /* ERR_RECENT_FILES     */ L"Recent Files",

    /* PRINT_NO_ELEMENTS    */ L"Nothing to print \u2014 add elements to the label first.",
    /* PRINT_TITLE          */ L"Print",
    /* PRINT_CANT_ENUM      */ L"Could not enumerate printers.",
    /* PRINT_ERR_TITLE      */ L"Print Error",
    /* PRINT_NO_PRINTERS    */ L"No printers found.",
    /* PRINT_SELECT         */ L"Select printer for raw ZPL output:",
    /* PRINT_DLG_TITLE      */ L"Print Label",
    /* PRINT_CANT_OPEN      */ L"Cannot open printer: ",
    /* PRINT_ERR_START      */ L"StartDocPrinter failed.",
    /* PRINT_WIN_ONLY       */ L"Raw ZPL printing is only supported on Windows.\nSave the file as .zpl and send it to your printer manually.",

    /* LANG_CHANGE_TITLE    */ L"Language Changed",
    /* LANG_CHANGE_MSG      */ L"Please restart the application to apply the new language.",

    /* CTX_DELETE           */ L"Delete",
    /* CTX_DUPLICATE        */ L"Duplicate",
    /* CTX_BRING_FRONT      */ L"Bring to Front",
    /* CTX_SEND_BACK        */ L"Send to Back",
    /* CTX_PASTE            */ L"Paste",
};

static const wchar_t* const kSr[] =
{
    /* APP_TITLE            */ L"ZPL Dizajner Etiketa",
    /* TITLE_NEW_LABEL      */ L"ZPL Dizajner Etiketa \u2014 Nova Etiketa",

    /* MENU_FILE            */ L"&Datoteka",
    /* MENU_NEW             */ L"&Nova Etiketa\tCtrl+N",
    /* MENU_OPEN            */ L"&Otvori...\tCtrl+O",
    /* MENU_RECENT_FILES    */ L"Nedavne &Datoteke",
    /* MENU_SAVE            */ L"&Sa\u010duvaj\tCtrl+S",
    /* MENU_SAVE_AS         */ L"Sa\u010duvaj &Kao...\tCtrl+Shift+S",
    /* MENU_PRINT           */ L"&\u0160tampaj (Raw ZPL)...\tCtrl+P",
    /* MENU_EXIT            */ L"&Izlaz\tAlt+F4",

    /* MENU_EDIT            */ L"&Uredi",
    /* MENU_UNDO            */ L"&Poni\u0161ti\tCtrl+Z",
    /* MENU_REDO            */ L"&Ponovi\tCtrl+Y",
    /* MENU_DUPLICATE       */ L"&Dupliraj Element\tCtrl+D",
    /* MENU_ALIGN           */ L"&Poravnaj na Etiketu",
    /* MENU_ALIGN_LEFT      */ L"Poravnaj &Levo",
    /* MENU_ALIGN_RIGHT     */ L"Poravnaj &Desno",
    /* MENU_ALIGN_TOP       */ L"Poravnaj &Gore",
    /* MENU_ALIGN_BOTTOM    */ L"Poravnaj &Dole",
    /* MENU_ALIGN_CENTERH   */ L"Centriraj &Horizontalno",
    /* MENU_ALIGN_CENTERV   */ L"Centriraj &Vertikalno",
    /* MENU_OPTIONS         */ L"&Opcije...",

    /* MENU_VIEW            */ L"&Prikaz",
    /* MENU_ZOOM_IN         */ L"Uve\u0107aj\tCtrl++",
    /* MENU_ZOOM_OUT        */ L"Umanji\tCtrl+-",
    /* MENU_ZOOM_FIT        */ L"Prilagodi Prikaz\tCtrl+0",
    /* MENU_SNAP_GRID       */ L"&Zaka\u010di za Mre\u017eu\tG",
    /* MENU_SHOW_ZPL        */ L"Prika\u017ei &ZPL Kod\tCtrl+Shift+Z",
    /* MENU_LANGUAGE        */ L"&Jezik",
    /* STR_LANG_ENGLISH     */ L"Engleski",
    /* STR_LANG_SERBIAN     */ L"Srpski",

    /* PANEL_CANVAS         */ L"Platno",
    /* PANEL_TOOLBOX        */ L"Alatke",
    /* PANEL_PROPERTIES     */ L"Svojstva",
    /* PANEL_ZPL_CODE       */ L"ZPL Kod",

    /* STATUS_READY         */ L"Spreman",
    /* STATUS_NO_LABEL      */ L"Nema otvorene etikete",
    /* STATUS_SNAP_OFF      */ L"Mre\u017ea: ISKLJ",
    /* STATUS_SNAP_ON       */ L" | Mre\u017ea: %d ta\u010daka",
    /* STATUS_LABEL_INFO    */ L"Etiketa: %d x %d ta\u010daka @ %d DPI",
    /* STATUS_ZOOM          */ L"Zum: %d%%%s",
    /* STATUS_PRINT_SENT    */ L"Poslato %zu bajtova na %s",

    /* TOOL_SELECT          */ L"Izbor",
    /* TOOL_TEXT            */ L"Tekst",
    /* TOOL_BARCODE         */ L"Barkod",
    /* TOOL_BOX             */ L"Okvir",
    /* TOOL_IMAGE           */ L"Slika",
    /* TOOLTIP_SELECT       */ L"Izaberi / Pomeri  [V ili Esc]",
    /* TOOLTIP_TEXT         */ L"Postavi element Tekst  [T]",
    /* TOOLTIP_BARCODE      */ L"Postavi element Barkod  [B]",
    /* TOOLTIP_BOX          */ L"Postavi element Okvir  [R]",
    /* TOOLTIP_IMAGE        */ L"Postavi element Slika  [I]",

    /* PROP_PANEL_TITLE     */ L"Svojstva",
    /* PROP_POS_SIZE        */ L"Pozicija / Veli\u010dina",
    /* PROP_X               */ L"X (ta\u010dke)",
    /* PROP_Y               */ L"Y (ta\u010dke)",
    /* PROP_WIDTH           */ L"\u0160irina (ta\u010dke)",
    /* PROP_HEIGHT          */ L"Visina (ta\u010dke)",
    /* PROP_TEXT_ELEM       */ L"Tekstualni Element",
    /* PROP_CONTENT         */ L"Sadr\u017eaj",
    /* PROP_FONT_SIZE       */ L"Veli\u010dina Fonta",
    /* PROP_ROTATION        */ L"Rotacija",
    /* PROP_BARCODE_ELEM    */ L"Element Barkoda",
    /* PROP_BARCODE_TYPE    */ L"Tip Barkoda",
    /* PROP_DATA            */ L"Podaci",
    /* PROP_BAR_HEIGHT      */ L"Visina Linija",
    /* PROP_SHOW_TEXT       */ L"Prika\u017ei Tekst",
    /* PROP_CHECK_DIGIT     */ L"Kontrolna Cifra",
    /* PROP_BOX_ELEM        */ L"Element Okvira",
    /* PROP_SHAPE           */ L"Oblik",
    /* PROP_SHAPE_RECT      */ L"Pravougaonik",
    /* PROP_SHAPE_CIRCLE    */ L"Krug",
    /* PROP_SHAPE_DIAG_BK   */ L"Dijagonala \\",
    /* PROP_SHAPE_DIAG_FW   */ L"Dijagonala /",
    /* PROP_SHAPE_ELLIPSE   */ L"Elipsa",
    /* PROP_BORDER_THICK    */ L"Debljina Okvira",
    /* PROP_BORDER_COLOR    */ L"Boja Okvira",
    /* PROP_FILL_COLOR      */ L"Boja Punjenja",
    /* PROP_FILLED          */ L"Popunjeno",
    /* PROP_IMAGE_ELEM      */ L"Element Slike",
    /* PROP_FILE_PATH       */ L"Putanja do Datoteke",
    /* PROP_FONT_WIDTH      */ L"\u0160irina Fonta (ta\u010dke)",
    /* PROP_FONT_PATH       */ L"Font (putanja)",
    /* PROP_BOLD            */ L"Podebljano",
    /* PROP_ITALIC          */ L"Kurziv",
    /* PROP_FIELD_BLOCK     */ L"Blok Teksta",
    /* PROP_FB_WIDTH        */ L"\u0160irina Bloka (ta\u010dke)",
    /* PROP_FB_MAX_LINES    */ L"Maks. Linija",
    /* PROP_FB_JUSTIFY      */ L"Poravnanje",

    /* NEWLABEL_TITLE       */ L"Nova Etiketa",
    /* NEWLABEL_PRINTER_RES */ L"\u0160tampa\u010d / Rezolucija",
    /* NEWLABEL_PRINTER     */ L"\u0160tampa\u010d:",
    /* NEWLABEL_DPI         */ L"Rezolucija:",
    /* NEWLABEL_MANUAL      */ L"(ru\u010dno - unesite DPI)",
    /* NEWLABEL_LABEL_SIZE  */ L"Veli\u010dina Etikete",
    /* NEWLABEL_UNITS       */ L"Jedinice:",
    /* NEWLABEL_MM          */ L"mm",
    /* NEWLABEL_INCHES      */ L"in\u010di",
    /* NEWLABEL_DOTS        */ L"ta\u010dke",
    /* NEWLABEL_WIDTH       */ L"\u0160irina:",
    /* NEWLABEL_HEIGHT      */ L"Visina:",
    /* NEWLABEL_MARGINS     */ L"Margine",
    /* NEWLABEL_TOP         */ L"Gore:",
    /* NEWLABEL_BOTTOM      */ L"Dole:",
    /* NEWLABEL_LEFT        */ L"Levo:",
    /* NEWLABEL_RIGHT       */ L"Desno:",
    /* NEWLABEL_ORIENTATION */ L"Orijentacija",
    /* NEWLABEL_PORTRAIT    */ L"Portret",
    /* NEWLABEL_LANDSCAPE   */ L"Pejza\u017e",
    /* NEWLABEL_LAYOUT      */ L"Raspored Vi\u0161e Etiketa",
    /* NEWLABEL_LABELS_PER_ROW */ L"Etiketa u redu:",
    /* NEWLABEL_LPR_TOOLTIP */ L"Broj kolona etiketa odštampanih jedne pored druge.",
    /* NEWLABEL_PREVIEW     */ L"Pregled",
    /* NEWLABEL_INVALID_SIZE*/ L"Neispravna Veli\u010dina",
    /* NEWLABEL_INVALID_MSG */ L"\u0160irina i visina moraju biti ve\u0107e od nule.",

    /* BTN_CREATE           */ L"Kreiraj Etiketu",
    /* BTN_CANCEL           */ L"Otka\u017ei",
    /* BTN_OK               */ L"U redu",

    /* DLG_OPEN_TITLE       */ L"Otvori ZPL Datoteku",
    /* DLG_SAVE_TITLE       */ L"Sa\u010duvaj ZPL Datoteku",
    /* DLG_CHOOSE_IMAGE     */ L"Izaberi Sliku",
    /* FILE_FILTER_ZPL      */ L"ZPL datoteke (*.zpl)|*.zpl|Sve datoteke (*.*)|*.*",
    /* FILE_FILTER_IMAGE    */ L"Datoteke slika (*.png;*.jpg;*.bmp;*.jpeg)|*.png;*.jpg;*.bmp;*.jpeg|Sve datoteke (*.*)|*.*",

    /* ERR_OPEN_FILE        */ L"Nije mogu\u0107e otvoriti datoteku: ",
    /* ERR_PARSE            */ L"Gre\u0161ka pri parsiranju ZPL datoteke:\n",
    /* ERR_PARSE_TITLE      */ L"Gre\u0161ka Parsiranja",
    /* ERR_WRITE            */ L"Nije mogu\u0107e zapisati datoteku: ",
    /* ERR_TITLE            */ L"Gre\u0161ka",
    /* ERR_FILE_NOT_FOUND   */ L"Datoteka nije prona\u0111ena:\n",
    /* ERR_RECENT_FILES     */ L"Nedavne Datoteke",

    /* PRINT_NO_ELEMENTS    */ L"Nema \u0161ta da se \u0161tampa \u2014 dodajte elemente na etiketu.",
    /* PRINT_TITLE          */ L"\u0160tampaj",
    /* PRINT_CANT_ENUM      */ L"Nije mogu\u0107e nabrojati \u0161tampa\u010de.",
    /* PRINT_ERR_TITLE      */ L"Gre\u0161ka \u0160tampanja",
    /* PRINT_NO_PRINTERS    */ L"Nisu prona\u0111eni \u0161tampa\u010di.",
    /* PRINT_SELECT         */ L"Izaberite \u0161tampa\u010d za ispis ZPL koda:",
    /* PRINT_DLG_TITLE      */ L"\u0160tampaj Etiketu",
    /* PRINT_CANT_OPEN      */ L"Nije mogu\u0107e otvoriti \u0161tampa\u010d: ",
    /* PRINT_ERR_START      */ L"Gre\u0161ka pri pokretanju dokumenta za \u0161tampanje.",
    /* PRINT_WIN_ONLY       */ L"Raw ZPL \u0161tampanje je podr\u017eano samo na Windows-u.\nSa\u010duvajte datoteku kao .zpl i po\u0161aljite je \u0161tampa\u010du ru\u010dno.",

    /* LANG_CHANGE_TITLE    */ L"Promena Jezika",
    /* LANG_CHANGE_MSG      */ L"Molimo ponovo pokrenite program da bi se promena primenila.",

    /* CTX_DELETE           */ L"Obri\u0161i",
    /* CTX_DUPLICATE        */ L"Dupliraj",
    /* CTX_BRING_FRONT      */ L"Pomeri Napred",
    /* CTX_SEND_BACK        */ L"Pomeri Nazad",
    /* CTX_PASTE            */ L"Nalepi",
};

static_assert(
    sizeof(kEn) / sizeof(kEn[0]) == static_cast<size_t>(StrId::COUNT),
    "English string table size does not match StrId::COUNT");
static_assert(
    sizeof(kSr) / sizeof(kSr[0]) == static_cast<size_t>(StrId::COUNT),
    "Serbian string table size does not match StrId::COUNT");

// ─────────────────────────────────────────────────────────────────────────────
// I18n implementation
// ─────────────────────────────────────────────────────────────────────────────
namespace I18n
{
    void SetLanguage(AppLang lang) { s_lang = lang; }
    AppLang GetLanguage()          { return s_lang; }

    const wxString& Tr(StrId id)
    {
        // Lazy-init wxString caches from the raw wchar_t* tables.
        // Static locals are initialized once and guaranteed thread-safe (C++11).
        static std::array<wxString, static_cast<int>(StrId::COUNT)> s_en;
        static std::array<wxString, static_cast<int>(StrId::COUNT)> s_sr;
        static bool s_inited = false;
        if (!s_inited)
        {
            s_inited = true;
            for (int i = 0; i < static_cast<int>(StrId::COUNT); ++i)
            {
                s_en[i] = wxString(kEn[i]);
                s_sr[i] = wxString(kSr[i]);
            }
        }
        int idx = static_cast<int>(id);
        return (s_lang == AppLang::Serbian) ? s_sr[idx] : s_en[idx];
    }
}
