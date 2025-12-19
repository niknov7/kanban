#ifndef DEVELOPERWINDOW_H
#define DEVELOPERWINDOW_H

#include <QDialog>
#include <QTableWidget>
#include "scrumboard.h"

namespace Ui {
class DeveloperWindow;
}

class DeveloperWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DeveloperWindow(ScrumBoard& board, QWidget *parent = nullptr);
    ~DeveloperWindow();

    void refreshDeveloperTable();

private slots:
    void onAddDeveloper();
    void onRemoveDeveloper();

private:
    Ui::DeveloperWindow *ui;
    ScrumBoard& board;
};

#endif
