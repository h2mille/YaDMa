import sqlite3
conn = sqlite3.connect('user_database.db')

c = conn.cursor()

c.execute('''CREATE TABLE users
	 (name text,card INTEGER,start_time TEXT, stop_time TEXT,permission INTEGER)''')
c.execute('''CREATE TABLE history
	 (date text, card INTEGER, name text, status INTEGER)''')

conn.commit()

conn.close()
