require 'string'
require 'math'

-- Lua classes
function class(class_init, bases)
    bases = bases or {}
    local c = {}
    for _, base in ipairs(bases) do
        for k, v in pairs(base) do
            c[k] = v
        end
    end
    c._bases = bases
    c = class_init(c)
    local mt = getmetatable(c) or {}
    mt.__call = function(_, ...)
        local object = {}
        setmetatable(object, {
            __index = function(tbl, idx)
                local method = c[idx]
                if type(method) == "function" then
                    return function(...)
                        return c[idx](object, ...) 
                    end
                end
                return method
            end,
        })
        if type(object.__init__) == "function" then
            object.__init__(...)
        end
        return object
    end
    setmetatable(c, mt)
    return c
end

_code = N('tokentest')

local multi_index = class(function(multi_index)
    function multi_index.__init__(self, code, scope, table_id)
        self.code = code
        self.scope = scope
        self.table_id = table_id
    end
    function multi_index.store(self, payer)
      itr = db_find_i64(self.code, self.scope, self.table_id, self.get_primary_key())
      if itr >= 0 then
        db_update_i64(itr, payer, self.pack())
      else
        print('+++db_store_i64:', self.scope, self.table_id, payer)
        db_store_i64(self.scope, self.table_id, payer, self.get_primary_key(), self.pack())
      end
      return 0;
    end

    function multi_index.load(self)
      itr = db_find_i64(self.code, self.scope, self.table_id, self.get_primary_key())
      if itr < 0 then
        return nil
      end
      data = db_get_i64(itr)
      return self.unpack(data)
    end
    
    function multi_index.erase(self)
        itr = db_find_i64(self.code, self.scope, self.table_id, self.get_primary_key())
        if it >= 0 then
            db_remove_i64(it)
        end
    end

    function multi_index.get_primary_key(self)
      error('get_primary_key not implemented by subclass')
      return 0;
    end
    
    function multi_index.pack(self)
      error('pack not implemented by subclass')
      return 0;
    end

    function multi_index.unapck(data)
      error('unapck not implemented by subclass')
      return 0;
    end

    return multi_index
end, {})


local B = class(function(B)
    function B.__init__(self, a, b, c)
        A.__init__(self, a, b)
        self.c = c
    end
    function B.callme(self)
        print("callme B")
    end
    return B
end, {A})

function to_symbol_name(s)
    n = string.unpack('n', s)
    ret = rshift(n, 8)
    print('+++:', n, ret)
    return ret
end

Asset = {}
Asset.__index = Asset
--constructor
function Asset:new(amount, symbol)
  local instance = {
    _amount = amount,
    _symbol = symbol
  }
  setmetatable(instance, Asset)
  return instance
end

function Asset:pack()
  return string.pack('JJ', self._amount, self._symbol)
end

function Asset:unpack(data)
  self._amount, self._symbol = string.unpack(data)
end


local Asset = class(function(A)
    function Asset.__init__(self, amount, symbol)
        self.amount = amount
        self.symbol = symbol
    end

    function Asset.pack(self)
      return string.pack('JJ', self.amount, self.symbol)
    end
    return Asset
end, {})

function Asset.unpack(self, data)
    amount, symbol = string.unpack(data)
    return Asset(amount, symbol)
end

local Balance = class(function(Balance)
    function Balance.__init__(self, owner, symbol)
        self.owner = owner
        self.a = Asset(0, symbol)
        self.symbol_name = to_symbol_name(symbol)

        table_id = N('accounts')
        multi_index.__init__(self, _code, self.owner, table_id)
        multi_index.load(self)
    end
    
    function Balance.add(self, amount, payer)
        self.a.amount = self.a.amount + amount
        self.store(payer)
    end
    
    function Balance.sub(self, amount)
        print('+++self.a.amount:', self.a.amount)
        assert(self.a.amount >= amount, 'balance not enough')
        self.a.amount = self.a.amount + amount
        if self.a.amount == 0 then
            self.erase()
        else
            self.store(self.owner)
        end
    end
    
    function Balance.get_primary_key(self)
        return self.symbol_name
    end
    
    function Balance.pack(self)
        return string.pack('JJ', self.a.amount, self.a.symbol)
    end

    function Balance.unpack(self, data)
        self.a.amount, self.a.symbol = string.unpack('JJ', data)
    end
    return Balance
end, {multi_index})

local currency_stats = class(function(currency_stats)
    function currency_stats.__init__(self, symbol)
        table_id = N('stat')
        self.primary_key = to_symbol_name(symbol)
        self.scope = self.primary_key
        print(self.primary_key, self.scope)

        multi_index.__init__(self, _code, self.scope, table_id)

        self.supply = Asset(0, symbol)
        self.max_supply =Asset(0, symbol)
        self.issuer = 0
        self.load()
    end

    function currency_stats.get_primary_key(self)
        return self.primary_key
    end

    function currency_stats.pack(self)
        s1 = self.supply
        s2 = self.max_supply
        return string.pack('JJJJJ', s1.amount, s1.symbol, s2.amount, s2.symbol, self.issuer)
    end
    
    function currency_stats.unpack(self, data)
        s1 = self.supply
        s2 = self.max_supply
        s1.amount, s1.symbol, s2.amount, s2.symbol, self.issuer = string.unpack('JJJJJ', data)
    end

    return currency_stats
end, {multi_index})


function _create(msg)
    require_auth(_code)
    issuer, amount, symbol = string.unpack('JJJ', msg)
    
    cs = currency_stats(symbol)
    if cs.issuer ~= 0 then
        error('currency has already been created')
    end

    cs.issuer = issuer
    cs.max_supply.amount = amount
    cs.store(_code)
end


function _issue(msg)
    _to, amount, symbol = string.unpack('JJJ', msg:sub(1,24))
--        memo = eoslib.unpack_bytes(msg[24:])
    cs = currency_stats(symbol)
    assert(cs.issuer, 'currency does not exists')
    print('----cs.issuer:', cs.issuer)
    require_auth(cs.issuer)
    cs.supply.amount = cs.supply.amount + amount
    assert(cs.supply.amount < cs.max_supply.amount, 'supply overflow!')
    cs.store(cs.issuer)

    acc = Balance(_to, symbol)
    acc.add(amount, cs.issuer)
end

function _transfer(msg)
    _from, to, amount, symbol = string.unpack('JJJJ', msg:sub(1,32))
--    print('transfer:', _from, to, amount, symbol)
    require_auth(_from)
    require_recipient(_from)
    require_recipient(to)
--        memo = eoslib.unpack_bytes(msg[32:])
    a1 = Balance(_from, symbol)
    a2 = Balance(to, symbol)
    a1.sub(amount)
    a2.add(amount, _from)
end

function apply(receiver, account, act)
  msg = read_action_data()
  if act == N('create') then
    _create(msg)
  elseif act == N('issue') then
    _issue(msg)
  elseif act == N('transfer') then
    _transfer(msg)
  end
  return 1
end
