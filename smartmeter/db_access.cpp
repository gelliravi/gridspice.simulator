#include "db_access.h"

#include <mysql++.h>

/* static variables */
static mysqlpp::Connection conn(true);

/* static helper functions */
static void store_in_datetime(mysqlpp::DateTime date, DATETIME &dt);

void db_access::init_connection(const char *db, const char *server,
                                const char *user, const char *password,
                                unsigned int port) {
    conn.connect(db, server, user, password, port);
}
                     

db_access::db_access(string cust_id) {
    customer_id = cust_id;
    mysqlpp::Query query = conn.query();
    query << "select CUSTID from masterload_all where CUSTID = "
          << mysqlpp::quote << customer_id << " limit 1";
    mysqlpp::StoreQueryResult res = query.store();
    if (res.num_rows() == 0) {
        throw runtime_error("invalid customer id " + customer_id);
    }
}

int db_access::get_earliest_date(DATETIME &dt) {
    mysqlpp::Query query = conn.query();
    query << "select min(DATE_) from masterload_all where CUSTID = "
          << mysqlpp::quote << customer_id;
    mysqlpp::StoreQueryResult res = query.store();
    if (res.num_rows() > 0) {
         mysqlpp::DateTime earliest_date = res[0][0];
         store_in_datetime(earliest_date, dt);
         return 1;
    }
    return 0;
}

int db_access::get_latest_date(DATETIME &dt) {
    mysqlpp::Query query = conn.query();
    query << "select max(DATE_) from masterload_all where CUSTID = "
          << mysqlpp::quote << customer_id;
    mysqlpp::StoreQueryResult res = query.store();
    if (res.num_rows() > 0) {
         mysqlpp::DateTime earliest_date = res[0][0];
         store_in_datetime(earliest_date, dt);
         return 1;
    }
    return 0;
}

double db_access::get_power_usage(DATETIME &dt) {
    stringstream date_builder;
    date_builder << dt.year << "-" << dt.month << "-" << dt.day;
    string date_field = date_builder.str();

    int interval_number = (dt.hour * 4) + (dt.minute / 15) + 1;
    stringstream interval_builder;
    interval_builder << "QKW" << interval_number;
    string interval_field = interval_builder.str();

    mysqlpp::Query query = conn.query();
    query << "select " << interval_field << " from masterload_all " 
          << " where DATE_ = " << mysqlpp::quote << date_field
          << " and CUSTID = " << mysqlpp::quote << customer_id
          << " and " << interval_field << " is not null ";
    mysqlpp::StoreQueryResult res = query.store();
    if (res.num_rows() > 0) {
        // guaranteed to not be null because of query
        return res[0][interval_field.c_str()];
    } else {
        return 0;
    }
}

void store_in_datetime(mysqlpp::DateTime date, DATETIME &dt) {
    dt.day = date.day(); 
    dt.month = date.month();
    dt.year = date.year();

    dt.hour = date.hour();    
    dt.minute = date.minute();
    dt.second = date.second();
}
