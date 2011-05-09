#ifndef DCTBANDPASSFILTER_H
#define DCTBANDPASSFILTER_H

#include "transformfilter.h"

class DctBandPassFilter : public TransformFilter
{
	Q_OBJECT
public:
	explicit DctBandPassFilter(QObject *parent = 0);
	virtual QString name() const;

signals:

public slots:
	virtual bool setup(const FilterData &data);
	virtual DisplayWindow *apply(QString windowBaseName);

private:
	int mRadiusNear, mRadiusFar;
	QImage::Format mFormat;
};

#endif // DCTBANDPASSFILTER_H
