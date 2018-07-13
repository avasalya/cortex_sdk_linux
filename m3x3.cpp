
/*====================================================================

File: m3x3.cpp

Written by: Ned Phipps

Functions:

    M3x3_MulMatrix
    M3x3_MulVector
    M3x3_LoadIdentity
    M3x3_Transpose
    M3x3_Copy
    M3x3_RotateX
    M3x3_RotateY
    M3x3_RotateZ
    M3x3_ExtractEulerAngles_ZYX
    M3x3_ExtractEulerAngles_XYZ
    M3x3_ExtractEulerAngles_YXZ
    M3x3_ExtractEulerAngles_YZX
    M3x3_ExtractEulerAngles_XZY
    M3x3_ExtractEulerAngles_ZXY
    M3x3_ExtractAndCorrectEulerAngles_ZYX
    M3x3_ExtractAndCorrectEulerAngles_XYZ
    M3x3_ExtractAndCorrectEulerAngles_YXZ
    M3x3_ExtractAndCorrectEulerAngles_YZX
    M3x3_ExtractAndCorrectEulerAngles_XZY
    M3x3_ExtractAndCorrectEulerAngles_ZXY
    M3x3_ExtractEulerAngles
    M3x3_ExtractAndCorrectEulerAngles
    M3x3_ConstructRotationMatrix


Comments:

    1) 3x3 matrices are defined as double m[3][3] = m[rows][cols].

    2) The output is always the last parameter.

    3) Output matrices can be input matrices.  That is:

           M3x3_MulMatrix(m1, m2, product);

       could be

           M3x3_MulMatrix(m1, m2, m1);

       This means:   m1 = m1 * m2.


    4) When using vectors:

           M3x3_MulVector(m1, v, product)

       This means:  product = m1 * v


    5) Rotation order should be interpreted as rotating
       around axes that are attached to the object.  If you
       want to interpret rotation order as rotations around
       fixed global axis, then read the order backwards.

       A ZYX_ORDER matrix will rotate an object first around
       its Z axis, then around its rotated Y axis, then around
       its doubly rotated X axis.  This is more understandable
       if you pick up an object and try it.

       A ZYX_ORDER matrix will rotate an object around fixed
       global axis in XYZ order.

--------------------------------------------------------------------*/

#include <math.h>
#include <memory.h>

#include "cortex_intern.h"

#include "m3x3.h"


double IdentityMatrix[3][3] =
{
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0}
};


void M3x3_MulMatrix(const double m1[3][3], const double m2[3][3], double product[3][3])
{
    int i, j, k;
    double temp[3][3];

    for (i=0 ; i<3 ; i++)
    {
        for (j=0 ; j<3 ; j++)
        {
            temp[i][j] = 0.0;

            for (k=0 ; k<3 ; k++)
            {
                temp[i][j] += m1[i][k] * m2[k][j];
            }
        }
    }
    memcpy(product, temp, 3*3*sizeof(double));
}


void M3x3_MulVector(const double m[3][3], const double v[3], double product[3])
{
    int i, j;
    double temp[3];

    for (i=0 ; i<3 ; i++)
    {
        temp[i] = 0.0;

        for (j=0 ; j<3 ; j++)
        {
            temp[i] += m[i][j] * v[j];
        }
    }
    memcpy(product, temp, 3*sizeof(double));
}


void M3x3_MulVector(const double v[3], const double m[3][3], double product[3])
{
    int i, j;
    double temp[3];

    for (i=0 ; i<3 ; i++)
    {
        temp[i] = 0.0;

        for (j=0 ; j<3 ; j++)
        {
            temp[i] += m[j][i] * v[j];
        }
    }
    memcpy(product, temp, 3*sizeof(double));
}


void M3x3_LoadIdentity(double m[3][3])
{
    memcpy(m, IdentityMatrix, 3*3*sizeof(double));
}


void M3x3_Transpose(double m[3][3])
{
   int i, j;
   double temp;

    for (i=0 ; i<2 ; i++)
    {
        for (j=i+1 ; j<3 ; j++)
        {
            temp   = m[i][j];
            m[i][j] = m[j][i];
            m[j][i] = temp;
        }
    }
}


void M3x3_Copy(double src[3][3], double dst[3][3])
{
    memcpy(dst, src, 3*3*sizeof(double));
}


