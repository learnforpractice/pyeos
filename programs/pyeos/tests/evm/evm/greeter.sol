pragma solidity ^0.4.8;
contract Greeter {
    mapping(address => uint) public mymap;
    
    event log(string s);

    function Greeter() {
        mymap[msg.sender] = 100;
        address evm = 0x56e4000000000000;
        
        if (msg.sender == evm) {
           mymap[msg.sender] = 1;
        } else {
           mymap[msg.sender] = 0;
        }
        mymap[msg.sender] = evm.balance;
        
        log("+++++++++Greeter\n");
    }

    function getValue() public returns (uint) {
        log("+++++++++getValue\n");
        return mymap[msg.sender];
    }

    function setValue(uint v) public {
        log("+++++++++setValue\n");
        mymap[msg.sender] = v;
    }
}
