#include "HelpDialog.h"
#include "../I18n.h"
#include <wx/html/htmlwin.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/utils.h>

// ── Embedded EN help HTML ─────────────────────────────────────────────────────
static const char* kHelpEn = R"html(
<html>
<body bgcolor="#ffffff">
<font face="Arial,Helvetica,sans-serif" size="3">

<h2>ZPL Editor &mdash; Help</h2>
<hr>

<h3>&#9741; Getting Started</h3>
<p>
ZPL Editor is a WYSIWYG (What You See Is What You Get) label designer for
<b>Zebra ZPL printers</b>. Design your label visually on the canvas and export
the generated ZPL code to send directly to your printer.
</p>

<h3>&#9741; File Operations</h3>
<table width="100%" cellpadding="4" cellspacing="0">
<tr><td width="140"><b>New Label</b></td>
    <td>Create a new label. Choose size in mm or inches, and DPI (dots-per-inch).</td></tr>
<tr><td><b>Open</b></td>
    <td>Open a previously saved <tt>.zpl.json</tt> label file.</td></tr>
<tr><td><b>Save / Save As</b></td>
    <td>Save the label as a <tt>.zpl.json</tt> project file.</td></tr>
<tr><td><b>Print</b></td>
    <td>Print a preview of the label using the system print dialog.</td></tr>
</table>

<h3>&#9741; Toolbox</h3>
<p>The toolbox is on the left side. Click a tool to activate it.</p>
<table width="100%" cellpadding="4" cellspacing="0">
<tr><td width="160"><b>&#10148; Select / Move</b></td>
    <td>Select, move, and resize elements. Click an element to select it;
        drag a blank area to draw a marquee selection rectangle.</td></tr>
<tr><td><b>T Text</b></td>
    <td>Click on the canvas to place a text element. Set font, size, and
        orientation in the Properties panel.</td></tr>
<tr><td><b>&#9612;&#9612;&#9612; Barcode</b></td>
    <td>Click on the canvas to place a barcode element. Choose the barcode
        type (Code 128, QR Code, etc.) and enter the data in the Properties
        panel.</td></tr>
<tr><td><b>&#9645; Rectangle</b></td>
    <td>Click and drag to draw a filled or outlined rectangle.</td></tr>
<tr><td><b>&#9651; Image</b></td>
    <td>Click to place an image. Use the Properties panel to load a PNG or
        BMP file.</td></tr>
</table>

<h3>&#9741; Canvas</h3>
<ul>
  <li><b>Pan</b> &mdash; Hold <b>Space</b> and drag, or use the middle mouse button.</li>
  <li><b>Zoom</b> &mdash; <b>Ctrl + Scroll</b>, or use the <b>+</b> / <b>-</b> toolbar buttons.
      Press <b>Ctrl+0</b> to fit the label in the window.</li>
  <li><b>Snap to Grid</b> &mdash; Toggle via <i>Options &rarr; Snap to Grid</i> or the status bar.</li>
</ul>

<h3>&#9741; Properties Panel</h3>
<p>
The <b>Properties Panel</b> (right side) shows editable properties for the currently selected element.
</p>
<table width="100%" cellpadding="4" cellspacing="0">
<tr><td width="160"><b>Text element</b></td>
    <td>Text content, font face, font size, bold/italic/underline, character width
        (unlink to set independently), orientation.</td></tr>
<tr><td><b>Barcode element</b></td>
    <td>Data string, barcode type (symbology), module width, bar height,
        human-readable text toggle.</td></tr>
<tr><td><b>Rectangle element</b></td>
    <td>Line thickness, fill color (black/white), corner rounding.</td></tr>
<tr><td><b>Image element</b></td>
    <td>Source file path, scaling mode.</td></tr>
<tr><td><b>All elements</b></td>
    <td>X/Y position and Width/Height (in dots), accessible at the bottom of the panel.</td></tr>
</table>

<h3>&#9741; Alignment Tools</h3>
<p>
When two or more elements are selected, the alignment toolbar becomes active.
</p>
<table width="100%" cellpadding="4" cellspacing="0">
<tr><td width="200"><b>Align Left Edges</b></td>       <td>Align all selected elements to the leftmost edge.</td></tr>
<tr><td><b>Align Right Edges</b></td>      <td>Align all selected elements to the rightmost edge.</td></tr>
<tr><td><b>Align Top Edges</b></td>        <td>Align all selected elements to the topmost edge.</td></tr>
<tr><td><b>Align Bottom Edges</b></td>     <td>Align all selected elements to the bottommost edge.</td></tr>
<tr><td><b>Center Horizontally</b></td>    <td>Center all selected elements horizontally relative to the label.</td></tr>
<tr><td><b>Center Vertically</b></td>      <td>Center all selected elements vertically relative to the label.</td></tr>
<tr><td><b>Center H (elements)</b></td>    <td>Center elements relative to each other on the horizontal axis.</td></tr>
<tr><td><b>Center V (elements)</b></td>    <td>Center elements relative to each other on the vertical axis.</td></tr>
</table>

