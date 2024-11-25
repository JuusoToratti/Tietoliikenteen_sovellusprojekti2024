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
DEVICE_MAC_ADDRESS = "CB:A1:21:38:E5:F2"  # Nordic-laitteen MAC-osoitte
CHARACTERISTIC_UUID = "00001526-1212-efde-1523-785feabcd123"  # datalähetyksen UUID (notify)

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
        groupid = 16  # Ryhmän numero
        values = (x, y, z, direction, groupid)
        cursor.execute(query, values)
        connection.commit()
        print(f"Data lisätty tietokantaan: X={x}, Y={y}, Z={z}, Direction={direction}, GroupID={groupid}")
    except Error as e:
        print(f"Virhe datan tallentamisessa: {e}")

# Callback functio tietojen tallentamiselle
def notification_handler(sender: int, data: bytearray):
    try:
        data_str = data.decode('utf-8')  # data on stringi "X,Y,Z,Direction"
        data_values = data_str.split(',')

        if len(data_values) == 4:
            try:
                x = int(data_values[0])
                y = int(data_values[1])
                z = int(data_values[2])
                direction = int(data_values[3])

                print(f"Vastaanotettu data: X={x}, Y={y}, Z={z}, Direction={direction}")

                # Kirjoitetaan data MySQL tietokantaan
                if direction != 0:
                 connection = connect_to_mysql()
                if connection:
                    insert_data_to_mysql(connection, x, y, z, direction)
                    connection.close()

                else:
                    print(f"Suunta-arvo on 0, ei tallenneta.")

            except ValueError:
                print("Virhe datan muunnossa. Varmista, että tiedot ovat kokonaislukuja.")
        else:
            print(f"Virheellinen data vastaanotettu: {data_str}")
    except Exception as e:
        print(f"Virhe datan käsittelyssä: {e}")


async def main():
    # Yhdistetään Bluetooth-laitteeseen
    async with BleakClient(DEVICE_MAC_ADDRESS) as client:
        print(f"Yhteys Bluetooth-laitteeseen {DEVICE_MAC_ADDRESS} muodostettu")

        # Subataan notifications
        await client.start_notify(CHARACTERISTIC_UUID, notification_handler)

        try:
            # Saadaan ilmoituksia (notify) kunnes ohjelma keskeytetään
            while True:
                await asyncio.sleep(1)
        except KeyboardInterrupt:
            print("Ohjelma keskeytetty")

if __name__ == "__main__":
    import asyncio
    asyncio.run(main())