void M3x3_RotateX(double input[3][3], double degrees, double result[3][3])
{
	double 	transformationmatrix[3][3];
  	double  sinvalue, cosvalue, radians;

    M3x3_LoadIdentity(transformationmatrix);

	radians = degrees * (M_PI/180.0);
	cosvalue = cos(radians);
	sinvalue = sin(radians);

	transformationmatrix[1][1] =  cosvalue;
	transformationmatrix[1][2] = -sinvalue;
	transformationmatrix[2][1] =  sinvalue;
	transformationmatrix[2][2] =  cosvalue;

	M3x3_MulMatrix(transformationmatrix, input, result);
}


void M3x3_RotateY(double input[3][3], double degrees, double result[3][3])
{
    double transformationmatrix[3][3];
    double sinvalue, cosvalue, radians;

    M3x3_LoadIdentity(transformationmatrix);

    radians = degrees * (M_PI/180.0);
    cosvalue = cos(radians);
    sinvalue = sin(radians);

    transformationmatrix[0][0] =  cosvalue;
    transformationmatrix[0][2] =  sinvalue;
    transformationmatrix[2][0] = -sinvalue;
    transformationmatrix[2][2] =  cosvalue;

    M3x3_MulMatrix(transformationmatrix, input, result);
}


void M3x3_RotateZ(double input[][3], double degrees, double result[3][3])
{
    double transformationmatrix[3][3];
    double sinvalue, cosvalue, radians;

    M3x3_LoadIdentity(transformationmatrix);

    radians = degrees * (M_PI/180.0);
    cosvalue = cos(radians);
    sinvalue = sin(radians);

    transformationmatrix[0][0] =  cosvalue;
    transformationmatrix[0][1] = -sinvalue;
    transformationmatrix[1][0] =  sinvalue;
    transformationmatrix[1][1] =  cosvalue;

    M3x3_MulMatrix(transformationmatrix, input, result);
}


void M3x3_ExtractEulerAngles_XYZ(
  double matrix[3][3],
  double angles[3])
{
    double ax,ay,az;
    double cosY;

/*
   R[XYZ] = Rx * Ry * Rz

            | 1   0    0 |   |  cy   0   sy |   | cz   -sz   0 |
          = | 0   cx  -sx| * |   0   1    0 | * | sz    cz   0 |
            | 0   sx   cx|   | -sy   0   cy |   |  0    0    1 |


            | cycz            -cysz             sy   |
          = | cxsz + sxsycz    cxcz - sxsysz   -sxcy |
            | sxsz - cxsycz    sxcz + cxsysz    cxcy |
*/

    if (matrix[0][0] == XEMPTY)
    {
        angles[0] = XEMPTY;
        angles[1] = XEMPTY;
        angles[2] = XEMPTY;
        return;
    }

    cosY = sqrt(matrix[0][0]*matrix[0][0] + matrix[0][1]*matrix[0][1]);
    ay = atan2(matrix[0][2], cosY);

    if (cosY < 0.0001) // cos(ay) < 0.01
    {
     // Assume sy=+-1,cy=0,sz=0,cz=1.0 and decode the matrix accordingly

        if (ay > 0)
            ax = atan2( matrix[1][0], matrix[1][1]);
        else
            ax = atan2(-matrix[1][0], matrix[1][1]);

        az = 0.0;
    }
    else
    {
        az = atan2(-matrix[0][1], matrix[0][0]);
        ax = atan2(-matrix[1][2], matrix[2][2]);
    }

    angles[0] = ax * (180.0/M_PI);
    angles[1] = ay * (180.0/M_PI);
    angles[2] = az * (180.0/M_PI);
}


void M3x3_ExtractEulerAngles_ZYX(
  double matrix[3][3],
  double angles[3])
{
    double ax,ay,az;
    double cosY;

/*
   R[ZYX] = Rz * Ry * Rx

            | cz   -sz   0 |   | cy   0   sy |   | 1   0    0 |
          = | sz    cz   0 | * |  0   1    0 | * | 0   cx  -sx|
            |  0    0    1 |   |-sy   0   cy |   | 0   sx   cx|


            | cycz    -cxsz + sxsycz    sxsz + cxsycz |
          = | cysz     cxcz + sxsysz   -sxcz + cxsysz |
            |  -sy     sxcy             cxcy          |
*/

    if (matrix[0][0] == XEMPTY)
    {
        angles[0] = XEMPTY;
        angles[1] = XEMPTY;
        angles[2] = XEMPTY;
        return;
    }

    cosY = sqrt(matrix[0][0]*matrix[0][0] + matrix[1][0]*matrix[1][0]);

    ay = atan2(-matrix[2][0], cosY);

    if (cosY < 0.0001)
    {
     // Assume sy=+-1,cy=0,sz=0,cz=1.0 and decode the matrix accordingly

        if (ay < 0)
            ax = -atan2(matrix[0][1], matrix[1][1]);
        else
            ax = atan2(matrix[0][1], matrix[1][1]);

        az = 0.0;
    }
    else
    {
        ax = atan2(matrix[2][1], matrix[2][2]);
        az = atan2(matrix[1][0], matrix[0][0]);
    }

    angles[0] = ax * (180.0/M_PI);
    angles[1] = ay * (180.0/M_PI);
    angles[2] = az * (180.0/M_PI);
}


