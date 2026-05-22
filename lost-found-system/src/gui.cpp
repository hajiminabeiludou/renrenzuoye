#include "../include/app.h"
#include "../include/file_io.h"
#include <windows.h>
#include <sstream>
#include <ctime>
#include <string>

#define ID_LOGIN_USER      101
#define ID_LOGIN_PASS      102
#define ID_LOGIN_BTN       103
#define ID_REG_USER        104
#define ID_REG_PASS        105
#define ID_REG_NAME        106
#define ID_REG_PHONE       107
#define ID_REG_BTN         108
#define ID_AUTH_HINT       109

#define ID_S_STATUS        201
#define ID_S_LOGOUT        202
#define ID_S_SAVE          203
#define ID_S_STATS         204
#define ID_S_TITLE         205
#define ID_S_LOCATION      206
#define ID_S_DESC          207
#define ID_S_TYPE          208
#define ID_S_CATEGORY      209
#define ID_S_PUBLISH       210
#define ID_S_KEYWORD       211
#define ID_S_FILTER        212
#define ID_S_SEARCH        213
#define ID_S_LIST          214
#define ID_S_DETAIL        215
#define ID_S_REASON        216
#define ID_S_CLAIM         217

#define ID_A_STATUS        301
#define ID_A_LOGOUT        302
#define ID_A_SAVE          303
#define ID_A_STATS         304
#define ID_A_REFRESH       305
#define ID_A_CLAIM_LIST    306
#define ID_A_DETAIL        307
#define ID_A_APPROVE       308
#define ID_A_REJECT        309

static AppState g_state;
static HINSTANCE g_inst;
static HWND g_authWnd;
static HWND g_studentWnd;
static HWND g_adminWnd;

static HWND g_loginUser, g_loginPass, g_regUser, g_regPass, g_regName, g_regPhone, g_authHint;

static HWND g_sStatus, g_sTitle, g_sLocation, g_sDesc, g_sType, g_sCategory;
static HWND g_sKeyword, g_sFilter, g_sList, g_sDetail, g_sReason;

static HWND g_aStatus, g_aClaimList, g_aDetail;

static HFONT g_font, g_titleFont;

static std::wstring utf8ToWide(const std::string &text) {
    if (text.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, 0, 0);
    if (len <= 0) return L"";
    std::wstring wide(len - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wide[0], len);
    return wide;
}

static std::string wideToUtf8(const std::wstring &text) {
    if (text.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, 0, 0, 0, 0);
    if (len <= 0) return "";
    std::string utf8(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, &utf8[0], len, 0, 0);
    return utf8;
}

static void showMessage(HWND hwnd, const std::string &body, const std::string &title, UINT flags) {
    std::wstring wBody = utf8ToWide(body);
    std::wstring wTitle = utf8ToWide(title);
    MessageBoxW(hwnd, wBody.c_str(), wTitle.c_str(), flags);
}

static void ensureFonts() {
    if (!g_font) {
        g_font = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                             OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                             DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei UI");
    }
    if (!g_titleFont) {
        g_titleFont = CreateFontW(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                  DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei UI");
    }
}

static std::string getText(HWND hwnd) {
    int len = GetWindowTextLengthW(hwnd);
    if (len <= 0) return "";
    std::wstring wide(len + 1, L'\0');
    GetWindowTextW(hwnd, &wide[0], len + 1);
    wide.resize(len);
    return wideToUtf8(wide);
}

static void setText(HWND hwnd, const std::string &text) {
    if (hwnd) {
        std::wstring wide = utf8ToWide(text);
        SetWindowTextW(hwnd, wide.c_str());
    }
}

static HWND label(HWND parent, const char *text, int x, int y, int w, int h) {
    std::wstring wide = utf8ToWide(text);
    HWND hnd = CreateWindowW(L"STATIC", wide.c_str(), WS_CHILD | WS_VISIBLE, x, y, w, h, parent, 0, 0, 0);
    SendMessageW(hnd, WM_SETFONT, (WPARAM)g_font, TRUE);
    return hnd;
}

static HWND titleLabel(HWND parent, const char *text, int x, int y, int w, int h) {
    std::wstring wide = utf8ToWide(text);
    HWND hnd = CreateWindowW(L"STATIC", wide.c_str(), WS_CHILD | WS_VISIBLE, x, y, w, h, parent, 0, 0, 0);
    SendMessageW(hnd, WM_SETFONT, (WPARAM)g_titleFont, TRUE);
    return hnd;
}

static HWND edit(HWND parent, int id, int x, int y, int w, int h, DWORD extra) {
    HWND hnd = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | extra,
                             x, y, w, h, parent, (HMENU)id, 0, 0);
    SendMessageW(hnd, WM_SETFONT, (WPARAM)g_font, TRUE);
    return hnd;
}

