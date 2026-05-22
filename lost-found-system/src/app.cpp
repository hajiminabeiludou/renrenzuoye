#include "../include/app.h"
#include "../include/file_io.h"
#include <iostream>
#include <sstream>
#include <ctime>

static int nextUserId(const AppState &s) {
    int maxId = 0;
    for (size_t i = 0; i < s.users.size(); ++i) if (s.users[i].id > maxId) maxId = s.users[i].id;
    return maxId + 1;
}

static int nextItemId(const AppState &s) {
    int maxId = 0;
    for (size_t i = 0; i < s.items.size(); ++i) if (s.items[i].id > maxId) maxId = s.items[i].id;
    return maxId + 1;
}

static int nextClaimId(const AppState &s) {
    int maxId = 0;
    for (size_t i = 0; i < s.claims.size(); ++i) if (s.claims[i].id > maxId) maxId = s.claims[i].id;
    return maxId + 1;
}

static User *currentUser(AppState &s) {
    for (size_t i = 0; i < s.users.size(); ++i) {
        if (s.users[i].id == s.currentUserId) return &s.users[i];
    }
    return 0;
}

static User *findUser(AppState &s, int id) {
    for (size_t i = 0; i < s.users.size(); ++i) if (s.users[i].id == id) return &s.users[i];
    return 0;
}

static Item *findItem(AppState &s, int id) {
    for (size_t i = 0; i < s.items.size(); ++i) if (s.items[i].id == id) return &s.items[i];
    return 0;
}

