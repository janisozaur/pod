#ifndef SIMPLEDCT_H
#define SIMPLEDCT_H

#include "imagetransformfilter.h"
#include "complexarray.h"

class SimpleDCT : public ImageTransformFilter
{
	Q_OBJECT
public:
	explicit SimpleDCT(QObject *parent = 0);
	~SimpleDCT();
	virtual QString name() const;
	void perform(ComplexArray *ca, bool inverse = false);
	qreal alpha(int u) const;
	virtual const QImages complexToImages(const ComplexArray *ca, QImage::Format format) const;

signals:

public slots:
	virtual DisplayWindow *apply(QString windowBaseName);
	virtual bool setup(const FilterData &data);

private:
	void transform(QVector<Complex> &elements, bool inverse);
	void test();
	ComplexArray *mCA;
	QSize mSize;
	qreal mAlphaAC, mAlphaDC;
};

#endif // SIMPLEDCT_H
