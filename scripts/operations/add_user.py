#!/usr/bin/python3

from argon2 import PasswordHasher
from argon2.exceptions import VerifyMismatchError
import argparse
import getpass
import sqlite3
import os
import sys

if __name__ != "__main__":
    print("Please run script as docker compose run -it core /app/scripts/operations/add_user.py")
    sys.exit(0)

parser = argparse.ArgumentParser(description="Create new user script")

parser.add_argument("--username", required=True, help="Enter a username")
parser.add_argument("--password", help="Enter a password (optional) (if not specified will prompt)")
parser.add_argument("--user-db", required=False, help="Location of sqlite3 user db location. Defaults to USER_DATABASE env var.")

args = parser.parse_args()

username = args.username

if args.password:
    password = args.password
else:
    password = getpass.getpass(prompt="Enter a password: ")

user_db_file = args.user_db or os.environ.get('USER_DATABASE')
if not user_db_file:
    raise Exception("put USER_DATABASE back into .env or specify on command line")

try:
    con = sqlite3.connect(user_db_file)
    cursor = con.cursor()

    ph = PasswordHasher()
    hash = ph.hash(password)

    insert_sql = '''
    INSERT INTO users (username, password)
    VALUES (?, ?);
    '''

    cursor.execute(insert_sql, (username, hash))

    con.commit()
    con.close()

    print(f"User '{username}' created ✅ Begin trading!")
except sqlite3.Error as e:
    print(f"Error inserting user: {e} ❌")
