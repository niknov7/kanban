#include "taskitemformat.h"
#include "scrumboard.h"   // где объявлен ScrumBoard
#include "task.h"         // где объявлен Task
#include <vector>

namespace TaskItemFormat {

QString developerNameById(const ScrumBoard& board, int devId) {
    for (const auto& [id, dev] : board.getAllDevelopers()) {
        if (id == devId) return QString::fromStdString(dev.name());
    }
    return QString("ID %1").arg(devId);
}

int extractTaskId(const QString& itemText) {
    int l = itemText.indexOf('[');
    int r = itemText.indexOf(']');
    if (l < 0 || r < 0 || r <= l + 1) return -1;
    bool ok = false;
    int id = itemText.mid(l + 1, r - l - 1).toInt(&ok);
    return ok ? id : -1;
}

QString makeTitleLine(const ScrumBoard& board, const Task& task) {
    QString text = QString("[%1] %2")
    .arg(task.id())
        .arg(QString::fromStdString(task.title()));

    if (task.assignedDeveloper()) {
        text += QString(" (%1)").arg(developerNameById(board, *task.assignedDeveloper()));
    } else {
        text += " (не назначен)";
    }

    if (task.status() == TaskStatus::Blocked) {
        text += " [ЗАБЛОКИРОВАНА]";
    }

    return text;
}

QString makeTooltip(const Task& task) {
    QString descr = QString::fromStdString(task.description()).trimmed();
    if (descr.isEmpty()) return "Описание задачи: отсутствует";
    return QString("Описание задачи: %1").arg(descr);
}

}
