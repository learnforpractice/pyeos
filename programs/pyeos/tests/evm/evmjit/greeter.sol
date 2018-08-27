pragma solidity ^0.4.8;
contract Greeter {
    uint myaddress;
    function Greeter() {
//        uint[] memory a = new uint[](11*1024*1024);
      myaddress = 111;
    }

    function getValue() public returns (uint) {
         myaddress = 112233;
         myaddress *= 1000;
         myaddress += 1000;
         myaddress -= 1000;
        return myaddress;
    }

    function setValue(uint v) public {
        myaddress = v;
    }
}
