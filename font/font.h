#ifndef FONT_H_INCLUDED
#define FONT_H_INCLUDED

#pragma once

#include "../myhdr.h"

class MyFont
{
public:
    MyFont(HWND MainForm);
    ~MyFont();

    int RelativeToNormalPixels2(int relativePixels);
    int R2NP(int relativePixels);
    void ApplyFont(HWND apHWND);
    HFONT GetHWNDFont();

private:

    HFONT hFont;
    float relativePixelSize;
    int fontHeightInPixels;

};

#endif // FONT_H_INCLUDED
