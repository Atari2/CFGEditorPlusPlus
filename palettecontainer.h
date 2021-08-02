#ifndef PALETTECONTAINER_H
#define PALETTECONTAINER_H

#include <QWidget>
#include "paletteview.h"

class PaletteContainer : public QWidget
{
	Q_OBJECT
public:
	explicit PaletteContainer(PaletteView* view, QWidget *parent = nullptr);
	void updateContainer(const QPixmap& image);
signals:
	void paletteChanged();
private:
	void closeEvent(QCloseEvent* event) override;
	PaletteView* m_view = nullptr;
	QPushButton* savePalette = nullptr;
	QPushButton* loadPalette = nullptr;
};

#endif // PALETTECONTAINER_H
