#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include "boardlistscontroller.h"
#include "scrumboard.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onOpenDevelopers();
    void onAddTask();
    void onDeleteTask();
    void onSaveBoard();
    void onLoadBoard();

private:
    void refreshBoardView();
    QListWidget* listForViewport(QObject* viewport) const;
    TaskStatus targetStatusForList(QListWidget* list) const;

private:
    Ui::MainWindow *ui;
    ScrumBoard board;
    std::unique_ptr<BoardListsController> m_listsController;
};

#endif
