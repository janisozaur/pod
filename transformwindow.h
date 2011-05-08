#ifndef TRANSFORMWINDOW_H
#define TRANSFORMWINDOW_H

#include "displaywindow.h"
#include "complexarray.h"

#include <QHash>
#include <QUuid>

namespace Ui {
	class TransformWindow;
}

class FilterInterface;
class ImageTransformFilter;

class TransformWindow : public DisplayWindow
{
	Q_OBJECT

public:
	explicit TransformWindow(QImage magnitude, QImage phase, ComplexArray *ca, ImageTransformFilter *inverter, QString title, QWidget *parent = 0);
	explicit TransformWindow(ComplexArray *ca, ImageTransformFilter *inverter, QImage::Format format, QString title, QWidget *parent);
	~TransformWindow();
	ImageTransformFilter *inverter() const;

private:
	void constructorInternals(QString title);
	void appendFilter(FilterInterface *filter);
	void rearrangeQuadrants(QImage &phase, QImage &magnitude) const;

	Ui::TransformWindow *ui;
	QImage mMagnitudeImage;
	QImage mPhaseImage;
	ComplexArray *mCA;
	QHash<QUuid, FilterInterface *> mFiltersHash;
	QMenu *mFiltersMenu;
	ImageTransformFilter *mInverter;

private slots:
	void applyFilter(QAction *action);
	void invert();
};

#endif // TRANSFORMWINDOW_H
