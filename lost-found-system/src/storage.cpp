#include "../include/app.h"
#include "../include/file_io.h"
#include <cstdlib>
#include <fstream>
#include <sstream>

static std::vector<std::string> split(const std::string &line, char sep) {
    std::vector<std::string> parts;
    std::string item;
    std::stringstream ss(line);
    while (std::getline(ss, item, sep)) {
        parts.push_back(item);
    }
    return parts;
}

static std::string clean(const std::string &value) {
    std::string out = value;
    for (size_t i = 0; i < out.size(); ++i) {
        if (out[i] == '|') {
            out[i] = '/';
        }
    }
    return out;
}

static int toInt(const std::string &value) {
    return std::atoi(value.c_str());
}

static bool loadUsers(AppState &state) {
    std::ifstream in("data/users.dat");
    if (!in.good()) return false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::vector<std::string> p = split(line, '|');
        if (p.size() < 7) continue;

        User u;
        u.id = toInt(p[0]);
        u.username = p[1];
        u.password = p[2];
        u.realName = p[3];
        u.role = toInt(p[4]);
        u.phone = p[5];
        u.status = toInt(p[6]);
        state.users.push_back(u);
    }
    return true;
}

static bool loadCategories(AppState &state) {
    std::ifstream in("data/categories.dat");
    if (!in.good()) return false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::vector<std::string> p = split(line, '|');
        if (p.size() < 4) continue;

        Category c;
        c.id = toInt(p[0]);
        c.name = p[1];
        c.sortOrder = toInt(p[2]);
        c.enabled = toInt(p[3]);
        state.categories.push_back(c);
    }
    return true;
}

static bool loadItems(AppState &state) {
    std::ifstream in("data/items.dat");
    if (!in.good()) return false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::vector<std::string> p = split(line, '|');
        if (p.size() < 8) continue;

        Item item;
        item.id = toInt(p[0]);
        item.title = p[1];
        item.type = toInt(p[2]);
        item.categoryId = toInt(p[3]);
        item.publisherId = toInt(p[4]);
        item.location = p[5];
        item.description = p[6];
        item.status = toInt(p[7]);
        state.items.push_back(item);
    }
    return true;
}

static bool loadClaims(AppState &state) {
    std::ifstream in("data/claims.dat");
    if (!in.good()) return false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::vector<std::string> p = split(line, '|');
        if (p.size() < 7) continue;

        Claim c;
        c.id = toInt(p[0]);
        c.itemId = toInt(p[1]);
        c.applicantId = toInt(p[2]);
        c.reason = p[3];
        c.status = toInt(p[4]);
        c.auditTime = p[5];
        c.auditRemark = p[6];
        state.claims.push_back(c);
    }
    return true;
}

bool loadAll(AppState &state) {
    state.users.clear();
    state.items.clear();
    state.claims.clear();
    state.categories.clear();
    state.currentUserId = NO_CURRENT_USER;

    bool usersLoaded = loadUsers(state);
    bool categoriesLoaded = loadCategories(state);
    bool itemsLoaded = loadItems(state);
    bool claimsLoaded = loadClaims(state);
    return usersLoaded && categoriesLoaded && itemsLoaded && claimsLoaded;
}

static bool saveUsers(const AppState &state) {
    std::ofstream out("data/users.dat");
    if (!out.good()) return false;
    for (size_t i = 0; i < state.users.size(); ++i) {
        const User &u = state.users[i];
        out << u.id << "|" << clean(u.username) << "|" << clean(u.password) << "|"
            << clean(u.realName) << "|" << u.role << "|" << clean(u.phone) << "|"
            << u.status << "\n";
    }
    return true;
}

static bool saveCategories(const AppState &state) {
    std::ofstream out("data/categories.dat");
    if (!out.good()) return false;
    for (size_t i = 0; i < state.categories.size(); ++i) {
        const Category &c = state.categories[i];
        out << c.id << "|" << clean(c.name) << "|" << c.sortOrder << "|"
            << c.enabled << "\n";
    }
    return true;
}

static bool saveItems(const AppState &state) {
    std::ofstream out("data/items.dat");
    if (!out.good()) return false;
    for (size_t i = 0; i < state.items.size(); ++i) {
        const Item &item = state.items[i];
        out << item.id << "|" << clean(item.title) << "|" << item.type << "|"
            << item.categoryId << "|" << item.publisherId << "|"
            << clean(item.location) << "|" << clean(item.description) << "|"
            << item.status << "\n";
    }
    return true;
}

static bool saveClaims(const AppState &state) {
    std::ofstream out("data/claims.dat");
    if (!out.good()) return false;
    for (size_t i = 0; i < state.claims.size(); ++i) {
        const Claim &c = state.claims[i];
        out << c.id << "|" << c.itemId << "|" << c.applicantId << "|"
            << clean(c.reason) << "|" << c.status << "|" << clean(c.auditTime)
            << "|" << clean(c.auditRemark) << "\n";
    }
    return true;
}

bool saveAll(const AppState &state) {
    if (!ensure_directory("data") || !ensure_directory("logs")) {
        return false;
    }
    return saveUsers(state) && saveCategories(state) && saveItems(state) && saveClaims(state);
}

void seedIfEmpty(AppState &state) {
    bool hasAnyData = !state.users.empty() || !state.categories.empty() ||
                      !state.items.empty() || !state.claims.empty();
    if (hasAnyData) return;

    User admin;
    admin.id = 1;
    admin.username = "admin";
    admin.password = "123456";
    admin.realName = "管理员";
    admin.role = ROLE_ADMIN;
    admin.phone = "13800000000";
    admin.status = USER_ENABLED;
    state.users.push_back(admin);

    User stu1;
    stu1.id = 2;
    stu1.username = "stu001";
    stu1.password = "123456";
    stu1.realName = "学生A";
    stu1.role = ROLE_STUDENT;
    stu1.phone = "13800000001";
    stu1.status = USER_ENABLED;
    state.users.push_back(stu1);

    User stu2;
    stu2.id = 3;
    stu2.username = "stu002";
    stu2.password = "123456";
    stu2.realName = "学生B";
    stu2.role = ROLE_STUDENT;
    stu2.phone = "13800000002";
    stu2.status = USER_ENABLED;
    state.users.push_back(stu2);

    const char *names[] = {"证件卡片", "电子产品", "书籍资料", "其他"};
    for (int i = 0; i < 4; ++i) {
        Category c;
        c.id = i + 1;
        c.name = names[i];
        c.sortOrder = i + 1;
        c.enabled = CATEGORY_ENABLED;
        state.categories.push_back(c);
    }

    Item item;
    item.id = 1;
    item.title = "图书馆附近拾到校园卡";
    item.type = ITEM_FOUND;
    item.categoryId = 1;
    item.publisherId = 2;
    item.location = "图书馆一楼";
    item.description = "蓝色卡套，15:20左右拾到";
    item.status = ITEM_OPEN;
    state.items.push_back(item);
}
