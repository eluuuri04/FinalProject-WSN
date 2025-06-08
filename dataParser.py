import csv
import re
from datetime import datetime

# Fitxers d'entrada i sortida
input_file = 'ProjectData.txt'
output_file = 'ProjectDataCalc.csv'

# Capsalera del CSV
header = ['Hora', 'Humitat aire (%)', 'Temperatura aire (°C)', 'Humitat sorra (%)', 'Temperatura sorra (°C)']

# Llegeix el fitxer de logs i processa cada bloc de dades
with open(input_file, 'r', encoding='utf-8') as f:
    content = f.read()

# Divideix els blocs de logs
logs = content.strip().split('-----------------------------')

# Llista per guardar les files processades
rows = []

for log in logs:
    lines = log.strip().split('\n')
    if len(lines) < 5:
        continue  # ignora blocs buits o incomplets

    # Extreu les dades amb expressions regulars
    timestamp = re.search(r'Temps: (.+)', lines[0]).group(1)
    time_only = datetime.fromisoformat(timestamp.replace('Z', '')).strftime('%H:%M:%S')
    humitat_aire = float(re.search(r'([\d.]+)', lines[1]).group(1))
    temperatura_aire = float(re.search(r'([\d.]+)', lines[2]).group(1))
    humitat_sorra = float(re.search(r'([\d.]+)', lines[3]).group(1))
    temperatura_sorra = float(re.search(r'([\d.]+)', lines[4]).group(1))

    rows.append([time_only, humitat_aire, temperatura_aire, humitat_sorra, temperatura_sorra])

# Escriu al fitxer CSV
with open(output_file, 'w', newline='', encoding='utf-8') as f:
    writer = csv.writer(f)
    writer.writerow(header)
    writer.writerows(rows)

print(f'Conversió completada. Fitxer guardat com: {output_file}')
