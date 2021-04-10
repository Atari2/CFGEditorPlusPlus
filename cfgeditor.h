#ifndef CFGEDITOR_H
#define CFGEDITOR_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>

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
private:
    Ui::CFGEditor *ui;
    QFileDialog *dialog;
};

class DefaultMissingImpl {
private:
    QString name;
    QMessageBox* messageBox = nullptr;
public:
    DefaultMissingImpl(const QString& impl_name);
    void operator()();
};

#endif // CFGEDITOR_H
