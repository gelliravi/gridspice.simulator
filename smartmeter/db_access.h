#include <iostream>
#include "timestamp.h"

//typedef struct DATETIME;
using namespace std;

bool db_is_valid_id(string customer_id);

int db_get_earliest_date(string customer_id, DATETIME &dt);

int db_get_latest_date(string customer_id, DATETIME &dt);

double db_get_power_usage(string customer_id, DATETIME &dt); 
