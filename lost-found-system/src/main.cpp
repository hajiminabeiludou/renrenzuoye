#include "../include/app.h"
#include "../include/file_io.h"
#include <iostream>

int main() {
    ensure_directory("data");
    ensure_directory("logs");

    AppState state;
    state.currentUserId = -1;
    loadAll(state);
    seedIfEmpty(state);
    saveAll(state);

    std::cout << "Default accounts:\n";
    std::cout << "  admin / 123456\n";
    std::cout << "  stu001 / 123456\n";
    std::cout << "  stu002 / 123456\n";
    runApp(state);
    return 0;
}
