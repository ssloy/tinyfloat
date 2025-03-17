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
    TstCond( Failure, (Z == 0.0f), "-0.0 != 0.0" );

    TstCond ( Failure, (Three == Two + One),           "3 != 2+1" );
    TstCond ( Failure, (Four == Three + One),          "4 != 3+1" );
    TstCond ( Failure, (Four + Two * (- Two) == Zero), "4+2*(-2) != 0" );
    TstCond ( Failure, (Four - Three - One == Zero),   "4-3-1 != 0" );

    TstCond ( Failure, (MinusOne == (0 - One)), "-1 != 0 - 1");
    TstCond ( Failure, (MinusOne + One == Zero ), "-1+1 != 0");
    TstCond ( Failure, (One + MinusOne == Zero), "1+(-1) != 0");
    TstCond ( Failure, (MinusOne + fabs(One) == Zero), "(-1)+abs(1) != 0");
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

    std::cerr << "Searching for Radix and Precision." << std::endl;
    TinyFloat W = One;
    TinyFloat Y;
    do {
        W = W + W;
        Y = W + One;
        Z = Y - W;
        Y = Z - One;
    } while (MinusOne + fabs(Y) < Zero);

    // Now W is just big enough that |((W+1)-W)-1| >= 1.

    TinyFloat Radix;
    TinyFloat Precision = Zero;
    Y = One;
    do {
        Radix = W + Y;
        Y = Y + Y;
        Radix = Radix - W;
    } while ( Radix == Zero);

    std::cerr << "Radix = " << Radix << std::endl;
    std::cerr << "Two = " << Two << std::endl;

    std::cerr << ( Radix < Two ) << std::endl;
    
    if ( Radix < Two ) {
        Radix = One;
    }

    std::cerr << "Radix = " << Radix << std::endl;

    if (Radix != 1) {
        W = One;
        do {
            Precision = Precision + One;
            W = W * Radix;
            Y = W + One;
        } while ((Y - W) == One);
    }

    // Now W == Radix^Precision is barely too big to satisfy (W+1)-W == 1

    TinyFloat U1 = One / W;
    TinyFloat U2 = Radix * U1;
    std::cerr <<  "Closest relative separation found is U1 = " << U1 << std::endl << "Recalculating radix and precision" <<std::endl;


    //  save old values

    TinyFloat E0 = Radix;
    TinyFloat E1 = U1;
    TinyFloat E9 = U2;
    TinyFloat E3 = Precision;

    TinyFloat X = Four / Three;
    TinyFloat Third = X - One;
    TinyFloat F6 = Half - Third;
    X = F6 + F6;
    X = fabs(X - Third);
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
    X = fabs(X + F6);

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
  TinyFloat F9 = (Half - U1) + Half;
  Radix = floor(0.01f + U2 / U1);

  if ( Radix == E0 ) {
          std:: cout <<  "Radix confirmed." << std::endl;
  } else {
        std:: cout << "MYSTERY: recalculated Radix = " << Radix << "." << std::endl;
  }

  TstCond (Defect, Radix <= Eight + Eight, "Radix is too big: roundoff problems");
  TstCond (Flaw, (Radix == Two) || (Radix == 10) || (Radix == One), "Radix is not as good as 2 or 10");


    return 0;
}