<h3>&#9741; Undo / Redo</h3>
<p>
Every canvas change is recorded. Use <b>Ctrl+Z</b> to undo and <b>Ctrl+Y</b>
(or <b>Ctrl+Shift+Z</b>) to redo.
</p>

<h3>&#9741; ZPL Code Panel</h3>
<p>
Toggle the ZPL code panel via <i>Options &rarr; Show ZPL Code</i>. The generated
ZPL code updates live as you edit the label. You can copy it from there to send
to your printer via a network socket or USB raw print.
</p>

<h3>&#9741; Keyboard Shortcuts</h3>
<table width="100%" cellpadding="4" cellspacing="0">
<tr><td width="200"><b>Ctrl+N</b></td>  <td>New label</td></tr>
<tr><td><b>Ctrl+O</b></td>  <td>Open label file</td></tr>
<tr><td><b>Ctrl+S</b></td>  <td>Save</td></tr>
<tr><td><b>Ctrl+Shift+S</b></td>  <td>Save As</td></tr>
<tr><td><b>Ctrl+Z</b></td>  <td>Undo</td></tr>
<tr><td><b>Ctrl+Y</b></td>  <td>Redo</td></tr>
<tr><td><b>Ctrl+D</b></td>  <td>Duplicate selected element(s)</td></tr>
<tr><td><b>Delete</b></td>  <td>Delete selected element(s)</td></tr>
<tr><td><b>Ctrl+P</b></td>  <td>Print</td></tr>
<tr><td><b>Ctrl+0</b></td>  <td>Zoom to fit</td></tr>
<tr><td><b>Ctrl++</b></td>  <td>Zoom in</td></tr>
<tr><td><b>Ctrl+-</b></td>  <td>Zoom out</td></tr>
<tr><td><b>Space + Drag</b></td>  <td>Pan the canvas</td></tr>
<tr><td><b>F1</b></td>  <td>Open this Help window</td></tr>
</table>

<h3>&#9741; Options</h3>
<ul>
  <li><b>Language</b> &mdash; Switch between English and Serbian. Restart not required.</li>
  <li><b>Units</b> &mdash; Switch between metric (mm) and imperial (inches) for the New Label dialog.</li>
  <li><b>Snap to Grid</b> &mdash; Enable/disable grid snapping. Grid size is configurable.</li>
  <li><b>Grid Size</b> &mdash; Choose from 2, 5, 10, or a custom number of dots.</li>
</ul>

<br>
<hr>
<font size="2" color="#888888">ZPL Editor v0.1.0 &mdash; <a href="https://github.com/ButcherZV/ZPL-Editor">https://github.com/ButcherZV/ZPL-Editor</a></font>

</font>
</body>
</html>
)html";


// ── Embedded SR help HTML ─────────────────────────────────────────────────────
static const char* kHelpSr = R"html(
<html>
<body bgcolor="#ffffff">
<font face="Arial,Helvetica,sans-serif" size="3">

<h2>ZPL Editor &mdash; Pomoć</h2>
<hr>

<h3>&#9741; Uvod</h3>
<p>
ZPL Editor je WYSIWYG (Šta vidiš, to i dobiješ) dizajner etiketa za
<b>Zebra ZPL štampače</b>. Dizajnirajte etiketu vizuelno na platnu i izvezite
generisani ZPL kod koji možete direktno poslati štampaču.
</p>

<h3>&#9741; Rad sa fajlovima</h3>
<table width="100%" cellpadding="4" cellspacing="0">
<tr><td width="160"><b>Nova etiketa</b></td>
    <td>Kreirajte novu etiketu. Izaberite veličinu u mm ili inčima i DPI (tačke po inču).</td></tr>
<tr><td><b>Otvori</b></td>
    <td>Otvorite prethodno sačuvan <tt>.zpl.json</tt> fajl etikete.</td></tr>
<tr><td><b>Sačuvaj / Sačuvaj kao</b></td>
    <td>Sačuvajte etiketu kao projektni fajl <tt>.zpl.json</tt>.</td></tr>
