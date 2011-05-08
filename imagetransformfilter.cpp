#include "imagetransformfilter.h"

#include <QMessageBox>

ImageTransformFilter::ImageTransformFilter(QObject *parent) :
	ImageFilter(parent)
{
}

bool ImageTransformFilter::setup(const FilterData &data)
{
	return ImageFilter::setup(data);
}

qreal ImageTransformFilter::extractColor(const QRgb &color, int which) const
{
	int c = ((color >> (8 * which)) & 0xff);
	return c / 255.0;
}

DisplayWindow *ImageTransformFilter::invert(ComplexArray *, QString,
											QImage::Format, QWidget *)
{
	QMessageBox::critical(qobject_cast<QWidget *>(parent()),
						  "Inversion not supported",
						  "This filter does not support inversion");
	return NULL;
}

void ImageTransformFilter::rearrangeQuadrants(QImage &phase, QImage &magnitude) const
{
	// http://paulbourke.net/miscellaneous/imagefilter/
	int hw = phase.width() / 2;
	int hh = phase.height() / 2;
	if (phase.format() != QImage::Format_Indexed8) {
		for (int i = 0; i < hw; i++) {
			for (int j = 0; j < hh; j++) {
				QRgb tempColor = phase.pixel(i, j);
				phase.setPixel(i, j, phase.pixel(i + hw, j + hh));
				phase.setPixel(i + hw, j + hh, tempColor);

				tempColor = phase.pixel(i + hw, j);
				phase.setPixel(i + hw, j, phase.pixel(i, j + hh));
				phase.setPixel(i, j + hh, tempColor);


				tempColor = magnitude.pixel(i, j);
				magnitude.setPixel(i, j, magnitude.pixel(i + hw, j + hh));
				magnitude.setPixel(i + hw, j + hh, tempColor);

				tempColor = magnitude.pixel(i + hw, j);
				magnitude.setPixel(i + hw, j, magnitude.pixel(i, j + hh));
				magnitude.setPixel(i, j + hh, tempColor);
			}
		}
	} else {
		for (int i = 0; i < hw; i++) {
			for (int j = 0; j < hh; j++) {
				int tempColor = phase.pixelIndex(i, j);
				phase.setPixel(i, j, phase.pixelIndex(i + hw, j + hh));
				phase.setPixel(i + hw, j + hh, tempColor);

				tempColor = phase.pixelIndex(i + hw, j);
				phase.setPixel(i + hw, j, phase.pixelIndex(i, j + hh));
				phase.setPixel(i, j + hh, tempColor);


				tempColor = magnitude.pixelIndex(i, j);
				magnitude.setPixel(i, j, magnitude.pixelIndex(i + hw, j + hh));
				magnitude.setPixel(i + hw, j + hh, tempColor);

				tempColor = magnitude.pixelIndex(i + hw, j);
				magnitude.setPixel(i + hw, j, magnitude.pixelIndex(i, j + hh));
				magnitude.setPixel(i, j + hh, tempColor);
			}
		}
	}
}
