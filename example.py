from dictionaries import SSDict


d = SSDict()

# d.__setitem__(2, 2)

d[1] = 2
d[3] = 4

print("\nSet item successfully!\n")

# print(f"\nd[2] = {d[2]}\n")

print(f"dictionary length: {d.__len__()}")

print("\n\n--------------------------------------")
print("Program ran successfully!")
print("--------------------------------------\n\n")
