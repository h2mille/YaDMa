import sqlite3
conn = sqlite3.connect('MY_DB.db')

c = conn.cursor()

c.execute('''CREATE TABLE users
	 (name text,card INTEGER,start_time TEXT, stop_time TEXT,permission INTEGER)''')

conn.commit()

conn.close()