static std::string nowText() {
    time_t t = time(0);
    tm *lt = localtime(&t);
    char buf[32];
    if (lt) {
        sprintf(buf, "%04d-%02d-%02d %02d:%02d", lt->tm_year + 1900, lt->tm_mon + 1,
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

static int inputInt(const std::string &prompt, int minValue, int maxValue) {
    while (true) {
        std::string s = inputLine(prompt);
        std::stringstream ss(s);
        int v;
        char extra;
        if ((ss >> v) && !(ss >> extra) && v >= minValue && v <= maxValue) {
            return v;
        }
        std::cout << "Invalid input. Please enter " << minValue << "-" << maxValue << ".\n";
    }
}

static std::string categoryName(const AppState &s, int id) {
    for (size_t i = 0; i < s.categories.size(); ++i) if (s.categories[i].id == id) return s.categories[i].name;
    return "Unknown";
}

static std::string userName(const AppState &s, int id) {
    for (size_t i = 0; i < s.users.size(); ++i) if (s.users[i].id == id) return s.users[i].username;
    return "Unknown";
}

static void printItem(const AppState &s, const Item &item) {
    std::cout << "[" << item.id << "] "
              << (item.type == 0 ? "Lost" : "Found") << " | "
              << item.title << " | category=" << categoryName(s, item.categoryId)
              << " | location=" << item.location
              << " | status=" << (item.status == 0 ? "open" : (item.status == 1 ? "processing" : "finished"))
              << " | publisher=" << userName(s, item.publisherId) << "\n";
    std::cout << "    " << item.description << "\n";
}

static void listItems(const AppState &s) {
    std::string keyword = inputLine("Keyword (empty for all): ");
    int status = inputInt("Status 0=open 1=processing 2=finished 3=all: ", 0, 3);
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
    std::cout << "Total: " << count << "\n";
}

static void registerUser(AppState &s) {
    User u;
    u.id = nextUserId(s);
    u.username = inputLine("Username: ");
    for (size_t i = 0; i < s.users.size(); ++i) {
        if (s.users[i].username == u.username) {
            std::cout << "Username already exists.\n";
            return;
        }
    }
    u.password = inputLine("Password: ");
    u.realName = inputLine("Real name: ");
    u.phone = inputLine("Phone: ");
    u.role = 0;
    u.status = 1;
    s.users.push_back(u);
    saveAll(s);
    logAction(nowText() + " register user " + u.username);
    std::cout << "Registered. Please login.\n";
}

static bool login(AppState &s) {
    std::string username = inputLine("Username: ");
    std::string password = inputLine("Password: ");
    for (size_t i = 0; i < s.users.size(); ++i) {
        if (s.users[i].username == username && s.users[i].password == password && s.users[i].status == 1) {
            s.currentUserId = s.users[i].id;
            logAction(nowText() + " login " + username);
            std::cout << "Login success. Hello, " << s.users[i].realName << ".\n";
            return true;
        }
    }
    std::cout << "Login failed.\n";
    return false;
}

static void publishItem(AppState &s) {
    User *u = currentUser(s);
    if (!u) {
        std::cout << "Please login first.\n";
        return;
    }
    Item item;
    item.id = nextItemId(s);
    item.type = inputInt("Type 0=lost 1=found: ", 0, 1);
    item.title = inputLine("Title: ");
    std::cout << "Categories:\n";
    for (size_t i = 0; i < s.categories.size(); ++i) {
        std::cout << s.categories[i].id << ". " << s.categories[i].name << "\n";
    }
    item.categoryId = inputInt("Category id: ", 1, 9999);
    item.publisherId = u->id;
    item.location = inputLine("Location: ");
    item.description = inputLine("Description: ");
    item.status = 0;
    s.items.push_back(item);
    saveAll(s);
    logAction(nowText() + " publish item #" + item.title);
    std::cout << "Item published. ID=" << item.id << "\n";
}

static bool duplicateClaim(const AppState &s, int itemId, int userId) {
    for (size_t i = 0; i < s.claims.size(); ++i) {
        if (s.claims[i].itemId == itemId && s.claims[i].applicantId == userId && s.claims[i].status != 2) {
            return true;
        }
    }
    return false;
}

static void submitClaim(AppState &s) {
    User *u = currentUser(s);
    if (!u) {
        std::cout << "Please login first.\n";
        return;
    }
    int id = inputInt("Item id to claim: ", 1, 999999);
    Item *item = findItem(s, id);
    if (!item) {
        std::cout << "Item not found.\n";
        return;
    }
    if (item->publisherId == u->id) {
        std::cout << "You cannot claim your own item.\n";
        return;
    }
    if (item->status == 2) {
        std::cout << "This item is already finished.\n";
        return;
    }
    if (duplicateClaim(s, id, u->id)) {
        std::cout << "Duplicate claim is not allowed.\n";
        return;
    }
    Claim c;
    c.id = nextClaimId(s);
    c.itemId = id;
    c.applicantId = u->id;
    c.reason = inputLine("Proof/reason: ");
    c.status = 0;
    c.auditTime = "-";
    c.auditRemark = "-";
    s.claims.push_back(c);
    item->status = 1;
    saveAll(s);
    logAction(nowText() + " submit claim item #" + item->title);
    std::cout << "Claim submitted. Please wait for admin review.\n";
}

static void listMyClaims(const AppState &s) {
    for (size_t i = 0; i < s.claims.size(); ++i) {
        const Claim &c = s.claims[i];
        if (c.applicantId == s.currentUserId) {
            std::cout << "Claim #" << c.id << " item=" << c.itemId
                      << " status=" << (c.status == 0 ? "pending" : (c.status == 1 ? "approved" : "rejected"))
                      << " remark=" << c.auditRemark << "\n";
        }
    }
}

static void reviewClaims(AppState &s) {
    User *u = currentUser(s);
    if (!u || u->role != 1) {
        std::cout << "Admin only.\n";
        return;
    }
    int pending = 0;
    for (size_t i = 0; i < s.claims.size(); ++i) {
        Claim &c = s.claims[i];
        if (c.status == 0) {
            ++pending;
            std::cout << "Claim #" << c.id << " item=" << c.itemId
                      << " applicant=" << userName(s, c.applicantId)
                      << " reason=" << c.reason << "\n";
        }
    }
    if (pending == 0) {
        std::cout << "No pending claims.\n";
        return;
    }
    int claimId = inputInt("Claim id to review: ", 1, 999999);
    for (size_t i = 0; i < s.claims.size(); ++i) {
        Claim &c = s.claims[i];
        if (c.id == claimId && c.status == 0) {
            int result = inputInt("1=approve 2=reject: ", 1, 2);
            c.status = result;
            c.auditTime = nowText();
            c.auditRemark = inputLine("Remark: ");
            Item *item = findItem(s, c.itemId);
            if (item) {
                item->status = (result == 1) ? 2 : 0;
            }
            saveAll(s);
            logAction(nowText() + " review claim #" + c.auditRemark);
            std::cout << "Reviewed.\n";
            return;
        }
    }
    std::cout << "Pending claim not found.\n";
}

static void stats(const AppState &s) {
    int open = 0, processing = 0, finished = 0;
    for (size_t i = 0; i < s.items.size(); ++i) {
        if (s.items[i].status == 0) ++open;
        else if (s.items[i].status == 1) ++processing;
        else if (s.items[i].status == 2) ++finished;
    }
    std::cout << "Users: " << s.users.size() << "\n";
    std::cout << "Items: " << s.items.size() << " open=" << open
              << " processing=" << processing << " finished=" << finished << "\n";
    std::cout << "Claims: " << s.claims.size() << "\n";
}

static void userMenu(AppState &s) {
    while (true) {
        User *u = currentUser(s);
        std::cout << "\n== User Menu (" << (u ? u->username : "-") << ") ==\n";
        std::cout << "1. List/search items\n2. Publish item\n3. Submit claim\n4. My claims\n5. Statistics\n";
        if (u && u->role == 1) std::cout << "6. Review claims\n";
        std::cout << "0. Logout\n";
        int choice = inputInt("Choose: ", 0, 6);
        if (choice == 0) {
            s.currentUserId = -1;
            return;
        } else if (choice == 1) listItems(s);
        else if (choice == 2) publishItem(s);
        else if (choice == 3) submitClaim(s);
        else if (choice == 4) listMyClaims(s);
        else if (choice == 5) stats(s);
        else if (choice == 6) reviewClaims(s);
    }
}

void runApp(AppState &s) {
    while (true) {
        std::cout << "\n==== Lost and Found System ====\n";
        std::cout << "1. Login\n2. Register student\n3. List/search items\n4. Save data\n0. Exit\n";
        int choice = inputInt("Choose: ", 0, 4);
        if (choice == 0) {
            saveAll(s);
            std::cout << "Bye.\n";
            return;
        } else if (choice == 1) {
            if (login(s)) userMenu(s);
        } else if (choice == 2) registerUser(s);
        else if (choice == 3) listItems(s);
        else if (choice == 4) {
            saveAll(s);
            std::cout << "Saved.\n";
        }
    }
}