void M3x3_ExtractEulerAngles_YXZ(
  double matrix[3][3],
  double angles[3])
{
    double ax,ay,az;
    double cosX;

/*
   R[YXZ] = Ry * Rx * Rz

            | cy   0   sy |   | 1    0    0 |   | cz   -sz   0 |
          = |  0   1    0 | * | 0   cx  -sx | * | sz    cz   0 |
            |-sy   0   cy |   | 0   sx   cx |   |  0     0   1 |


            | cycz + sxsysz    -cysz + sxsycz      cxsy |
          = | cxsz              cxcz              -sx   |
            |-sycz + sxsysz     sysz + sxcycz      cxcy |
*/

    if (matrix[0][0] == XEMPTY)
    {
        angles[0] = XEMPTY;
        angles[1] = XEMPTY;
        angles[2] = XEMPTY;
        return;
    }

    cosX = sqrt(matrix[1][0]*matrix[1][0] + matrix[1][1]*matrix[1][1]);

    ax = atan2(-matrix[1][2], cosX);

    if (cosX < 0.0001)
    {
     // Gimble lock: set first angle to zero.
     // Assume sx=+-1,cx=0,sy=0,cy=1.0 and decode the matrix accordingly

        az = -atan2(matrix[0][1], matrix[0][0]);
        ay = 0.0;
    }
    else
    {
        az = atan2(matrix[1][0], matrix[1][1]);
        ay = atan2(matrix[0][2], matrix[2][2]);
    }

    angles[0] = ax * (180.0/M_PI);
    angles[1] = ay * (180.0/M_PI);
    angles[2] = az * (180.0/M_PI);
}


void M3x3_ExtractEulerAngles_YZX(
  double matrix[3][3],
  double angles[3])
{
    double ax,ay,az;
    double cosZ;

/*
   R[YZX] = Ry * Rz * Rx

             | cy   0   sy |   | cz   -sz   0 |   | 1    0    0 |
          =  |  0   1    0 | * | sz    cz   0 | * | 0   cx  -sx |
             |-sy   0   cy |   |  0     0   1 |   | 0   sx   cx |


            | cycz    -cxcysz + sxsy    sxcysz + cxsy |
          = |   sz     cxcz            -sxcz          |
            |-sycz     cxsysz + sxcy   -sxsysz + cxcy |
*/

    if (matrix[0][0] == XEMPTY)
    {
        angles[0] = XEMPTY;
        angles[1] = XEMPTY;
        angles[2] = XEMPTY;
        return;
    }

    cosZ = sqrt(matrix[0][0]*matrix[0][0] + matrix[2][0]*matrix[2][0]);

    az = atan2(matrix[1][0], cosZ);

    if (cosZ < 0.0001)
    {
     // Gimble lock: set first angle to zero.
     // Assume sz=+-1,cz=0,sy=0,cy=1.0 and decode the matrix accordingly

        ax = atan2(matrix[2][1], matrix[2][2]);
        ay = 0.0;
    }
    else
    {
        ax = atan2(-matrix[1][2], matrix[1][1]);
        ay = atan2(-matrix[2][0], matrix[0][0]);
    }

    angles[0] = ax * (180.0/M_PI);
    angles[1] = ay * (180.0/M_PI);
    angles[2] = az * (180.0/M_PI);
}


