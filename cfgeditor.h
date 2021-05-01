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
#include "jsonsprite.h"
#include "snesgfxconverter.h"
#include "eightbyeightview.h"
#include "paletteview.h"
#include "map16provider.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CFGEditor; }
QT_END_NAMESPACE

class CFGEditor : public QMainWindow
{
    Q_OBJECT

public:
    CFGEditor(QWidget *parent = nullptr);
    ~CFGEditor();
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
    void setCollectionModel();
    void setDisplayModel();
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

    void changeAllCheckBoxState(bool state);
    void setupForNormal();
    void setupForCustom();
    void setupForGenShoot();

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
    QVector<QStandardItemModel*> models;
    QImage* full8x8Bitmap = nullptr;
    EightByEightView* view8x8 = nullptr;
    PaletteView* viewPalette = nullptr;
    ClipboardTile copiedTile;
    QVector<DisplayData> displays;
    QAtomicInteger<int> currentDisplayIndex = -1;
};

class DefaultMissingImpl {
private:
    QString name;
    QMessageBox* messageBox = nullptr;
public:
    DefaultMissingImpl(const QString& impl_name);
    void operator()();
};

class DefaultAlertImpl {
private:
    QMessageBox* messageBox = nullptr;
public:
    DefaultAlertImpl(const QString& message);
    void operator()();
};

#endif // CFGEDITOR_H
