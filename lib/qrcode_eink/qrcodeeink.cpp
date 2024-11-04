#include <Arduino.h>
#include "qrencode.h"
#include "qrcodeeink.h"

QRcodeEink::QRcodeEink(GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> *display) {
    this->display = display;
}

void QRcodeEink::init(int x_offset, int y_offset) {
    this->screenwidth = display->width();
    this->screenheight = display->height();


    int min = screenwidth < screenheight ? screenwidth : screenheight;
    multiply = min / WD;

    offsetsX = (screenwidth-(WD*multiply))/2 + x_offset;
    offsetsY = (screenheight-(WD*multiply))/2 + y_offset;
}

void QRcodeEink::screenwhite() {
    display->setFullWindow();
    display->fillScreen(GxEPD_WHITE);
    display->display(false);
}

void QRcodeEink::screenupdate() {
    // do nothing
}

void QRcodeEink::drawPixel(int x, int y, int color) {
    display->fillRect(x, y, multiply, multiply, color == 1 ? GxEPD_BLACK : GxEPD_WHITE);
}