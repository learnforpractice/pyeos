from eostypes_ import *
from zmq.utils.constant_names import ctx_opt_names

cdef Message to_message(msg):
    cdef Message msg_ 
    cdef Vector[AccountPermission] permissions
    msg_.code = Name(msg.code)
    msg_.type = Name(msg.type)

    if msg.authorization:
        for a in msg.authroization:
            permissions.push_back(AccountPermission(Name(<char*>a[0]),Name(<char*>a[1])))
        msg_.authorization = permissions
    if msg.data:
        msg_.data = msg.data
    return msg_
'''
        AccountName                      code;
        FuncName                         type;
        Vector[AccountPermission]        authorization;
        Bytes                            data;
'''

'''
        UInt16                           refBlockNum;
        UInt32                           refBlockPrefix;
        Time                             expiration;
        Vector<AccountName>              scope;
        Vector<AccountName>              readscope;
        Vector<Message>                  messages;

        Vector<Signature>                signatures;
'''
cdef SignedTransaction to_signed_transaction(tx):
    cdef SignedTransaction tx_
    cdef Signature sss
    
    tx_.refBlockNum = tx.refBlockNum
    tx_.refBlockPrefix = tx.refBlockPrefix 
    tx_.expiration = Time(tx.expiration)
    if tx.scope:
        for s in tx.scope:
            tx_.scope.push_back(Name(<char*>s))
    if tx.messages:
        for msg in tx.messages:
            tx_.messages.push_back(to_message(msg))
    if tx.signatures:
        for s in tx.signatures:
            sss = Signature()
            memcpy(sss.data,<char*>s,sizeof(sss.data))
            tx_.signatures.push_back(sss)
    return tx_



