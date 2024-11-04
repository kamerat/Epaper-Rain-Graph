#ifndef ESPQRCODEEINK_H
#define ESPQRCODEEINK_H

/* ESP_QRcode. e-ink version
 * Import this .h when using some e-ink display
 */

#define EINKDISPLAY

#include <qrcodedisplay.h>
#include <Adafruit_GFX.h>
#include <GxEPD2_BW.h>

#ifndef EINK_MODEL
#define EINK_MODEL 128
#endif

class QRcodeEink : public QRcodeDisplay
{
	private:
		GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> *display;
        void drawPixel(int x, int y, int color);
	public:

		QRcodeEink(GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> *display);
		void init(int x_offset = 0, int y_offset = 0);
		void screenwhite();
		void screenupdate();
};
#endif