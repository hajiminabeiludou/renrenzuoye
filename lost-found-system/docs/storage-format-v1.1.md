# v1.1 数据文件格式说明

本系统 v1.1 使用文本文件保存数据，文件统一放在 `data` 目录下。

## 通用规则

1. 每一行表示一条记录。
2. 字段之间使用英文竖线 `|` 分隔。
3. 保存数据时，如果用户输入内容中包含 `|`，程序会自动替换为 `/`，避免破坏字段结构。
4. 状态字段统一保存为数字，数字含义以 `models.h` 中的枚举定义为准。

## 文件结构

`users.dat`

```text
id|username|password|realName|role|phone|status
```

`categories.dat`

```text
id|name|sortOrder|enabled
```

`items.dat`

```text
id|title|type|categoryId|publisherId|location|description|status
```

`claims.dat`

```text
id|itemId|applicantId|reason|status|auditTime|auditRemark
```
