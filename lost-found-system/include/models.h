#ifndef MODELS_H
#define MODELS_H

#include <string>

struct User {
    int id;
    std::string username;
    std::string password;
    std::string realName;
    int role;   /* 0 student, 1 admin */
    std::string phone;
    int status; /* 1 enabled, 0 disabled */
};

struct Item {
    int id;
    std::string title;
    int type; /* 0 lost, 1 found */
    int categoryId;
    int publisherId;
    std::string location;
    std::string description;
    int status; /* 0 open, 1 processing, 2 finished */
};

struct Claim {
    int id;
    int itemId;
    int applicantId;
    std::string reason;
    int status; /* 0 pending, 1 approved, 2 rejected */
    std::string auditTime;
    std::string auditRemark;
};

struct Category {
    int id;
    std::string name;
    int sortOrder;
    int enabled;
};

#endif
