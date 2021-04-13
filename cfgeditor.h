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
#include "jsonsprite.h"
#include "spritepalettecreator.h"
#include "collectiondatamodel.h"

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
    void bindCollectionButtons();
    void initCompleter();
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
