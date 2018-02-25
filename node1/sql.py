import mysql.connector

cnx = mysql.connector.connect(user='dg', password='q',
                              host='192.168.0.17',
                              database='proto0')
#cnx.close()

cursor = cnx.cursor()

query = ("SELECT account_id, balance from accounts ")

#hire_start = datetime.date(1999, 1, 1)
#hire_end = datetime.date(1999, 12, 31)

cursor.execute(query) #, (hire_start, hire_end))

for (account_id, balance) in cursor:
      print("{},{} ".format(
              account_id, balance))

cursor.close()
cnx.close()
