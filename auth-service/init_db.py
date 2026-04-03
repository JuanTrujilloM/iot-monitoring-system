import sqlite3

conn = sqlite3.connect('users.db')
c = conn.cursor()

c.execute('''CREATE TABLE IF NOT EXISTS users
             (username TEXT PRIMARY KEY, password TEXT, role TEXT)''')

c.execute("INSERT OR IGNORE INTO users VALUES ('admin', '123', 'ADMIN')")
c.execute("INSERT OR IGNORE INTO users VALUES ('operador1', 'abc', 'OPERATOR')")

conn.commit()
conn.close()