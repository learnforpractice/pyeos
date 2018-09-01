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
        db_update_i64(itr, self.pack())
      else
        db_store_i64(self.scope, self.table_id, self.get_primary_key(), self.pack())
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
  return string.pack('nn', self._amount, self._symbol)
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
      return string.pack('nn', self.amount, self.symbol)
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
    end
    
    function Balance.sub(self, amount)
    end
    
    function Balance.get_primary_key(self)
        return self.symbol_name
    end
    
    function Balance.pack(self)
      return string.pack('nn', self.a.amount, self.a.symbol)
    end
    return Balance
end, {multi_index})

function Balance.unpack(self, data)
  owner, symbol = string.unpack('nn', data)
  return Balance(owner,symbol)
end


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
        s1 = self._supply
        s2 = self._max_supply
        return string.pack('nc8nc8n', s1._amount, s1._symbol, s2._amount, s2._symbol, self._issuer)
    end
    
    function currency_stats.unpack(self, data)
        s1 = self.supply
        s2 = self.max_supply
        s1._amount, s1._symbol, s2._amount, s2._symbol, self._issuer = string.unpack('nc8nc8n', data)
    end

    return currency_stats
end, {multi_index})

function _create(msg)
    require_auth(_code)
    issuer, amount, symbol = string.unpack('JJJ', msg)
    print(issuer, amount, symbol)
    
    cs = currency_stats(symbol)
   
    if cs.issuer then
        error('currency has already been created')
    end

    cs._issuer = issuer
    cs._max_supply.amount = amount
    cs:store(_code)
end


function _issue(msg)
end

function _transfer(msg)
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
