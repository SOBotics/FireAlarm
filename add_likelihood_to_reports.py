import json

with open("reports.json") as file_handle:
    data = json.load(file_handle)

for item in data:
    print("Updating item with id: " + str(item["id"]))
    try:
        item["l"] = item["d"]
    except KeyError as e:
        print("KeyError occurred: ")
        print(str(e))
        print("Writing likelihood as -1")
        item["l"] = -1

with open("reports.json", "w") as file_handle:
    json.dump(data, file_handle)
