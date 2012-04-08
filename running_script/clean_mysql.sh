#! /bin/bash

musr="root"
mpass="smartgrid"
mdb="mysql_wholesale"

TABLES=$(mysql -u $musr -p$mpass $mdb -e 'show tables')

for t in $TABLES
do
	echo "Truncate $t table from $mdb database ..."
	mysql -u $musr -p$mpass $mdb -e "truncate $t"
done
