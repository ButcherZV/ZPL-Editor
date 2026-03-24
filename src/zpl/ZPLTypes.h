#pragma once
#include <cmath>

// ── DPI constants ─────────────────────────────────────────────────────────────
// Zebra printer resolutions in dots-per-inch.
// "200 DPI" printers are technically 203 DPI per Zebra spec.
enum class PrinterDPI
{
    DPI_152 = 152,
    DPI_203 = 203,
    DPI_300 = 300,
    DPI_600 = 600,
};

// Dots per millimetre for each DPI class
inline double DotsPerMM(PrinterDPI dpi)
{
    switch (dpi)
    {
        case PrinterDPI::DPI_152: return 152.0 / 25.4;   // ~5.98
        case PrinterDPI::DPI_203: return 203.0 / 25.4;   // ~7.99
        case PrinterDPI::DPI_300: return 300.0 / 25.4;   // ~11.81
        case PrinterDPI::DPI_600: return 600.0 / 25.4;   // ~23.62
        default:                  return 203.0 / 25.4;
    }
}

inline double DotsPerInch(PrinterDPI dpi)
{
    return static_cast<double>(dpi);
}

// Convert dots → mm
inline double DotsToMM(int dots, PrinterDPI dpi)
{
    return dots / DotsPerMM(dpi);
}

// Convert dots → inches
inline double DotsToInches(int dots, PrinterDPI dpi)
{
    return static_cast<double>(dots) / DotsPerInch(dpi);
}

// Convert mm → dots (rounded)
inline int MMToDots(double mm, PrinterDPI dpi)
{
    return static_cast<int>(std::round(mm * DotsPerMM(dpi)));
}

// Convert inches → dots (rounded)
inline int InchesToDots(double inches, PrinterDPI dpi)
{
    return static_cast<int>(std::round(inches * DotsPerInch(dpi)));
}

// ── Measurement units ─────────────────────────────────────────────────────────
enum class MeasureUnit
{
    Metric,    // millimetres
    Imperial,  // inches
};

// ── Label orientation ─────────────────────────────────────────────────────────
enum class LabelOrientation
{
    Portrait,
    Landscape,
};

// ── Label configuration (the result of NewLabelDialog) ────────────────────────
struct LabelConfig
{
    PrinterDPI      dpi         = PrinterDPI::DPI_203;
    int             widthDots   = 0;   // print area width in dots (after margins)
    int             heightDots  = 0;   // print area height in dots
    int             marginTop   = 0;   // in dots
    int             marginBottom= 0;
    int             marginLeft  = 0;
    int             marginRight = 0;
    LabelOrientation orientation  = LabelOrientation::Portrait;
    int              labelsPerRow  = 1;    // labels across media width (for multi-up)

    // Total canvas width/height including margins
    int totalWidth()  const { return widthDots  + marginLeft + marginRight; }
    int totalHeight() const { return heightDots + marginTop  + marginBottom; }
};