static HWND button(HWND parent, int id, const char *text, int x, int y, int w, int h) {
    std::wstring wide = utf8ToWide(text);
    HWND hnd = CreateWindowW(L"BUTTON", wide.c_str(), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                             x, y, w, h, parent, (HMENU)id, 0, 0);
    SendMessageW(hnd, WM_SETFONT, (WPARAM)g_font, TRUE);
    return hnd;
}

static HWND groupBox(HWND parent, const char *text, int x, int y, int w, int h) {
    std::wstring wide = utf8ToWide(text);
    HWND hnd = CreateWindowW(L"BUTTON", wide.c_str(), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                             x, y, w, h, parent, 0, 0, 0);
    SendMessageW(hnd, WM_SETFONT, (WPARAM)g_font, TRUE);
    return hnd;
}

static int comboAdd(HWND combo, const std::string &text) {
    std::wstring wide = utf8ToWide(text);
    return (int)SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)wide.c_str());
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

static User *currentUser() {
    for (size_t i = 0; i < g_state.users.size(); ++i) {
        if (g_state.users[i].id == g_state.currentUserId) return &g_state.users[i];
    }
    return 0;
}

static Item *findItem(int id) {
    for (size_t i = 0; i < g_state.items.size(); ++i) {
        if (g_state.items[i].id == id) return &g_state.items[i];
    }
    return 0;
}

static Claim *findClaim(int id) {
    for (size_t i = 0; i < g_state.claims.size(); ++i) {
        if (g_state.claims[i].id == id) return &g_state.claims[i];
    }
    return 0;
}

static std::string userName(int id) {
    for (size_t i = 0; i < g_state.users.size(); ++i) {
        if (g_state.users[i].id == id) return g_state.users[i].username;
    }
    return "未知用户";
}

static std::string categoryName(int id) {
    for (size_t i = 0; i < g_state.categories.size(); ++i) {
        if (g_state.categories[i].id == id) {
            std::string name = g_state.categories[i].name;
            if (name == "Card") return "证件卡片";
            if (name == "Electronics") return "电子产品";
            if (name == "Book") return "书籍资料";
            if (name == "Other") return "其他";
            return name;
        }
    }
    return "未知分类";
}

static int nextUserId() {
    int maxId = 0;
    for (size_t i = 0; i < g_state.users.size(); ++i) if (g_state.users[i].id > maxId) maxId = g_state.users[i].id;
    return maxId + 1;
}

static int nextItemId() {
    int maxId = 0;
    for (size_t i = 0; i < g_state.items.size(); ++i) if (g_state.items[i].id > maxId) maxId = g_state.items[i].id;
    return maxId + 1;
}

static int nextClaimId() {
    int maxId = 0;
    for (size_t i = 0; i < g_state.claims.size(); ++i) if (g_state.claims[i].id > maxId) maxId = g_state.claims[i].id;
    return maxId + 1;
}

static bool usernameExists(const std::string &username) {
    for (size_t i = 0; i < g_state.users.size(); ++i) {
        if (g_state.users[i].username == username) return true;
    }
    return false;
}

static std::string itemStatusText(int status) {
    if (status == 0) return "待认领";
    if (status == 1) return "处理中";
    return "已完成";
}

static std::string claimStatusText(int status) {
    if (status == 0) return "待审核";
    if (status == 1) return "已通过";
    return "已驳回";
}

static std::string itemLine(const Item &item) {
    std::stringstream ss;
    ss << "#" << item.id << "  " << (item.type == 0 ? "失物" : "招领")
       << "  " << item.title << "  [" << itemStatusText(item.status) << "]";
    return ss.str();
}

