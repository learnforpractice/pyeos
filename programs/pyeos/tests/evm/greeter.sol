pragma solidity ^0.4.8;
contract Greeter {
    mapping(address => uint) public mymap;
    
    event log(string s);

    function Greeter() {
        mymap[msg.sender] = 100;
        mymap[msg.sender] = 101111;
        log("+++++++++Greeter\n");
    }

    function getValue() public returns (uint) {
        log("+++++++++getValue\n");
        return mymap[msg.sender];
    }

    function setValue(uint v) public {
        log("+++++++++setValue\n");
        mymap[msg.sender] = v;
        mymap[msg.sender] = v+99;
    }
}
