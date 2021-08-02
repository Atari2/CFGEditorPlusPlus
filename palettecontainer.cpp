#include "palettecontainer.h"
#include <QVBoxLayout>
#include <QIcon>

PaletteContainer::PaletteContainer(PaletteView* view, QWidget *parent) : QWidget(parent), m_view(view)
{
	setWindowTitle("Palette viewer");
	setWindowIcon(QIcon{":/Resources/ButtonIcons/palette.png"});
	auto layout = new QVBoxLayout(this);
	setLayout(layout);
	layout->addWidget(view);
	QWidget* buttonContainer = new QWidget(this);
	auto buttonLayout = new QHBoxLayout(buttonContainer);
	buttonContainer->setLayout(buttonLayout);
	savePalette = new QPushButton{"Save Palette to file", buttonContainer};
	loadPalette = new QPushButton{"Load Palette from file", buttonContainer};
	QObject::connect(loadPalette, &QPushButton::clicked, this, [this]{
		QString filename = QFileDialog::getOpenFileName(this, "Open Pal File", "", tr("Palette Files (*.pal)"));
		if (filename.length() == 0)
			return;
		if (!assert_filesize(filename, 768))
			return;
		SpritePaletteCreator::ReadPaletteFile(0, 16, 16, filename);
		m_view->updateForChange(SpritePaletteCreator::MakeFullPalette());
		emit paletteChanged();
	});
	QObject::connect(savePalette, &QPushButton::clicked, this, [this]{
		QString filename = QFileDialog::getSaveFileName(this, "Save Pal/Palmask File", "", tr("Palette Files (*.pal)"));
		if (filename.length() == 0)
			return;
		SpritePaletteCreator::PaletteToFile(m_view->getCurrentItem()->pixmap().toImage(), filename);
	});
	savePalette->adjustSize();
	loadPalette->adjustSize();
	buttonContainer->adjustSize();
	buttonLayout->addWidget(savePalette);
	buttonLayout->addWidget(loadPalette);
	layout->addWidget(buttonContainer);
	adjustSize();
	setFixedSize(size());
}

void PaletteContainer::updateContainer(const QPixmap& image) {
	m_view->updateForChange(image);
	m_view->open();
	show();
	raise();
}

void PaletteContainer::closeEvent(QCloseEvent *event) {
	m_view->close(event);
}
