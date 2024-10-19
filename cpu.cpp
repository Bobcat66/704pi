#include <iostream>
#include <cstdint>
#include <fstream>
#include <cmath>
#include "utils.h"

using std::cin;
using std::cout;

struct registers
{
    //Arithmetic registers
    uint38 ac; //38-bit accumulator
    uint36 mq; //36-bit multiplier/quotient register
    uint36 sr; //36-bit storage register

    //Program control registers
    uint15 indx[3]; //An array containing the three index registers
    uint15 ilc; //15-bit Instruction Location Register. This tracks the location of the current instruction in memory, and is updated after every instruction
    uint18 ir; //18-bit Instruction Register
};

struct flagstruct
{
    //Overflow flags
    bool acoflag; // Accumulator overflow flag
    bool mqoflag; // MQ Register overflow flag
    //Check flags
    bool dcflag; // Divide check flag
    bool rwcflag; // Read/Write check flag
    bool tcflag; // Tape check flag
    //Misc flags
    bool rwsflag; // Read/Write select flag
    bool senseflags[4]; // Sense flags
    bool trflag; // Trapping mode flag
    //Control flags
    bool haltFlag; // Halting flag
    bool autoFlag; // Automatic/Manual mode flag
    //System flags (These are for interacting with the system itself, and do not correspond to anything in the original IBM 704)
    bool verbose; // Whether or not the VM should record logs
};

class centralProcessingUnit 
{
    public:
    centralProcessingUnit(bool verbose){
        flags.verbose = verbose;
    }

    /*Tests fixed-point arithmetic*/
    void FiPA_Test1(){
        //Testing integer addition
        core[0] = 5;
        core[1] = 4;
        CLA(0);
        ADD(1);
        cout << "TEST0: 5 + 4 = " << rgstrs.ac << std::endl;
        clearCore();
        clearARgstrs();
        //Testing integer subtraction
        core[0] = 5;
        core[1] = 4;
        CLA(0);
        SUB(1);
        cout << "TEST1: 5 - 4 = " << rgstrs.ac << std::endl;
        clearCore();
        clearARgstrs();
        //Testing integer multiplication
        core[0] = 5;
        core[1] = 4;
        LDQ(0);
        MPY(1);
        cout << "TEST2: 5 * 4 = " << rgstrs.mq << std::endl;
        //Testing integer division
        clearCore();
        clearARgstrs();
        core[0] = 113;
        core[1] = 25;
        LDQ(0);
        DVH(1);
        cout << "TEST3: 113/25 Q = " << rgstrs.mq << " R = " << rgstrs.ac << std::endl;
    }

    /*Tests negative numbers*/
    void NegTest1(){
        clearCore();
        clearRgstrs();
        core[0] = 50;
        core[1] = int36_unsign(-20);
        CLA(0);
        ADD(1);
        cout << "TEST0: 50+(-20)=" << rgstrs.ac << std::endl;
    }

    private:
    const std::string drumName = "drum.bin"; //Name of the drum storage file
    registers rgstrs = {
        .ac = 0,
        .mq = 0,
        .sr = 0,
        .indx = {0,0,0},
        .ilc = 0,
        .ir = 0
    };
    flagstruct flags = {
        .acoflag = false,
        .mqoflag = false,

        .dcflag = false,
        .rwcflag = false,
        .tcflag = false,

        .rwsflag = false,
        .senseflags = {false,false,false,false},
        .trflag = false,

        .haltFlag = false,
        .autoFlag = false
    };
    std::string tapeMounts[10] = {
        "tape/tape0.bin",
        "tape/tape1.bin",
        "tape/tape2.bin",
        "tape/tape3.bin",
        "tape/tape4.bin",
        "tape/tape5.bin",
        "tape/tape6.bin",
        "tape/tape7.bin",
        "tape/tape8.bin",
        "tape/tape9.bin",
    };
    uint36 core[8192]; // Magnetic core storage, contains 8,192 36-bit registers
    std::fstream drum; // Drum storage stream
    std::fstream tape; // Tape stream

