#pragma once
#include "common.h"
#include <sqlite3.h>

ToSingle(DB)
private:
    sqlite3 *db_;
    bool is_opened_;
public:
    ~DB();
    std::string exec_sql(const std::string& sql);
    std::tuple<std::string, std::string> get_pass();
    std::tuple<std::string, std::string> get_acc();
private:
    void init();
    void populate_data();
    int usr_count();
};
