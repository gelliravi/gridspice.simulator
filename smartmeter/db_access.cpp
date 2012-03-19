#include "db_access.h"

#include <mysql++.h>
#include <iostream>

using namespace std;

/* static variables */
static mysqlpp::Connection conn(true);

/* static helper functions */
static void store_in_datetime(mysqlpp::DateTime date, DATETIME &dt);

bool db_access::is_connected()
{
    return conn.connected();
}

void db_access::init_connection(const char *db, const char *server,
                                const char *user, const char *password,
                                unsigned int port) 
{
    conn.connect(db, server, user, password, port);
}
                     

db_access::db_access(string cust_id) 
{
    customer_id = cust_id;

    // verify the customer_id
    mysqlpp::Query query = conn.query();
    query << "select CUSTID from masterload_all where CUSTID = "
          << mysqlpp::quote << customer_id << " limit 1";
    mysqlpp::StoreQueryResult res = query.store();
    if (res.num_rows() == 0) {
        throw runtime_error("invalid customer id " + customer_id);
    }

    // TODO init weather_id
    mysqlpp::Query query2 = conn.query();
    query2 << "select WEATHRID from master_tb5_all where "
           << " CUSTID = " << mysqlpp::quote << customer_id;
    mysqlpp::StoreQueryResult res2 = query2.store();
    if (res2.num_rows() > 0) {
        weather_id = (string) res2[0]["WEATHRID"];
    } else {
        weather_id = "";
    }
}

bool db_access::get_earliest_date(DATETIME &dt) 
{
    mysqlpp::Query query = conn.query();
    query << "select min(DATE_) from masterload_all where CUSTID = "
          << mysqlpp::quote << customer_id;
    mysqlpp::StoreQueryResult res = query.store();
    if (res.num_rows() > 0) {
         mysqlpp::DateTime earliest_date = res[0][0];
         store_in_datetime(earliest_date, dt);
         return true;
    }
    return false; 
}

bool db_access::get_latest_date(DATETIME &dt) 
{
    mysqlpp::Query query = conn.query();
    query << "select max(DATE_) from masterload_all where CUSTID = "
          << mysqlpp::quote << customer_id;
    mysqlpp::StoreQueryResult res = query.store();
    if (res.num_rows() > 0) {
         mysqlpp::DateTime earliest_date = res[0][0];
         store_in_datetime(earliest_date, dt);
         return true;
    }
    return false;
}

double db_access::get_power_usage(const DATETIME &dt) 
{
    string date_field = dt_to_string(dt);
    /*
    stringstream date_builder;
    date_builder << dt.year << "-" << dt.month << "-" << dt.day;
    string date_field = date_builder.str();
    */

    int interval_number = (dt.hour * 4) + (dt.minute / 15) + 1; // out of 96
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

double db_access::get_humidity(const DATETIME &dt)
{
    if (weather_id == "")
        return -1;

    string date_field = dt_to_string(dt);

    stringstream hum_builder;
    hum_builder << "RHUM" << (dt.hour + 1) << endl;
    string hum_field = hum_builder.str();

    mysqlpp::Query query = conn.query();
    query << "select " << hum_field << " from master_tb8_all "
          << " where DATE_ = " << mysqlpp::quote << date_field
          << " and WEATHRID = " << mysqlpp::quote << weather_id
          << " and " << hum_field << " is not null ";
    mysqlpp::StoreQueryResult res = query.store();
    if (res.num_rows() > 0) {
       return res[0][hum_field.c_str()]; 
    } else {
        return -1;
    }
}

double db_access::get_temp(const DATETIME &dt)
{
    if (weather_id == "")
        return -1;
    
    string date_field = dt_to_string(dt);

    stringstream temp_builder;
    temp_builder << "TMP" << (dt.hour + 1) << endl;
    string temp_field = temp_builder.str();

    mysqlpp::Query query = conn.query();
    query << "select " << temp_field << " from master_tb8_all "
          << " where DATE_ = " << mysqlpp::quote << date_field
          << " and WEATHRID = " << mysqlpp::quote << weather_id
          << " and " << temp_field << " is not null ";
    mysqlpp::StoreQueryResult res = query.store();
    if (res.num_rows() > 0) {
       return res[0][temp_field.c_str()]; 
    } else {
        return -1;
    }
}   

double db_access::is_dr(const DATETIME &dt)
{
    string date_field = dt_to_string(dt);

    mysqlpp::Query query = conn.query();
    query << "select CPVSTART, CPVSTOP from master_tb3_all "
          << " where date_ = " << mysqlpp::quote << date_field
          << " and custid = " << mysqlpp::quote << customer_id
          << " CPVSTART is not null " 
          << " and CPVSTOP is not null ";
    mysqlpp::StoreQueryResult res = query.store();
    
    for (int i = 0; i < res.num_rows(); i++) {
        int start_time = res[i]["CPVSTART"] / 100;    
        int end_time = res[i]["CPVSTOP"] / 100;
        if (start_time <= (dt.hour - 1) && (dt.hour - 1) <= end_time) {
            return true;
        }
    }
    return false;
}

string db_access::dt_to_string(const DATETIME &dt)
{
    stringstream date_builder;
    date_builder << dt.year << "-" << dt.month << "-" << dt.day;
    return date_builder.str();
}

void store_in_datetime(mysqlpp::DateTime date, DATETIME &dt) 
{
    dt.day = date.day(); 
    dt.month = date.month();
    dt.year = date.year();

    dt.hour = date.hour();    
    dt.minute = date.minute();
    dt.second = date.second();
}
