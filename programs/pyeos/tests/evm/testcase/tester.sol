pragma solidity ^0.4.8;
contract Tester {
    uint myvalue;
    address owner;
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

    function testSuicide() public {
        suicide(owner);
    }
}