<tr><td><b>Štampaj</b></td>
    <td>Odštampajte pregled etikete koristeći sistemski dijalog za štampanje.</td></tr>
</table>

<h3>&#9741; Kutija s alatima</h3>
<p>Kutija s alatima se nalazi na levoj strani. Kliknite na alat da ga aktivirate.</p>
<table width="100%" cellpadding="4" cellspacing="0">
<tr><td width="180"><b>&#10148; Selektuj / Pomeri</b></td>
    <td>Selektujte, pomerajte i menjajte veličinu elemenata. Kliknite na element da
        ga selektujete; prevucite po praznom prostoru da nacrtate okvir za višestruku selekciju.</td></tr>
<tr><td><b>T Tekst</b></td>
    <td>Kliknite na platno da postavite tekstualni element. Podesite font, veličinu
        i orijentaciju u panelu Svojstava.</td></tr>
<tr><td><b>&#9612;&#9612;&#9612; Barkod</b></td>
    <td>Kliknite na platno da postavite barkod element. Izaberite tip barkoda
        (Code 128, QR kod, itd.) i unesite podatke u panelu Svojstava.</td></tr>
<tr><td><b>&#9645; Pravougaonik</b></td>
    <td>Kliknite i prevucite da nacrtate popunjen ili ispražnjen pravougaonik.</td></tr>
<tr><td><b>&#9651; Slika</b></td>
    <td>Kliknite da postavite sliku. Koristite panel Svojstava da učitate PNG ili BMP fajl.</td></tr>
</table>

<h3>&#9741; Platno</h3>
<ul>
  <li><b>Pomeranje prikaza</b> &mdash; Držite <b>Razmaknica</b> i prevucite, ili koristite srednji taster miša.</li>
  <li><b>Zum</b> &mdash; <b>Ctrl + Scroll</b>, ili koristite dugmad <b>+</b> / <b>-</b> na traci s alatima.
      Pritisnite <b>Ctrl+0</b> da prilagodite etiketu prozoru.</li>
  <li><b>Poravnanje na mrežu</b> &mdash; Uključite/isključite putem <i>Opcije &rarr; Poravnanje na mrežu</i> ili statusne trake.</li>
</ul>

<h3>&#9741; Panel svojstava</h3>
<p>
<b>Panel Svojstava</b> (desna strana) prikazuje editabilna svojstva za trenutno selektovani element.
</p>
<table width="100%" cellpadding="4" cellspacing="0">
<tr><td width="180"><b>Tekstualni element</b></td>
    <td>Sadržaj teksta, font, veličina fonta, podebljano/kurziv/podvučeno, širina znaka
        (odvojite od visine za nezavisno podešavanje), orijentacija.</td></tr>
<tr><td><b>Barkod element</b></td>
    <td>Niz podataka, tip barkoda (simbologija), širina modula, visina pruga,
        uključivanje teksta ispod barkoda.</td></tr>
<tr><td><b>Element pravougaonika</b></td>
    <td>Debljina linije, boja popune (crna/bela), zaobljenost uglova.</td></tr>
<tr><td><b>Element slike</b></td>
    <td>Putanja do izvorne datoteke, način skaliranja.</td></tr>
<tr><td><b>Svi elementi</b></td>
    <td>Pozicija X/Y i širina/visina (u tačkama), dostupno na dnu panela.</td></tr>
</table>

<h3>&#9741; Alati za poravnanje</h3>
<p>
Kada su selektovana dva ili više elemenata, traka za poravnanje postaje aktivna.
</p>
<table width="100%" cellpadding="4" cellspacing="0">
<tr><td width="220"><b>Poravnaj leve ivice</b></td>      <td>Poravnajte sve selektovane elemente na krajnju levu ivicu.</td></tr>
<tr><td><b>Poravnaj desne ivice</b></td>     <td>Poravnajte sve selektovane elemente na krajnju desnu ivicu.</td></tr>
<tr><td><b>Poravnaj gornje ivice</b></td>    <td>Poravnajte sve selektovane elemente na gornju ivicu.</td></tr>
<tr><td><b>Poravnaj donje ivice</b></td>     <td>Poravnajte sve selektovane elemente na donju ivicu.</td></tr>
<tr><td><b>Centriraj horizontalno</b></td>   <td>Centrirajte sve selektovane elemente horizontalno u odnosu na etiketu.</td></tr>
<tr><td><b>Centriraj vertikalno</b></td>     <td>Centrirajte sve selektovane elemente vertikalno u odnosu na etiketu.</td></tr>
<tr><td><b>Centriraj H (elementi)</b></td>   <td>Centrirajte elemente međusobno na horizontalnoj osi.</td></tr>
<tr><td><b>Centriraj V (elementi)</b></td>   <td>Centrirajte elemente međusobno na vertikalnoj osi.</td></tr>
</table>

