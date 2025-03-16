#include <iostream>
#include <cstring>
#include "tinyfloat.h"

// PARANOIA tests the floating point arithmetic implementation on a computer.



int ErrCnt[4] = {0,0,0,0};

enum Level {
    Failure = 0,
    Serious = 1,
    Defect  = 2,
    Flaw    = 3
};

void BadCond(Level level, std::string msg) {
    std::string text[] = { "FAILURE", "SERIOUS DEFECT", "DEFECT", "FLAW" };
    ErrCnt[level]++;
    std::cerr << text[level] << ": " << msg;
}

void TstCond(Level level, bool valid, std::string msg) {
    if (!valid) {
        BadCond(level, msg);
        std::cout << "." << std::endl;
    }
    return;
}

int main() {
    //  First two assignments use integer right-hand sides.
    TinyFloat Zero = 0;
    TinyFloat One = 1;
    TinyFloat Two = One + One;
    TinyFloat Three = Two + One;
    TinyFloat Four = Three + One;
    TinyFloat Five = Four + One;
    TinyFloat Eight = Four + Four;
    TinyFloat Nine = Three * Three;
    TinyFloat TwentySeven = Nine * Three;
    TinyFloat ThirtyTwo = Four * Eight;
    TinyFloat TwoForty = Four * Five * Three * Four;
    TinyFloat MinusOne = -One;
    TinyFloat Half = One / Two;
    TinyFloat OneAndHalf = One + Half;

    std::cout << "Program is now RUNNING tests on small integers:" << std::endl;

    TstCond( Failure, (Zero + Zero == Zero), "0+0 != 0" );
    TstCond( Failure, (One - One == Zero),   "1-1 != 0" );
    TstCond( Failure, (One > Zero),          "1 <= 0"   );
    TstCond( Failure, (One + One == Two),    "1+1 != 2" );

    TinyFloat Z = - Zero;

    if ( Z != Zero ) {
        ErrCnt[Failure] = ErrCnt[Failure] + 1;
        printf ( "Comparison alleges that -0.0 is Non-zero!\n" );
//      U2 = 0.001;
//      Radix = 1;
//      TstPtUf();
    }

    TstCond ( Failure, (Three == Two + One),           "3 != 2+1" );
    TstCond ( Failure, (Four == Three + One),          "4 != 3+1" );
    TstCond ( Failure, (Four + Two * (- Two) == Zero), "4+2*(-2) != 0" );
    TstCond ( Failure, (Four - Three - One == Zero),   "4-3-1 != 0" );

    TstCond (Failure, (MinusOne == (0 - One))
            && (MinusOne + One == Zero ) && (One + MinusOne == Zero)
            && (MinusOne + fabs(One) == Zero)
            && (MinusOne + MinusOne * MinusOne == Zero),
            "-1+1 != 0, (-1)+abs(1) != 0, or -1+(-1)*(-1) != 0");

    TstCond (Failure, Half + MinusOne + Half == Zero,
            "1/2 + (-1) + 1/2 != 0");



    return 0;
}

