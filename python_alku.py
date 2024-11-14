from bleak import BleakClient
import mysql.connector
from mysql.connector import Error
import time
import asyncio
import json
import requests

# MySQL-tietokantayhteyden tiedot
MYSQL_HOST = "172.20.241.9" #avoin tietokanta
#MYSQL_HOST = "172.20.241.41"
MYSQL_USER = "dbaccess_rw"
MYSQL_PASSWORD = "fasdjkf2389vw2c3k234vk2f3" #avoin tietokanta
#MYSQL_PASSWORD = "jokusalasana123"
MYSQL_DATABASE = "measurements"  # Tietokannan nimi

# Bluetooth-yhteyden tiedot
DEVICE_MAC_ADDRESS = "CB:A1:21:38:E5:F2"  # Korvaa tämä Nordic-laitteesi MAC-osoitteella
CHARACTERISTIC_UUID = "00001526-1212-efde-1523-785feabcd123"  # Korvaa datalähetyksen UUID:llä

def connect_to_mysql():
    try:
        connection = mysql.connector.connect(
            host=MYSQL_HOST,
            user=MYSQL_USER,
            password=MYSQL_PASSWORD,
            database=MYSQL_DATABASE
        )
        if connection.is_connected():
            print("Yhdistetty MySQL-tietokantaan")
        return connection
    except Error as e:
        print(f"Virhe MySQL-yhteydessä: {e}")
        return None

def insert_data_to_mysql(connection, x, y, z, direction):
    try:
        cursor = connection.cursor()
        query = """
            INSERT INTO rawdata (sensorvalue_a, sensorvalue_b, sensorvalue_c, sensorvalue_d, timestamp, groupid) 
            VALUES (%s, %s, %s, %s, NOW(), %s)
        """
        groupid = 16  # Assigning the groupid value as 16
        values = (x, y, z, direction, groupid)
        cursor.execute(query, values)
        connection.commit()
        print(f"Data lisätty tietokantaan: X={x}, Y={y}, Z={z}, Direction={direction}, GroupID={groupid}")
    except Error as e:
        print(f"Virhe datan tallentamisessa: {e}")

# This is the callback function for receiving notifications
def notification_handler(sender: int, data: bytearray):
    try:
        data_str = data.decode('utf-8')  # Assuming data is a string like "X,Y,Z,Direction"
        data_values = data_str.split(',')

        if len(data_values) == 4:
            try:
                x = int(data_values[0])
                y = int(data_values[1])
                z = int(data_values[2])
                direction = int(data_values[3])

                print(f"Vastaanotettu data: X={x}, Y={y}, Z={z}, Direction={direction}")

                # Insert data into MySQL database
                connection = connect_to_mysql()
                if connection:
                    insert_data_to_mysql(connection, x, y, z, direction)
                    connection.close()

            except ValueError:
                print("Virhe datan muunnossa. Varmista, että tiedot ovat kokonaislukuja.")
        else:
            print(f"Virheellinen data vastaanotettu: {data_str}")
    except Exception as e:
        print(f"Virhe datan käsittelyssä: {e}")


async def main():
    # Yhdistä Bluetooth-laitteeseen
    async with BleakClient(DEVICE_MAC_ADDRESS) as client:
        print(f"Yhteys Bluetooth-laitteeseen {DEVICE_MAC_ADDRESS} muodostettu")

        # Subscribe to notifications
        await client.start_notify(CHARACTERISTIC_UUID, notification_handler)

        try:
            # Keep the script running to receive notifications
            while True:
                await asyncio.sleep(1)
        except KeyboardInterrupt:
            print("Ohjelma keskeytetty")

if __name__ == "__main__":
    import asyncio
    asyncio.run(main())