void M3x3_ExtractEulerAngles_ZXY(
  double matrix[3][3],
  double angles[3])
{
    double ax,ay,az;
    double cosX;

/*
   R[ZXY] = Rz * Rx * Ry

            | cz   -sz   0 |   | 1    0    0 |   | cy   0   sy |
          = | sz    cz   0 | * | 0   cx  -sx | * |  0   1    0 |
            |  0     0   1 |   | 0   sx   cx |   |-sy   0   cy |


            | cycz - sxsysz    -cxsz    sycz + sxcysz |
          = | cysz + sxsycz     cxcz    sysz - sxcycz |
            |-cxsy              sx      cxcy          |
*/

    if (matrix[0][0] == XEMPTY)
    {
        angles[0] = XEMPTY;
        angles[1] = XEMPTY;
        angles[2] = XEMPTY;
        return;
    }

    cosX = sqrt(matrix[0][1]*matrix[0][1] + matrix[1][1]*matrix[1][1]);

    ax = atan2(matrix[2][1], cosX);

    if (cosX < 0.0001)
    {
     // Gimble lock: set first angle to zero.
     // Assume sx=+-1,cx=0,sz=0,cz=1.0 and decode the matrix accordingly

        ay = atan2(matrix[0][2], matrix[0][0]);
        az = 0.0;
    }
    else
    {
        ay = atan2(-matrix[2][0], matrix[2][2]);
        az = atan2(-matrix[0][1], matrix[1][1]);
    }

    angles[0] = ax * (180.0/M_PI);
    angles[1] = ay * (180.0/M_PI);
    angles[2] = az * (180.0/M_PI);
}


void M3x3_ExtractEulerAngles_XZY(
  double matrix[3][3],
  double angles[3])
{
    double ax,ay,az;
    double cosZ;

/*
   R[XZY] = Rx * Rz * Ry

            | 1    0    0 |   | cz   -sz   0 |   | cy   0   sy |
          = | 0   cx  -sx | * | sz    cz   0 | * |  0   1    0 |
            | 0   sx   cx |   |  0     0   1 |   |-sy   0   cy |


            | cycz            -sz      sycz          |
          = | cxcysz + sxsy    cxcz    cxsysz - sxcz |
            | sxcysz - cxsy    sxcz    sxsysz + cxcy |
*/

    if (matrix[0][0] == XEMPTY)
    {
        angles[0] = XEMPTY;
        angles[1] = XEMPTY;
        angles[2] = XEMPTY;
        return;
    }

    cosZ = sqrt(matrix[2][1]*matrix[2][1] + matrix[1][1]*matrix[1][1]);

    az = atan2(-matrix[0][1], cosZ);

    if (cosZ < 0.0001)
    {
     // Gimble lock: set first angle to zero.
     // Assume sz=+-1,cz=0,sx=0,cx=1.0 and decode the matrix accordingly

        ay = atan2(-matrix[2][0], matrix[1][0]);
        ax = 0.0;
    }
    else
    {
//        ay = atan2(matrix[2][1], matrix[2][2]);
//        ax = atan2(matrix[2][1], matrix[1][1]);
        ay = atan2(matrix[0][2], matrix[0][0]);
        ax = atan2(matrix[2][1], matrix[1][1]);
    }

    angles[0] = ax * (180.0/M_PI);
    angles[1] = ay * (180.0/M_PI);
    angles[2] = az * (180.0/M_PI);
}


//==================================================================
// The following six variations are Tilt&Twist rotations
// The three angles returned are in the order given by the function name.
// For this function (YXY):
// The Tilt is Y,X,-Y. The twist is Y2. The matrix decoding combines
// the -Y,Y2 and then adjusts the final angle.
//------------------------------------------------------------------

void M3x3_ExtractEulerAngles_YZY(
  const double matrix[3][3],
  double angles[3])
{
    double ay1,az,ay2;

/*
   R[YZY] = Ry1 * Rz * Ry2

            | cy1   0  sy1 |   | cz  -sz  0 |   | cy2  0   sy2 |
          = |   0   1    0 | * | sz   cz  0 | * |   0  1     0 | 
            |-sy1   0  cy1 |   |  0    0  1 |   |-sy2  0   cy2 |   


          = |  cy1czcy2 - sy1sy2   -cy1sz    cy1czsy2 + sy1cy2 |
            |              szcy2       cz                szsy2 |
            | -sy1czcy2 - cy1sy2    sy1sz   -sy1czsy2 + cy1cy2 |
*/

    if (matrix[0][0] == XEMPTY)
    {
        angles[0] = XEMPTY;
        angles[1] = XEMPTY;
        angles[2] = XEMPTY;
        return;
    }

    if (ABS(matrix[2][2]) > 0.9999)
    {
        az = 0.0;
        ay1 = 0.0; // The ay1 rotation is only to accomplish the az rotation.
        ay2 = atan2(-matrix[2][0], matrix[2][2]); // because sy1=0 and cy1=1
    }
    else
    {
        az = acos(matrix[1][1]); // 0 <= ax <= 180 by definition
        ay1 = atan2(matrix[2][1], -matrix[0][1]);
        ay2 = atan2(matrix[1][2],  matrix[1][0]);
    }

    angles[0] = ay1 * (180.0/M_PI);
    angles[1] = az * (180.0/M_PI);
    angles[2] = (ay2+ay1) * (180.0/M_PI);
    //angles[2] = ay2 * (180.0/M_PI);

    if (angles[0] > +180.0) angles[0] -= 360.0;
    if (angles[0] < -180.0) angles[0] += 360.0;

    if (angles[2] > +180.0) angles[2] -= 360.0;
    if (angles[2] < -180.0) angles[2] += 360.0;
}


