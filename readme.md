# BLE Tester

Usage instructions:

```
sudo apt update
sudo apt upgrade
sudo apt install cmake build-essential ninja-build libglib-2.0-dev libbluetooth-dev

mkdir -p build && cd build

cmake .. -GNinja

ninja

./ble_test <mac_address>
```
