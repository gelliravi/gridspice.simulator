/*
 * Creates a connection to a database of Customer Power Logs, based
 * on the customer's id. 
 *
 * You MUST call init_connection to initialize the connection to the db
 * before you create an instance of db_access
 */

#ifndef DB_ACCESS_H
#define DB_ACCESS_H

#include <iostream>
#include "timestamp.h"

class db_access 
{
public:

    db_access(std::string cust_id);
    bool get_earliest_date(DATETIME &dt);
    bool get_latest_date(DATETIME &dt); 
    double get_power_usage(const DATETIME &dt);
    double get_humidity(const DATETIME &dt);
    double get_temp(const DATETIME &dt);
    double is_dr(const DATETIME &dt);

    static bool is_connected();
    static void init_connection(const char *db = 0, 
                                const char *server = 0,
                                const char *user = 0,
                                const char *password = 0,
                                unsigned int port = 0);

private:
    std::string customer_id;
    std::string weather_id;

    /* static private helper functions */
    static std::string dt_to_string(const DATETIME &dt);
};

#endif
