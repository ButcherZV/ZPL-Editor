#pragma once
#include "LabelElement.h"
#include <wx/bitmap.h>
#include <string>

class ImageElement : public LabelElement
{
public:
    wxBitmap    bitmap;
    std::string filePath;

    ImageElement() { w = 200; h = 200; }

    void Draw(wxDC& dc, wxPoint off, double zoom) const override;
    std::string GetZPL() const override;
    std::unique_ptr<LabelElement> Clone() const override;

    bool LoadFromFile(const wxString& path);
    // Load a monochrome ZPL Z64-encoded ^GF image (base64 + zlib compressed)
    bool LoadFromZ64(int rowBytes, int totalBytes, const std::string& data);
};
