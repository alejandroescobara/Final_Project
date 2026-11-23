#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "LevelManager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("El Camino del Errante"));
    resize(1280, 720);
    setCentralWidget(new LevelManager(this));
}

MainWindow::~MainWindow()
{
    delete ui;
}
