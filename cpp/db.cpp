#include "db.h"

using namespace std;

void DB::init()
{
    std::string path = AP::instance().exe_path() + "/py.db";
    // SQLITE_CONFIG_SINGLETHREAD, SQLITE_CONFIG_MULTITHREAD, SQLITE_CONFIG_SERIALIZED 
    // int rc = sqlite3_config( SQLITE_CONFIG_SINGLETHREAD );
    // if(SQLITE_OK == rc)
    // {
    //     LOGI("sqlite3_config successfully"); 
    // }
    // else 
    // {
    //     LOGI("sqlite3_config failed"); 
    // }
    int rc = sqlite3_open(path.c_str(), &db_);
    if (rc)
    {
        LOGE("Opened sqlite database failed: %s", sqlite3_errmsg(db_)); 
        is_opened_ = false;
    }
    else
    {
        LOGI("Opened sqlite database successfully"); 
        is_opened_ = true;
        populate_data();
    }
}
DB::~DB()
{
    if(is_opened_)
    {
        sqlite3_close(db_);
    }
}
std::string DB::exec_sql(const std::string& sql)
{
    Json j;
    j["result"] = Json::array();
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        j["ret"] = -1;
        j["msg"] = sqlite3_errmsg(db_);
        return j.dump();
    }
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        Json row;
        int num_cols = sqlite3_column_count(stmt);
        for (int i = 0; i < num_cols; ++i)
        {           
            switch (sqlite3_column_type(stmt, i))
            {
            case (SQLITE3_TEXT):
                row[sqlite3_column_name(stmt, i)] = (char*)sqlite3_column_text(stmt, i);
                break;
            case (SQLITE_INTEGER):
                row[sqlite3_column_name(stmt, i)] = sqlite3_column_int(stmt, i);
                break;
            case (SQLITE_FLOAT):
                row[sqlite3_column_name(stmt, i)] = sqlite3_column_double(stmt, i);
                break;
            default:
                break;
            }       
        }
        if(!row.empty())
        {
            j["result"].push_back(row);
        }
    }
    if (rc != SQLITE_DONE) {
        j["ret"] = -1;      
        j["msg"] = sqlite3_errmsg(db_);       
    }
    else 
    {
        j["ret"] = 0;
    }
    sqlite3_finalize(stmt);
    if ( !boost::ifind_first(sql, "select ") )   
    {
        j.erase("result");
    }
    // if(j["result"].empty()) j.erase("result");
    return j.dump();
}
int DB::usr_count()
{
    string sql = R"(
        select count(*) from user;
    )";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return 0;
    }
    sqlite3_step(stmt);
    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return count;
}
void DB::populate_data()
{
    string sql = R"(
        create table if not exists user (
            _id integer primary key autoincrement not null,
            admin text,
            client text,
            usr text,
            pass text
        );
    )";
    exec_sql(sql);
    int count = usr_count();
    LOGD("usr_count=%d", count);
    if(count == 0)
    {
        sql = R"(
            insert into user (admin,client,usr,pass) values ('root','guest','mystore','letmein');
        )";
        exec_sql(sql);
        LOGD("begin insert default pass -----------------------------------");
    }
}

std::tuple<std::string, std::string> DB::get_pass(){
    string sql = R"(
        select admin,client from user where _id=1;
    )";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        LOGE("get pass failed? This will never happen");
        return {"", ""};
    }
    sqlite3_step(stmt);
    string pass0 = (char*)sqlite3_column_text(stmt, 0);
    string pass1 = (char*)sqlite3_column_text(stmt, 1);
    sqlite3_finalize(stmt);
    return {pass0, pass1};
}
std::tuple<std::string, std::string> DB::get_acc(){
    string sql = R"(
        select usr,pass from user where _id=1;
    )";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        LOGE("get acc failed? This will never happen");
        return {"", ""};
    }
    sqlite3_step(stmt);
    string usr = (char*)sqlite3_column_text(stmt, 0);
    string pass = (char*)sqlite3_column_text(stmt, 1);
    sqlite3_finalize(stmt);
    return {usr, pass};
}