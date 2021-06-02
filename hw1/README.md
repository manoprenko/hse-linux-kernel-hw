# Kernel-mode phone book

## Build and load module

Requires sudo priveleges
```bash
./load.sh
```

## Usage

### Add new user
```bash
echo "add <name> <surname> <age> <phone number> <email>" > /dev/phonebook
```
Name, surname, phone number and email lengths should not exceed 50 characters.

### Delete user by phone number
```bash
echo "del <phone number>" > /dev/phonebook
```
EINVAL is returned if requested phone number is not present in phonebook

### Get all users with specified surname
```bash
echo "get <surname>" > /dev/phonebook && cat /dev/phonebook
```

## Usage example
```bash
echo "add Invan Petrov 30 88005553535 ivan.petrov@gmail.com" > /dev/phonebook
echo "add Petr Petrov 22 +79001234567 a@a.a" > /dev/phonebook
echo "add Mikhail Mikhailov 18 +1123456 mikhail@mikhail.ru" > /dev/phonebook

echo "get Petrov" > /dev/phonebook && cat /dev/phonebook  # prints two entries
echo "get Mikhailov" > /dev/phonebook && cat /dev/phonebook  # prints one entry

echo "del 88005553535" > /dev/phonebook
echo "get Petrov" > /dev/phonebook && cat /dev/phonebook  # prints one left entry
echo "del 000" > /dev/phonebook  # fails
echo "get Nonexisting" > /dev/phonebook && cat /dev/phonebook  # no entries
```