static std::string claimLine(const Claim &claim) {
    Item *item = findItem(claim.itemId);
    std::stringstream ss;
    ss << "#" << claim.id << "  " << claimStatusText(claim.status)
       << "  物品：" << (item ? item->title : "未知物品")
       << "  申请人：" << userName(claim.applicantId);
    return ss.str();
}

static int selectedStudentItemId() {
    int index = (int)SendMessageW(g_sList, LB_GETCURSEL, 0, 0);
    if (index == LB_ERR) return -1;
    return (int)SendMessageW(g_sList, LB_GETITEMDATA, index, 0);
}

static int selectedAdminClaimId() {
    int index = (int)SendMessageW(g_aClaimList, LB_GETCURSEL, 0, 0);
    if (index == LB_ERR) return -1;
    return (int)SendMessageW(g_aClaimList, LB_GETITEMDATA, index, 0);
}

static int selectedCategoryId() {
    int index = (int)SendMessageW(g_sCategory, CB_GETCURSEL, 0, 0);
    if (index == CB_ERR || g_state.categories.empty()) return 1;
    return (int)SendMessageW(g_sCategory, CB_GETITEMDATA, index, 0);
}

static void updateStudentStatus() {
    User *u = currentUser();
    if (!u) setText(g_sStatus, "当前没有登录用户");
    else setText(g_sStatus, "学生端 当前用户：" + u->username);
}

static void updateAdminStatus() {
    User *u = currentUser();
    if (!u) setText(g_aStatus, "当前没有登录用户");
    else setText(g_aStatus, "管理员端 当前用户：" + u->username);
}

static void showStats(HWND hwnd) {
    int open = 0, processing = 0, finished = 0;
    for (size_t i = 0; i < g_state.items.size(); ++i) {
        if (g_state.items[i].status == 0) ++open;
        else if (g_state.items[i].status == 1) ++processing;
        else ++finished;
    }
    std::stringstream ss;
    ss << "用户数：" << g_state.users.size() << "\n";
    ss << "物品数：" << g_state.items.size() << "\n";
    ss << "待认领：" << open << "\n处理中：" << processing << "\n已完成：" << finished << "\n";
    ss << "认领申请数：" << g_state.claims.size();
    showMessage(hwnd, ss.str(), "统计信息", MB_OK | MB_ICONINFORMATION);
}

static void studentShowDetails(int itemId) {
    Item *item = findItem(itemId);
    if (!item) {
        setText(g_sDetail, "");
        return;
    }
    std::stringstream ss;
    ss << "物品编号 #" << item->id << "\r\n";
    ss << "类型：" << (item->type == 0 ? "失物" : "招领") << "\r\n";
    ss << "标题：" << item->title << "\r\n";
    ss << "分类：" << categoryName(item->categoryId) << "\r\n";
    ss << "发布人：" << userName(item->publisherId) << "\r\n";
    ss << "地点：" << item->location << "\r\n";
    ss << "状态：" << itemStatusText(item->status) << "\r\n";
    ss << "描述：" << item->description << "\r\n";
    setText(g_sDetail, ss.str());
}

static void studentRefreshItems() {
    std::string keyword = getText(g_sKeyword);
    int statusFilter = (int)SendMessageW(g_sFilter, CB_GETCURSEL, 0, 0);
    SendMessageW(g_sList, LB_RESETCONTENT, 0, 0);
    for (size_t i = 0; i < g_state.items.size(); ++i) {
        Item &item = g_state.items[i];
        if (!keyword.empty() &&
            item.title.find(keyword) == std::string::npos &&
            item.location.find(keyword) == std::string::npos &&
            item.description.find(keyword) == std::string::npos) {
            continue;
        }
        if (statusFilter >= 1 && item.status != statusFilter - 1) continue;
        std::string line = itemLine(item);
        std::wstring wide = utf8ToWide(line);
        int pos = (int)SendMessageW(g_sList, LB_ADDSTRING, 0, (LPARAM)wide.c_str());
        SendMessageW(g_sList, LB_SETITEMDATA, pos, item.id);
    }
}

