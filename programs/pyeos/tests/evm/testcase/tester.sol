pragma solidity ^0.4.8;
contract Tester {
    uint myaddress;
    function Greeter() public {
    }

    function testTransfer() payable public returns (uint) {
        msg.sender.transfer(1); //transfer back 0.0001 EOS
        return 0;
    }
    
    function testMemory() payable public {
        uint256[] memory b = new uint256[](2*1024);
    }

    function testMemory2() payable public {
        uint256[] memory b = new uint256[](64*1024);
    }

    function setValue(uint v) payable public {
        myaddress = v*100;
        myaddress = v*100;
        myaddress = v*100;
    }
}
