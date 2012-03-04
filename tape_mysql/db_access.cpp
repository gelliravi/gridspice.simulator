#include <vector>
#include "db_access.h"

#include <mysql++.h>

/* static variables */
static mysqlpp::Connection conn(true);

/* static helper functions */


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

std::vector<std::string> *db_access::read_properties( std::string ts ){


  std::stringstream query;

  query << "SELECT ";
  for(int i=0; i<this->columnNames.size() ; i++){

    query << this->columnNames[i];
    if( i<(this->columnNames.size()-1) ){
      query << ",";
    }

  }
  query <<" FROM "<<this->tableName;
  query <<" WHERE time>"<< ts;
  std::cout<< query.str();

  mysqlpp::Query mysqlQuery =  conn.query();
  mysqlQuery << query.str();
  mysqlpp::StoreQueryResult res = mysqlQuery.store();
  std::vector<std::string> *values = new std::vector<std::string>();
  return values;
}


int db_access::write_properties( std::string t0, std::string t1, std::vector<std::string> values ){
  assert( values.size() == this->columnNames.size() );
  std::stringstream query;

  query << "INSERT INTO "<<this->tableName<<" SET t0='"<<ts<<"', t1='"<<t1<<"', ";
  for(int i=0; i<this->columnNames.size() ; i++){

    query << this->columnNames[i] << "='"<<values[i]<< "'";
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
  query << "CREATE TABLE "<<this->tableName<<" (time timestamp, ";
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
