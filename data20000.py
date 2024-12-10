import socket

# Avaataan socket-yhteys palvelimelle
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('server_ip', 20000))
s.sendall(b'16\n')

chunks = []

# Vastaanotetaan data osissa
while True:
    data = s.recv(1024)
    if len(data) == 0:
        break
    chunks.append(data.decode('utf-8'))

# Yhdistetään osat yhdeksi kokonaisuudeksi
received_data = ''.join(chunks)

# Tulostetaan vastaanotettu data
print(received_data)

# Tallennetaan data tiedostoon
with open("vastaanotetut_datat.txt", "w") as file:
    file.write(received_data)

# Suljetaan yhteys
s.close()

