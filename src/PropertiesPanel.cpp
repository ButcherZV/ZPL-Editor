#include "PropertiesPanel.h"
#include "LabelCanvas.h"
#include "AppConfig.h"
#include "elements/LabelElement.h"
#include "elements/TextElement.h"
#include "elements/BarcodeElement.h"
#include "elements/BoxElement.h"
#include "elements/ImageElement.h"
#include "commands/EditPropertyCommand.h"
#include "I18n.h"
#include <wx/fontenum.h>

wxBEGIN_EVENT_TABLE(PropertiesPanel, wxPanel)
    EVT_PG_CHANGED(wxID_ANY, PropertiesPanel::OnPropertyChanged)
wxEND_EVENT_TABLE()

PropertiesPanel::PropertiesPanel(wxWindow* parent, LabelCanvas* canvas)
    : wxPanel(parent, wxID_ANY)
    , m_canvas(canvas)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(new wxStaticText(this, wxID_ANY, TR(PROP_PANEL_TITLE)),
               0, wxALL | wxALIGN_CENTER, 4);
    m_grid = new wxPropertyGrid(this, wxID_ANY,
                                wxDefaultPosition, wxDefaultSize,
                                wxPG_SPLITTER_AUTO_CENTER | wxPG_BOLD_MODIFIED);
    sizer->Add(m_grid, 1, wxEXPAND);
    SetSizer(sizer);
}

void PropertiesPanel::ShowElement(LabelElement* el)
{
    m_element = el;
    m_grid->Clear();

    if (!el)
        return;

    PopulateCommon(el);

    if      (dynamic_cast<TextElement*>   (el)) PopulateText(el);
    else if (dynamic_cast<BarcodeElement*>(el)) PopulateBarcode(el);
    else if (dynamic_cast<BoxElement*>    (el)) PopulateBox(el);
    else if (dynamic_cast<ImageElement*>  (el)) PopulateImage(el);
}

void PropertiesPanel::RefreshUnits()
{
    ShowElement(m_element);
}

// ── Grid population ───────────────────────────────────────────────────────────

void PropertiesPanel::PopulateCommon(LabelElement* el)
{
    PrinterDPI dpi    = m_canvas ? m_canvas->GetConfig().dpi : PrinterDPI::DPI_203;
    bool       metric = (AppConfig::Get().units == MeasureUnit::Metric);
    const wxString unit = metric ? " (mm)" : " (in)";
    auto toDisp = [&](int dots) -> double {
        return metric ? DotsToMM(dots, dpi) : DotsToInches(dots, dpi);
    };
    m_grid->Append(new wxPropertyCategory(TR(PROP_POS_SIZE)));
    m_grid->Append(new wxFloatProperty(TR(PROP_X)      + unit, "x", toDisp(el->x)));
    m_grid->Append(new wxFloatProperty(TR(PROP_Y)      + unit, "y", toDisp(el->y)));
    m_grid->Append(new wxFloatProperty(TR(PROP_WIDTH)  + unit, "w", toDisp(el->w)));
    m_grid->Append(new wxFloatProperty(TR(PROP_HEIGHT) + unit, "h", toDisp(el->h)));
    m_grid->SetPropertyAttribute("x", wxPG_FLOAT_PRECISION, 2L);
    m_grid->SetPropertyAttribute("y", wxPG_FLOAT_PRECISION, 2L);
    m_grid->SetPropertyAttribute("w", wxPG_FLOAT_PRECISION, 2L);
    m_grid->SetPropertyAttribute("h", wxPG_FLOAT_PRECISION, 2L);
}