void M3x3_ExtractEulerAngles_YXY(
  const double matrix[3][3],
  double angles[3])
{
    double ay1,ax,ay2;

/*
   R[YXY] = Ry1 * Rx * Ry2

            | cy1   0   sy1 |   | 1    0    0 |   | cy2   0   sy2 |
          = |   0   1     0 | * | 0   cx  -sx | * |   0   1     0 |
            |-sy1   0   cy1 |   | 0   sx   cx |   |-sy2   0   cy2 |


            | cy1cy2 - sy1cxsy2    sy1sx    cy1sy2 + sy1cxcy2 |
          = |             sxsy2       cx               -sxcy2 |
            |-sy1cy2 - cy1cxsy2    cy1sx   -sy1sy2 + cy1cxcy2 |
*/

    if (matrix[0][0] == XEMPTY)
    {
        angles[0] = XEMPTY;
        angles[1] = XEMPTY;
        angles[2] = XEMPTY;
        return;
    }

    if (ABS(matrix[1][1]) > 0.9999)
    {
        ax = 0.0;
        ay1 = 0.0; // The ay1 rotation is only to accomplish the ax rotation.
        ay2 = atan2(matrix[0][2], matrix[0][0]);
    }
    else
    {
        ax = acos(matrix[1][1]); // 0 <= ax <= 180 by definition
        ay1 = atan2(matrix[0][1], matrix[2][1]);
        ay2 = atan2(matrix[1][0],-matrix[1][2]);
    }

    angles[0] = ay1 * (180.0/M_PI);
    angles[1] = ax * (180.0/M_PI);
    angles[2] = (ay2+ay1) * (180.0/M_PI);
    //angles[2] = ay2 * (180.0/M_PI);

    if (angles[0] > +180.0) angles[0] -= 360.0;
    if (angles[0] < -180.0) angles[0] += 360.0;

    if (angles[2] > +180.0) angles[2] -= 360.0;
    if (angles[2] < -180.0) angles[2] += 360.0;
}


void M3x3_ExtractEulerAngles_ZXZ(
  const double matrix[3][3],
  double angles[3])
{
    double az1,ax,az2;

/*
   R[ZXZ] = Rz1 * Rx * Rz2

            | cz1  -sz1   0 |   | 1    0    0 |   | cz2  -sz2  0 |
          = | sz1   cz1   0 | * | 0   cx  -sx | * | sz2   cz2  0 |
            |   0     0   1 |   | 0   sx   cx |   |   0   0    1 |


            | cz1cz2 - sz1cxsz2    -cz1sz2 - sz1cxdz2    sz1sx |
          = | sz1cz2 + cz1cxsz2    -sz1sz2 + cz1cxcz2   -cz1sx |
            |             sxsz2                 sxcz2       cx |
*/

    if (matrix[0][0] == XEMPTY)
    {
        angles[0] = XEMPTY;
        angles[1] = XEMPTY;
        angles[2] = XEMPTY;
        return;
    }

    if (ABS(matrix[2][2]) > 0.9999)
    {
        ax = 0.0;
        az1 = 0.0; // The az1 rotation is only to accomplish the ax rotation.
        az2 = atan2(-matrix[0][1], matrix[0][0]); // because cz1=1 and sz1=0
    }
    else
    {
        ax = acos(matrix[2][2]); // 0 <= ax <= 180 by definition
        az1 = atan2(matrix[0][2], -matrix[1][2]);
        az2 = atan2(matrix[2][0],  matrix[2][1]);
    }

    angles[0] = az1 * (180.0/M_PI);
    angles[1] = ax * (180.0/M_PI);
    angles[2] = (az2+az1) * (180.0/M_PI);
    //angles[2] = ay2 * (180.0/M_PI);

    if (angles[0] > +180.0) angles[0] -= 360.0;
    if (angles[0] < -180.0) angles[0] += 360.0;

    if (angles[2] > +180.0) angles[2] -= 360.0;
    if (angles[2] < -180.0) angles[2] += 360.0;
}