static void adminShowClaimDetails(int claimId) {
    Claim *claim = findClaim(claimId);
    if (!claim) {
        setText(g_aDetail, "");
        return;
    }
    Item *item = findItem(claim->itemId);
    std::stringstream ss;
    ss << "申请编号 #" << claim->id << "\r\n";
    ss << "申请状态：" << claimStatusText(claim->status) << "\r\n";
    ss << "申请人：" << userName(claim->applicantId) << "\r\n";
    ss << "认领理由：" << claim->reason << "\r\n";
    ss << "审核时间：" << claim->auditTime << "\r\n";
    ss << "审核备注：" << claim->auditRemark << "\r\n\r\n";
    if (item) {
        ss << "关联物品 #" << item->id << "\r\n";
        ss << "标题：" << item->title << "\r\n";
        ss << "地点：" << item->location << "\r\n";
        ss << "发布人：" << userName(item->publisherId) << "\r\n";
        ss << "物品状态：" << itemStatusText(item->status) << "\r\n";
        ss << "描述：" << item->description << "\r\n";
    } else {
        ss << "关联物品不存在。\r\n";
    }
    setText(g_aDetail, ss.str());
}

static void adminRefreshClaims() {
    SendMessageW(g_aClaimList, LB_RESETCONTENT, 0, 0);
    for (size_t i = 0; i < g_state.claims.size(); ++i) {
        Claim &claim = g_state.claims[i];
        if (claim.status != 0) continue;
        std::string line = claimLine(claim);
        std::wstring wide = utf8ToWide(line);
        int pos = (int)SendMessageW(g_aClaimList, LB_ADDSTRING, 0, (LPARAM)wide.c_str());
        SendMessageW(g_aClaimList, LB_SETITEMDATA, pos, claim.id);
    }
}

static void returnToAuth() {
    g_state.currentUserId = -1;
    saveAll(g_state);
    if (g_studentWnd) ShowWindow(g_studentWnd, SW_HIDE);
    if (g_adminWnd) ShowWindow(g_adminWnd, SW_HIDE);
    ShowWindow(g_authWnd, SW_SHOW);
    SetForegroundWindow(g_authWnd);
}

static void showStudentWindow() {
    if (!g_studentWnd) {
        std::wstring title = utf8ToWide("校园失物招领系统 - 学生端");
        g_studentWnd = CreateWindowW(L"LostFoundStudentWindow", title.c_str(),
                                     WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                     CW_USEDEFAULT, CW_USEDEFAULT, 920, 620, 0, 0, g_inst, 0);
    }
    updateStudentStatus();
    studentRefreshItems();
    ShowWindow(g_authWnd, SW_HIDE);
    if (g_adminWnd) ShowWindow(g_adminWnd, SW_HIDE);
    ShowWindow(g_studentWnd, SW_SHOW);
    SetForegroundWindow(g_studentWnd);
}

static void showAdminWindow() {
    if (!g_adminWnd) {
        std::wstring title = utf8ToWide("校园失物招领系统 - 管理员端");
        g_adminWnd = CreateWindowW(L"LostFoundAdminWindow", title.c_str(),
                                   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                   CW_USEDEFAULT, CW_USEDEFAULT, 820, 560, 0, 0, g_inst, 0);
    }
    updateAdminStatus();
    adminRefreshClaims();
    ShowWindow(g_authWnd, SW_HIDE);
    if (g_studentWnd) ShowWindow(g_studentWnd, SW_HIDE);
    ShowWindow(g_adminWnd, SW_SHOW);
    SetForegroundWindow(g_adminWnd);
}

static void doLogin(HWND hwnd) {
    std::string username = getText(g_loginUser);
    std::string password = getText(g_loginPass);
    for (size_t i = 0; i < g_state.users.size(); ++i) {
        User &u = g_state.users[i];
        if (u.username == username && u.password == password && u.status == 1) {
            g_state.currentUserId = u.id;
            logAction(nowText() + " gui login " + username);
            setText(g_authHint, "登录成功。");
            if (u.role == 1) showAdminWindow();
            else showStudentWindow();
            return;
        }
    }
    setText(g_authHint, "登录失败，请检查账号和密码。");
    showMessage(hwnd, "登录失败，请检查账号和密码。", "失物招领系统", MB_OK | MB_ICONWARNING);
}