    /*Clears arithmetic registers*/
    void clearARgstrs(){
        rgstrs.ac = 
        rgstrs.mq = 
        rgstrs.sr = 
        0;
        if(flags.verbose){
            cout << "CPU_704: Cleared arithmetic registers\n";
        }
    }

    /*Clears program registers*/
    void clearPRgstrs(){
        rgstrs.ilc = 
        rgstrs.ir = 
        rgstrs.sr = 
        rgstrs.indx[0] =
        rgstrs.indx[1] =
        rgstrs.indx[2] =
        0;
        if(flags.verbose){
            cout << "CPU_704: Cleared program registers\n";
        }
    }

    /*Clears all registers*/
    void clearRgstrs(){
        rgstrs.ac = 
        rgstrs.mq = 
        rgstrs.sr = 
        rgstrs.ilc = 
        rgstrs.ir = 
        rgstrs.indx[0] = 
        rgstrs.indx[1] =
        rgstrs.indx[2] =
        0;
        if(flags.verbose){
            cout << "CPU_704: Cleared all registers\n";
        }
    }

    /*Clears core*/
    void clearCore(){
        std::fill(core,core + 8192,0);
        if(flags.verbose){
            cout << "CPU_704: Cleared core\n";
        }
    }

    //Control
    void runInstruction(uint36 instruction){
        uint8_t prefix = (instruction >> 33); // prefix is the first three bits of the instruction.
        if(prefix % 8) {
            // Type A instruction
        } else {
            // Type B instruction
        }
    }

    /*
    Arithmetic & Load/Store instructions
    Shorthand cheat sheet:
    y = the address
    ac = the accumulator
    mq = the MQ register
    sr = the storage register

    cy = value of memory register at address
    cac = the value of the accumulator
    cmq = the value of the MQ register
    csr = the value of the Storage register

    return codes:
    0 = Successful Execution
    1 = Halt
    */

    /*Stores cy in ac. Octal: +0500 Y*/
    void CLA(uint15 y) {
        uint36 cy = core[y];
        uint38 res = (getKthBit(cy,35) << 37) + clearKthBit(cy,35);
        rgstrs.ac = res;
    }

    /*Stores cy in mq. Octal: +0560*/
    void LDQ(uint15 y) {
        uint36 cy = core[y];
        rgstrs.mq = cy;
    }

    /*Stores cac at y. Octal: +0601*/
    void STO(uint15 y) {
        core[y] = ((getKthBit(rgstrs.ac,37) << 37) + rgstrs.ac % 35);
    }

    /*Stores cmq at y. Octal: -0600*/
    void STQ(uint15 y){
        core[y] = rgstrs.mq;
    }

    /*Adds cy to cac, stores the sum in ac. Octal: +0400 Y*/
    void ADD(uint15 y) {
        uint36 cy = core[y];
        if (getKthBit(cy,35)) {
            rgstrs.ac -= clearKthBit(cy,35);
        } else {
            rgstrs.ac += cy;
        }
    }

    /*Adds absolute value of cy to cac, stores the sum in ac. Octal: +0401*/
    void ADM(uint15 y) {
        uint36 cy = core[y];
        rgstrs.ac += clearKthBit(cy,35);
    }

    /*Round. If the most significant bit of cmq is 1, increment cac by 1.
    If the most significant bit of cmq is zero, nothing happens. Octal: +0760...010*/
    void RND(){
        uint8_t cmqMSD = getKthBit(rgstrs.mq,34);
        if(cmqMSD){
            rgstrs.ac++;
        }
    }

    /*Stores the negative of cy in ac. Octal: +0502*/
    void CLS(uint15 y){
        uint36 cy = core[y];
        uint38 res = toggleKthBit((getKthBit(cy,35) << 37) + clearKthBit(cy,35),37);
        rgstrs.ac = res;
    }

    /*Subtracts cy from cac, stores difference in ac. Octal: +0402*/
    void SUB(uint15 y){
        uint36 cy = core[y];
        if (getKthBit(cy,35)){
            rgstrs.ac += clearKthBit(cy,35);
        } else {
            rgstrs.ac -= cy;
        }
    }

