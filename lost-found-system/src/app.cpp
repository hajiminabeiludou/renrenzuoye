#include "../include/app.h"
#include "../include/file_io.h"
#include <cstdio>
#include <ctime>
#include <iostream>
#include <sstream>

static int nextUserId(const AppState &s) {
    int maxId = 0;
    for (size_t i = 0; i < s.users.size(); ++i) {
        if (s.users[i].id > maxId) maxId = s.users[i].id;
    }
    return maxId + 1;
}

static int nextItemId(const AppState &s) {
    int maxId = 0;
    for (size_t i = 0; i < s.items.size(); ++i) {
        if (s.items[i].id > maxId) maxId = s.items[i].id;
    }
    return maxId + 1;
}

static int nextClaimId(const AppState &s) {
    int maxId = 0;
    for (size_t i = 0; i < s.claims.size(); ++i) {
        if (s.claims[i].id > maxId) maxId = s.claims[i].id;
    }
    return maxId + 1;
}

static User *currentUser(AppState &s) {
    for (size_t i = 0; i < s.users.size(); ++i) {
        if (s.users[i].id == s.currentUserId) return &s.users[i];
    }
    return 0;
}

static User *findUser(AppState &s, int id) {
    for (size_t i = 0; i < s.users.size(); ++i) {
        if (s.users[i].id == id) return &s.users[i];
    }
    return 0;
}

static Item *findItem(AppState &s, int id) {
    for (size_t i = 0; i < s.items.size(); ++i) {
        if (s.items[i].id == id) return &s.items[i];
    }
    return 0;
}

static bool categoryExists(const AppState &s, int id) {
    for (size_t i = 0; i < s.categories.size(); ++i) {
        if (s.categories[i].id == id && s.categories[i].enabled == CATEGORY_ENABLED) {
            return true;
        }
    }
    return false;
}

static std::string nowText() {
    time_t t = time(0);
    tm *lt = localtime(&t);
    char buf[32];
    if (lt) {
        std::sprintf(buf, "%04d-%02d-%02d %02d:%02d", lt->tm_year + 1900, lt->tm_mon + 1,
                     lt->tm_mday, lt->tm_hour, lt->tm_min);
        return buf;
    }
    return "unknown";
}

static void logAction(const std::string &text) {
    append_line("logs/operation.log", text.c_str());
}