void PropertiesPanel::PopulateText(LabelElement* elBase)
{
    auto* el = static_cast<TextElement*>(elBase);
    m_grid->Append(new wxPropertyCategory(TR(PROP_TEXT_ELEM)));
    m_grid->Append(new wxStringProperty(TR(PROP_CONTENT),    "text",     wxString::FromUTF8(el->text)));
    m_grid->Append(new wxIntProperty(   TR(PROP_FONT_SIZE),  "fontSize", el->fontSize));
    m_grid->Append(new wxIntProperty(   TR(PROP_FONT_WIDTH), "fontWidth", el->fontWidth));
    m_grid->Append(new wxBoolProperty(  TR(PROP_FONT_SIZE_LINKED), "fontSizeLinked", el->fontSizeLinked));
    m_grid->SetPropertyAttribute("fontSizeLinked", wxPG_BOOL_USE_CHECKBOX, true);

    // Fonts category: printer font for ZPL output + system font for canvas rendering
    m_grid->Append(new wxPropertyCategory(TR(PROP_FONTS_CATEGORY)));
    m_grid->Append(new wxStringProperty(TR(PROP_PRINTER_FONT), "fontPath", wxString::FromUTF8(el->fontPath)));
    {
        wxArrayString faces = wxFontEnumerator::GetFacenames(); faces.Sort();
        wxArrayString fontChoices; wxArrayInt fontValues;
        fontChoices.Add(TR(PROP_FONT_DEFAULT)); fontValues.Add(0);
        for (size_t i = 0; i < faces.size(); ++i)
        {
            fontChoices.Add(faces[i]);
            fontValues.Add(static_cast<int>(i) + 1);
        }
        wxString curFace = wxString::FromUTF8(el->fontName);
        int selVal = 0;
        if (!el->fontName.empty())
        {
            int idx = faces.Index(curFace);
            if (idx != wxNOT_FOUND) selVal = idx + 1;
        }
        m_grid->Append(new wxEnumProperty(TR(PROP_DISPLAY_FONT), "fontName", fontChoices, fontValues, selVal));
    }
    m_grid->Append(new wxBoolProperty(  TR(PROP_BOLD),       "bold",      el->bold));
    m_grid->Append(new wxBoolProperty(  TR(PROP_ITALIC),     "italic",    el->italic));
    m_grid->SetPropertyAttribute("bold",   wxPG_BOOL_USE_CHECKBOX, true);
    m_grid->SetPropertyAttribute("italic", wxPG_BOOL_USE_CHECKBOX, true);

    wxArrayString rotChoices; rotChoices.Add("0");  rotChoices.Add("90");
                              rotChoices.Add("180"); rotChoices.Add("270");
    wxArrayInt    rotValues;  rotValues.Add(0); rotValues.Add(90);
                              rotValues.Add(180); rotValues.Add(270);
    m_grid->Append(new wxEnumProperty(TR(PROP_ROTATION), "rotation", rotChoices, rotValues, el->rotation));

    // Field Block (^FB)
    m_grid->Append(new wxPropertyCategory(TR(PROP_FIELD_BLOCK)));
    m_grid->Append(new wxBoolProperty(TR(PROP_FIELD_BLOCK),  "useFieldBlock",      el->useFieldBlock));
    m_grid->Append(new wxIntProperty( TR(PROP_FB_WIDTH),     "fieldBlockWidth",    el->fieldBlockWidth));
    m_grid->Append(new wxIntProperty( TR(PROP_FB_MAX_LINES), "fieldBlockMaxLines", el->fieldBlockMaxLines));
    m_grid->Append(new wxIntProperty( TR(PROP_LINE_SPACING),  "fieldBlockLineSpacing", el->fieldBlockLineSpacing));
    m_grid->SetPropertyAttribute("useFieldBlock", wxPG_BOOL_USE_CHECKBOX, true);
    wxArrayString justChoices;
    justChoices.Add(TR(PROP_JUSTIFY_LEFT));      justChoices.Add(TR(PROP_JUSTIFY_RIGHT));
    justChoices.Add(TR(PROP_JUSTIFY_CENTRE));    justChoices.Add(TR(PROP_JUSTIFY_JUSTIFIED));
    wxArrayInt    justValues;  for (int i = 0; i < 4; ++i) justValues.Add(i);
    m_grid->Append(new wxEnumProperty(TR(PROP_FB_JUSTIFY), "fieldBlockJustify",
                                      justChoices, justValues, el->fieldBlockJustify));
}

void PropertiesPanel::PopulateBarcode(LabelElement* elBase)
{
    auto* el = static_cast<BarcodeElement*>(elBase);
    m_grid->Append(new wxPropertyCategory(TR(PROP_BARCODE_ELEM)));

    wxArrayString typeChoices;
    typeChoices.Add("Code 128"); typeChoices.Add("Code 39");
    typeChoices.Add("EAN-13");   typeChoices.Add("UPC-A");
    typeChoices.Add("I2of5");    typeChoices.Add("QR Code");
    wxArrayInt typeValues;
    for (int i = 0; i < 6; ++i) typeValues.Add(i);

    m_grid->Append(new wxEnumProperty(TR(PROP_BARCODE_TYPE), "barcodeType",
                                      typeChoices, typeValues,
                                      static_cast<int>(el->barcodeType)));
    m_grid->Append(new wxStringProperty(TR(PROP_DATA),        "data",       wxString::FromUTF8(el->data)));
    m_grid->Append(new wxIntProperty(  TR(PROP_BAR_HEIGHT),  "barHeight",  el->barHeight));
    m_grid->Append(new wxBoolProperty( TR(PROP_SHOW_TEXT),   "showText",   el->showText));
    m_grid->Append(new wxBoolProperty( TR(PROP_CHECK_DIGIT), "checkDigit", el->checkDigit));
    m_grid->SetPropertyAttribute("showText",   wxPG_BOOL_USE_CHECKBOX, true);
    m_grid->SetPropertyAttribute("checkDigit", wxPG_BOOL_USE_CHECKBOX, true);
}

