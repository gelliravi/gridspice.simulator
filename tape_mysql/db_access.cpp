#include <vector>
#include <unistd.h>                                                                                                           
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


bool table_exists( std::string tableName ){
  mysqlpp::Query mysqlQuery =  conn.query();
  mysqlQuery << "show tables like \""<<tableName<<"\""; 
  mysqlpp::StoreQueryResult res = mysqlQuery.store();
  if( res.num_rows() > 0 )
    return true;
  else
    return false;
}
                    
db_access::db_access( std::string _tableName,  std::vector<std::string> myProperties, bool readOrWrite ){
  this->tableName = _tableName;
  this->columnNames = myProperties;
  
  if( !readOrWrite ){
    this->create_table(  );
  } else {
    if( !table_exists( this->tableName ) ){
      std::cout<<"TABLE DOES NOT EXIST"<<std::endl;
      throw "TABLE DOES NOT EXIST";
    }
  }
}




std::map<std::string, std::string> *db_access::read_properties( std::string ts ){
 
    
    std::stringstream query;
  
    query << "SELECT UNIX_TIMESTAMP(t1), UNIX_TIMESTAMP(t2), ";
    for(int i=0; i<this->columnNames.size() ; i++){
      
      query << this->columnNames[i];
      if( i<(this->columnNames.size()-1) ){
	query << ",";
      }
      
    }
    query <<" FROM "<<this->tableName;
    query <<" WHERE t2>=FROM_UNIXTIME("<< ts<<")";
    std::cout<<query<<std::endl;
    
    while(true){
      mysqlpp::Query mysqlQuery =  conn.query();
      mysqlQuery << query.str();
      mysqlpp::StoreQueryResult res = mysqlQuery.store();
      if (res.num_rows() >0) 
      {
	std::map<std::string, std::string> *values = new std::map<std::string, std::string>();
	mysqlpp::DateTime t1 = res[0][0];
	mysqlpp::DateTime t2 = res[0][1];
	std::stringstream ss;
	ss<<t1;
	
	std::stringstream ss2;
	ss2<<t2;
	
	values->insert ( std::pair<std::string, std::string>("t1",ss.str()) );
	values->insert ( std::pair<std::string, std::string>("t2",ss2.str()));
	for (int j=0; j<this->columnNames.size(); j++) {
	  mysqlpp::String value = res[0][j+2];
	  std::stringstream ss;
	  ss<<value;
	  values->insert ( std::pair<std::string, std::string>(columnNames[j].c_str(),ss.str()) );
	  
	}
	return values;
      }
      std::cout<<"Nothing in database... sleeping..."<<std::endl;
      usleep(4000000);
   }
}


int db_access::write_properties( std::string t0, std::string t1, std::vector<std::string> values ){
  assert( values.size() == this->columnNames.size() );
  std::stringstream query;

  query << "INSERT INTO "<<this->tableName<<" SET t1=FROM_UNIXTIME("<<t0<<"), t2=FROM_UNIXTIME("<<t1<<"), ";
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
  std::cout<<"COLUMNS: "<<this->columnNames.size()<<std::endl;
  try{
    std::stringstream dropQuery;
    dropQuery << "DROP TABLE IF EXISTS "<<this->tableName;
    std::cout<<dropQuery.str()<<std::endl;
    mysqlpp::Query dropQueryMysql = conn.query();
    dropQueryMysql<<dropQuery.str();
    mysqlpp::StoreQueryResult dropRes = dropQueryMysql.store();
  } catch (char *msg){
    std::cout<<msg<<std::endl;
  }
  std::stringstream query;
  query << "CREATE TABLE "<<this->tableName<<" (t1 timestamp, t2 timestamp, ";
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
