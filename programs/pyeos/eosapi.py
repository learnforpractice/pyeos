import eostypes_
'''
    struct AccountPermission { 
        AccountPermission() = default;
        AccountPermission(const AccountName& account, const PermissionName& permission)
           : account(account), permission(permission) {}

        AccountName                      account;
        PermissionName                   permission;
    };
'''



class AccountPermission(object):
    def __init__(self,account,permission):
        self.account = account
        self.permission = permission

'''
    struct Message { 
        Message() = default;
        Message(const AccountName& code, const FuncName& type, const Vector<AccountPermission>& authorization, const Bytes& data)
           : code(code), type(type), authorization(authorization), data(data) {}

        AccountName                      code;
        FuncName                         type;
        Vector<AccountPermission>        authorization;
        Bytes                            data;
    };
'''

class Message(object):
    def __init__(self,code,type,authorization,data):
        self.code = code
        self.type = type
        self.authorization = authorization
        self.data = data

'''
    struct Transaction { 
        Transaction() = default;
        Transaction(const UInt16& refBlockNum, const UInt32& refBlockPrefix, const Time& expiration, 
            const Vector<AccountName>& scope, 
            const Vector<AccountName>& readscope, const Vector<Message>& messages)
           : refBlockNum(refBlockNum), refBlockPrefix(refBlockPrefix), expiration(expiration), scope(scope), readscope(readscope), messages(messages) {}

        UInt16                           refBlockNum;
        UInt32                           refBlockPrefix;
        Time                             expiration;
        Vector<AccountName>              scope;
        Vector<AccountName>              readscope;
        Vector<Message>                  messages;
    };
'''

class Transaction:
    def __init__(self,refBlockNum,refBlockPrefix,expiration,scope,readscope,messages):
        self.refBlockNum = refBlockNum
        self.refBlockPrefix = refBlockPrefix
        self.expiration = expiration
        self.scope = scope
        self.readscope = readscope
        self.messages = messages



