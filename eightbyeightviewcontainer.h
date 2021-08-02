#ifndef EIGHTBYEIGHTVIEWCONTAINER_H
#define EIGHTBYEIGHTVIEWCONTAINER_H

#include <QWidget>
#include <QLabel>

class EightByEightView;

class EightByEightViewContainer : public QWidget
{
	Q_OBJECT
public:
	explicit EightByEightViewContainer(EightByEightView* view, QWidget *parent = nullptr);
	void updateForChange(QImage* image, bool firstTime = false);
	void closeEvent(QCloseEvent* event);
	void updateTileLabel(const QString& tile);
signals:

private:
	EightByEightView* m_view = nullptr;
	QLabel* label = nullptr;
};

#endif // EIGHTBYEIGHTVIEWCONTAINER_H
