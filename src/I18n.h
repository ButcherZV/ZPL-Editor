#pragma once
#include <wx/wx.h>

// ─────────────────────────────────────────────────────────────────────────────
// Supported UI languages
// ─────────────────────────────────────────────────────────────────────────────
enum class AppLang { English = 0, Serbian = 1 };

// ─────────────────────────────────────────────────────────────────────────────
// All user-visible string identifiers  (keep in the SAME ORDER as the
// string tables in I18n.cpp)
// ─────────────────────────────────────────────────────────────────────────────
enum class StrId
{
    // Application titles
    APP_TITLE = 0,
    TITLE_NEW_LABEL,

    // ── Menu – File ───────────────────────────────────────────────────────────
    MENU_FILE,
    MENU_NEW,
    MENU_OPEN,
    MENU_RECENT_FILES,
    MENU_SAVE,
    MENU_SAVE_AS,
    MENU_PRINT,
    MENU_EXIT,

    // ── Menu – Edit ───────────────────────────────────────────────────────────
    MENU_EDIT,
    MENU_UNDO,
    MENU_REDO,
    MENU_DUPLICATE,
    MENU_ALIGN,
    MENU_ALIGN_LEFT,
    MENU_ALIGN_RIGHT,
    MENU_ALIGN_TOP,
    MENU_ALIGN_BOTTOM,
    MENU_ALIGN_CENTERH,
    MENU_ALIGN_CENTERV,
    MENU_OPTIONS,

    // ── Menu – View ───────────────────────────────────────────────────────────
    MENU_VIEW,
    MENU_ZOOM_IN,
    MENU_ZOOM_OUT,
    MENU_ZOOM_FIT,
    MENU_SNAP_GRID,
    MENU_SHOW_ZPL,
    MENU_LANGUAGE,
    STR_LANG_ENGLISH,
    STR_LANG_SERBIAN,

    // ── AUI panel captions ────────────────────────────────────────────────────
    PANEL_CANVAS,
    PANEL_TOOLBOX,
    PANEL_PROPERTIES,
    PANEL_ZPL_CODE,

    // ── Status bar ────────────────────────────────────────────────────────────
    STATUS_READY,
    STATUS_NO_LABEL,
    STATUS_SNAP_OFF,
    STATUS_SNAP_ON,
    STATUS_LABEL_INFO,
    STATUS_ZOOM,
    STATUS_PRINT_SENT,

    // ── Toolbox ───────────────────────────────────────────────────────────────
    TOOL_SELECT,
    TOOL_TEXT,
    TOOL_BARCODE,
    TOOL_BOX,
    TOOL_IMAGE,
    TOOLTIP_SELECT,
    TOOLTIP_TEXT,
    TOOLTIP_BARCODE,
    TOOLTIP_BOX,
    TOOLTIP_IMAGE,

    // ── Properties panel ─────────────────────────────────────────────────────
    PROP_PANEL_TITLE,
    PROP_POS_SIZE,
    PROP_X,
    PROP_Y,
    PROP_WIDTH,
    PROP_HEIGHT,
    PROP_TEXT_ELEM,
    PROP_CONTENT,
    PROP_FONT_SIZE,
    PROP_ROTATION,
    PROP_BARCODE_ELEM,
    PROP_BARCODE_TYPE,
    PROP_DATA,
    PROP_BAR_HEIGHT,
    PROP_SHOW_TEXT,
    PROP_CHECK_DIGIT,
    PROP_BOX_ELEM,
    PROP_SHAPE,
    PROP_SHAPE_RECT,
    PROP_SHAPE_CIRCLE,
    PROP_SHAPE_DIAG_BK,
    PROP_SHAPE_DIAG_FW,
    PROP_SHAPE_ELLIPSE,
    PROP_BORDER_THICK,
    PROP_BORDER_COLOR,
    PROP_FILL_COLOR,
    PROP_FILLED,
    PROP_IMAGE_ELEM,
    PROP_FILE_PATH,
    PROP_FONT_WIDTH,
    PROP_FONT_PATH,
    PROP_BOLD,
    PROP_ITALIC,
    PROP_FIELD_BLOCK,
    PROP_FB_WIDTH,
    PROP_FB_MAX_LINES,
    PROP_FB_JUSTIFY,

    // ── New Label dialog ──────────────────────────────────────────────────────
    NEWLABEL_TITLE,
    NEWLABEL_PRINTER_RES,
    NEWLABEL_PRINTER,
    NEWLABEL_DPI,
    NEWLABEL_MANUAL,
    NEWLABEL_LABEL_SIZE,
    NEWLABEL_UNITS,
    NEWLABEL_MM,
    NEWLABEL_INCHES,
    NEWLABEL_DOTS,
    NEWLABEL_WIDTH,
    NEWLABEL_HEIGHT,
    NEWLABEL_MARGINS,
    NEWLABEL_TOP,
    NEWLABEL_BOTTOM,
    NEWLABEL_LEFT,
    NEWLABEL_RIGHT,
    NEWLABEL_ORIENTATION,
    NEWLABEL_PORTRAIT,
    NEWLABEL_LANDSCAPE,
    NEWLABEL_LAYOUT,
    NEWLABEL_LABELS_PER_ROW,
    NEWLABEL_LPR_TOOLTIP,
    NEWLABEL_PREVIEW,
    NEWLABEL_INVALID_SIZE,
    NEWLABEL_INVALID_MSG,

    // ── Buttons ───────────────────────────────────────────────────────────────
    BTN_CREATE,
    BTN_CANCEL,
    BTN_OK,

    // ── File dialogs ─────────────────────────────────────────────────────────
    DLG_OPEN_TITLE,
    DLG_SAVE_TITLE,
    DLG_CHOOSE_IMAGE,
    FILE_FILTER_ZPL,
    FILE_FILTER_IMAGE,

    // ── Error messages ────────────────────────────────────────────────────────
    ERR_OPEN_FILE,
    ERR_PARSE,
    ERR_PARSE_TITLE,
    ERR_WRITE,
    ERR_TITLE,
    ERR_FILE_NOT_FOUND,
    ERR_RECENT_FILES,

    // ── Print ─────────────────────────────────────────────────────────────────
    PRINT_NO_ELEMENTS,
    PRINT_TITLE,
    PRINT_CANT_ENUM,
    PRINT_ERR_TITLE,
    PRINT_NO_PRINTERS,
    PRINT_SELECT,
    PRINT_DLG_TITLE,
    PRINT_CANT_OPEN,
    PRINT_ERR_START,
    PRINT_WIN_ONLY,

    // ── Language change ───────────────────────────────────────────────────────
    LANG_CHANGE_TITLE,
    LANG_CHANGE_MSG,

    // ── Context menu ─────────────────────────────────────────────────────────────
    CTX_DELETE,
    CTX_DUPLICATE,
    CTX_BRING_FRONT,
    CTX_SEND_BACK,
    CTX_PASTE,

    COUNT   // must be last
};

// ─────────────────────────────────────────────────────────────────────────────
// I18n API
// ─────────────────────────────────────────────────────────────────────────────
namespace I18n
{
    void            SetLanguage(AppLang lang);
    AppLang         GetLanguage();
    const wxString& Tr(StrId id);
}

// Convenience macro:  TR(APP_TITLE)  →  I18n::Tr(StrId::APP_TITLE)
#define TR(id) I18n::Tr(StrId::id)
