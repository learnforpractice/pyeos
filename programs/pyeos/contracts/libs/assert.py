from eoslib import eosio_assert, N, S

   static constexpr uint32_t symbol_name_length( symbol_name tmp ) {
      tmp >>= 8; /// skip precision
      uint32_t length = 0;
      while( tmp & 0xff && length <= 7) {
         ++length;
         tmp >>= 8;
      }

      return length;
   }

class symbol_type(object):
    def __init__(self, _symbol):
        self.symbol = _symbol

    def is_valid() -> bool:
        #FIXME
#        return is_valid_symbol( value )
        return True

    def precision(self):
        return self.symbol[0]

    def name(self):
        return bytes(self.symbol[1:], 'utf8')
    #FIXME
#    uint32_t name_length()const { return symbol_name_length( value ); }

class asset(object):
    max_amount    = (1 << 62) - 1;
    def __init__(self, a = 0, s = 'EOS'):
        self.amount = a
        self.symbol = S(s)
        eosio_assert(self.is_amount_within_range(), "magnitude of asset amount must be less than 2^62");
        #FIXME
#        eosio_assert( symbol.is_valid(),        "invalid symbol name" );

    def is_amount_within_range(self):
        return -asset.max_amount <= self.amount and self.amount <= asset.max_amount

    def is_valid(self) -> bool:
        #FIXME: 
        #return is_amount_within_range() && symbol.is_valid(); }
        return self.is_amount_within_range()

    def set_amount(self, a: int):
        self.amount = a
        eosio_assert(self.is_amount_within_range(), "magnitude of asset amount must be less than 2^62")

    def __add__(self, other):
        self.amount += other.amount
        return self.__class__(self.amount, self.symbol)

    def __sub__(self, other):
        self.amount -= other.amount
        return self.__class__(self.amount, self.symbol)

    def __mul__(self, other):
        _amount = self.amount
        if isinstance(other, int):
            _amount *= other
        else:
            _amount *= self._amount
        return self.__class__(_amount, self.symbol)

    def __div__(self, other):
        _amount = self.amount
        if isinstance(other, int):
            _amount /= other
        else:
            _amount /= self._amount
        return self.__class__(_amount, self.symbol)

    def __truediv__(self, other):
        _amount = self.amount
        if isinstance(other, int):
            _amount /= other
        else:
            _amount /= self._amount
        return self.__class__(_amount, self.symbol)

    def __eq__(self, other):
        if isinstance(other, int):
            return self.amount == other
        else:
            return self.amount == other.amount

    def __gt__(self,other):
        if isinstance(other, int):
            return self.amount > other
        else:
            return self.amount > other.amount

    def __ge__(self,other):
        if isinstance(other, int):
            return self.amount >= other
        else:
            return self.amount >= other.amount

    def __lt__(self,other):
        if isinstance(other, int):
            return self.amount < other
        else:
            return self.amount < other.amount

    def __le__(self,other):
        if isinstance(other, int):
            return self.amount <= other
        else:
            return self.amount <= other.amount
