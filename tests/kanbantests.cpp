#include <gtest/gtest.h>

#include "task.h"
#include "developer.h"
#include "scrumboard.h"
#include "boardserializer.h"
#include "taskstatus.h"

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

TEST(TaskTests, Constructor_EmptyTitle_Throws) {
    EXPECT_THROW(Task(1, "", "desc"), std::invalid_argument);
}

TEST(TaskTests, AssignDeveloper_FromBacklog_SetsAssignedStatus) {
    Task t(1, "T", "D");
    EXPECT_EQ(t.status(), TaskStatus::Backlog);

    t.assignDeveloper(42);
    ASSERT_TRUE(t.assignedDeveloper().has_value());
    EXPECT_EQ(*t.assignedDeveloper(), 42);
    EXPECT_EQ(t.status(), TaskStatus::Assigned);
}

TEST(TaskTests, ChangeStatus_AssignedWithoutDeveloper_Throws) {
    Task t(1, "T", "D");
    EXPECT_THROW(t.changeStatus(TaskStatus::Assigned), std::logic_error);
}

TEST(TaskTests, ChangeStatus_InProgressWithoutDeveloper_Throws) {
    Task t(1, "T", "D");
    EXPECT_THROW(t.changeStatus(TaskStatus::InProgress), std::logic_error);
}

TEST(ScrumBoardTests, AddDeveloper_DuplicateId_Throws) {
    ScrumBoard b;
    b.addDeveloper(Developer(1, "A"));
    EXPECT_THROW(b.addDeveloper(Developer(1, "B")), std::runtime_error);
}

TEST(ScrumBoardTests, AddTask_DuplicateId_Throws) {
    ScrumBoard b;
    b.addTask(Task(1, "T1", "D1"));
    EXPECT_THROW(b.addTask(Task(1, "T2", "D2")), std::runtime_error);
}

TEST(ScrumBoardTests, AssignTask_UnknownDeveloper_Throws) {
    ScrumBoard b;
    b.addTask(Task(1, "T", "D"));
    EXPECT_THROW(b.assignTask(1, 999), std::out_of_range);
}

TEST(ScrumBoardTests, AssignTask_UnknownTask_Throws) {
    ScrumBoard b;
    b.addDeveloper(Developer(1, "Dev"));
    EXPECT_THROW(b.assignTask(999, 1), std::out_of_range);
}

TEST(ScrumBoardTests, ChangeStatus_ToInProgressRequiresAssignee) {
    ScrumBoard b;
    b.addTask(Task(1, "T", "D"));
    EXPECT_THROW(b.changeTaskStatus(1, TaskStatus::InProgress), std::logic_error);

    b.addDeveloper(Developer(1, "Dev"));
    b.assignTask(1, 1);
    EXPECT_NO_THROW(b.changeTaskStatus(1, TaskStatus::InProgress));
    EXPECT_EQ(b.getTask(1).status(), TaskStatus::InProgress);
}

TEST(ScrumBoardTests, RemoveTask_RemovesExisting) {
    ScrumBoard b;
    b.addTask(Task(1, "T", "D"));
    EXPECT_NO_THROW(b.removeTask(1));
    EXPECT_THROW((void)b.getTask(1), std::out_of_range);
}

TEST(ScrumBoardTests, RemoveTask_Unknown_Throws) {
    ScrumBoard b;
    EXPECT_THROW(b.removeTask(123), std::runtime_error);
}

static fs::path makeTempJsonPath(const std::string& name) {
    auto p = fs::temp_directory_path() / name;
    std::error_code ec;
    fs::remove(p, ec);
    return p;
}

TEST(SerializerTests, SerializeDeserialize_RoundTrip_PreservesTasksAndDevelopers) {
    ScrumBoard b;
    b.addDeveloper(Developer(1, "Alice"));
    b.addDeveloper(Developer(2, "Bob"));

    Task t1(1, "Login", "Сделать форму логина");
    b.addTask(t1);
    b.assignTask(1, 1);

    Task t2(2, "API", "Сделать эндпоинт /tasks");
    b.addTask(t2);
    b.assignTask(2, 2);
    b.changeTaskStatus(2, TaskStatus::InProgress);

    Task t3(3, "UI", "Сверстать колонки");
    b.addTask(t3);
    b.assignTask(3, 1);
    b.changeTaskStatus(3, TaskStatus::Blocked);

    Task t4(4, "Docs", "Описать проект");
    b.addTask(t4);
    b.assignTask(4, 2);
    b.changeTaskStatus(4, TaskStatus::Done);

    auto tmp = makeTempJsonPath("scrum_board_test.json");
    saveBoardToFile(b, tmp.string());

    ScrumBoard loaded = loadBoardFromFile(tmp.string());

    EXPECT_EQ(loaded.getAllDevelopers().size(), 2u);
    ASSERT_EQ(loaded.getAllTasks().size(), 4u);

    {
        const Task& x = loaded.getTask(1);
        EXPECT_EQ(x.title(), "Login");
        EXPECT_EQ(x.description(), "Сделать форму логина");
        ASSERT_TRUE(x.assignedDeveloper().has_value());
        EXPECT_EQ(*x.assignedDeveloper(), 1);
        EXPECT_EQ(x.status(), TaskStatus::Assigned);
    }
    {
        const Task& x = loaded.getTask(2);
        EXPECT_EQ(x.title(), "API");
        EXPECT_EQ(x.description(), "Сделать эндпоинт /tasks");
        ASSERT_TRUE(x.assignedDeveloper().has_value());
        EXPECT_EQ(*x.assignedDeveloper(), 2);
        EXPECT_EQ(x.status(), TaskStatus::InProgress);
    }
    {
        const Task& x = loaded.getTask(3);
        EXPECT_EQ(x.title(), "UI");
        EXPECT_EQ(x.description(), "Сверстать колонки");
        ASSERT_TRUE(x.assignedDeveloper().has_value());
        EXPECT_EQ(*x.assignedDeveloper(), 1);
        EXPECT_EQ(x.status(), TaskStatus::Blocked);
    }
    {
        const Task& x = loaded.getTask(4);
        EXPECT_EQ(x.title(), "Docs");
        EXPECT_EQ(x.description(), "Описать проект");
        ASSERT_TRUE(x.assignedDeveloper().has_value());
        EXPECT_EQ(*x.assignedDeveloper(), 2);
        EXPECT_EQ(x.status(), TaskStatus::Done);
    }

    std::error_code ec;
    fs::remove(tmp, ec);
}
