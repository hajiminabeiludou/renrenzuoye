#include "../include/app.h"
#include "../include/file_io.h"
#include <iostream>

int main() {
    if (!ensure_directory("data") || !ensure_directory("logs")) {
        std::cout << "目录初始化失败，请检查程序所在目录权限。\n";
        return 1;
    }

    AppState state;
    state.currentUserId = NO_CURRENT_USER;
    bool loaded = loadAll(state);

    if (state.users.empty() && state.categories.empty() &&
        state.items.empty() && state.claims.empty()) {
        seedIfEmpty(state);
        if (!saveAll(state)) {
            std::cout << "默认数据保存失败，请检查 data 目录。\n";
            return 1;
        }
    } else if (!loaded) {
        std::cout << "提示：检测到部分数据文件缺失或无法读取，请检查 data 目录。\n";
    }

    std::cout << "默认账号：\n";
    std::cout << "  管理员：admin / 123456\n";
    std::cout << "  学生：stu001 / 123456\n";
    std::cout << "  学生：stu002 / 123456\n";
    runApp(state);
    return 0;
}
