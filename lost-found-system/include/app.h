#ifndef APP_H
#define APP_H

#include "models.h"
#include <vector>

const int NO_CURRENT_USER = -1;

struct AppState {
    std::vector<User> users;
    std::vector<Item> items;
    std::vector<Claim> claims;
    std::vector<Category> categories;
    int currentUserId;

    AppState() : currentUserId(NO_CURRENT_USER) {}
};

void seedIfEmpty(AppState &state);
bool loadAll(AppState &state);
bool saveAll(const AppState &state);
void runApp(AppState &state);

#endif
