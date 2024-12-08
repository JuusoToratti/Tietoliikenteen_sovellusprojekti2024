import mysql.connector
import csv
import os
from dotenv import load_dotenv

# Ladataan ympäristömuuttujat .env-tiedostosta
load_dotenv()

# Haetaan MySQL-tietokannan yhteyden tiedot ympäristömuuttujista
DB_HOST = os.getenv('DB_HOST')
DB_USER = os.getenv('DB_USER')
DB_PASSWORD = os.getenv('DB_PASSWORD')
DB_NAME = os.getenv('DB_NAME')

print(f"DB_HOST: {DB_HOST}")
print(f"DB_USER: {DB_USER}")
print(f"DB_PASSWORD: {DB_PASSWORD}")
print(f"DB_NAME: {DB_NAME}")


# Yhdistetään MySQL-tietokantaan
try:
    connection = mysql.connector.connect(
        host=DB_HOST,
        user=DB_USER,
        password=DB_PASSWORD,
        database=DB_NAME
    )
    print("Yhteys MySQL-tietokantaan onnistui.")
    
    # Luo kursori ja suorita SQL-kysely
    cursor = connection.cursor()
    cursor.execute("SELECT * FROM rawdata")  # Muokkaa tämä kysely haluamasi datan mukaan
    
    # Hae kaikki rivit tuloksista
    rows = cursor.fetchall()

    # Tallennetaan tulokset CSV-tiedostoon
    with open('data_from_mysql.csv', mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow([i[0] for i in cursor.description])  # Kirjoittaa sarakeotsikot
        writer.writerows(rows)

    print("Data tallennettu CSV-tiedostoon.")

except mysql.connector.Error as err:
    print(f"Virhe: {err}")
finally:
    # Suljetaan yhteys ja kursori
    if connection.is_connected():
        cursor.close()
        connection.close()
        print("MySQL-yhteys suljettu.")