void PropertiesPanel::PopulateBox(LabelElement* elBase)
{
    auto* el = static_cast<BoxElement*>(elBase);
    m_grid->Append(new wxPropertyCategory(TR(PROP_BOX_ELEM)));

    wxArrayString shapeChoices;
    shapeChoices.Add(TR(PROP_SHAPE_RECT));    shapeChoices.Add(TR(PROP_SHAPE_CIRCLE));
    shapeChoices.Add(TR(PROP_SHAPE_DIAG_BK)); shapeChoices.Add(TR(PROP_SHAPE_DIAG_FW));
    shapeChoices.Add(TR(PROP_SHAPE_ELLIPSE));
    wxArrayInt shapeValues;
    for (int i = 0; i < 5; ++i) shapeValues.Add(i);

    m_grid->Append(new wxEnumProperty(  TR(PROP_SHAPE),            "shape",       shapeChoices, shapeValues, static_cast<int>(el->shape)));
    m_grid->Append(new wxIntProperty(   TR(PROP_BORDER_THICK), "borderThick", el->borderThick));
    m_grid->Append(new wxColourProperty(TR(PROP_BORDER_COLOR),    "borderColour",el->borderColour));
    m_grid->Append(new wxColourProperty(TR(PROP_FILL_COLOR),      "fillColour",  el->fillColour));
    m_grid->Append(new wxBoolProperty(  TR(PROP_FILLED),           "filled",      el->filled));
    m_grid->SetPropertyAttribute("filled", wxPG_BOOL_USE_CHECKBOX, true);
}

void PropertiesPanel::PopulateImage(LabelElement* elBase)
{
    auto* el = static_cast<ImageElement*>(elBase);
    m_grid->Append(new wxPropertyCategory(TR(PROP_IMAGE_ELEM)));
    m_grid->Append(new wxStringProperty(TR(PROP_FILE_PATH), "filePath",
                                        wxString::FromUTF8(el->filePath)));
    m_grid->SetPropertyReadOnly("filePath", true);
}

// ── Property change handler ───────────────────────────────────────────────────

