#include "font.h"

MyFont::MyFont(HWND MainForm)
{
    if(!MainForm) return;
    NONCLIENTMETRICS nonClientMetrics;
    nonClientMetrics.cbSize = sizeof(nonClientMetrics);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, nonClientMetrics.cbSize, &nonClientMetrics, 0);
    hFont = CreateFontIndirect(&nonClientMetrics.lfMessageFont);

    HDC displayDevice = GetDC(MainForm);
    SelectObject(displayDevice, hFont);

    TEXTMETRIC textMetrics;
    GetTextMetrics(displayDevice, &textMetrics);

    fontHeightInPixels = textMetrics.tmHeight;

    // Get a relative pixel size in pixels.
    relativePixelSize = 1.0f / 96.0f * (float)GetDeviceCaps(displayDevice, LOGPIXELSX);
}

MyFont::~MyFont()
{
    if(hFont)
        DeleteObject(hFont);
}

int MyFont::RelativeToNormalPixels2(int relativePixels)
{
    return (int)floor(relativePixels * relativePixelSize + 0.5);
}

int MyFont::R2NP(int relativePixels)
{
    return RelativeToNormalPixels2(relativePixels);
}

void MyFont::ApplyFont(HWND apHWND)
{
    if(hFont)
        SendMessage(apHWND, WM_SETFONT, (WPARAM)hFont, 0);
}

HFONT MyFont::GetHWNDFont()
{
    if(hFont) return hFont;
    else return NULL;
}
