#ifndef MODELS_H
#define MODELS_H

#include <string>

enum UserRole {
    ROLE_STUDENT = 0,
    ROLE_ADMIN = 1
};

enum UserStatus {
    USER_DISABLED = 0,
    USER_ENABLED = 1
};

enum ItemType {
    ITEM_LOST = 0,
    ITEM_FOUND = 1
};

enum ItemStatus {
    ITEM_OPEN = 0,
    ITEM_PROCESSING = 1,
    ITEM_FINISHED = 2
};

enum ClaimStatus {
    CLAIM_PENDING = 0,
    CLAIM_APPROVED = 1,
    CLAIM_REJECTED = 2
};

enum CategoryStatus {
    CATEGORY_DISABLED = 0,
    CATEGORY_ENABLED = 1
};

struct User {
    int id;
    std::string username;
    std::string password;
    std::string realName;
    int role;
    std::string phone;
    int status;

    User() : id(0), role(ROLE_STUDENT), status(USER_ENABLED) {}
};

struct Item {
    int id;
    std::string title;
    int type;
    int categoryId;
    int publisherId;
    std::string location;
    std::string description;
    int status;

    Item()
        : id(0),
          type(ITEM_LOST),
          categoryId(0),
          publisherId(0),
          status(ITEM_OPEN) {}
};

struct Claim {
    int id;
    int itemId;
    int applicantId;
    std::string reason;
    int status;
    std::string auditTime;
    std::string auditRemark;

    Claim()
        : id(0),
          itemId(0),
          applicantId(0),
          status(CLAIM_PENDING),
          auditTime("-"),
          auditRemark("-") {}
};

struct Category {
    int id;
    std::string name;
    int sortOrder;
    int enabled;

    Category() : id(0), sortOrder(0), enabled(CATEGORY_ENABLED) {}
};

#endif
