#ifndef DCTHIGHPASSFILTER_H
#define DCTHIGHPASSFILTER_H

#include "transformfilter.h"

class DctHighPassFilter : public TransformFilter
{
	Q_OBJECT
public:
	explicit DctHighPassFilter(QObject *parent = 0);
	virtual QString name() const;

signals:

public slots:
	virtual bool setup(const FilterData &data);
	virtual DisplayWindow *apply(QString windowBaseName);

private:
	int mRadius;
	QImage::Format mFormat;
};

#endif // DCTHIGHPASSFILTER_H
