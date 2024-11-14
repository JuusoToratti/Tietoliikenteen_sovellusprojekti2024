import requests
import csv
from dotenv import load_dotenv
import os

# Lataa .env-tiedoston ympäristömuuttujat
load_dotenv()

# Hae SERVER_IP-ympäristömuuttuja
server_ip = os.getenv('SERVER_IP')

# Varmista, että IP-osoite on asetettu
if not server_ip:
    print("SERVER_IP-osoite ei ole määritetty .env-tiedostossa.")
    exit(1)

# url
url = f"http://{server_ip}/luedataa_kannasta_groupid_csv.php?groupid=16"

# HTTP GET -pyynnön lähettäminen palvelimelle
try:
    response = requests.get(url)
    
    # Tarkistetaan, että pyyntö onnistui
    if response.status_code == 200:
        print("Data haettu onnistuneesti.")

        # Oletetaan, että palvelin palauttaa CSV-muotoista dataa
        data = response.text
        
        # Tallennetaan data CSV-tiedostoon
        with open('data.csv', 'w', newline='') as csvfile:
            csv_writer = csv.writer(csvfile)
            
            # Kirjoitetaan CSV-tiedostoon rivit
            # Oletetaan, että data on pilkottu rivikohtaisesti
            rows = data.splitlines()  # Pilkotaan rivit
            for row in rows:
                csv_writer.writerow(row.split(','))  # Pilkotaan sarakkeet pilkulla ja kirjoitetaan
        print("Data tallennettu CSV-tiedostoon.")
    else:
        print(f"Pyyntö epäonnistui. Statuskoodi: {response.status_code}")
except Exception as e:
    print(f"Virhe HTTP-pyynnössä: {e}")