static std::string inputLine(const std::string &prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

static std::string inputRequired(const std::string &prompt) {
    while (true) {
        std::string value = inputLine(prompt);
        if (!value.empty()) return value;
        std::cout << "该项不能为空，请重新输入。\n";
    }
}

static int inputInt(const std::string &prompt, int minValue, int maxValue) {
    while (true) {
        std::string s = inputLine(prompt);
        std::stringstream ss(s);
        int v;
        char extra;
        if ((ss >> v) && !(ss >> extra) && v >= minValue && v <= maxValue) {
            return v;
        }
        std::cout << "输入无效，请输入 " << minValue << "-" << maxValue << " 之间的数字。\n";
    }
}

static bool saveAndReport(const AppState &s, const std::string &successText) {
    if (saveAll(s)) {
        std::cout << successText << "\n";
        return true;
    }
    std::cout << "保存失败，请检查 data 目录权限或文件占用情况。\n";
    return false;
}

static std::string intText(int value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

static std::string categoryName(const AppState &s, int id) {
    for (size_t i = 0; i < s.categories.size(); ++i) {
        if (s.categories[i].id == id) return s.categories[i].name;
    }
    return "未知分类";
}

static std::string userName(const AppState &s, int id) {
    for (size_t i = 0; i < s.users.size(); ++i) {
        if (s.users[i].id == id) return s.users[i].username;
    }
    return "未知用户";
}

static std::string itemTypeName(int type) {
    return type == ITEM_LOST ? "失物" : "招领";
}

static std::string itemStatusName(int status) {
    if (status == ITEM_OPEN) return "待处理";
    if (status == ITEM_PROCESSING) return "认领中";
    if (status == ITEM_FINISHED) return "已完成";
    return "未知状态";
}

static std::string claimStatusName(int status) {
    if (status == CLAIM_PENDING) return "待审核";
    if (status == CLAIM_APPROVED) return "已通过";
    if (status == CLAIM_REJECTED) return "已驳回";
    return "未知状态";
}

static void printItem(const AppState &s, const Item &item) {
    std::cout << "[" << item.id << "] "
              << itemTypeName(item.type) << " | "
              << item.title << " | 分类：" << categoryName(s, item.categoryId)
              << " | 地点：" << item.location
              << " | 状态：" << itemStatusName(item.status)
              << " | 发布人：" << userName(s, item.publisherId) << "\n";
    std::cout << "    " << item.description << "\n";
}

static void listItems(const AppState &s) {
    std::string keyword = inputLine("关键词（直接回车表示全部）：");
    int status = inputInt("状态 0=待处理 1=认领中 2=已完成 3=全部：", 0, 3);
    int count = 0;
    for (size_t i = 0; i < s.items.size(); ++i) {
        const Item &item = s.items[i];
        bool ok = true;
        if (!keyword.empty() && item.title.find(keyword) == std::string::npos &&
            item.location.find(keyword) == std::string::npos &&
            item.description.find(keyword) == std::string::npos) {
            ok = false;
        }
        if (status != 3 && item.status != status) ok = false;
        if (ok) {
            printItem(s, item);
            ++count;
        }
    }
    std::cout << "共找到 " << count << " 条记录。\n";
}

static void registerUser(AppState &s) {
    User u;
    u.id = nextUserId(s);
    u.username = inputRequired("用户名：");
    for (size_t i = 0; i < s.users.size(); ++i) {
        if (s.users[i].username == u.username) {
            std::cout << "用户名已存在，请换一个。\n";
            return;
        }
    }
    u.password = inputRequired("密码：");
    u.realName = inputRequired("姓名：");
    u.phone = inputLine("手机号：");
    u.role = ROLE_STUDENT;
    u.status = USER_ENABLED;
    s.users.push_back(u);
    if (saveAndReport(s, "注册成功，请登录。")) {
        logAction(nowText() + " 注册学生账号 " + u.username);
    }
}

static bool login(AppState &s) {
    std::string username = inputRequired("用户名：");
    std::string password = inputRequired("密码：");
    for (size_t i = 0; i < s.users.size(); ++i) {
        if (s.users[i].username == username && s.users[i].password == password &&
            s.users[i].status == USER_ENABLED) {
            s.currentUserId = s.users[i].id;
            logAction(nowText() + " 登录 " + username);
            std::cout << "登录成功，欢迎 " << s.users[i].realName << "。\n";
            return true;
        }
    }
    std::cout << "登录失败，请检查用户名或密码。\n";
    return false;
}

static void publishItem(AppState &s) {
    User *u = currentUser(s);
    if (!u || u->role != ROLE_STUDENT) {
        std::cout << "只有学生账号可以发布失物招领信息。\n";
        return;
    }

    Item item;
    item.id = nextItemId(s);
    item.type = inputInt("类型 0=失物 1=招领：", ITEM_LOST, ITEM_FOUND);
    item.title = inputRequired("标题：");
    std::cout << "可选分类：\n";
    for (size_t i = 0; i < s.categories.size(); ++i) {
        if (s.categories[i].enabled == CATEGORY_ENABLED) {
            std::cout << s.categories[i].id << ". " << s.categories[i].name << "\n";
        }
    }
    item.categoryId = inputInt("分类编号：", 1, 9999);
    if (!categoryExists(s, item.categoryId)) {
        std::cout << "分类不存在，发布已取消。\n";
        return;
    }
    item.publisherId = u->id;
    item.location = inputRequired("地点：");
    item.description = inputRequired("描述：");
    item.status = ITEM_OPEN;
    s.items.push_back(item);
    if (saveAndReport(s, "发布成功，编号为 " + intText(item.id) + "。")) {
        logAction(nowText() + " 发布信息 " + item.title);
    }
}

static bool duplicateClaim(const AppState &s, int itemId, int userId) {
    for (size_t i = 0; i < s.claims.size(); ++i) {
        if (s.claims[i].itemId == itemId && s.claims[i].applicantId == userId &&
            s.claims[i].status != CLAIM_REJECTED) {
            return true;
        }
    }
    return false;
}

static void submitClaim(AppState &s) {
    User *u = currentUser(s);
    if (!u || u->role != ROLE_STUDENT) {
        std::cout << "只有学生账号可以提交认领申请。\n";
        return;
    }

    int id = inputInt("请输入要认领的信息编号：", 1, 999999);
    Item *item = findItem(s, id);
    if (!item) {
        std::cout << "未找到该信息。\n";
        return;
    }
    if (item->publisherId == u->id) {
        std::cout << "不能认领自己发布的信息。\n";
        return;
    }
    if (item->status == ITEM_FINISHED) {
        std::cout << "该信息已经完成处理。\n";
        return;
    }
    if (duplicateClaim(s, id, u->id)) {
        std::cout << "你已经提交过该信息的认领申请。\n";
        return;
    }

    Claim c;
    c.id = nextClaimId(s);
    c.itemId = id;
    c.applicantId = u->id;
    c.reason = inputRequired("认领凭证或说明：");
    c.status = CLAIM_PENDING;
    c.auditTime = "-";
    c.auditRemark = "-";
    s.claims.push_back(c);
    item->status = ITEM_PROCESSING;
    if (saveAndReport(s, "认领申请已提交，请等待管理员审核。")) {
        logAction(nowText() + " 提交认领 " + item->title);
    }
}

static void listMyClaims(const AppState &s) {
    int count = 0;
    for (size_t i = 0; i < s.claims.size(); ++i) {
        const Claim &c = s.claims[i];
        if (c.applicantId == s.currentUserId) {
            std::cout << "申请 #" << c.id << " 信息编号=" << c.itemId
                      << " 状态=" << claimStatusName(c.status)
                      << " 审核备注=" << c.auditRemark << "\n";
            ++count;
        }
    }
    if (count == 0) {
        std::cout << "暂无认领申请。\n";
    }
}

static void reviewClaims(AppState &s) {
    User *u = currentUser(s);
    if (!u || u->role != ROLE_ADMIN) {
        std::cout << "只有管理员可以审核认领申请。\n";
        return;
    }

    int pending = 0;
    for (size_t i = 0; i < s.claims.size(); ++i) {
        Claim &c = s.claims[i];
        if (c.status == CLAIM_PENDING) {
            Item *item = findItem(s, c.itemId);
            User *applicant = findUser(s, c.applicantId);
            ++pending;
            std::cout << "申请 #" << c.id
                      << " | 信息编号=" << c.itemId
                      << " | 标题=" << (item ? item->title : "未知信息")
                      << " | 申请人=" << (applicant ? applicant->realName : "未知用户")
                      << " | 电话=" << (applicant ? applicant->phone : "-")
                      << "\n    说明：" << c.reason << "\n";
        }
    }
    if (pending == 0) {
        std::cout << "暂无待审核申请。\n";
        return;
    }

    int claimId = inputInt("请输入要审核的申请编号：", 1, 999999);
    for (size_t i = 0; i < s.claims.size(); ++i) {
        Claim &c = s.claims[i];
        if (c.id == claimId && c.status == CLAIM_PENDING) {
            int result = inputInt("审核结果 1=通过 2=驳回：", CLAIM_APPROVED, CLAIM_REJECTED);
            c.status = result;
            c.auditTime = nowText();
            c.auditRemark = inputRequired("审核备注：");
            Item *item = findItem(s, c.itemId);
            if (item) {
                item->status = result == CLAIM_APPROVED ? ITEM_FINISHED : ITEM_OPEN;
            }
            if (saveAndReport(s, "审核完成。")) {
                logAction(nowText() + " 审核认领申请 #" + intText(c.id));
            }
            return;
        }
    }
    std::cout << "未找到待审核的申请。\n";
}

static void stats(const AppState &s) {
    int open = 0, processing = 0, finished = 0;
    for (size_t i = 0; i < s.items.size(); ++i) {
        if (s.items[i].status == ITEM_OPEN) ++open;
        else if (s.items[i].status == ITEM_PROCESSING) ++processing;
        else if (s.items[i].status == ITEM_FINISHED) ++finished;
    }
    std::cout << "用户数：" << s.users.size() << "\n";
    std::cout << "信息数：" << s.items.size()
              << "，待处理=" << open
              << "，认领中=" << processing
              << "，已完成=" << finished << "\n";
    std::cout << "认领申请数：" << s.claims.size() << "\n";
}

static void studentMenu(AppState &s) {
    while (true) {
        User *u = currentUser(s);
        std::cout << "\n== 学生菜单（" << (u ? u->username : "-") << "）==\n";
        std::cout << "1. 浏览/搜索信息\n2. 发布失物招领\n3. 提交认领申请\n4. 我的认领申请\n5. 查看统计\n0. 退出登录\n";
        int choice = inputInt("请选择：", 0, 5);
        if (choice == 0) {
            s.currentUserId = NO_CURRENT_USER;
            return;
        } else if (choice == 1) listItems(s);
        else if (choice == 2) publishItem(s);
        else if (choice == 3) submitClaim(s);
        else if (choice == 4) listMyClaims(s);
        else if (choice == 5) stats(s);
    }
}

static void adminMenu(AppState &s) {
    while (true) {
        User *u = currentUser(s);
        std::cout << "\n== 管理员菜单（" << (u ? u->username : "-") << "）==\n";
        std::cout << "1. 浏览/搜索信息\n2. 审核认领申请\n3. 查看统计\n0. 退出登录\n";
        int choice = inputInt("请选择：", 0, 3);
        if (choice == 0) {
            s.currentUserId = NO_CURRENT_USER;
            return;
        } else if (choice == 1) listItems(s);
        else if (choice == 2) reviewClaims(s);
        else if (choice == 3) stats(s);
    }
}

void runApp(AppState &s) {
    while (true) {
        std::cout << "\n==== 校园失物招领系统 ====\n";
        std::cout << "1. 登录\n2. 注册学生账号\n3. 浏览/搜索信息\n4. 保存数据\n0. 退出系统\n";
        int choice = inputInt("请选择：", 0, 4);
        if (choice == 0) {
            saveAndReport(s, "数据已保存，系统退出。");
            return;
        } else if (choice == 1) {
            if (login(s)) {
                User *u = currentUser(s);
                if (u && u->role == ROLE_ADMIN) adminMenu(s);
                else studentMenu(s);
            }
        } else if (choice == 2) registerUser(s);
        else if (choice == 3) listItems(s);
        else if (choice == 4) saveAndReport(s, "数据已保存。");
    }
}