static void doRegister(HWND hwnd) {
    std::string username = getText(g_regUser);
    std::string password = getText(g_regPass);
    std::string realName = getText(g_regName);
    std::string phone = getText(g_regPhone);
    if (username.empty() || password.empty() || realName.empty()) {
        showMessage(hwnd, "账号、密码和姓名不能为空。", "失物招领系统", MB_OK | MB_ICONWARNING);
        return;
    }
    if (usernameExists(username)) {
        showMessage(hwnd, "这个账号已经存在。", "失物招领系统", MB_OK | MB_ICONWARNING);
        return;
    }
    User u;
    u.id = nextUserId();
    u.username = username;
    u.password = password;
    u.realName = realName;
    u.phone = phone;
    u.role = 0;
    u.status = 1;
    g_state.users.push_back(u);
    saveAll(g_state);
    logAction(nowText() + " gui register " + username);
    setText(g_loginUser, username);
    setText(g_loginPass, password);
    setText(g_authHint, "注册成功，现在可以登录。");
    showMessage(hwnd, "注册成功，请登录。", "失物招领系统", MB_OK | MB_ICONINFORMATION);
}

static void publishItem(HWND hwnd) {
    User *u = currentUser();
    if (!u || u->role != 0) {
        showMessage(hwnd, "只有学生端可以发布失物招领。", "失物招领系统", MB_OK | MB_ICONWARNING);
        return;
    }
    std::string title = getText(g_sTitle);
    std::string location = getText(g_sLocation);
    std::string desc = getText(g_sDesc);
    if (title.empty() || location.empty()) {
        showMessage(hwnd, "标题和地点不能为空。", "失物招领系统", MB_OK | MB_ICONWARNING);
        return;
    }
    Item item;
    item.id = nextItemId();
    item.type = (int)SendMessageW(g_sType, CB_GETCURSEL, 0, 0);
    if (item.type < 0) item.type = 0;
    item.categoryId = selectedCategoryId();
    item.publisherId = u->id;
    item.title = title;
    item.location = location;
    item.description = desc.empty() ? "暂无描述" : desc;
    item.status = 0;
    g_state.items.push_back(item);
    saveAll(g_state);
    logAction(nowText() + " gui publish " + title);
    setText(g_sTitle, "");
    setText(g_sLocation, "");
    setText(g_sDesc, "");
    studentRefreshItems();
    showMessage(hwnd, "物品信息已发布。", "失物招领系统", MB_OK | MB_ICONINFORMATION);
}

static bool duplicateClaim(int itemId, int userId) {
    for (size_t i = 0; i < g_state.claims.size(); ++i) {
        Claim &c = g_state.claims[i];
        if (c.itemId == itemId && c.applicantId == userId && c.status != 2) return true;
    }
    return false;
}

static void submitClaim(HWND hwnd) {
    User *u = currentUser();
    if (!u || u->role != 0) {
        showMessage(hwnd, "只有学生端可以提交认领申请。", "失物招领系统", MB_OK | MB_ICONWARNING);
        return;
    }
    int id = selectedStudentItemId();
    Item *item = findItem(id);
    if (!item) {
        showMessage(hwnd, "请先选择一个物品。", "失物招领系统", MB_OK | MB_ICONWARNING);
        return;
    }
    if (item->publisherId == u->id) {
        showMessage(hwnd, "不能认领自己发布的物品。", "失物招领系统", MB_OK | MB_ICONWARNING);
        return;
    }
    if (item->status == 2) {
        showMessage(hwnd, "这个物品已经处理完成。", "失物招领系统", MB_OK | MB_ICONWARNING);
        return;
    }
    if (duplicateClaim(item->id, u->id)) {
        showMessage(hwnd, "不能重复提交认领申请。", "失物招领系统", MB_OK | MB_ICONWARNING);
        return;
    }
    Claim c;
    c.id = nextClaimId();
    c.itemId = item->id;
    c.applicantId = u->id;
    c.reason = getText(g_sReason);
    if (c.reason.empty()) c.reason = "学生通过界面提交认领";
    c.status = 0;
    c.auditTime = "-";
    c.auditRemark = "-";
    g_state.claims.push_back(c);
    item->status = 1;
    saveAll(g_state);
    studentRefreshItems();
    studentShowDetails(item->id);
    setText(g_sReason, "");
    showMessage(hwnd, "认领申请已提交，等待管理员审核。", "失物招领系统", MB_OK | MB_ICONINFORMATION);
}

