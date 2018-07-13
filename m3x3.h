/*=========================================================
//
// File: M3x3.h  v100
//
// Created by Ned Phipps, Nov-98
//
// This file defines functions to create and decode
// Euler angle rotation matrices.
//
//------------------------------------------------------*/

#ifndef M3x3_H
#define M3x3_H

#define X_AXIS  0
#define Y_AXIS  1
#define Z_AXIS  2

#define ZYX_ORDER 1
#define XYZ_ORDER 2
#define YXZ_ORDER 3
#define YZX_ORDER 4
#define ZXY_ORDER 5
#define XZY_ORDER 6

// Special rotation orders
#define XYX_ORDER 7
#define XZX_ORDER 8
#define YZY_ORDER 9
#define YXY_ORDER 10
#define ZXZ_ORDER 11
#define ZYZ_ORDER 12

void  M3x3_MulMatrix(const double m1[3][3], const double m2[3][3], double p[3][3]);
void  M3x3_MulVector(const double m[3][3], const double v[3], double p[3]);
void  M3x3_MulVector(const double v[3], const double m[3][3], double p[3]);
void  M3x3_LoadIdentity(double m[3][3]);
void  M3x3_Transpose(double m[3][3]);
void  M3x3_Copy(double src[3][3], double dst[3][3]);
void  M3x3_RotateX(double input[3][3], double degrees, double result[3][3]);
void  M3x3_RotateY(double input[3][3], double degrees, double result[3][3]);
void  M3x3_RotateZ(double input[3][3], double degrees, double result[3][3]);

void  M3x3_ExtractEulerAngles(
        double matrix[3][3],
        int    iRotationOrder,
        double angles[3]);

void  M3x3_ExtractAndCorrectEulerAngles(
        double matrix[3][3],
        int    iRotationOrder,
        double prevangles[3],
        double angles[3]);

void  M3x3_ConstructRotationMatrix(
        double ax,
        double ay,
        double az,
        int iRotationOrder,
        double matrix[3][3]);

int M3x3_BuildVMarkerRotationMatrix(
        double p0[3], // origin marker
        double p1[3], // long axis marker
        double p2[3], // plane marker
        double matrix[3][3]);

double MakeAngleContinuous(double angle, double prevangle);

#endif
