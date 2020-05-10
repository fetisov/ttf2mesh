#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QScrollArea>
#include <QLayout>
#include <QTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <qmwrap.h>
#include "linalg.h"
#include "ttf2mesh.h"
#include "uranges.h"

namespace Ui {
class MainWindow;
}

class FontTest;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onTestFont();
    void onTestAllFonts();
    void onFontSelect();
    void onGlyphSelect();
    void onLoadDir();
    void onLoadSys();
    void onPoly2tri();
    void onGlyph2Obj();
    void onFont2Obj();
    void onAppLink(const QString &link);
    void updateUi();

private:
    int updating;
    ttf_t **fonts;
    ttf_t *ttf;
    ttf_glyph_t *g;
    Ui::MainWindow *ui;
    int lastSelectedUtf;
    void setupFontList();
    void updateFontInfo();
    void updateGlyphsList();
    void updateCurvesPlot();
    void updateDebugPage();
    void updateTextPage();
    void applyTestResults(FontTest *t);
};

#endif // MAINWINDOW_H