void M3x3_ExtractEulerAngles_ZYZ(
  const double matrix[3][3],
  double angles[3])
{
    double az1,ay,az2;

/*
   R[ZYZ] = Rz1 * Ry * Rz2

            | cz1  -sz1   0 |   | cy   0   sy |   | cz2  -sz2   0 |
          = | sz1   cz1   0 | * |  0   1    0 | * | sz2   cz2   0 |
            |   0     0   1 |   |-sy   0   cy |   |   0     0   1 |

            | -sz1sz2 + cz1cycz2   -sz1cz2 - cz1cysz2    cz1sy |
          = |  cz1sz2 + sz1cycz2    cz1cz2 - sz1cysz2    sz1sy |
            |             -sycz2                sysz2       cy |
*/

    if (matrix[0][0] == XEMPTY)
    {
        angles[0] = XEMPTY;
        angles[1] = XEMPTY;
        angles[2] = XEMPTY;
        return;
    }

    if (ABS(matrix[2][2]) > 0.9999)
    {
        ay = 0.0;
        az1 = 0.0; // The ay1 rotation is only to accomplish the ax rotation.
        az2 = atan2(matrix[1][0], matrix[1][1]);
    }
    else
    {
        ay = acos(matrix[2][2]); // 0 <= ay <= 180 by definition
        az1 = atan2(matrix[1][2], matrix[0][2]);
        az2 = atan2(matrix[2][1],-matrix[2][0]);
    }

    angles[0] = az1 * (180.0/M_PI);
    angles[1] = ay * (180.0/M_PI);
    angles[2] = (az2+az1) * (180.0/M_PI);
    //angles[2] = az2 * (180.0/M_PI);

    if (angles[0] > +180.0) angles[0] -= 360.0;
    if (angles[0] < -180.0) angles[0] += 360.0;

    if (angles[2] > +180.0) angles[2] -= 360.0;
    if (angles[2] < -180.0) angles[2] += 360.0;
}


void M3x3_ExtractEulerAngles_XYX(
  const double matrix[3][3],
  double angles[3])
{
    double ax1,ay,ax2;

/*
   R[XYX] = Rx1 * Ry * Rx2

            | 1     0     0 |   | cy   0  sy |   | 1     0     0 |
          = | 0   cx1  -sx1 | * |  0   1   0 | * | 0   cx2  -sx2 |
            | 0   sx1   cx1 |   |-sy   0  cy |   | 0   sx2   cx2 |


            |    cy                 sysx2                 sycx2 |
          = | sx1sy     cx1cx2 - sx1cysx2    -cx1sx2 - sx1cycx2 |
            |-cx1sy     sx1cx2 + cx1cysx2    -sx1sx2 + cx1cycx2 |
*/

    if (matrix[0][0] == XEMPTY)
    {
        angles[0] = XEMPTY;
        angles[1] = XEMPTY;
        angles[2] = XEMPTY;
        return;
    }

    if (ABS(matrix[2][2]) > 0.9999)
    {
        ay = 0.0;
        ax1 = 0.0; // The ax1 rotation is only to accomplish the ay rotation.
        ax2 = atan2(-matrix[1][2], matrix[1][1]); // because cx1=1 and sx1=0
    }
    else
    {
        ay = acos(matrix[0][0]); // 0 <= ax <= 180 by definition
        ax1 = atan2(matrix[1][0], -matrix[2][0]);
        ax2 = atan2(matrix[0][1],  matrix[0][2]);
    }

    angles[0] = ax1 * (180.0/M_PI);
    angles[1] = ay * (180.0/M_PI);
    angles[2] = (ax2+ax1) * (180.0/M_PI);
    //angles[2] = ay2 * (180.0/M_PI);

    if (angles[0] > +180.0) angles[0] -= 360.0;
    if (angles[0] < -180.0) angles[0] += 360.0;

    if (angles[2] > +180.0) angles[2] -= 360.0;
    if (angles[2] < -180.0) angles[2] += 360.0;
}


