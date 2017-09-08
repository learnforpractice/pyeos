cython --cplus eosapi.pyx -o eosapi_wrap.cpp
#cython --cplus eostypes_.pyx -o eostypes_wrap.cpp
cython --cplus wallet.pyx -o wallet_wrap.cpp
cython --cplus eostypes_.pyx -o eostypes_wrap.cpp
