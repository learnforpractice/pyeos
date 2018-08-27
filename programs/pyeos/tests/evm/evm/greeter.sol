pragma solidity ^0.4.8;
contract Greeter {
    uint myaddress;
    function Greeter() {
    }

    function getValue() payable public returns (uint){
        msg.sender.transfer(1000);
        return 0;
    }

    function setValue(uint v) payable public {
        myaddress = v;
    }
}
