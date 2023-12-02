import requests
import json

url = 'https://apis.data.go.kr/1613000/ArvlInfoInqireService/getSttnAcctoArvlPrearngeInfoList?serviceKey=7fbmhI1JTcfdHRUVxPC1Q5dMRWaGT1iO9DR2RfJtxsghQ7Xc4tqtwAi3rBu5vIZmStXhsobOsYAC7m8qUCS3GA%3D%3D&pageNo=1&numOfRows=100&_type=json&cityCode=37010&nodeId=PHB351001140'
response = requests.get(url)
data = response.json()

bus_info = {}
for item in data['response']['body']['items']['item']:
    bus_info[item['routeno']] = round(item['arrtime']/60)

print(json.dumps(bus_info))
