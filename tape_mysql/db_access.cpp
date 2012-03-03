#include "db_access.h"

#include <mysql++.h>

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
                    
db_access::db_access( std::string Objname,  std::vector<std::string> myProperties, bool readOrWrite ){
  this->tableName = Objname;
  this->columnNames = myProperties;
  
  if( !readOrWrite ){
    this->create_table();
  }
}


int db_access::write_properties( std::vector<std::string> values ){
  assert( values.size() == this->columnNames.size() );
  std::stringstream query;
  query << "INSERT INTO "<<this->tableName<<" SET ";
  for(int i=0; i<this->columnNames.size() ; i++){
    query << this->columnNames[i] << "='"<<values[i]<<query<< "'";
    if( i<(this->columnNames.size()-1) ){
      query << ",";
    }

  }
  std::cout<< query.str();

  mysqlpp::Query mysqlQuery =  conn.query();
  mysqlQuery << query.str();
  mysqlpp::StoreQueryResult res = mysqlQuery.store();

  return 0;
}

int db_access::create_table(){
  mysqlpp::Query dropQuery = conn.query();
  dropQuery << "DROP TABLE "<<this->tableName;
  mysqlpp::StoreQueryResult dropRes = dropQuery.store();

  std::stringstream query;
  query << "CREATE TABLE "<<this->tableName<<" (";
  for(int i=0; i<this->columnNames.size() ; i++){
    query<<this->columnNames[i]<<" varchar(50)";
    if( i<(this->columnNames.size()-1) ){
      query << ",";
    }

  }
  query << ")";
  std::cout << query.str() << std::endl;
  mysqlpp::Query mysqlQuery =  conn.query();
  mysqlQuery << query.str();
  mysqlpp::StoreQueryResult res = mysqlQuery.store();
  return 0;
}

int db_access::get_earliest_date(DATETIME &dt) 
{
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

int db_access::get_latest_date(DATETIME &dt) 
{
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

double db_access::get_power_usage(DATETIME &dt) 
{
  std::stringstream date_builder;
  date_builder << dt.year << "-" << dt.month << "-" << dt.day;
  std::string date_field = date_builder.str();

  int interval_number = (dt.hour * 4) + (dt.minute / 15) + 1;
  std::stringstream interval_builder;
  interval_builder << "QKW" << interval_number;
  std::string interval_field = interval_builder.str();

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

void store_in_datetime(mysqlpp::DateTime date, DATETIME &dt) 
{
  dt.day = date.day(); 
  dt.month = date.month();
  dt.year = date.year();

  dt.hour = date.hour();    
  dt.minute = date.minute();
  dt.second = date.second();
}
