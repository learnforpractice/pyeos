pragma solidity ^0.4.8;
contract Callee {
    uint myvalue;
    function Callee() public {
    }
    
    function setValue(uint v) public returns(uint) {
        myvalue = v;
        return v;
    }
}
