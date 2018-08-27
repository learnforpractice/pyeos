function apply(receiver, account, act)
    code = receiver
    scope = receiver
    table = receiver
    payer = receiver
    id = receiver
--    print(id)

    if act == N('sayhello') then
        msg = read_action_data()
--        print(msg)
        itr = db_find_i64(code, scope, table, id)
--        print("itr:", itr)
      
        if itr >= 0 then
            data = db_get_i64(itr)
--            print(data)
        else
            db_store_i64(scope, table, payer, id, msg)
        end

--        while true do
--        end
    end
    

     return 1
end
