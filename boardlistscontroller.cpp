#include "boardlistscontroller.h"
#include "taskitemformat.h"
#include "scrumboard.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QEvent>
#include <QDropEvent>
#include <QTimer>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>

BoardListsController::BoardListsController(ScrumBoard& board,
                                           QListWidget* assigned,
                                           QListWidget* inProgress,
                                           QListWidget* done,
                                           RefreshFn refresh,
                                           QObject* parent)
    : QObject(parent),
    m_board(board),
    m_assigned(assigned),
    m_inProgress(inProgress),
    m_done(done),
    m_refresh(std::move(refresh))
{
    QListWidget* lists[] = { m_assigned, m_inProgress, m_done };
    for (QListWidget* l : lists) {
        setupDnD(l);
        setupContextMenu(l);
    }
}

void BoardListsController::setupDnD(QListWidget* list)
{
    if (!list) return;

    list->setDragEnabled(true);
    list->setAcceptDrops(true);
    list->setDropIndicatorShown(true);
    list->setDefaultDropAction(Qt::MoveAction);
    list->setDragDropMode(QAbstractItemView::DragDrop);
    list->viewport()->installEventFilter(this);
}

void BoardListsController::setupContextMenu(QListWidget* list)
{
    if (!list) return;

    list->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(list, &QListWidget::customContextMenuRequested, this,
            [this, list](const QPoint& pos) {
                QListWidgetItem* item = list->itemAt(pos);
                if (!item) return;

                QMenu menu(list);
                QAction* assignAction = menu.addAction("Назначить разработчика");
                if (menu.exec(list->viewport()->mapToGlobal(pos)) != assignAction) return;

                int taskId = TaskItemFormat::extractTaskId(item->text());
                if (taskId < 0) {
                    QMessageBox::warning(qobject_cast<QWidget*>(parent()),
                                         "Ошибка", "Не удалось определить ID задачи из строки.");
                    return;
                }

                const auto& devs = m_board.getAllDevelopers();
                if (devs.empty()) {
                    QMessageBox::warning(qobject_cast<QWidget*>(parent()),
                                         "Предупреждение", "Сначала добавьте разработчиков.");
                    return;
                }

                QStringList names;
                std::vector<int> devIds;

                for (auto it = devs.begin(); it != devs.end(); ++it) {
                    names << QString::fromStdString(it->second.name());
                    devIds.push_back(it->first);
                }

                bool ok = false;
                int index = names.indexOf(QInputDialog::getItem(
                    qobject_cast<QWidget*>(parent()),
                    "Назначить задачу", "Разработчик:",
                    names, 0, false, &ok));

                if (!ok || index < 0 || index >= (int)devIds.size()) return;

                try {
                    m_board.assignTask(taskId, devIds[(size_t)index]);
                    if (m_refresh) m_refresh();
                } catch (const std::exception& e) {
                    QMessageBox::critical(qobject_cast<QWidget*>(parent()), "Ошибка", e.what());
                }
            });
}

QListWidget* BoardListsController::listForViewport(QObject* viewport) const
{
    if (m_assigned && viewport == m_assigned->viewport()) return m_assigned;
    if (m_inProgress && viewport == m_inProgress->viewport()) return m_inProgress;
    if (m_done && viewport == m_done->viewport()) return m_done;
    return 0;
}

TaskStatus BoardListsController::targetStatusForList(QListWidget* list) const
{
    if (list == m_assigned) return TaskStatus::Backlog;
    if (list == m_inProgress) return TaskStatus::InProgress;
    return TaskStatus::Done;
}

bool BoardListsController::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() != QEvent::Drop)
        return QObject::eventFilter(obj, event);

    QListWidget* targetList = listForViewport(obj);
    if (!targetList) return QObject::eventFilter(obj, event);

    QDropEvent* dropEvent = (QDropEvent*)event;
    QPoint dropPos = dropEvent->position().toPoint();

    QTimer::singleShot(0, this, [this, targetList, dropPos]() {
        QListWidgetItem* item = targetList->itemAt(dropPos);
        if (!item) item = targetList->currentItem();
        if (!item && targetList->count() > 0) item = targetList->item(targetList->count() - 1);
        if (!item) return;

        int taskId = TaskItemFormat::extractTaskId(item->text());
        if (taskId < 0) return;

        TaskStatus newStatus = targetStatusForList(targetList);

        try {
            m_board.changeTaskStatus(taskId, newStatus);
        } catch (const std::exception& e) {
            QMessageBox::warning(qobject_cast<QWidget*>(parent()), "Нельзя переместить", e.what());
        }

        if (m_refresh) m_refresh();
    });

    return false;
}
