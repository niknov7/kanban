#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "developerwindow.h"
#include "boardserializer.h"
#include "taskutils.h"
#include "taskitemformat.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QEvent>
#include <QDropEvent>
#include <QTimer>

static QString developerNameById(const ScrumBoard& board, int devId)
{
    auto devs = board.getAllDevelopers();
    for (auto it = devs.begin(); it != devs.end(); ++it) {
        if (it->first == devId) return QString::fromStdString(it->second.name());
    }
    return QString("ID %1").arg(devId);
}

static int extractTaskId(const QString& text)
{
    int l = text.indexOf('[');
    int r = text.indexOf(']');
    if (l == -1 || r == -1) return -1;
    if (r <= l + 1) return -1;

    bool ok = false;
    int id = text.mid(l + 1, r - l - 1).toInt(&ok);
    if (!ok) return -1;
    return id;
}

QListWidget* MainWindow::listForViewport(QObject* viewport) const
{
    if (!ui) return 0;
    if (ui->listAssigned && viewport == ui->listAssigned->viewport()) return ui->listAssigned;
    if (ui->listInProgress && viewport == ui->listInProgress->viewport()) return ui->listInProgress;
    if (ui->listDone && viewport == ui->listDone->viewport()) return ui->listDone;
    return 0;
}

TaskStatus MainWindow::targetStatusForList(QListWidget* list) const
{
    if (list == ui->listAssigned) return TaskStatus::Backlog;
    if (list == ui->listInProgress) return TaskStatus::InProgress;
    return TaskStatus::Done;
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() != QEvent::Drop)
        return QMainWindow::eventFilter(obj, event);

    QListWidget* targetList = listForViewport(obj);
    if (!targetList)
        return QMainWindow::eventFilter(obj, event);

    QDropEvent* dropEvent = (QDropEvent*)event;
    QPoint dropPos = dropEvent->position().toPoint();

    QTimer::singleShot(0, this, [this, targetList, dropPos]() {
        QListWidgetItem* item = targetList->itemAt(dropPos);
        if (!item) item = targetList->currentItem();
        if (!item && targetList->count() > 0) item = targetList->item(targetList->count() - 1);
        if (!item) return;

        int taskId = extractTaskId(item->text());
        if (taskId < 0) return;

        TaskStatus newStatus = targetStatusForList(targetList);

        try {
            board.changeTaskStatus(taskId, newStatus);
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Нельзя переместить", e.what());
        }

        refreshBoardView();
    });

    return false;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btnOpenDevelopers, &QPushButton::clicked, this, &MainWindow::onOpenDevelopers);
    connect(ui->btnAddTask, &QPushButton::clicked, this, &MainWindow::onAddTask);
    connect(ui->btnDeleteTask, &QPushButton::clicked, this, &MainWindow::onDeleteTask);
    connect(ui->btnSaveBoard, &QPushButton::clicked, this, &MainWindow::onSaveBoard);
    connect(ui->btnLoadBoard, &QPushButton::clicked, this, &MainWindow::onLoadBoard);

    m_listsController = std::make_unique<BoardListsController>(
        board,
        ui->listAssigned,
        ui->listInProgress,
        ui->listDone,
        [this]() { refreshBoardView(); },
        this
        );

    refreshBoardView();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onOpenDevelopers()
{
    DeveloperWindow w(board, this);
    w.exec();
    refreshBoardView();
}

void MainWindow::onAddTask()
{
    bool ok = false;

    QString title = QInputDialog::getText(this, "Добавить задачу", "Заголовок:",
                                          QLineEdit::Normal, "", &ok);
    if (!ok) return;
    if (title.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка ввода", "Заголовок задачи не может быть пустым!");
        return;
    }

    QString desc = QInputDialog::getText(this, "Добавить задачу", "Описание:",
                                         QLineEdit::Normal, "", &ok);
    if (!ok) return;
    if (desc.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка ввода", "Описание задачи не может быть пустым!");
        return;
    }

    try {
        int id = board.getNextTaskId();
        board.addTask(Task(id, title.toStdString(), desc.toStdString()));
        refreshBoardView();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", e.what());
    }
}

void MainWindow::onDeleteTask()
{
    QListWidgetItem* item = 0;

    if (ui->listAssigned && ui->listAssigned->currentItem()) item = ui->listAssigned->currentItem();
    if (!item && ui->listInProgress && ui->listInProgress->currentItem()) item = ui->listInProgress->currentItem();
    if (!item && ui->listDone && ui->listDone->currentItem()) item = ui->listDone->currentItem();

    if (!item) {
        QMessageBox::information(this, "Удаление", "Выберите задачу в любой колонке.");
        return;
    }

    int taskId = extractTaskId(item->text());
    if (taskId < 0) {
        QMessageBox::warning(this, "Ошибка", "Не удалось определить ID задачи из строки.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Подтверждение удаления",
        QString("Удалить задачу #%1?\nЭто действие нельзя отменить.").arg(taskId),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes) return;

    try {
        board.removeTask(taskId);
        refreshBoardView();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", e.what());
    }
}

void MainWindow::onSaveBoard()
{
    try {
        saveBoardToFile(board, "board.json");
        QMessageBox::information(this, "Сохранено", "Доска сохранена в board.json");
    } catch (std::exception& e) {
        QMessageBox::critical(this, "Ошибка", e.what());
    }
}

void MainWindow::onLoadBoard()
{
    try {
        board = loadBoardFromFile("board.json");
        QMessageBox::information(this, "Загружено", "Доска загружена из board.json");
        refreshBoardView();
    } catch (std::exception& e) {
        QMessageBox::critical(this, "Ошибка", e.what());
    }
}

void MainWindow::refreshBoardView()
{
    if (!ui->listAssigned || !ui->listInProgress || !ui->listDone) return;

    ui->listAssigned->clear();
    ui->listInProgress->clear();
    ui->listDone->clear();

    auto tasks = board.getAllTasks();
    for (auto it = tasks.begin(); it != tasks.end(); ++it) {
        const Task& task = it->second;

        QString title = TaskItemFormat::makeTitleLine(board, task);
        QString tip = TaskItemFormat::makeTooltip(task);

        QListWidget* list = ui->listDone;

        if (task.status() == TaskStatus::Backlog || task.status() == TaskStatus::Assigned)
            list = ui->listAssigned;
        else if (task.status() == TaskStatus::InProgress || task.status() == TaskStatus::Blocked)
            list = ui->listInProgress;

        QListWidgetItem* item = new QListWidgetItem(title);
        item->setToolTip(tip);
        list->addItem(item);
    }
}
