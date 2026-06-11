# 成员 A 基础数据管理模块设计

## 目标与范围

成员 A 负责搭建可独立编译、运行和测试的 C 语言控制台程序框架，并实现：

- 统一的物品记录数据模型
- 失物与拾物单链表的新增、删除、修改、按编号查询和遍历
- 按关键词、地点、类别进行基础线性查询
- CSV 文件读取与保存
- 新增和删除操作的撤销栈
- 可调用成员 B 模块的基础主菜单

本阶段不实现成员 B 负责的智能匹配、匹配结果排序、哈希索引和地点统计，只为这些模块保留稳定的公共数据接口。

## 工具链

- 编译器：`D:\mingw\mingw64\bin\gcc.exe`
- 构建工具：`D:\mingw\mingw64\bin\mingw32-make.exe`
- 语言标准：C11
- 第三方库：无
- 禁止使用 SageMath 目录中的 C 编译器或工具

## 工程结构

```text
include/
  item.h          公共数据类型、字段长度和枚举
  item_list.h     单链表操作接口
  undo_stack.h    撤销栈接口
  storage.h       CSV 文件读写接口
  input.h         安全控制台输入接口
src/
  item.c
  item_list.c
  undo_stack.c
  storage.c
  input.c
  main.c
tests/
  test_item_list.c
  test_undo_stack.c
  test_storage.c
data/
  items.csv
Makefile
```

每个模块只负责一种能力，成员 B 可以只包含公共头文件，不需要依赖菜单实现。

## 公共数据模型

`Item` 使用固定长度字符数组保存文本字段，避免把内存所有权暴露给成员 B。时间拆分为年、月、日、时、分；类型与状态使用枚举。

```c
typedef enum {
    ITEM_LOST,
    ITEM_FOUND
} ItemType;

typedef enum {
    STATUS_UNPROCESSED,
    STATUS_MATCHED,
    STATUS_COMPLETED
} ItemStatus;

typedef struct Item {
    int id;
    ItemType type;
    char name[64];
    char category[64];
    char color[32];
    char location[128];
    char keywords[256];
    char contact[64];
    int year;
    int month;
    int day;
    int hour;
    int minute;
    ItemStatus status;
    struct Item *next;
} Item;

typedef struct {
    Item *head;
    size_t size;
} ItemList;
```

失物和拾物分别存放在两个 `ItemList` 中。插入时拒绝重复编号，并要求记录类型与目标链表一致。

## 模块设计

### 链表与查询

链表模块负责节点复制、尾部插入、按编号删除、按编号修改、查询和释放。删除接口可以返回被删除记录的副本，供撤销栈保存。

按关键词、地点和类别的基础查询采用线性遍历，并通过回调访问匹配记录。成员 B 后续可在不修改链表模块的情况下增加哈希索引。

### 撤销栈

撤销栈使用单链表实现，只记录新增和删除操作：

- 撤销新增：从对应列表删除该编号
- 撤销删除：将备份记录重新插入对应列表

成功修改数据后才压栈。撤销过程中产生的反向操作不再次压栈。

### CSV 文件读写

文件格式遵循开发方案中的字段顺序：

```text
id,type,name,category,color,location,keywords,contact,year,month,day,hour,minute,status
```

加载文件时先解析到临时列表，整行有效且编号不重复才插入；格式错误的行会被统计并跳过，不导致程序崩溃。保存时完整重写文件，并写入表头。

首版 CSV 约定文本字段本身不包含英文逗号，避免引入超出课程要求的引号转义解析器。

### 菜单与成员 B 接口

主菜单完成成员 A 的录入、显示、修改、删除、查询、加载、保存和撤销流程。成员 B 功能在菜单中保留清晰入口；在其模块接入前，入口显示“功能尚未接入”，程序仍能完整编译运行。

成员 B 通过 `const ItemList *lost_items` 和 `const ItemList *found_items` 读取数据。需要改变记录状态时，通过成员 A 暴露的按编号修改接口完成，避免直接重连链表节点。

## 数据流

1. 程序初始化失物链表、拾物链表和撤销栈。
2. 可选择从 `data/items.csv` 加载数据。
3. 菜单操作调用链表、存储和撤销模块。
4. 成员 B 模块从两条链表读取当前数据并生成自己的结果。
5. 退出时可保存两条链表到 CSV，随后释放全部节点。

## 错误处理

- 所有公共操作返回明确的状态码，不使用异常或直接退出进程。
- 输入模块使用 `fgets` 读取整行，再进行整数或文本校验。
- 文本超长、非法枚举、非法日期、重复编号、文件无法打开和内存分配失败均返回错误。
- 菜单遇到错误时显示提示并返回主菜单。

## 测试策略

- 链表测试：空列表、插入、重复编号、查询、修改、头/中/尾删除、释放。
- 撤销测试：撤销新增、撤销删除、空栈、连续操作的后进先出顺序。
- 文件测试：正常往返、空文件、无效行、重复编号、文件不存在。
- 构建测试：使用 MinGW GCC 严格警告编译，运行全部测试，并构建主程序。
- 菜单在完成模块测试后进行脚本化输入冒烟测试。

## Git 与协作

- 开发分支：`feature/member-a-core`
- 基本框架可编译并通过首批测试后推送并创建 Draft PR。
- 后续成员 A 功能以小步提交继续推送到该 PR。
- 不直接修改或合并成员 B 的 `youzi` 分支，不直接向 `main` 推送。
- 暂不编写作业报告，等待成员 B 开发完成后再统一整理。
