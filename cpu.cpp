#include <iostream>
#include <cstdint>
#include <fstream>
#include <cmath>
#include <bitset>
#include "utils.h"

using std::cin;
using std::cout;

using bits36 = std::bitset<36>;
using bits38 = std::bitset<38>;

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

    /*Tests bitwise operations*/
    void kbitTest(){
        //gKthBitTest
        uint36 testWord = 0b110101101101001001001011101011010110;
        cout << getKthBit(testWord,11) << getKthBit(testWord,10) << getKthBit(testWord,9) << getKthBit(testWord,8) << getKthBit(testWord,7) << getKthBit(testWord,6) << getKthBit(testWord,5) << getKthBit(testWord,4) << getKthBit(testWord,3) << getKthBit(testWord,2) << getKthBit(testWord,1) << getKthBit(testWord,0) << std::endl; //Should print "10"
        clearCore();
        clearRgstrs();
        core[0] = (uint36)0b111111111111111111111111111111111111;
        core[1] = (uint36)0b000000000000000000000000000000000000;
        LDQ(0);
        SLQ(1);
        cout << bits36(core[1]) << std::endl;
        clearCore();
        clearRgstrs();
        core[0] = (uint36)0b111111111111111111111111111111111111;
        cout << "CORE0 " << bits36(core[0]) << std::endl;
        STP(0);
        cout << "CORE0 " << bits36(core[0]) << std::endl;
        clearCore();
        clearRgstrs();
        core[0] = (uint36)0b111000000000000000111111111111111111;
        rgstrs.ac = (uint38)0b11111111111111111111111111111111111111;
        cout << "CORE0 " << bits36(core[0]) << std::endl;
        STD(0);
        cout << "CORE0 " << bits36(core[0]) << std::endl;
        clearCore();
        clearRgstrs();
        rgstrs.ac = (uint38)0b11111111111111111111111111111111111111;
        cout << "CAC " << bits38(rgstrs.ac) << std::endl;
        CLM();
        cout << "CAC " << bits38(rgstrs.ac) << std::endl;
        clearCore();
        clearRgstrs();
        core[0] = 0b111111111111111111111111111111111111;
        STA(0);
        cout << "CORE0 " << bits36(core[0]) << std::endl;
        clearCore();
        clearRgstrs();
        rgstrs.ac = 0b10000000000000000000000000000000000000;
        cout << "CAC " << bits38(rgstrs.ac) << std::endl;
        CHS();
        cout << "CAC " << bits38(rgstrs.ac) << std::endl;
        CHS();
        cout << "CAC " << bits38(rgstrs.ac) << std::endl;
        SSM();
        cout << "CAC " << bits38(rgstrs.ac) << std::endl;
        SSP();
        cout << "CAC " << bits38(rgstrs.ac) << std::endl;
        clearCore();
        clearRgstrs();
    }

    /*Tests logical operations*/
    void logicTest() {
        rgstrs.ac = 0b11111111111111111111111111111111111111;
        core[0] = 0b111111111111111111111111111111111111;
        CAL(0);
        cout << "CAC " << bits38(rgstrs.ac) << std::endl;
        clearCore();
        clearRgstrs();
        rgstrs.ac = 0b00111111111111111111111111111111111111;
        core[0] = 0b111111111111111111111111111111111111;
        ACL(0);
        cout << "CAC " << bits38(rgstrs.ac) << std::endl;
        rgstrs.ac = 0b10111111111111111111111111111111111111;
        core[0] = 0b111111111111111111111111111111111111;
        ACL(0);
        cout << "CAC " << bits38(rgstrs.ac) << std::endl;
        rgstrs.ac = 0b11111111111111111111111111111111111111;
        core[0] = 0b111111111111111111111111111111111111;
        ACL(0);
        cout << "CAC " << bits38(rgstrs.ac) << std::endl;
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

    // Helper functions

    /*returns cac(P,1,2)*/

    inline uint36 getCacPref() {
        return (getKthBit(rgstrs.ac,35)*4 + getKthBit(rgstrs.ac,34)*2 + getKthBit(rgstrs.ac,33));
    }

    /*returns cac(3-17)*/
    inline uint36 getCacDec() {
        return (rgstrs.ac % ((uint36)1 << 33))/((uint36)1 << 18);
    }

    /*returns cy(S,1,2)*/
    inline uint36 getCyPref(uint15 y) {
        return core[y] / ((uint36)1 << 33);
    }

    /*returns cy(3-17)*/
    inline uint36 getCyDec(uint15 y) {
        return (core[y] % ((uint36)1 << 33))/((uint36)1 << 18);
    }

    /*returns cy(21-35)*/
    inline uint36 getCyAdd(uint15 y) {
        return (core[y] % ((uint36)1 << 15));
    }

    /*returns cac(21-35)*/
    inline uint36 getCacAdd() {
        return (rgstrs.ac % ((uint36)1 << 15));
    }

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
    Shorthand cheat sheet:
    y = the address
    ac = the accumulator
    mq = the MQ register
    sr = the storage register

    cy = value of memory register at address
    cac = the value of the accumulator
    cmq = the value of the MQ register
    csr = the value of the Storage register
    */

    /*Load/Store Instructions*/

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

    /*Replaces the 18 most significant bits of cy with the 18 most significant bits of cmq. Octal: -0620*/
    void SLQ(uint15 y){
        uint36 cy = core[y];
        uint36 cmqS1_17 = rgstrs.mq/((uint36)1 << 18);
        cy %= ((uint36)1 << 18);
        core[y] = (cmqS1_17 << 18) + cy;
    }

    /*Replaces cy(S,1-2) with cac(P,1-2). Octal: +0630*/
    void STP(uint15 y){
        uint36 cy3_36 = core[y] % ((uint36)1 << 33);
        uint36 cacPref = getCacPref();
        core[y] = cacPref + cy3_36;
    }

    /*Replaces cy(3-17) with cac(3-17)*/
    void STD(uint15 y){
        uint36 cyPref = getCyPref(y) << 33;
        uint36 cacDec = getCacDec() << 18;
        uint36 cy18End = core[y]%((uint36)1 << 18);
        core[y] = cyPref + cacDec + cy18End;
    }

    /*Replaces cy(21-35) with cac(21-35)*/
    void STA(uint15 y){
        uint36 cyS120 = (core[y] / (1 << 15)) << 15;
        uint36 cacAdd = getCacAdd();
        core[y] = cyS120 + cacAdd;
    }

    /*Clears magnitude of ac. The sign bit is unaffected*/
    void CLM(){
        uint38 signbit = getKthBit(rgstrs.ac,37) << 37;
        rgstrs.ac = signbit;
    }

    /*Flips sign bit of AC*/
    void CHS(){
        uint38 signbit = getKthBit(rgstrs.ac,37);
        rgstrs.ac = signbit ? clearKthBit(rgstrs.ac,37) : setKthBit(rgstrs.ac,37);
    }

    /*Sets sign of AC to positive*/
    void SSP(){
        rgstrs.ac = setKthBit(rgstrs.ac,37);
    }

    /*Sets sign of AC to negative*/
    void SSM(){
        rgstrs.ac = clearKthBit(rgstrs.ac,37);
    }

    /*Stores cy(S,1-35) in ac(P,1-35). S and Q bits of ac are cleared*/
    void CAL(uint15 y){
        rgstrs.ac = core[y];
    }

    /*Adds CAC(P,1-35) to CY(S,1,35), and stores the value in Y. The sign of CAC is ignored, and the sign bit of CY is treated as a numerical bit. Overflow wraps around to least significant bit. No overflow is possible.
    The sum is stored in AC(P,1-35) CAC(P) and CAC(S) are unchanged*/
    void ACL(uint15 y){
        uint38 cacp = getKthBit(rgstrs.ac,35);
        uint36 cys = getKthBit(rgstrs.ac,35);
        uint36 sum = ((((rgstrs.ac)%((uint38)1 << 36)) + core[y])%((uint36)1 << 36));
        if (cacp && cys) {
            sum += 1;
        }
        rgstrs.ac = ((rgstrs.ac / ((uint38)1 << 36)) << 36) + sum;
    }
    

    /*Fixed-Point Arithmetic Instructions*/

    /*Adds cy to cac, stores the sum in ac. Octal: +0400*/
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
    the computer halts and sets the divide-check flag to true. Octal: +0220*/
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

    /*Uses cmq and cac as a 72-bit dividend with sign, treats cy as the divisor. 
    If cy > cac, division takes place, and the 35-bit signed quotient is stored in mq,
    while the 35-bit signed remainder is stored in ac. The sign of the remainder always
    agrees with the sign of the quotient. If cy <= cac, division does not take place andds
    the computer proceeds to the next instruction with the divide-check flag to true. Octal: +0221*/
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
    cout << "Running tests\n";
    cpu.FiPA_Test1();
    cpu.NegTest1();
    cpu.kbitTest();
    cpu.logicTest();
}