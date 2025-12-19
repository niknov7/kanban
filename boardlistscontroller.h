#pragma once
#include <QObject>
#include <QPoint>
#include "taskstatus.h"

class QListWidget;
class QEvent;
class ScrumBoard;

class BoardListsController : public QObject {
    Q_OBJECT
public:
    using RefreshFn = std::function<void()>;

    BoardListsController(ScrumBoard& board,
                         QListWidget* assigned,
                         QListWidget* inProgress,
                         QListWidget* done,
                         RefreshFn refresh,
                         QObject* parent = nullptr);

    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QListWidget* listForViewport(QObject* viewport) const;
    TaskStatus targetStatusForList(QListWidget* list) const;

    void setupDnD(QListWidget* list);
    void setupContextMenu(QListWidget* list);

private:
    ScrumBoard& m_board;
    QListWidget* m_assigned{};
    QListWidget* m_inProgress{};
    QListWidget* m_done{};
    RefreshFn m_refresh;
};
