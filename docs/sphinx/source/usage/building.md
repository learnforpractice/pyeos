# Building PyEos

## Downloading Source Code

```bash
git clone https://www.github.com/learnforpractice/pyeos
cd pyeos
git submodule update --init --recursive
```

## Installing dependencies (Ubuntu)

```
sudo apt-get install libleveldb-dev
sudo apt-get install libreadline-dev
```

## Installing dependencies (macOS)

```
brew install leveldb
brew install readline
```

## Building

```bash
./eosio_build.sh
```
