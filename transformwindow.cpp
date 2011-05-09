#include "transformwindow.h"
#include "ui_transformwindow.h"
#include "displaywindow.h"
#include "filterinterface.h"
#include "imagetransformfilter.h"
#include "colorparser.h"
#include "photowindow.h"
#include "highpassfilter.h"
#include "lowpassfilter.h"
#include "bandpassfilter.h"
#include "bandstopfilter.h"
#include "phaseshiftfilter.h"
#include "dcthighpassfilter.h"
#include "dctlowpassfilter.h"
#include "dctbandpassfilter.h"
#include "dctbandstopfilter.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>

#include <QDebug>

Q_DECLARE_METATYPE(QUuid)

TransformWindow::TransformWindow(QImage magnitude, QImage phase, ComplexArray *ca, ImageTransformFilter *inverter, QString title, QWidget *parent) :
	DisplayWindow(parent),
	ui(new Ui::TransformWindow),
	mMagnitudeImage(magnitude),
	mPhaseImage(phase),
	mCA(ca),
	mInverter(inverter)
{
	constructorInternals(title);
}

TransformWindow::TransformWindow(ComplexArray *ca, ImageTransformFilter *inverter, QImage::Format format, QString title, QWidget *parent) :
	DisplayWindow(parent),
	ui(new Ui::TransformWindow),
	mCA(ca),
	mInverter(inverter)
{
	QSize size(ca->shape()[1], ca->shape()[2]);
	const ImageTransformFilter::QImages images =
			inverter->complexToImages(ca, format);
	mPhaseImage = images.magnitude;
	mMagnitudeImage = images.phase;
	constructorInternals(title);
}

TransformWindow::~TransformWindow()
{
    delete ui;
	delete mCA;
}

void TransformWindow::constructorInternals(QString title)
{
	ui->setupUi(this);
	ui->magnitudeDisplayLabel->setPixmap(QPixmap::fromImage(mMagnitudeImage));
	ui->phaseDisplayLabel->setPixmap(QPixmap::fromImage(mPhaseImage));
	setWindowTitle(title);

	mFiltersMenu = menuBar()->addMenu("Filters");

	connect(mFiltersMenu, SIGNAL(triggered(QAction*)), this, SLOT(applyFilter(QAction*)));

	appendFilter(new HighPassFilter(this));
	appendFilter(new LowPassFilter(this));
	appendFilter(new BandPassFilter(this));
	appendFilter(new BandStopFilter(this));
	appendFilter(new PhaseShiftFilter(this));

	mFiltersMenu->addSeparator();

	appendFilter(new DctHighPassFilter(this));
	appendFilter(new DctLowPassFilter(this));
	appendFilter(new DctBandPassFilter(this));
	appendFilter(new DctBandStopFilter(this));

	QAction *fftAction = new QAction("Invert", this);
	connect(fftAction, SIGNAL(triggered()), this, SLOT(invert()));
	menuBar()->addAction(fftAction);
}

void TransformWindow::appendFilter(FilterInterface *filter)
{
	mFiltersHash.insert(filter->uuid(), filter);
	QAction *menuAction = new QAction(filter->name(), this);
	QVariant v;
	v.setValue(filter->uuid());
	menuAction->setData(v);
	mFiltersMenu->addAction(menuAction);
}

void TransformWindow::applyFilter(QAction *action)
{
	qDebug() << "applying filter" << action->data().value<QUuid>();
	FilterInterface *filter = mFiltersHash[action->data().value<QUuid>()];
	qDebug() << "filter name:" << filter->name();
	FilterData fd;
	fd.image = mMagnitudeImage;
	fd.transformData.resize(boost::extents[mCA->shape()[0]][mCA->shape()[1]][mCA->shape()[2]]);
	fd.transformData = *mCA;
	if (filter->setup(fd)) {
		DisplayWindow *dw = filter->apply(windowTitle());
		q_check_ptr(dw)->show();
	}
}

void TransformWindow::invert()
{
	int layers = mCA->shape()[0];
	int w = mCA->shape()[1];
	int h = mCA->shape()[2];
	ComplexArray *ca = new ComplexArray(boost::extents[layers][w][h]);
	*ca = *mCA;
	QWidget *mainWindow = qobject_cast<QWidget *>(parent());
	if (mainWindow == NULL) {
		qDebug() << "parent is null in TransformWindow::invert (" << windowTitle() << ")";
	}
	DisplayWindow *dw = mInverter->invert(ca, windowTitle(), mMagnitudeImage.format(), mainWindow);
	PhotoWindow *pw = qobject_cast<PhotoWindow *>(dw);
	if (pw != NULL) {
		pw->show();
	} else {
		QMessageBox::warning(this, "no PhotoWindow returned", "The inversion did not return a valid PhotoWindow pointer.");
	}
	delete ca;
}

ImageTransformFilter *TransformWindow::inverter() const
{
	return mInverter;
}

void TransformWindow::on_actionSave_triggered()
{
	QString url = QFileDialog::getSaveFileName(this, tr("Save Image"), "", tr("png image (*.png)"));
	if (url.isEmpty()) {
		return;
	}
	QFileInfo fi(url);
	mPhaseImage.save(fi.path() + "/" + fi.baseName() + "_phase." + fi.completeSuffix());
	mMagnitudeImage.save(fi.path() + "/" + fi.baseName() + "_magnitude." + fi.completeSuffix());
}