void M3x3_ExtractEulerAngles_XZX(
  const double matrix[3][3],
  double angles[3])
{
    double ax1,az,ax2;

/*
   R[XZX] = Rx1 * Rz * Rx2

            | 1     0     0 |   | cz   -sz   0 |   | 1     0     0 |
          = | 0   cx1  -sx1 | * | sz    cz   0 | * | 0   cx2  -sx2 |
            | 0   sx1   cx1 |   |  0     0   1 |   | 0   sx2   cx2 |

            |    cz                -szcx2                 szsx2 |
          = | cx1sz    -sx1sx2 + cx1czcx2    -sx1cx2 - cx1czsx2 |
            | sx1sz     cx1sx2 + sx1czcx2     cx1cx2 - sx1czsx2 |
*/

    if (matrix[0][0] == XEMPTY)
    {
        angles[0] = XEMPTY;
        angles[1] = XEMPTY;
        angles[2] = XEMPTY;
        return;
    }

    if (ABS(matrix[0][0]) > 0.9999)
    {
        az = 0.0;
        ax1 = 0.0; // The ay1 rotation is only to accomplish the ax rotation.
        ax2 = atan2(matrix[2][1], matrix[2][2]);
    }
    else
    {
        az = acos(matrix[0][0]); // 0 <= az <= 180 by definition
        ax1 = atan2(matrix[2][0], matrix[1][0]);
        ax2 = atan2(matrix[0][2],-matrix[0][1]);
    }

    angles[0] = ax1 * (180.0/M_PI);
    angles[1] = az * (180.0/M_PI);
    angles[2] = (ax2+ax1) * (180.0/M_PI);
    //angles[2] = ax2 * (180.0/M_PI);

    if (angles[2] > +180.0) angles[2] -= 360.0;
    if (angles[2] < -180.0) angles[2] += 360.0;

    if (angles[0] > +180.0) angles[0] -= 360.0;
    if (angles[0] < -180.0) angles[0] += 360.0;
}


double MakeAngleContinuous(double angle, double lastangle)
{
    while (angle > lastangle + 180.0)
        angle -= 360.0;

    while (angle < lastangle - 180.0)
        angle += 360.0;

    return angle;
}


void M3x3_ExtractAndCorrectEulerAngles_ZYX(
  double matrix[3][3],
  double prevangles[3],
  double angles[3])
{
    M3x3_ExtractEulerAngles_ZYX(matrix, angles);

    angles[0] = MakeAngleContinuous(angles[0], prevangles[0]);
    angles[1] = MakeAngleContinuous(angles[1], prevangles[1]);
    angles[2] = MakeAngleContinuous(angles[2], prevangles[2]);
}


void M3x3_ExtractAndCorrectEulerAngles_XYZ(
  double matrix[3][3],
  double prevangles[3],
  double angles[3])
{
    M3x3_ExtractEulerAngles_XYZ(matrix, angles);

    angles[0] = MakeAngleContinuous(angles[0], prevangles[0]);
    angles[1] = MakeAngleContinuous(angles[1], prevangles[1]);
    angles[2] = MakeAngleContinuous(angles[2], prevangles[2]);
}


void M3x3_ExtractEulerAngles(
  double matrix[3][3],
  int   iRotationOrder,
  double angles[3])
{
    switch (iRotationOrder)
    {
    case ZYX_ORDER:
        M3x3_ExtractEulerAngles_ZYX(matrix, angles);
        break;

    case XYZ_ORDER:
        M3x3_ExtractEulerAngles_XYZ(matrix, angles);
        break;

    case YXZ_ORDER:
        M3x3_ExtractEulerAngles_YXZ(matrix, angles);
        break;

    case YZX_ORDER:
        M3x3_ExtractEulerAngles_YZX(matrix, angles);
        break;

    case XZY_ORDER:
        M3x3_ExtractEulerAngles_XZY(matrix, angles);
        break;

    case ZXY_ORDER:
        M3x3_ExtractEulerAngles_ZXY(matrix, angles);
        break;

    case YXY_ORDER:
        M3x3_ExtractEulerAngles_YXY(matrix, angles);
        break;
    }
}


void M3x3_ExtractAndCorrectEulerAngles(
  double matrix[3][3],
  int    iRotationOrder,
  double prevangles[3],
  double angles[3])
{
    M3x3_ExtractEulerAngles(matrix, iRotationOrder, angles);

    angles[0] = MakeAngleContinuous(angles[0], prevangles[0]);
    angles[1] = MakeAngleContinuous(angles[1], prevangles[1]);
    angles[2] = MakeAngleContinuous(angles[2], prevangles[2]);
}


