'''
  "structs": [{
      "name": "OrderID",
      "fields": {
        "name" : "AccountName",
        "id"   : "UInt64"
      }
    },{
      "name" : "Bid",
      "fields" : {
         "buyer" : "OrderID",
         "price" : "UInt128",
         "quantity" : "UInt64",
         "expiration" : "Time"
       }
    },{
      "name" : "Ask",
      "fields" : {
         "seller" : "OrderID",
         "price" : "UInt128",
         "quantity" : "UInt64",
         "expiration" : "Time"
       }
    },{
      "name" : "Account",
      "fields" : {
         "owner" : "AccountName",
         "eos_balance" : "UInt64",
         "currency_balance" : "UInt64",
         "open_orders" : "UInt32"
       }
    },{
      "name" : "BuyOrder",
      "base" : "Bid",
      "fields" : {
         "fill_or_kill" : "UInt8"
       }
    },{
      "name" : "SellOrder",
      "base" : "Ask",
      "fields" : {
         "fill_or_kill" : "UInt8"
       }
    }
  ],
'''
objDict={}
class AbiStruct(object):
    def __init__(self, js):
        pass
    def __str__(self):
        return str(self.__dict__)
    def __repr__(self):
        return str(self.__dict__)
    

def parse(bs,js,obj):
    pass
