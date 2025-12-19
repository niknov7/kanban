#include "developerwindow.h"
#include "ui_developerwindow.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QTableWidgetItem>

DeveloperWindow::DeveloperWindow(ScrumBoard& board, QWidget *parent)
    : QDialog(parent), ui(new Ui::DeveloperWindow), board(board)
{
    ui->setupUi(this);
    ui->tableDevelopers->horizontalHeader()->setStretchLastSection(true);

    connect(ui->btnAddDeveloper, &QPushButton::clicked,
            this, &DeveloperWindow::onAddDeveloper);

    connect(ui->btnRemoveDeveloper, &QPushButton::clicked,
            this, &DeveloperWindow::onRemoveDeveloper);

    refreshDeveloperTable();
}

DeveloperWindow::~DeveloperWindow() {
    delete ui;
}

void DeveloperWindow::refreshDeveloperTable() {
    ui->tableDevelopers->setRowCount(0);

    for (const auto& [id, dev] : board.getAllDevelopers()) {
        int row = ui->tableDevelopers->rowCount();
        ui->tableDevelopers->insertRow(row);

        auto* item = new QTableWidgetItem(
            QString::fromStdString(dev.name()));

        item->setData(Qt::UserRole, id);

        ui->tableDevelopers->setItem(row, 0, item);
    }
}

void DeveloperWindow::onAddDeveloper() {
    bool ok = false;
    QString name = QInputDialog::getText(
        this,
        "Добавить разработчика",
        "Имя:",
        QLineEdit::Normal,
        "",
        &ok);

    if (!ok || name.trimmed().isEmpty()) {
        QMessageBox::warning(
            this,
            "Ошибка ввода",
            "Имя разработчика не может быть пустым!");
        return;
    }

    int id = board.getNextDeveloperId();
    board.addDeveloper(Developer(id, name.toStdString()));

    refreshDeveloperTable();
}

void DeveloperWindow::onRemoveDeveloper() {
    auto* item = ui->tableDevelopers->currentItem();
    if (!item) {
        QMessageBox::information(
            this,
            "Удаление",
            "Выберите разработчика в таблице.");
        return;
    }

    int id = item->data(Qt::UserRole).toInt();
    QString name = item->text();

    auto reply = QMessageBox::question(
        this,
        "Подтверждение удаления",
        QString("Удалить разработчика «%1»?\n"
                "Все его задачи останутся без исполнителя.")
            .arg(name),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    try {
        board.removeDeveloper(id);
        refreshDeveloperTable();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", e.what());
    }
}
