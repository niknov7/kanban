#pragma once
#include <QString>

class ScrumBoard;
class Task;

namespace TaskItemFormat {
QString developerNameById(const ScrumBoard& board, int devId);
int extractTaskId(const QString& itemText);

QString makeTitleLine(const ScrumBoard& board, const ::Task& task);
QString makeTooltip(const ::Task& task);
}
