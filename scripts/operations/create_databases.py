#!/usr/bin/python3

import sqlite3
import os
import sys
import argparse

if __name__ != "__main__":
    print("Please run script as docker compose run -it core /app/scripts/operations/create_databases.py")
    sys.exit(0)

parser = argparse.ArgumentParser(description="Create databases")

parser.add_argument("--user-db", required=False, help="Location of sqlite3 db location")

args = parser.parse_args()

user_db_file = args.user_db or os.environ.get('USER_DATABASE')

if not user_db_file:
    raise Exception("put USER_DATABASE back into .env or specify on command line")

con = sqlite3.connect(user_db_file)
cursor = con.cursor()

create_table_sql = '''
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL,
    password TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    active INTEGER DEFAULT 1
);
'''

cursor.execute(create_table_sql)

con.commit()
con.close()

print("Created users table ✅")