void M3x3_ConstructRotationMatrix(
  double ax,
  double ay,
  double az,
  int iRotationOrder,
  double matrix[3][3])
{
    M3x3_LoadIdentity(matrix);

    switch (iRotationOrder)
    {
    case XYZ_ORDER:
        M3x3_RotateZ(matrix, az, matrix);
        M3x3_RotateY(matrix, ay, matrix);
        M3x3_RotateX(matrix, ax, matrix);
        break;

    case XZY_ORDER:
        M3x3_RotateY(matrix, ay, matrix);
        M3x3_RotateZ(matrix, az, matrix);
        M3x3_RotateX(matrix, ax, matrix);
        break;

    case YXZ_ORDER:
        M3x3_RotateZ(matrix, az, matrix);
        M3x3_RotateX(matrix, ax, matrix);
        M3x3_RotateY(matrix, ay, matrix);
        break;

    case YZX_ORDER:
        M3x3_RotateX(matrix, ax, matrix);
        M3x3_RotateZ(matrix, az, matrix);
        M3x3_RotateY(matrix, ay, matrix);
        break;

    case ZXY_ORDER:
        M3x3_RotateY(matrix, ay, matrix);
        M3x3_RotateX(matrix, ax, matrix);
        M3x3_RotateZ(matrix, az, matrix);
        break;

    case ZYX_ORDER:
        M3x3_RotateX(matrix, ax, matrix);
        M3x3_RotateY(matrix, ay, matrix);
        M3x3_RotateZ(matrix, az, matrix);
        break;

    default:
//        PopupNotice("Unsupported rotation sequence\n");
        break;
    }
}

/*{==============================================================================
//----------------------------------------------------------------------------}*/

int M3x3_BuildVMarkerRotationMatrix(
    double p0[3], // origin marker
    double p1[3], // long axis marker
    double p2[3], // plane marker
    double m[3][3])
{
    double dist;
    double dx, dy, dz;
    double v1[3];
    double v2[3];
    double v3[3];

 // Here's some basic vector algebra. Figure it out.

 // First unit vector

    dx = p1[0] - p0[0];
    dy = p1[1] - p0[1];
    dz = p1[2] - p0[2];

    dist = sqrt(dx*dx + dy*dy + dz*dz);
    if (dist == 0)
    {
        m[0][0] = XEMPTY;
        return ERRFLAG;
    }

    v1[0] = dx / dist;
    v1[1] = dy / dist;
    v1[2] = dz / dist;


 // Second unit vector

    dx = p2[0] - p0[0];
    dy = p2[1] - p0[1];
    dz = p2[2] - p0[2];

    dist = sqrt(dx*dx + dy*dy + dz*dz);
    if (dist == 0)
    {
        m[0][0] = XEMPTY;
        return ERRFLAG;
    }

    v2[0] = dx / dist;
    v2[1] = dy / dist;
    v2[2] = dz / dist;


 // Cross product v3 = Normalized (v1 X v2) give perpendicular vector

    dx = v1[1]*v2[2] - v1[2]*v2[1];
    dy = v1[2]*v2[0] - v1[0]*v2[2];
    dz = v1[0]*v2[1] - v1[1]*v2[0];

    dist = sqrt(dx*dx + dy*dy + dz*dz);
    if (dist == 0)
    {
        m[0][0] = XEMPTY;
        return ERRFLAG;
    }

    v3[0] = dx / dist;
    v3[1] = dy / dist;
    v3[2] = dz / dist;


 // Note: v2 is the only vector not completed as perpendicular
 // Cross product v2 = Normalized (v3 X v1) makes the final perpendicular vector

    dx = v3[1]*v1[2] - v3[2]*v1[1];
    dy = v3[2]*v1[0] - v3[0]*v1[2];
    dz = v3[0]*v1[1] - v3[1]*v1[0];

    dist = sqrt(dx*dx + dy*dy + dz*dz);

    v2[0] = dx / dist;
    v2[1] = dy / dist;
    v2[2] = dz / dist;


 // OK, now fill in the matrix

    m[0][0] = v1[0];
    m[1][0] = v1[1];
    m[2][0] = v1[2];

    m[0][1] = v2[0];
    m[1][1] = v2[1];
    m[2][1] = v2[2];

    m[0][2] = v3[0];
    m[1][2] = v3[1];
    m[2][2] = v3[2];

    return OK;
}

