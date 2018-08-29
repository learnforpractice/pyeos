pragma solidity ^0.4.8;
contract Callee {
    function setValue(uint) public returns (uint) {}
}

contract Tester {
    uint myvalue;
    address owner;
    address calleeAddress;
    address calleeAddress2;
    function Greeter() public {
      owner = msg.sender;
    }

    function testTransfer() payable public returns (uint) {
        msg.sender.transfer(1); //transfer back 0.0001 EOS
        return 0;
    }
    
    function testMemory() public {
        uint256[] memory b = new uint256[](2*1024);
    }

    function testMemory2() public {
        uint256[] memory b = new uint256[](64*1024);
    }

    function testSetValue(uint v) public {
        myvalue = v;
    }

    function testGetValue() public returns (uint) {
        return myvalue;
    }

    function testCall(address a, uint v) public returns (uint) {
        calleeAddress = a;
        calleeAddress2 = a;
        return Callee(a).setValue(v);
    }

    function testSuicide() public {
       if (owner == msg.sender) { // We check who is calling
          selfdestruct(owner); //Destruct the contract
       }
    }
    
    function () payable public {//get called when transfer with no parameters
    }
}

