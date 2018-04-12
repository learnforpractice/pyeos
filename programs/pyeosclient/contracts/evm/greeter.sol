pragma solidity ^0.4.0;
contract Greeter {
    mapping(address => uint) public mymap;

    function Greeter() {
        mymap[msg.sender] = 100;
    }

    function getValue() public returns (uint) {
        return mymap[msg.sender];
    }

    function setValue(uint v) public {
        mymap[msg.sender] = v;
    }
}
