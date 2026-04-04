import sqlite3

conn = sqlite3.connect('users.db')
c = conn.cursor()

c.execute('''CREATE TABLE IF NOT EXISTS users
             (username TEXT PRIMARY KEY, password TEXT, role TEXT)''')

c.execute("INSERT OR IGNORE INTO users VALUES ('admin',     'Admin@2024!',  'ADMIN')")
c.execute("INSERT OR IGNORE INTO users VALUES ('jtrujillo', 'Telematica#1', 'OPERATOR')")
c.execute("INSERT OR IGNORE INTO users VALUES ('mgarcia',   'Garcia$2024',  'OPERATOR')")
c.execute("INSERT OR IGNORE INTO users VALUES ('clopez',    'Lopez@IoT1',   'OPERATOR')")
c.execute("INSERT OR IGNORE INTO users VALUES ('amorales',  'Morales#456',  'OPERATOR')")
c.execute("INSERT OR IGNORE INTO users VALUES ('dherrera',  'Herrera$789',  'OPERATOR')")
c.execute("INSERT OR IGNORE INTO users VALUES ('lcastro',   'Castro@2024',  'OPERATOR')")
c.execute("INSERT OR IGNORE INTO users VALUES ('fmendoza',  'Mendoza#IoT',  'OPERATOR')")
c.execute("INSERT OR IGNORE INTO users VALUES ('pjimenez',  'Jimenez$321',  'OPERATOR')")
c.execute("INSERT OR IGNORE INTO users VALUES ('rvargas',   'Vargas@654',   'OPERATOR')")
c.execute("INSERT OR IGNORE INTO users VALUES ('nrojas',    'Rojas#2024!',  'OPERATOR')")

conn.commit()
conn.close()
