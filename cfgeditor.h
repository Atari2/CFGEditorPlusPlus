#ifndef CFGEDITOR_H
#define CFGEDITOR_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QProperty>
#include <QCompleter>
#include <QLineEdit>
#include <QCheckBox>
#include <QRegularExpressionValidator>
#include <QStandardItemModel>
#include <QDir>
#include <QMap>
#include "utils.h"
#include "jsonsprite.h"
#include "snesgfxconverter.h"
#include "eightbyeightviewcontainer.h"
#include "palettecontainer.h"
#include "map16provider.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CFGEditor; }
QT_END_NAMESPACE

class CFGEditor : public QMainWindow
{
    Q_OBJECT

public:
    CFGEditor(const QStringList& argv, QWidget *parent = nullptr);
    ~CFGEditor();
    void deleteInstaller();
    void setUpMenuBar(QMenuBar*);
    void bindSpriteProp();
    void setUpImages();
    void bindTweak1656();
    void bindTweak1662();
    void bindTweak166E();
    void bindTweak167A();
    void bindTweak1686();
    void bindTweak190F();
    void resetTweaks();
    void resetAll();
    void saveSprite();
    void setCollectionModel();
    void setDisplayModel();
    void setGFXInfoModel();
    void bindCollectionButtons();
    void bindDisplayButtons();
    void bindGFXSelector();
    void initCompleter();
    void loadFullbitmap(int index = -1);
    void addLunarMagicIcons();
    void closeEvent(QCloseEvent *event);
    void advanceDisplayIndex();
    void addBlankRow();
    void addCloneRow();
    void removeExistingRow();
    void changeTilePropGroupState(bool, TileChangeType type = TileChangeType::All);
    void setTilePropGroupState(FullTile tileInfo);
    Display createDisplay(const DisplayData& data);
    void populateDisplays();
    void populateGFXFiles();

    void changeAllCheckBoxState(bool state);
    void setupForNormal();
    void setupForCustom();
	void setupForGenShootOther();

    QStandardItemModel* getGfxInfoModel() {
        return gfxinfoModel;
    }

    template <typename J>
    void connectCheckBox(QLineEdit* edit, QCheckBox* box, J* tweak, bool& tochange) {
        QObject::connect(box, &QCheckBox::stateChanged, this, [=, &tochange]() mutable {
            qDebug() << "Checkbox " << box->objectName() << " changed";
            tochange = box->checkState() == Qt::CheckState::Checked;
            edit->setText(QString::asprintf("%02X", tweak->to_byte()));
        });
    }
private:
    Ui::CFGEditor *ui;
    JsonSprite* sprite;
    QRegularExpressionValidator* hexValidator;
    QStringList* hexNumberList;
    QCompleter* hexCompleter;
    QVector<QPixmap> paletteImages;
    QVector<QPixmap> objClipImages;
    QVector<QPixmap> sprClipImages;
    QStandardItemModel* collectionModel = nullptr;
    QStandardItemModel* displayModel = nullptr;
    QStandardItemModel* gfxinfoModel = nullptr;
    QImage* full8x8Bitmap = nullptr;
	EightByEightViewContainer* view8x8Container = nullptr;
	PaletteContainer* paletteContainer = nullptr;
    ClipboardTile copiedTile;
    QVector<DisplayData> displays;
    QAtomicInteger<int> currentDisplayIndex = -1;
    int currentGFXFileIndex = -1;
};


#endif // CFGEDITOR_H
