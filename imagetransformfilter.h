#ifndef IMAGETRANSFORMFILTER_H
#define IMAGETRANSFORMFILTER_H

#include "imagefilter.h"
#include "complexarray.h"

class ImageTransformFilter : public ImageFilter
{
	Q_OBJECT
public:
	struct QImages {
		QImage phase, magnitude;
	};

	explicit ImageTransformFilter(QObject *parent);
	virtual QString name() const = 0;
	virtual DisplayWindow *invert(ComplexArray *ca, QString title, QImage::Format format, QWidget *parent);
	virtual const QImages complexToImages(const ComplexArray *ca, QImage::Format format) const = 0;

signals:

public slots:
	virtual bool setup(const FilterData &data);
	virtual DisplayWindow *apply(QString windowBaseName) = 0;

protected:
	qreal extractColor(const QRgb &color, int which) const;
	void rearrangeQuadrants(QImage &phase, QImage &magnitude) const;
};

#endif // IMAGETRANSFORMFILTER_H