    /*Subtracts absolute value of cy from cac, stores difference in ac. Octal: -0400*/
    void SBM(uint15 y){
        uint36 cy = core[y];
        rgstrs.ac -= clearKthBit(cy,35);
    }

    /*Multiplies cy by cmq. the 35 most significant bits of the 70-bit product is stored in ac. 
    the 35 least significant bits are stored in mq. 
    The sign of the product is the sign of both mq and ac.
    The Q and P bits are cleared. Octal: +0200*/
    void MPY(uint15 y){
        uint36 cy = core[y];
        bool neg = (getKthBit(cy,35) != getKthBit(rgstrs.mq,35)); //Whether or not the product is negative
        uint36 resMS35 = ((clearKthBit(cy,35) * clearKthBit(rgstrs.mq,35))/lshft35);//Most significant 35 bits
        uint36 resLS35 = (clearKthBit(cy,35) * clearKthBit(rgstrs.mq,35))%lshft35; //Least significant 35 bits
        if(neg){
            resMS35 += lshft35;
            resLS35 += lshft35;
        }
        rgstrs.ac = ((getKthBit(resMS35,35) << 37) + clearKthBit(resMS35,35));
        rgstrs.mq = resLS35;
    }

    /*Executes MPY Y followed by RND Y Octal: -0200*/
    void MPR(uint15 y){
        MPY(y);
        RND();
    }

    /*Uses cmq and cac as a 72-bit dividend with sign, treats cy as the divisor. 
    If cy > cac, division takes place, and the 35-bit signed quotient is stored in mq,
    while the 35-bit signed remainder is stored in ac. The sign of the remainder always
    agrees with the sign of the quotient. If cy <= cac, division does not take place and
    the computer halts and sets the divide-check flag to true*/
    void DVH(uint15 y){
        uint36 cy = core[y];
        if (clearKthBit(cy,35) <= clearKthBit(rgstrs.ac,37)){
            flags.dcflag = true;
            flags.haltFlag = true;
            return;
        }
        bool neg = (getKthBit(rgstrs.ac,37) != getKthBit(cy,35)); //Is result negive
        __uint128_t dividend = (clearKthBit(rgstrs.ac,37) | lshft35) - lshft35 + clearKthBit(rgstrs.mq,35);
        int64_t divisor = clearKthBit(cy,35);
        uint36 q = (uint36)(dividend/divisor) + (neg ? lshft35 : 0); //Quotient
        uint36 r = (uint36)(dividend % divisor) + (neg ? lshft37 : 0); //Remainder
        rgstrs.ac = r;
        rgstrs.mq = q;
    }

    /*Appends cmq to cac to form a 72-bit dividend with sign, treats cy as the divisor. 
    If cy > cac, division takes place, and the 35-bit signed quotient is stored in mq,
    while the 35-bit signed remainder is stored in ac. The sign of the remainder always
    agrees with the sign of the quotient. If cy <= cac, division does not take place and
    the computer proceeds to the next instruction with the divide-check flag to true*/
    void DVP(uint15 y){
        uint36 cy = core[y];
        if (clearKthBit(cy,35) <= clearKthBit(rgstrs.ac,37)){
            flags.dcflag = true;
            flags.haltFlag = false;
            return;
        }
        bool neg = (getKthBit(rgstrs.ac,37) != getKthBit(cy,35)); //Is result negive
        __uint128_t dividend = (clearKthBit(rgstrs.ac,37) | lshft35) - lshft35 + clearKthBit(rgstrs.mq,35);
        int64_t divisor = clearKthBit(cy,35);
        uint36 q = (uint36)(dividend/divisor) + (neg ? lshft35 : 0); //Quotient
        uint36 r = (uint36)(dividend % divisor) + (neg ? lshft37 : 0); //Remainder
        rgstrs.ac = r;
        rgstrs.mq = q;
    }

};

int main(){
    centralProcessingUnit cpu = centralProcessingUnit(true);
    cpu.FiPA_Test1();
    cpu.NegTest1();
}
