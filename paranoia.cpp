#include <iostream>
#include <cstring>
#include <cmath>
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

#if 1
#define FLOAT TinyFloat
#else
#define FLOAT float
#endif

int main() {
    //  First two assignments use integer right-hand sides.
    const FLOAT Zero = 0;
    const FLOAT One = 1;
    const FLOAT Two = One + One;
    const FLOAT Three = Two + One;
    const FLOAT Four = Three + One;
    const FLOAT Five = Four + One;
    const FLOAT Eight = Four + Four;
    const FLOAT Nine = Three * Three;
    const FLOAT TwentySeven = Nine * Three;
    const FLOAT ThirtyTwo = Four * Eight;
    const FLOAT TwoForty = Four * Five * Three * Four;
    const FLOAT MinusOne = -One;
    const FLOAT Half = One / Two;
    const FLOAT OneAndHalf = One + Half;

    std::cout << "Program is now RUNNING tests on small integers:" << std::endl;

    TstCond( Failure, (Zero + Zero == Zero), "0+0 != 0" );
    TstCond( Failure, (One - One == Zero),   "1-1 != 0" );
    TstCond( Failure, (One > Zero),          "1 <= 0"   );
    TstCond( Failure, (One + One == Two),    "1+1 != 2" );

    FLOAT Z = - Zero;
    TstCond( Failure, (Z == (FLOAT)0.0f), "-0.0 != 0.0" );

    TstCond ( Failure, (Three == Two + One),           "3 != 2+1" );
    TstCond ( Failure, (Four == Three + One),          "4 != 3+1" );
    TstCond ( Failure, (Four + Two * (- Two) == Zero), "4+2*(-2) != 0" );
    TstCond ( Failure, (Four - Three - One == Zero),   "4-3-1 != 0" );

    TstCond ( Failure, (MinusOne == ((FLOAT)0 - One)), "-1 != 0 - 1");
    TstCond ( Failure, (MinusOne + One == Zero ), "-1+1 != 0");
    TstCond ( Failure, (One + MinusOne == Zero), "1+(-1) != 0");
    TstCond ( Failure, (MinusOne + (FLOAT)fabsf(One) == Zero), "(-1)+abs(1) != 0");
    TstCond ( Failure, (MinusOne + MinusOne * MinusOne == Zero), "-1+(-1)*(-1) != 0");

    TstCond (Failure, Half + MinusOne + Half == Zero, "1/2 + (-1) + 1/2 != 0");

    TstCond (Failure, (Nine == Three * Three)
            && (TwentySeven == Nine * Three) && (Eight == Four + Four)
            && (ThirtyTwo == Eight * Four)
            && (ThirtyTwo - TwentySeven - Four - One == Zero),
            "9 != 3*3, 27 != 9*3, 32 != 8*4, or 32-27-4-1 != 0");

    TstCond (Failure, (Five == Four + One) &&
            (TwoForty == Four * Five * Three * Four)
            && (TwoForty / Three - Four * Four * Five == Zero)
            && ( TwoForty / Four - Five * Three * Four == Zero)
            && ( TwoForty / Five - Four * Three * Four == Zero),
            "5 != 4+1, 240/3 != 80, 240/4 != 60, or 240/5 != 48");

    if (ErrCnt[Failure] == 0)
        std::cout <<  "-1, 0, 1/2, 1, 2, 3, 4, 5, 9, 27, 32 & 240 are O.K." << std::endl << std::endl;

    std::cout << "Searching for Radix and Precision." << std::endl;
    FLOAT W = One;
    FLOAT Y;
    do {
        W = W + W;
        Y = W + One;
        Z = Y - W;
        Y = Z - One;
    } while (MinusOne + (FLOAT)fabsf(Y) < Zero);

    // Now W is just big enough that |((W+1)-W)-1| >= 1.

    FLOAT Radix;
    FLOAT Precision = Zero;
    Y = One;
    do {
        Radix = W + Y;
        Y = Y + Y;
        Radix = Radix - W;
    } while (Radix == Zero);

    if ( Radix < Two ) {
        Radix = One;
    }

    std::cout << "Radix = " << Radix << std::endl;

    if (Radix != (FLOAT)1) {
        W = One;
        do {
            Precision = Precision + One;
            W = W * Radix;
            Y = W + One;
        } while ((Y - W) == One);
    }

    // Now W == Radix^Precision is barely too big to satisfy (W+1)-W == 1

    FLOAT U1 = One / W;
    FLOAT U2 = Radix * U1;
    std::cout <<  "Closest relative separation found is U1 = " << U1 << std::endl << "Recalculating radix and precision" <<std::endl;

    //  save old values

    FLOAT E0 = Radix;
    FLOAT E1 = U1;
    FLOAT E9 = U2;
    FLOAT E3 = Precision;

    FLOAT X = Four / Three;
    FLOAT Third = X - One;
    FLOAT F6 = Half - Third;
    X = F6 + F6;
    X = (FLOAT)fabsf(X - Third);
    if (X < U2) X = U2;

    //  now X = (unknown no.) ulps of 1+

    do  {
        U2 = X;
        Y = Half * U2 + ThirtyTwo * U2 * U2;
        Y = One + Y;
        X = Y - One;
    } while ( ! ((U2 <= X) || (X <= Zero)));

    //  now U2 == 1 ulp of 1 +

    X = Two / Three;
    F6 = X - Half;
    Third = F6 + F6;
    X = Third - Half;
    X = (FLOAT)fabsf(X + F6);

    if ( X < U1 ) {
        X = U1;
    }

    //  now  X == (unknown no.) ulps of 1 -

    do  {
        U1 = X;
        Y = Half * U1 + ThirtyTwo * U1 * U1;
        Y = Half - Y;
        X = Half + Y;
        Y = Half - X;
        X = Half + Y;
    } while ( ! ((U1 <= X) || (X <= Zero)));

    //  now U1 == 1 ulp of 1 -

    if ( U1 == E1 ) {
        std:: cout <<  "confirms closest relative separation U1." << std::endl;
    } else {
        std:: cout << "gets better closest relative separation U1 = " << U1 << "." << std::endl;
    }


    W = One / U1;
    FLOAT F9 = (Half - U1) + Half;
    Radix = (FLOAT)floorf((FLOAT)0.01f + U2 / U1);

    if ( Radix == E0 ) {
        std:: cout <<  "Radix confirmed." << std::endl;
    } else {
        std:: cout << "MYSTERY: recalculated Radix = " << Radix << "." << std::endl;
    }

    TstCond (Defect, Radix <= Eight + Eight, "Radix is too big: roundoff problems");
    TstCond (Flaw, (Radix == Two) || (Radix == (FLOAT)10) || (Radix == One), "Radix is not as good as 2 or 10");

    TstCond (Failure, F9 - Half < Half, "(1-U1)-1/2 < 1/2 is FALSE, prog. fails?");
    X = F9;
    FLOAT I = 1;
    Y = X - Half;
    Z = Y - Half;
    TstCond (Failure, (X != One) || (Z == Zero), "Comparison is fuzzy,X=1 but X-1/2-1/2 != 0");
    X = One + U2;
    I = 0;

    /*... BMinusU2 = nextafter(Radix, 0) */
    FLOAT BMinusU2 = Radix - One;
    BMinusU2 = (BMinusU2 - U2) + One;
    /* 
       Purify Integers.
       */
    if (Radix != One) {
        X = - TwoForty * (FLOAT)logf(U1) / (FLOAT)logf(Radix);

        Y = (FLOAT)floorf(Half + X);
        if ((FLOAT)fabsf(X - Y) * Four < One) X = Y;
        Precision = X / TwoForty;
        Y = (FLOAT)floorf(Half + Precision);
        if ((FLOAT)fabsf(Precision - Y) * TwoForty < Half) Precision = Y;
    }

    if ((Precision != (FLOAT)floorf(Precision)) || (Radix == One)) {
        std::cout << "Precision cannot be characterized by an Integer number" << std::endl;
        std::cout << "of significant digits but, by itself, this is a minor flaw." << std::endl;
    }

    if (Radix == One) {
        std::cout << "logarithmic encoding has precision characterized solely by U1." << std::endl;
    } else {
        std::cout << "The number of significant digits of the Radix is "<< Precision << "." << std::endl;
    }

    TstCond(Serious, U2 * Nine * Nine * TwoForty < One, "Precision worse than 5 decimal figures  ");


    return 0;
}

