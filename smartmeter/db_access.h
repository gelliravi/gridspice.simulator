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

using namespace std;

class db_access 
{
public:

    db_access(string cust_id);
    int get_earliest_date(DATETIME &dt);
    int get_latest_date(DATETIME &dt); 
    double get_power_usage(DATETIME &dt);

    static void init_connection(const char *db = 0, 
                                const char *server = 0,
                                const char *user = 0,
                                const char *password = 0,
                                unsigned int port = 0);

private:
    string customer_id;
};

#endif
