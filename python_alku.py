from bleak import BleakClient
import mysql.connector
from mysql.connector import Error
import time

# MySQL-tietokantayhteyden tiedot
MYSQL_HOST = "172.20.241.9"
MYSQL_USER = "dbaccess_rw"
MYSQL_PASSWORD = "fasdjkf2389vw2c3k234vk2f3"
MYSQL_DATABASE = "rawdata"  # Tietokannan nimi

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
            INSERT INTO rawdata (sensorvalue_a, sensorvalue_b, sensorvalue_c, sensorvalue_d, timestamp) 
            VALUES (%s, %s, %s, %s, NOW())
        """
        values = (x, y, z, direction)
        cursor.execute(query, values)
        connection.commit()
        print(f"Data lisätty tietokantaan: X={x}, Y={y}, Z={z}, Direction={direction}")
    except Error as e:
        print(f"Virhe datan tallentamisessa: {e}")

async def main():
    # Yhdistä MySQL-tietokantaan
    connection = connect_to_mysql()
    if connection is None:
        print("Tietokantayhteyden luominen epäonnistui. Ohjelma päättyy.")
        return

    try:
        # Yhdistä Bluetooth-laitteeseen
        async with BleakClient(DEVICE_MAC_ADDRESS) as client:
            print(f"Yhteys Bluetooth-laitteeseen {DEVICE_MAC_ADDRESS} muodostettu")

            while True:
                # Lue data Bluetooth-laitteen ominaisuudesta
                raw_data = await client.read_gatt_char(CHARACTERISTIC_UUID)
                # Oletetaan, että vastaanotettu data on merkkijono, jossa tiedot ovat "X,Y,Z,Direction"
                data_str = raw_data.decode('utf-8')  # Esim. "123,456,789,1"
                data_values = data_str.split(',')

                # Varmista, että data on oikeassa muodossa
                if len(data_values) == 4:
                    try:
                        # Muunna arvot kokonaisluvuiksi
                        x = int(data_values[0])
                        y = int(data_values[1])
                        z = int(data_values[2])
                        direction = int(data_values[3])

                        print(f"Vastaanotettu data: X={x}, Y={y}, Z={z}, Direction={direction}")

                        # Tallenna data MySQL-tietokantaan
                        insert_data_to_mysql(connection, x, y, z, direction)

                    except ValueError:
                        print("Virhe datan muunnossa. Varmista, että tiedot ovat kokonaislukuja.")
                else:
                    print("Virheellinen data vastaanotettu:", data_str)

                # Odota ennen seuraavaa lukemista
                time.sleep(1)

    except Exception as e:
        print(f"Virhe yhteydessä: {e}")

    finally:
        if connection.is_connected():
            connection.close()
            print("MySQL-yhteys suljettu")

if __name__ == "__main__":
    import asyncio
    asyncio.run(main())