void PropertiesPanel::OnPropertyChanged(wxPropertyGridEvent& evt)
{
    if (!m_element || !m_canvas)
        return;

    wxString  name  = evt.GetPropertyName();
    wxVariant value = evt.GetPropertyValue();
    LabelElement* el = m_element;
    PrinterDPI dpi   = m_canvas->GetConfig().dpi;
    bool metric      = (AppConfig::Get().units == MeasureUnit::Metric);
    auto fromDisp    = [&](double v) {
        return metric ? MMToDots(v, dpi) : InchesToDots(v, dpi);
    };

    // ── Common position / size ───────────────────────────────────────────────
    if (name == "x")
    {
        int o = el->x, n = fromDisp(value.GetDouble());
        m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<int>>(
            [el]{ return el->x; }, [el](int v){ el->x = v; }, o, n));
    }
    else if (name == "y")
    {
        int o = el->y, n = fromDisp(value.GetDouble());
        m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<int>>(
            [el]{ return el->y; }, [el](int v){ el->y = v; }, o, n));
    }
    else if (name == "w")
    {
        int o = el->w, n = fromDisp(value.GetDouble());
        m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<int>>(
            [el]{ return el->w; }, [el](int v){ el->w = v; }, o, n));
    }
    else if (name == "h")
    {
        int o = el->h, n = fromDisp(value.GetDouble());
        m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<int>>(
            [el]{ return el->h; }, [el](int v){ el->h = v; }, o, n));
    }
    // ── TextElement ──────────────────────────────────────────────────────────
    else if (name == "text")
    {
        if (auto* t = dynamic_cast<TextElement*>(el))
        {
            std::string o = t->text;
            std::string n = value.GetString().ToUTF8().data();
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<std::string>>(
                [t]{ return t->text; }, [t](std::string v){ t->text = v; }, o, n));
        }
    }
    else if (name == "fontSize")
    {
        if (auto* t = dynamic_cast<TextElement*>(el))
        {
            int o = t->fontSize, n = static_cast<int>(value.GetLong());
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<int>>(
                [t]{ return t->fontSize; }, [t](int v){ t->fontSize = v; }, o, n));
            if (t->fontSizeLinked)
            {
                int ow = t->fontWidth;
                m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<int>>(
                    [t]{ return t->fontWidth; }, [t](int v){ t->fontWidth = v; }, ow, n));
                m_grid->SetPropertyValue("fontWidth", n);
            }
        }
    }
    else if (name == "rotation")
    {
        if (auto* t = dynamic_cast<TextElement*>(el))
        {
            int o = t->rotation, n = static_cast<int>(value.GetLong());
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<int>>(
                [t]{ return t->rotation; }, [t](int v){ t->rotation = v; }, o, n));
        }
    }
    else if (name == "fontWidth")
    {
        if (auto* t = dynamic_cast<TextElement*>(el))
        {
            int o = t->fontWidth, n = static_cast<int>(value.GetLong());
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<int>>(
                [t]{ return t->fontWidth; }, [t](int v){ t->fontWidth = v; }, o, n));
            if (t->fontSizeLinked)
            {
                int oh = t->fontSize;
                m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<int>>(
                    [t]{ return t->fontSize; }, [t](int v){ t->fontSize = v; }, oh, n));
                m_grid->SetPropertyValue("fontSize", n);
            }
        }
    }
    else if (name == "fontSizeLinked")
    {
        if (auto* t = dynamic_cast<TextElement*>(el))
        {
            bool o = t->fontSizeLinked, n = value.GetBool();
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<bool>>(
                [t]{ return t->fontSizeLinked; }, [t](bool v){ t->fontSizeLinked = v; }, o, n));
            // When linking is enabled, immediately sync width = height
            if (n)
            {
                int ow = t->fontWidth;
                m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<int>>(
                    [t]{ return t->fontWidth; }, [t](int v){ t->fontWidth = v; }, ow, t->fontSize));
                m_grid->SetPropertyValue("fontWidth", t->fontSize);
            }
        }
    }
    else if (name == "fontPath")
    {
        if (auto* t = dynamic_cast<TextElement*>(el))
        {
            std::string o = t->fontPath, n = value.GetString().ToUTF8().data();
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<std::string>>(
                [t]{ return t->fontPath; }, [t](std::string v){ t->fontPath = v; }, o, n));
        }
    }
    else if (name == "fontName")
    {
        if (auto* t = dynamic_cast<TextElement*>(el))
        {
            std::string o = t->fontName;
            wxString label = evt.GetProperty()->GetValueAsString();
            std::string n = (label == "(default)") ? "" : label.ToUTF8().data();
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<std::string>>(
                [t]{ return t->fontName; }, [t](std::string v){ t->fontName = v; }, o, n));
        }
    }
    else if (name == "bold")
    {
        if (auto* t = dynamic_cast<TextElement*>(el))
        {
            bool o = t->bold, n = value.GetBool();
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<bool>>(
                [t]{ return t->bold; }, [t](bool v){ t->bold = v; }, o, n));
        }
    }
    else if (name == "italic")
    {
        if (auto* t = dynamic_cast<TextElement*>(el))
        {
            bool o = t->italic, n = value.GetBool();
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<bool>>(
                [t]{ return t->italic; }, [t](bool v){ t->italic = v; }, o, n));
        }
    }
    else if (name == "useFieldBlock")
    {
        if (auto* t = dynamic_cast<TextElement*>(el))
        {
            bool o = t->useFieldBlock, n = value.GetBool();
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<bool>>(
                [t]{ return t->useFieldBlock; }, [t](bool v){ t->useFieldBlock = v; }, o, n));
        }
    }
    else if (name == "fieldBlockWidth")
    {
        if (auto* t = dynamic_cast<TextElement*>(el))
        {
            int o = t->fieldBlockWidth, n = static_cast<int>(value.GetLong());
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<int>>(
                [t]{ return t->fieldBlockWidth; }, [t](int v){ t->fieldBlockWidth = v; t->w = v; }, o, n));
        }
    }
    else if (name == "fieldBlockMaxLines")
    {
        if (auto* t = dynamic_cast<TextElement*>(el))
        {
            int o = t->fieldBlockMaxLines, n = static_cast<int>(value.GetLong());
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<int>>(
                [t]{ return t->fieldBlockMaxLines; }, [t](int v){ t->fieldBlockMaxLines = v; }, o, n));
        }
    }
    else if (name == "fieldBlockLineSpacing")
    {
        if (auto* t = dynamic_cast<TextElement*>(el))
        {
            int o = t->fieldBlockLineSpacing, n = static_cast<int>(value.GetLong());
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<int>>(
                [t]{ return t->fieldBlockLineSpacing; }, [t](int v){ t->fieldBlockLineSpacing = v; }, o, n));
        }
    }
    else if (name == "fieldBlockJustify")
    {
        if (auto* t = dynamic_cast<TextElement*>(el))
        {
            int o = t->fieldBlockJustify, n = static_cast<int>(value.GetLong());
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<int>>(
                [t]{ return t->fieldBlockJustify; }, [t](int v){ t->fieldBlockJustify = v; }, o, n));
        }
    }
    // ── BarcodeElement ───────────────────────────────────────────────────────
    else if (name == "barcodeType")
    {
        if (auto* b = dynamic_cast<BarcodeElement*>(el))
        {
            auto o = b->barcodeType;
            auto n = static_cast<BarcodeType>(value.GetLong());
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<BarcodeType>>(
                [b]{ return b->barcodeType; }, [b](BarcodeType v){ b->barcodeType = v; }, o, n));
        }
    }
    else if (name == "data")
    {
        if (auto* b = dynamic_cast<BarcodeElement*>(el))
        {
            std::string o = b->data;
            std::string n = value.GetString().ToUTF8().data();
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<std::string>>(
                [b]{ return b->data; }, [b](std::string v){ b->data = v; }, o, n));
        }
    }
    else if (name == "barHeight")
    {
        if (auto* b = dynamic_cast<BarcodeElement*>(el))
        {
            int o = b->barHeight, n = static_cast<int>(value.GetLong());
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<int>>(
                [b]{ return b->barHeight; }, [b](int v){ b->barHeight = v; }, o, n));
        }
    }
    else if (name == "showText")
    {
        if (auto* b = dynamic_cast<BarcodeElement*>(el))
        {
            bool o = b->showText, n = value.GetBool();
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<bool>>(
                [b]{ return b->showText; }, [b](bool v){ b->showText = v; }, o, n));
        }
    }
    else if (name == "checkDigit")
    {
        if (auto* b = dynamic_cast<BarcodeElement*>(el))
        {
            bool o = b->checkDigit, n = value.GetBool();
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<bool>>(
                [b]{ return b->checkDigit; }, [b](bool v){ b->checkDigit = v; }, o, n));
        }
    }
    // ── BoxElement ───────────────────────────────────────────────────────────
    else if (name == "shape")
    {
        if (auto* bx = dynamic_cast<BoxElement*>(el))
        {
            auto o = bx->shape;
            auto n = static_cast<BoxShape>(value.GetLong());
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<BoxShape>>(
                [bx]{ return bx->shape; }, [bx](BoxShape v){ bx->shape = v; }, o, n));
        }
    }
    else if (name == "borderThick")
    {
        if (auto* bx = dynamic_cast<BoxElement*>(el))
        {
            int o = bx->borderThick, n = static_cast<int>(value.GetLong());
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<int>>(
                [bx]{ return bx->borderThick; }, [bx](int v){ bx->borderThick = v; }, o, n));
        }
    }
    else if (name == "filled")
    {
        if (auto* bx = dynamic_cast<BoxElement*>(el))
        {
            bool o = bx->filled, n = value.GetBool();
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<bool>>(
                [bx]{ return bx->filled; }, [bx](bool v){ bx->filled = v; }, o, n));
        }
    }
    else if (name == "borderColour")
    {
        if (auto* bx = dynamic_cast<BoxElement*>(el))
        {
            wxColour o = bx->borderColour;
            wxColour n; n << value;
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<wxColour>>(
                [bx]{ return bx->borderColour; }, [bx](wxColour v){ bx->borderColour = v; }, o, n));
        }
    }
    else if (name == "fillColour")
    {
        if (auto* bx = dynamic_cast<BoxElement*>(el))
        {
            wxColour o = bx->fillColour;
            wxColour n; n << value;
            m_canvas->ExecuteCommand(std::make_unique<EditPropertyCommand<wxColour>>(
                [bx]{ return bx->fillColour; }, [bx](wxColour v){ bx->fillColour = v; }, o, n));
        }
    }
}