static void reviewClaim(HWND hwnd, int approve) {
    User *u = currentUser();
    if (!u || u->role != 1) {
        showMessage(hwnd, "只有管理员可以审核申请。", "失物招领系统", MB_OK | MB_ICONWARNING);
        return;
    }
    int claimId = selectedAdminClaimId();
    Claim *claim = findClaim(claimId);
    if (!claim || claim->status != 0) {
        showMessage(hwnd, "请先选择一条待审核申请。", "失物招领系统", MB_OK | MB_ICONWARNING);
        return;
    }
    claim->status = approve ? 1 : 2;
    claim->auditTime = nowText();
    claim->auditRemark = approve ? "管理员审核通过" : "管理员审核驳回";
    Item *item = findItem(claim->itemId);
    if (item) item->status = approve ? 2 : 0;
    saveAll(g_state);
    adminRefreshClaims();
    setText(g_aDetail, "");
    showMessage(hwnd, approve ? "已通过该认领申请。" : "已驳回该认领申请。", "失物招领系统", MB_OK | MB_ICONINFORMATION);
}

static LRESULT CALLBACK AuthWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        ensureFonts();
        titleLabel(hwnd, "校园失物招领系统", 24, 18, 360, 32);
        label(hwnd, "请先登录或注册，系统会按账号角色进入不同页面。", 26, 54, 460, 22);

        groupBox(hwnd, "登录", 22, 88, 220, 190);
        label(hwnd, "账号", 42, 122, 80, 22);
        g_loginUser = edit(hwnd, ID_LOGIN_USER, 112, 119, 105, 25, 0);
        setText(g_loginUser, "admin");
        label(hwnd, "密码", 42, 156, 80, 22);
        g_loginPass = edit(hwnd, ID_LOGIN_PASS, 112, 153, 105, 25, ES_PASSWORD);
        setText(g_loginPass, "123456");
        button(hwnd, ID_LOGIN_BTN, "登录", 112, 195, 105, 30);
        label(hwnd, "admin / 123456", 42, 236, 160, 22);

        groupBox(hwnd, "学生注册", 262, 88, 250, 230);
        label(hwnd, "账号", 282, 122, 80, 22);
        g_regUser = edit(hwnd, ID_REG_USER, 362, 119, 125, 25, 0);
        label(hwnd, "密码", 282, 156, 80, 22);
        g_regPass = edit(hwnd, ID_REG_PASS, 362, 153, 125, 25, ES_PASSWORD);
        label(hwnd, "姓名", 282, 190, 80, 22);
        g_regName = edit(hwnd, ID_REG_NAME, 362, 187, 125, 25, 0);
        label(hwnd, "电话", 282, 224, 80, 22);
        g_regPhone = edit(hwnd, ID_REG_PHONE, 362, 221, 125, 25, 0);
        button(hwnd, ID_REG_BTN, "注册", 362, 266, 125, 30);

        std::wstring hint = utf8ToWide("默认账号：admin、stu001、stu002 / 123456");
        g_authHint = CreateWindowW(L"STATIC", hint.c_str(), WS_CHILD | WS_VISIBLE,
                                   24, 330, 470, 22, hwnd, (HMENU)ID_AUTH_HINT, 0, 0);
        SendMessageW(g_authHint, WM_SETFONT, (WPARAM)g_font, TRUE);
        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_LOGIN_BTN) doLogin(hwnd);
        else if (LOWORD(wParam) == ID_REG_BTN) doRegister(hwnd);
        return 0;
    case WM_CLOSE:
        saveAll(g_state);
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        saveAll(g_state);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK StudentWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        ensureFonts();
        titleLabel(hwnd, "学生端", 20, 14, 120, 32);
        g_sStatus = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 145, 22, 430, 22,
                                  hwnd, (HMENU)ID_S_STATUS, 0, 0);
        SendMessageW(g_sStatus, WM_SETFONT, (WPARAM)g_font, TRUE);
        button(hwnd, ID_S_STATS, "统计", 620, 17, 80, 30);
        button(hwnd, ID_S_SAVE, "保存", 710, 17, 80, 30);
        button(hwnd, ID_S_LOGOUT, "退出登录", 800, 17, 90, 30);

        groupBox(hwnd, "发布失物招领", 18, 62, 322, 202);
        label(hwnd, "标题", 38, 95, 70, 22);
        g_sTitle = edit(hwnd, ID_S_TITLE, 112, 92, 205, 25, 0);
        label(hwnd, "地点", 38, 127, 70, 22);
        g_sLocation = edit(hwnd, ID_S_LOCATION, 112, 124, 205, 25, 0);
        label(hwnd, "类型", 38, 159, 70, 22);
        g_sType = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
                                112, 156, 92, 100, hwnd, (HMENU)ID_S_TYPE, 0, 0);
        SendMessageW(g_sType, WM_SETFONT, (WPARAM)g_font, TRUE);
        comboAdd(g_sType, "失物");
        comboAdd(g_sType, "招领");
        SendMessageW(g_sType, CB_SETCURSEL, 0, 0);
        label(hwnd, "分类", 210, 159, 38, 22);
        g_sCategory = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
                                    248, 156, 69, 140, hwnd, (HMENU)ID_S_CATEGORY, 0, 0);
        SendMessageW(g_sCategory, WM_SETFONT, (WPARAM)g_font, TRUE);
        for (size_t i = 0; i < g_state.categories.size(); ++i) {
            int pos = comboAdd(g_sCategory, categoryName(g_state.categories[i].id));
            SendMessageW(g_sCategory, CB_SETITEMDATA, pos, g_state.categories[i].id);
        }
        SendMessageW(g_sCategory, CB_SETCURSEL, 0, 0);
        label(hwnd, "描述", 38, 191, 70, 22);
        g_sDesc = edit(hwnd, ID_S_DESC, 112, 188, 205, 42, ES_MULTILINE | ES_AUTOVSCROLL);
        button(hwnd, ID_S_PUBLISH, "发布", 215, 232, 102, 28);

        groupBox(hwnd, "浏览与认领", 18, 278, 322, 282);
        g_sKeyword = edit(hwnd, ID_S_KEYWORD, 38, 310, 135, 25, 0);
        g_sFilter = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
                                  183, 310, 92, 120, hwnd, (HMENU)ID_S_FILTER, 0, 0);
        SendMessageW(g_sFilter, WM_SETFONT, (WPARAM)g_font, TRUE);
        comboAdd(g_sFilter, "全部");
        comboAdd(g_sFilter, "待认领");
        comboAdd(g_sFilter, "处理中");
        comboAdd(g_sFilter, "已完成");
        SendMessageW(g_sFilter, CB_SETCURSEL, 0, 0);
        button(hwnd, ID_S_SEARCH, "查询", 282, 309, 42, 27);
        g_sList = CreateWindowW(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL,
                                38, 348, 286, 190, hwnd, (HMENU)ID_S_LIST, 0, 0);
        SendMessageW(g_sList, WM_SETFONT, (WPARAM)g_font, TRUE);

        groupBox(hwnd, "物品详情与认领申请", 360, 62, 530, 498);
        g_sDetail = edit(hwnd, ID_S_DETAIL, 382, 92, 486, 320, ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY);
        label(hwnd, "认领理由", 382, 428, 100, 22);
        g_sReason = edit(hwnd, ID_S_REASON, 482, 425, 386, 25, 0);
        button(hwnd, ID_S_CLAIM, "提交认领", 382, 470, 120, 30);

        updateStudentStatus();
        studentRefreshItems();
        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_S_LOGOUT) returnToAuth();
        else if (LOWORD(wParam) == ID_S_SAVE) { saveAll(g_state); showMessage(hwnd, "数据已保存。", "失物招领系统", MB_OK); }
        else if (LOWORD(wParam) == ID_S_STATS) showStats(hwnd);
        else if (LOWORD(wParam) == ID_S_PUBLISH) publishItem(hwnd);
        else if (LOWORD(wParam) == ID_S_SEARCH) studentRefreshItems();
        else if (LOWORD(wParam) == ID_S_CLAIM) submitClaim(hwnd);
        else if (LOWORD(wParam) == ID_S_LIST && HIWORD(wParam) == LBN_SELCHANGE) studentShowDetails(selectedStudentItemId());
        return 0;
    case WM_CLOSE:
        returnToAuth();
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK AdminWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        ensureFonts();
        titleLabel(hwnd, "管理员端", 20, 14, 140, 32);
        g_aStatus = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 165, 22, 360, 22,
                                  hwnd, (HMENU)ID_A_STATUS, 0, 0);
        SendMessageW(g_aStatus, WM_SETFONT, (WPARAM)g_font, TRUE);
        button(hwnd, ID_A_STATS, "统计", 520, 17, 80, 30);
        button(hwnd, ID_A_SAVE, "保存", 610, 17, 80, 30);
        button(hwnd, ID_A_LOGOUT, "退出登录", 700, 17, 90, 30);

        groupBox(hwnd, "待审核认领申请", 18, 62, 330, 438);
        g_aClaimList = CreateWindowW(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL,
                                     38, 95, 290, 350, hwnd, (HMENU)ID_A_CLAIM_LIST, 0, 0);
        SendMessageW(g_aClaimList, WM_SETFONT, (WPARAM)g_font, TRUE);
        button(hwnd, ID_A_REFRESH, "刷新", 238, 455, 90, 30);

        groupBox(hwnd, "申请详情与审核", 370, 62, 410, 438);
        g_aDetail = edit(hwnd, ID_A_DETAIL, 392, 95, 366, 300, ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY);
        button(hwnd, ID_A_APPROVE, "通过认领", 392, 425, 110, 32);
        button(hwnd, ID_A_REJECT, "驳回认领", 515, 425, 110, 32);

        updateAdminStatus();
        adminRefreshClaims();
        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_A_LOGOUT) returnToAuth();
        else if (LOWORD(wParam) == ID_A_SAVE) { saveAll(g_state); showMessage(hwnd, "数据已保存。", "失物招领系统", MB_OK); }
        else if (LOWORD(wParam) == ID_A_STATS) showStats(hwnd);
        else if (LOWORD(wParam) == ID_A_REFRESH) adminRefreshClaims();
        else if (LOWORD(wParam) == ID_A_APPROVE) reviewClaim(hwnd, 1);
        else if (LOWORD(wParam) == ID_A_REJECT) reviewClaim(hwnd, 0);
        else if (LOWORD(wParam) == ID_A_CLAIM_LIST && HIWORD(wParam) == LBN_SELCHANGE) adminShowClaimDetails(selectedAdminClaimId());
        return 0;
    case WM_CLOSE:
        returnToAuth();
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    g_inst = hInst;
    ensure_directory("data");
    ensure_directory("logs");
    g_state.currentUserId = -1;
    loadAll(g_state);
    seedIfEmpty(g_state);
    saveAll(g_state);

    WNDCLASSW authClass;
    ZeroMemory(&authClass, sizeof(authClass));
    authClass.lpfnWndProc = AuthWndProc;
    authClass.hInstance = hInst;
    authClass.hCursor = LoadCursor(0, IDC_ARROW);
    authClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    authClass.lpszClassName = L"LostFoundAuthWindow";
    RegisterClassW(&authClass);

    WNDCLASSW studentClass;
    ZeroMemory(&studentClass, sizeof(studentClass));
    studentClass.lpfnWndProc = StudentWndProc;
    studentClass.hInstance = hInst;
    studentClass.hCursor = LoadCursor(0, IDC_ARROW);
    studentClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    studentClass.lpszClassName = L"LostFoundStudentWindow";
    RegisterClassW(&studentClass);

    WNDCLASSW adminClass;
    ZeroMemory(&adminClass, sizeof(adminClass));
    adminClass.lpfnWndProc = AdminWndProc;
    adminClass.hInstance = hInst;
    adminClass.hCursor = LoadCursor(0, IDC_ARROW);
    adminClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    adminClass.lpszClassName = L"LostFoundAdminWindow";
    RegisterClassW(&adminClass);

    std::wstring authTitle = utf8ToWide("校园失物招领系统 - 登录注册");
    g_authWnd = CreateWindowW(L"LostFoundAuthWindow", authTitle.c_str(),
                              WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                              CW_USEDEFAULT, CW_USEDEFAULT, 545, 410, 0, 0, hInst, 0);
    ShowWindow(g_authWnd, nCmdShow);
    UpdateWindow(g_authWnd);

    MSG msg;
    while (GetMessageW(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
