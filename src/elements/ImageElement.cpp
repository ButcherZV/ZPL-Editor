#include "ImageElement.h"
#include <wx/image.h>
#include <wx/zstream.h>
#include <wx/mstream.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cstring>

void ImageElement::Draw(wxDC& dc, wxPoint off, double zoom) const
{
    int px = off.x + static_cast<int>(x * zoom);
    int py = off.y + static_cast<int>(y * zoom);
    int pw = std::max(4, static_cast<int>(w * zoom));
    int ph = std::max(4, static_cast<int>(h * zoom));

    if (bitmap.IsOk())
    {
        wxImage scaled = bitmap.ConvertToImage().Scale(pw, ph, wxIMAGE_QUALITY_NORMAL);
        dc.DrawBitmap(wxBitmap(scaled), px, py, false);
    }
    else
    {
        // Placeholder
        dc.SetBrush(wxBrush(wxColour(220, 220, 255)));
        dc.SetPen(*wxBLACK_PEN);
        dc.DrawRectangle(px, py, pw, ph);
        dc.DrawText("Image", px + 4, py + 4);
    }
}

std::string ImageElement::GetZPL() const
{
    if (!bitmap.IsOk()) return "";

    // Convert bitmap to mono and encode as Z64/ASCII hex for ^GF
    wxImage img = bitmap.ConvertToImage().Scale(w, h).ConvertToGreyscale();
    int bytesPerRow = (w + 7) / 8;
    int totalBytes  = bytesPerRow * h;

    std::ostringstream hex;
    for (int row = 0; row < h; ++row)
    {
        unsigned char byte = 0;
        int bit = 7;
        for (int col = 0; col < w; ++col)
        {
            unsigned char grey = img.GetRed(col, row);
            if (grey < 128) byte |= (1 << bit);  // dark pixel = 1 in ZPL
            --bit;
            if (bit < 0 || col == w - 1)
            {
                hex << std::hex << std::uppercase
                    << std::setw(2) << std::setfill('0')
                    << static_cast<int>(byte);
                byte = 0;
                bit  = 7;
            }
        }
    }

    std::ostringstream oss;
    oss << "^FO" << x << "," << y
        << "^GFA," << totalBytes << "," << totalBytes << "," << bytesPerRow
        << "," << hex.str()
        << "^FS";
    return oss.str();
}

std::unique_ptr<LabelElement> ImageElement::Clone() const
{
    return std::make_unique<ImageElement>(*this);
}

bool ImageElement::LoadFromFile(const wxString& path)
{
    wxImage img;
    if (!img.LoadFile(path)) return false;
    bitmap    = wxBitmap(img);
    filePath  = path.ToStdString();
    w         = img.GetWidth();
    h         = img.GetHeight();
    return true;
}

// ── Z64 decoder (base64 + zlib) ───────────────────────────────────────────────

static std::vector<uint8_t> b64Decode(const std::string& s)
{
    static const char* kAlpha =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::vector<uint8_t> out;
    out.reserve(s.size() * 3 / 4);
    int val  = 0;
    int bits = -8;
    for (unsigned char c : s)
    {
        if (c == '=') break;
        const char* p = std::strchr(kAlpha, c);
        if (!p) continue;
        val   = (val << 6) | static_cast<int>(p - kAlpha);
        bits += 6;
        if (bits >= 0)
        {
            out.push_back(static_cast<uint8_t>((val >> bits) & 0xFF));
            bits -= 8;
        }
    }
    return out;
}

static std::vector<uint8_t> zlibInflate(const std::vector<uint8_t>& comp)
{
    if (comp.empty()) return {};
    wxMemoryInputStream  mIn(comp.data(), comp.size());
    wxZlibInputStream    zIn(mIn);
    std::vector<uint8_t> out;
    char buf[4096];
    for (;;)
    {
        zIn.Read(buf, sizeof(buf));
        size_t n = zIn.LastRead();
        if (n > 0) out.insert(out.end(), buf, buf + n);
        if (zIn.Eof() || n == 0) break;
    }
    return out;
}

bool ImageElement::LoadFromZ64(int rowBytes, int totalBytes, const std::string& data)
{
    // Locate :Z64: marker
    auto z64pos = data.find(":Z64:");
    if (z64pos == std::string::npos) return false;

    std::string b64 = data.substr(z64pos + 5);
    // Strip trailing CRC ":XXXX"
    auto lastColon = b64.rfind(':');
    if (lastColon != std::string::npos) b64 = b64.substr(0, lastColon);
    // Strip any ^FS suffix
    auto fsPos = b64.find("^FS");
    if (fsPos != std::string::npos) b64 = b64.substr(0, fsPos);
    // Remove whitespace
    b64.erase(std::remove_if(b64.begin(), b64.end(),
              [](unsigned char c){ return std::isspace(c); }), b64.end());

    auto compressed = b64Decode(b64);
    if (compressed.empty()) return false;

    auto raw = zlibInflate(compressed);
    if (raw.empty()) return false;

    int height = (rowBytes > 0) ? static_cast<int>(raw.size()) / rowBytes : 0;
    int width  = rowBytes * 8;
    if (height <= 0 || width <= 0) return false;

    wxImage img(width, height, false);
    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < rowBytes; ++col)
        {
            int idx = row * rowBytes + col;
            if (idx >= static_cast<int>(raw.size())) break;
            uint8_t byte = raw[idx];
            for (int bit = 7; bit >= 0; --bit)
            {
                int px = col * 8 + (7 - bit);
                if (px >= width) break;
                bool dark = (byte >> bit) & 1;
                img.SetRGB(px, row, dark ? 0 : 255, dark ? 0 : 255, dark ? 0 : 255);
            }
        }
    }

    bitmap   = wxBitmap(img);
    filePath = "";  // loaded inline from ZPL
    w        = width;
    h        = height;
    return true;
}
