#include "eightbyeightviewcontainer.h"
#include "eightbyeightview.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>

EightByEightViewContainer::EightByEightViewContainer(EightByEightView* view, QComboBox* paletteComboBox, QWidget *parent) : QWidget(parent), m_view(view)
{
	setWindowIcon(QIcon{":/Resources/ButtonIcons/8x8.png"});
    setWindowTitle("8x8 Tile Viewer");
	auto layout = new QVBoxLayout(this);
	setLayout(layout);
    m_view->setComboBox(paletteComboBox);
	layout->addWidget(m_view);
	label = new QLabel(this);
	label->setText("Tile 000");
	layout->addWidget(label);
	adjustSize();
	setFixedSize(size());
}

void EightByEightViewContainer::updateForChange(QImage* image, bool firstTime) {
    m_view->updateForChange(image);
    if (!firstTime) {
        m_view->open();
        show();
        raise();
    }
}

void EightByEightViewContainer::closeEvent(QCloseEvent *event) {
	m_view->close(event);
}

void EightByEightViewContainer::updateTileLabel(const QString &tile) {
	label->setText(tile);
}
