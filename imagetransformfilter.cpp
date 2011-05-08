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
