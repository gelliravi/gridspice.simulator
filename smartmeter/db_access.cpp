#include <mysql++.h>

#include "db_access.h"

/* mysql connection initialization */
static mysqlpp::Connection conn("lidb", 0, "lidb_user", "smartgrid!!", 0);

bool db_is_valid_id(string customer_id) {
    mysqlpp::Query query = conn.query();
    query << "select CUSTID from masterload_all where CUSTID = "
          << mysqlpp::quote << customer_id << " limit 1";
    mysqlpp::StoreQueryResult res = query.store();
    return (res.num_rows() > 0);
}

static void store_in_datetime(mysqlpp::DateTime date, DATETIME &dt) {
    dt.day = date.day(); 
    dt.month = date.month();
    dt.year = date.year();

    dt.hour = date.hour();    
    dt.minute = date.minute();
    dt.second = date.second();

    // zero out other fields
    dt.microsecond = 0;
}

int db_get_earliest_date(string customer_id, DATETIME &dt) {
    mysqlpp::Query query = conn.query();
    query << "select min(DATE_) from masterload_all where CUSTID = "
          << mysqlpp::quote << customer_id;
    mysqlpp::StoreQueryResult res = query.store();
    if (res.num_rows() > 0) {
         // TODO - add error checking below
         mysqlpp::DateTime earliest_date = res[0][0];
         store_in_datetime(earliest_date, dt);
         return 1;
    }
    return 0;
}

int db_get_latest_date(string customer_id, DATETIME &dt) {
    mysqlpp::Query query = conn.query();
    query << "select max(DATE_) from masterload_all where CUSTID = "
          << mysqlpp::quote << customer_id;
    mysqlpp::StoreQueryResult res = query.store();
    if (res.num_rows() > 0) {
         // TODO - add error checking below
         mysqlpp::DateTime earliest_date = res[0][0];
         store_in_datetime(earliest_date, dt);
         return 1;
    }
    return 0;
}

double db_get_power_usage(string customer_id, DATETIME &dt) {
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
