#!/bin/bash

# Make a backup of the Graco pump control web site database, then sync it to the dev server

DB_USER="gracoweb"
DB_PASS="lKJSDfkljhkwjQ\$2"
DB_NAME="gracopump"
DB_BACK_DIR="/home/graco/backups"
HN=`hostname | awk -F. '{print $1}'`
DATE=`date +%Y-%m-%d`
SCP_HN="graco-dev.graniteriver.com"
SCP_USER="graco"

# Make the backup directory if it does not yet exist
mkdir -p $DB_BACK_DIR

BACKUP_FILE_NAME="mysqldump-$HN-$DB_NAME-$DATE.sql.gz"

# Not bothering with incremental backups at this point
/usr/bin/mysqldump --user=$DB_USER --password=$DB_PASS --opt $DB_NAME | /bin/gzip > "$DB_BACK_DIR/$BACKUP_FILE_NAME"

# Backup to the dev server
if [ -f "$DB_BACK_DIR/$BACKUP_FILE_NAME" ]
then
    /usr/bin/scp -q "$DB_BACK_DIR/$BACKUP_FILE_NAME" $SCP_USER@$SCP_HN:/home/$SCP_USER/backups/from_$HN/
fi


