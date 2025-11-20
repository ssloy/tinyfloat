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
    TstCond( Failure, (Z == Zero), "-0.0 != 0.0" );

    TstCond( Failure, (Three == Two + One),           "3 != 2+1" );
    TstCond( Failure, (Four == Three + One),          "4 != 3+1" );
    TstCond( Failure, (Four + Two * (- Two) == Zero), "4+2*(-2) != 0" );
    TstCond( Failure, (Four - Three - One == Zero),   "4-3-1 != 0" );

    TstCond( Failure, (MinusOne == (Zero - One)), "-1 != 0 - 1");
    TstCond( Failure, (MinusOne + One == Zero),   "-1+1 != 0");
    TstCond( Failure, (One + MinusOne == Zero),   "1+(-1) != 0");
    TstCond( Failure, (MinusOne + (FLOAT)fabsf(One) == Zero), "(-1)+abs(1) != 0");
    TstCond( Failure, (MinusOne + MinusOne * MinusOne == Zero), "-1+(-1)*(-1) != 0");
    TstCond( Failure, Half + MinusOne + Half == Zero, "1/2 + (-1) + 1/2 != 0");

    TstCond( Failure, (Nine == Three * Three)
            && (TwentySeven == Nine * Three) && (Eight == Four + Four)
            && (ThirtyTwo == Eight * Four)
            && (ThirtyTwo - TwentySeven - Four - One == Zero),
            "9 != 3*3, 27 != 9*3, 32 != 8*4, or 32-27-4-1 != 0");

    TstCond( Failure, (Five == Four + One) &&
            (TwoForty == Four * Five * Three * Four)
            && ( TwoForty / Three - Four * Four  * Five == Zero)
            && ( TwoForty / Four  - Five * Three * Four == Zero)
            && ( TwoForty / Five  - Four * Three * Four == Zero),
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

    if (Radix != One) {
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

    do {
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

    do {
        U1 = X;
        Y = Half * U1 + ThirtyTwo * U1 * U1;
        Y = Half - Y;
        X = Half + Y;
        Y = Half - X;
        X = Half + Y;
    } while ( ! ((U1 <= X) || (X <= Zero)));

    //  now U1 == 1 ulp of 1 -

    if ( U1 == E1 ) {
        std:: cout << "confirms closest relative separation U1." << std::endl;
    } else {
        std:: cout << "gets better closest relative separation U1 = " << U1 << "." << std::endl;
    }


    W = One / U1;
    FLOAT F9 = (Half - U1) + Half;
    Radix = (FLOAT)floorf((FLOAT)0.01f + U2 / U1); // TODO

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

    FLOAT Z1, Z2, X1, Y1, Q;

    /* Test for extra-precise subexpressions */

    X = (FLOAT)fabsf(((Four / Three - One) - One / Four) * Three - One / Four);
    do {
        Z2 = X;
        X = (One + (Half * Z2 + ThirtyTwo * Z2 * Z2)) - One;
    } while ( ! ((Z2 <= X) || (X <= Zero)));

    X = Y = Z = (FLOAT)fabsf((Three / Four - Two / Three) * Three - One / Four);
    do {
        Z1 = Z;
        Z = (One / Two - ((One / Two - (Half * Z1 + ThirtyTwo * Z1 * Z1)) + One / Two)) + One / Two;
    } while ( ! ((Z1 <= Z) || (Z <= Zero)));

    do {
        do {
            Y1 = Y;
            Y = (Half - ((Half - (Half * Y1 + ThirtyTwo * Y1 * Y1)) + Half)) + Half;
        } while ( ! ((Y1 <= Y) || (Y <= Zero)));
        X1 = X;
        X = ((Half * X1 + ThirtyTwo * X1 * X1) - F9) + F9;
    } while ( ! ((X1 <= X) || (X <= Zero)));

    if ((X1 != Y1) || (X1 != Z1)) {
        BadCond(Serious, "Disagreements among the values X1, Y1, Z1,\n");
        std::cout << "respectively " << X1 << ", " << Y1 << ", " << Z1 << std::endl;
        std::cout << "are symptoms of inconsistencies introduced" << std::endl;;
        std::cout << "by extra-precise evaluation of arithmetic subexpressions." << std::endl;;
        //  notify("Possibly some part of this");
        if ((X1 == U1) || (Y1 == U1) || (Z1 == U1))
            std::cout << "That feature is not tested further by this program." << std::endl;
    } else {
        if ((Z1 != U1) || (Z2 != U2)) {
            if ((Z1 >= U1) || (Z2 >= U2)) {
                BadCond(Failure, "");
                //      notify("Precision");
                // printf("\tU1 = %.7e, Z1 - U1 = %.7e\n",U1,Z1-U1);
                // printf("\tU2 = %.7e, Z2 - U2 = %.7e\n",U2,Z2-U2);
            }
            else {
                if ((Z1 <= Zero) || (Z2 <= Zero)) {
                    printf("Because of unusual Radix = %f", Radix);
                    printf(", or exact rational arithmetic a result\n");
                    printf("Z1 = %.7e, or Z2 = %.7e ", Z1, Z2);
                    //        notify("of an\nextra-precision");
                }
                if (Z1 != Z2 || Z1 > Zero) {
                    X = Z1 / U1;
                    Y = Z2 / U2;
                    if (Y > X) X = Y;
                    Q = - (FLOAT)logf(X);
                    printf("Some subexpressions appear to be calculated extra\n");
                    printf("precisely with about %g extra B-digits, i.e.\n",
                            (Q / (FLOAT)logf(Radix)));
                    printf("roughly %g extra significant decimals.\n",
                            Q / (FLOAT)logf(10.));
                }
                printf("That feature is not tested further by this program.\n");
            }
        }
    }

#if 0

FLOAT T, R;

  if (Radix >= Two) {
    X = W / (Radix * Radix);
    Y = X + One;
    Z = Y - X;
    T = Z + U2;
    X = T - Z;
    TstCond (Failure, X == U2,
      "Subtraction is not normalized X=Y,X+Z != Y+Z!");
    if (X == U2) printf(
      "Subtraction appears to be normalized, as it should be.");
    }
  printf("\nChecking for guard digit in *, /, and -.\n");
  Y = F9 * One;
  Z = One * F9;
  X = F9 - Half;
  Y = (Y - Half) - X;
  Z = (Z - Half) - X;
  X = One + U2;
  T = X * Radix;
  R = Radix * X;
  X = T - Radix;
  X = X - Radix * U2;
  T = R - Radix;
  T = T - Radix * U2;
  X = X * (Radix - One);
  T = T * (Radix - One);
  if ((X == Zero) && (Y == Zero) && (Z == Zero) && (T == Zero)) GMult = Yes;
  else {
    GMult = No;
    TstCond (Serious, False,
      "* lacks a Guard Digit, so 1*X != X");
    }
  Z = Radix * U2;
  X = One + Z;
  Y = FABS((X + Z) - X * X) - U2;
  X = One - U2;
  Z = FABS((X - U2) - X * X) - U1;
  TstCond (Failure, (Y <= Zero)
       && (Z <= Zero), "* gets too many final digits wrong.\n");
  Y = One - U2;
  X = One + U2;
  Z = One / Y;
  Y = Z - X;
  X = One / Three;
  Z = Three / Nine;
  X = X - Z;
  T = Nine / TwentySeven;
  Z = Z - T;
  TstCond(Defect, X == Zero && Y == Zero && Z == Zero,
    "Division lacks a Guard Digit, so error can exceed 1 ulp\n\
or  1/3  and  3/9  and  9/27 may disagree");
  Y = F9 / One;
  X = F9 - Half;
  Y = (Y - Half) - X;
  X = One + U2;
  T = X / One;
  X = T - X;
  if ((X == Zero) && (Y == Zero) && (Z == Zero)) GDiv = Yes;
  else {
    GDiv = No;
    TstCond (Serious, False,
      "Division lacks a Guard Digit, so X/1 != X");
    }
  X = One / (One + U2);
  Y = X - Half - Half;
  TstCond (Serious, Y < Zero,
       "Computed value of 1/1.000..1 >= 1");
  X = One - U2;
  Y = One + Radix * U2;
  Z = X * Radix;
  T = Y * Radix;
  R = Z / Radix;
  StickyBit = T / Radix;
  X = R - X;
  Y = StickyBit - Y;
  TstCond (Failure, X == Zero && Y == Zero,
      "* and/or / gets too many last digits wrong");
  Y = One - U1;
  X = One - F9;
  Y = One - Y;
  T = Radix - U2;
  Z = Radix - BMinusU2;
  T = Radix - T;
  if ((X == U1) && (Y == U1) && (Z == U2) && (T == U2)) GAddSub = Yes;
  else {
    GAddSub = No;
    TstCond (Serious, False,
      "- lacks Guard Digit, so cancellation is obscured");
    }
  if (F9 != One && F9 - One >= Zero) {
    BadCond(Serious, "comparison alleges  (1-U1) < 1  although\n");
    printf("  subtraction yields  (1-U1) - 1 = 0 , thereby vitiating\n");
    printf("  such precautions against division by zero as\n");
    printf("  ...  if (X == 1.0) {.....} else {.../(X-1.0)...}\n");
    }
  if (GMult == Yes && GDiv == Yes && GAddSub == Yes) printf(
    "     *, /, and - appear to have guard digits, as they should.\n");
/*=============================================*/
  Milestone = 40;
/*=============================================*/
  Pause();
  printf("Checking rounding on multiply, divide and add/subtract.\n");
  RMult = Other;
  RDiv = Other;
  RAddSub = Other;
  RadixD2 = Radix / Two;
  A1 = Two;
  Done = False;
  do {
    AInvrse = Radix;
    do {
      X = AInvrse;
      AInvrse = AInvrse / A1;
      } while ( ! (FLOOR(AInvrse) != AInvrse));
    Done = (X == One) || (A1 > Three);
    if (! Done) A1 = Nine + One;
    } while ( ! (Done));
  if (X == One) A1 = Radix;
  AInvrse = One / A1;
  X = A1;
  Y = AInvrse;
  Done = False;
  do {
    Z = X * Y - Half;
    TstCond (Failure, Z == Half,
      "X * (1/X) differs from 1");
    Done = X == Radix;
    X = Radix;
    Y = One / X;
    } while ( ! (Done));
  Y2 = One + U2;
  Y1 = One - U2;
  X = OneAndHalf - U2;
  Y = OneAndHalf + U2;
  Z = (X - U2) * Y2;
  T = Y * Y1;
  Z = Z - X;
  T = T - X;
  X = X * Y2;
  Y = (Y + U2) * Y1;
  X = X - OneAndHalf;
  Y = Y - OneAndHalf;

  if ((X == Zero) && (Y == Zero) && (Z == Zero) && (T <= Zero))
 {
    X = (OneAndHalf + U2) * Y2;
    Y = OneAndHalf - U2 - U2;
    Z = OneAndHalf + U2 + U2;
    T = (OneAndHalf - U2) * Y1;
    X = X - (Z + U2);
    StickyBit = Y * Y1;
    S = Z * Y2;
    T = T - Y;
    Y = (U2 - Y) + StickyBit;
    Z = S - (Z + U2 + U2);
    StickyBit = (Y2 + U2) * Y1;
    Y1 = Y2 * Y1;
    StickyBit = StickyBit - Y2;
    Y1 = Y1 - Half;

    if ((X == Zero) && (Y == Zero) && (Z == Zero) && (T == Zero)
      && ( StickyBit == Zero) && (Y1 == Half))
   {
      RMult = Rounded;
      printf("Multiplication appears to round correctly.\n");
    }
    else if ((X + U2 == Zero) && (Y < Zero) && (Z + U2 == Zero)
        && (T < Zero) && (StickyBit + U2 == Zero)
        && (Y1 < Half))
   {
        RMult = Chopped;
        printf("Multiplication appears to chop.\n");
    }
    else
   { 
      printf("* is neither chopped nor correctly rounded.\n");
    }

//  if ((RMult == Rounded) && (GMult == No)) notify("Multiplication");
    }
  else printf("* is neither chopped nor correctly rounded.\n");
/*=============================================*/
  Milestone = 45;
/*=============================================*/
  Y2 = One + U2;
  Y1 = One - U2;
  Z = OneAndHalf + U2 + U2;
  X = Z / Y2;
  T = OneAndHalf - U2 - U2;
  Y = (T - U2) / Y1;
  Z = (Z + U2) / Y2;
  X = X - OneAndHalf;
  Y = Y - T;
  T = T / Y1;
  Z = Z - (OneAndHalf + U2);
  T = (U2 - OneAndHalf) + T;
  if (! ((X > Zero) || (Y > Zero) || (Z > Zero) || (T > Zero))) {
    X = OneAndHalf / Y2;
    Y = OneAndHalf - U2;
    Z = OneAndHalf + U2;
    X = X - Y;
    T = OneAndHalf / Y1;
    Y = Y / Y1;
    T = T - (Z + U2);
    Y = Y - Z;
    Z = Z / Y2;
    Y1 = (Y2 + U2) / Y2;
    Z = Z - OneAndHalf;
    Y2 = Y1 - Y2;
    Y1 = (F9 - U1) / F9;
    if ((X == Zero) && (Y == Zero) && (Z == Zero) && (T == Zero)
      && (Y2 == Zero) && (Y2 == Zero)
      && (Y1 - Half == F9 - Half )) {
      RDiv = Rounded;
      printf("Division appears to round correctly.\n");
//    if (GDiv == No) notify("Division");
      }
    else if ((X < Zero) && (Y < Zero) && (Z < Zero) && (T < Zero)
      && (Y2 < Zero) && (Y1 - Half < F9 - Half)) {
      RDiv = Chopped;
      printf("Division appears to chop.\n");
      }
    }
  if (RDiv == Other) printf("/ is neither chopped nor correctly rounded.\n");
  BInvrse = One / Radix;
  TstCond (Failure, (BInvrse * Radix - Half == Half),
       "Radix * ( 1 / Radix ) differs from 1");
/*=============================================*/
  Milestone = 50;
/*=============================================*/
  TstCond (Failure, ((F9 + U1) - Half == Half)
       && ((BMinusU2 + U2 ) - One == Radix - One),
       "Incomplete carry-propagation in Addition");
  X = One - U1 * U1;
  Y = One + U2 * (One - U2);
  Z = F9 - Half;
  X = (X - Half) - Z;
  Y = Y - One;
  if ((X == Zero) && (Y == Zero)) {
    RAddSub = Chopped;
    printf("Add/Subtract appears to be chopped.\n");
    }
  if (GAddSub == Yes)
 {
    X = (Half + U2) * U2;
    Y = (Half - U2) * U2;
    X = One + X;
    Y = One + Y;
    X = (One + U2) - X;
    Y = One - Y;
    if ((X == Zero) && (Y == Zero))
   {
      X = (Half + U2) * U1;
      Y = (Half - U2) * U1;
      X = One - X;
      Y = One - Y;
      X = F9 - X;
      Y = One - Y;
      if ((X == Zero) && (Y == Zero)) {
        RAddSub = Rounded;
        printf("Addition/Subtraction appears to round correctly.\n");
//      if (GAddSub == No) notify("Add/Subtract");
        }
      else printf("Addition/Subtraction neither rounds nor chops.\n");
      }
    else printf("Addition/Subtraction neither rounds nor chops.\n");
    }
  else printf("Addition/Subtraction neither rounds nor chops.\n");
  S = One;
  X = One + Half * (One + Half);
  Y = (One + U2) * Half;
  Z = X - Y;
  T = Y - X;
  StickyBit = Z + T;
  if (StickyBit != Zero) {
    S = Zero;
    BadCond(Flaw, "(X - Y) + (Y - X) is non zero!\n");
    }
  StickyBit = Zero;
  if ((GMult == Yes) && (GDiv == Yes) && (GAddSub == Yes)
    && (RMult == Rounded) && (RDiv == Rounded)
    && (RAddSub == Rounded) && (FLOOR(RadixD2) == RadixD2)) {
    printf("Checking for sticky bit.\n");
    X = (Half + U1) * U2;
    Y = Half * U2;
    Z = One + Y;
    T = One + X;
    if ((Z - One <= Zero) && (T - One >= U2)) {
      Z = T + Y;
      Y = Z - X;
      if ((Z - T >= U2) && (Y - T == Zero)) {
        X = (Half + U1) * U1;
        Y = Half * U1;
        Z = One - Y;
        T = One - X;
        if ((Z - One == Zero) && (T - F9 == Zero)) {
          Z = (Half - U1) * U1;
          T = F9 - Z;
          Q = F9 - Y;
          if ((T - F9 == Zero) && (F9 - U1 - Q == Zero)) {
            Z = (One + U2) * OneAndHalf;
            T = (OneAndHalf + U2) - Z + U2;
            X = One + Half / Radix;
            Y = One + Radix * U2;
            Z = X * Y;
            if (T == Zero && X + Radix * U2 - Z == Zero) {
              if (Radix != Two) {
                X = Two + U2;
                Y = X / Two;
                if ((Y - One == Zero)) StickyBit = S;
                }
              else StickyBit = S;
              }
            }
          }
        }
      }
    }

  if (StickyBit == One) {
    printf("Sticky bit apparently used correctly.\n");
  } else { 
    printf("Sticky bit used incorrectly or not at all.\n");
  }

  TstCond (Flaw, !(GMult == No || GDiv == No || GAddSub == No ||
      RMult == Other || RDiv == Other || RAddSub == Other),
    "lack(s) of guard digits or failure(s) to correctly round or chop\n\
(noted above) count as one flaw in the final tally below");
    
#endif

    return 0;
}