<h3>&#9741; Opoziv / Ponovi</h3>
<p>
Svaka promena na platnu se beleži. Koristite <b>Ctrl+Z</b> za opoziv i <b>Ctrl+Y</b>
(ili <b>Ctrl+Shift+Z</b>) za ponavljanje.
</p>

<h3>&#9741; Panel ZPL koda</h3>
<p>
Uključite panel ZPL koda putem <i>Opcije &rarr; Prikaži ZPL kod</i>. Generisani ZPL kod
se ažurira u realnom vremenu dok uređujete etiketu. Možete ga kopirati i poslati
štampaču putem mrežne veze ili USB direktnog štampanja.
</p>

<h3>&#9741; Tastaturni prečaci</h3>
<table width="100%" cellpadding="4" cellspacing="0">
<tr><td width="200"><b>Ctrl+N</b></td>  <td>Nova etiketa</td></tr>
<tr><td><b>Ctrl+O</b></td>  <td>Otvori fajl etikete</td></tr>
<tr><td><b>Ctrl+S</b></td>  <td>Sačuvaj</td></tr>
<tr><td><b>Ctrl+Shift+S</b></td>  <td>Sačuvaj kao</td></tr>
<tr><td><b>Ctrl+Z</b></td>  <td>Opoziv</td></tr>
<tr><td><b>Ctrl+Y</b></td>  <td>Ponovi</td></tr>
<tr><td><b>Ctrl+D</b></td>  <td>Dupliraj selektovane elemente</td></tr>
<tr><td><b>Delete</b></td>  <td>Obriši selektovane elemente</td></tr>
<tr><td><b>Ctrl+P</b></td>  <td>Štampaj</td></tr>
<tr><td><b>Ctrl+0</b></td>  <td>Zum - uklopi etiketu</td></tr>
<tr><td><b>Ctrl++</b></td>  <td>Uvećaj</td></tr>
<tr><td><b>Ctrl+-</b></td>  <td>Umanji</td></tr>
<tr><td><b>Razmaknica + Prevlačenje</b></td>  <td>Pomeri prikaz platna</td></tr>
<tr><td><b>F1</b></td>  <td>Otvori ovaj prozor pomoći</td></tr>
</table>

<h3>&#9741; Opcije</h3>
<ul>
  <li><b>Jezik</b> &mdash; Promenite između srpskog i engleskog. Restart nije potreban.</li>
  <li><b>Jedinice</b> &mdash; Promenite između metričkih (mm) i imperijalnih (inči) za dijalog Nova etiketa.</li>
  <li><b>Poravnanje na mrežu</b> &mdash; Uključite/isključite poravnanje na mrežu. Veličina mreže se može podesiti.</li>
  <li><b>Veličina mreže</b> &mdash; Izaberite između 2, 5, 10 ili prilagođenog broja tačaka.</li>
</ul>

<br>
<hr>
<font size="2" color="#888888">ZPL Editor v0.1.0 &mdash; <a href="https://github.com/ButcherZV/ZPL-Editor">https://github.com/ButcherZV/ZPL-Editor</a></font>

</font>
</body>
</html>
)html";


// ── HelpDialog implementation ─────────────────────────────────────────────────

HelpDialog::HelpDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, wxEmptyString,
               wxDefaultPosition, wxSize(760, 580),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    SetTitle(TR(HELP_TITLE));

    auto* html = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                  wxHW_SCROLLBAR_AUTO);
    html->SetBorders(8);

    // Link clicks → open in default browser
    html->Bind(wxEVT_HTML_LINK_CLICKED, [](wxHtmlLinkEvent& e) {
        wxLaunchDefaultBrowser(e.GetLinkInfo().GetHref());
    });

    const bool isSr = (I18n::GetLanguage() == AppLang::Serbian);
    html->SetPage(wxString::FromUTF8(isSr ? kHelpSr : kHelpEn));

    auto* close = new wxButton(this, wxID_CLOSE, _("Close"));
    close->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { EndModal(wxID_CLOSE); });

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(html, 1, wxEXPAND | wxALL, 4);
    sizer->Add(close, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 8);
    SetSizerAndFit(sizer);
    SetSize(760, 580);
    CentreOnParent();
}
