/******************************************************************************

envmaprender.in: Shaders used for rendering irradiance environment
                 maps with spherical harmonic coefficients.  This file 
		 is designed to be used with the Stanford Real-Time
		 programmable shading system.  This system is available 
		 from http://graphics.stanford.edu/projects/shading/

Other files    : The other files required are grace.scene which
                 defines a simple scene.  You will also need to add
		 the following 2 lines to scviewer.in when using scviewer

		 file envmaprender.in
		 scene grace.scene

Functions      : We provide general functions for rendering, plus the
                 spherical harmonic coefficients for the Grace
		 Cathedral as an example.  These coefficients were
		 generated using prefilter.c  from the environment 
		 available from http://www.debevec.org

Usage          : When running scviewer, simply choose the scene Grace
                 Cathedral.  It should be fairly obvious how to modify 
		 the source to handle your own environments.

Reference      : This is an implementation of the method described by
                 Ravi Ramamoorthi and Pat Hanrahan in their SIGGRAPH 2001 
	         paper, "An Efficient Representation for Irradiance
	         Environment Maps".

Author         : Ravi Ramamoorthi

Date           : Last modified on April 14, 2001

******************************************************************************/

// irradmat is an implementation of equation 11 in our paper.  The
// inputs are the matrix M for one color channel and a vector
// corresponding to the surface normal n.  The output is the irradiance.

surface float1 irradmat (matrix4 M, float3 v)
{
    float4 n = { v, 1 };
    return dot(n, M * n);
}

// irradcoeffs is an alternative form, corresponding to equation 13 in 
// our paper.  The inputs are 9 RGB values (i.e. 3 vectors) for the 9
// spherical harmonic lighting coefficients.  The vector n is the
// surface normal.

surface float3 irradcoeffs ( 
      float3 L00, float3 L1_1, float3 L10, float3 L11, 
      float3 L2_2, float3 L2_1, float3 L20, float3 L21, float3 L22,
      float3 n) 
{
  //------------------------------------------------------------------
  // These are variables to hold x,y,z and squares and products

	float1 x2 ;
	float1  y2 ;
	float1 z2 ;
	float1 xy ;
	float1  yz ;
	float1  xz ;
	float1 x ;
	float1 y ;
	float1 z ;
	float3 col ;
  //------------------------------------------------------------------       
  // We now define the constants and assign values to x,y, and z 
	
	constant float1 c1 = 0.429043 ;
	constant float1 c2 = 0.511664 ;
	constant float1 c3 = 0.743125 ;
	constant float1 c4 = 0.886227 ;
	constant float1 c5 = 0.247708 ;
	x = n[0] ; y = n[1] ; z = n[2] ;
  //------------------------------------------------------------------ 
  // We now compute the squares and products needed 

	x2 = x*x ; y2 = y*y ; z2 = z*z ;
	xy = x*y ; yz = y*z ; xz = x*z ;
  //------------------------------------------------------------------ 
  // Finally, we compute equation 13

	col = c1*L22*(x2-y2) + c3*L20*z2 + c4*L00 - c5*L20 
            + 2*c1*(L2_2*xy + L21*xz + L2_1*yz) 
            + 2*c2*(L11*x+L1_1*y+L10*z) ;

	return col ;
}

// irrad defines a surface shader that uses the
// matrix form irradmat above.  albedo is a constant diffuse intensity; it is 
// straightforward to extend this to handle textures.  
// viewmat is the view matrix that needs to be undone to get the
// normal in world coordinates.  

surface shader float4 irrad (matrix4 Mr, matrix4 Mg, matrix4 Mb,
			     float1 albedo, matrix4 viewmat)
{
    float3 n = invert(affine(viewmat))*N ; // normal in world coords

    return { albedo*irradmat(Mr, n), 
	     albedo*irradmat(Mg, n), 
	     albedo*irradmat(Mb, n) , 1.0 };
}

// We now define all the coefficients for the irradiance environment
// map corresponding to the Grace Cathedral.  These coefficients are
// obtained using prefilter

constant float3 grace00  = { 0.078908,  0.043710,  0.054161 } ;
constant float3 grace1_1 = { 0.039499,  0.034989,  0.060488 } ;
constant float3 grace10  = {-0.033974, -0.018236, -0.026940 } ;
constant float3 grace11  = {-0.029213, -0.005562,  0.000944 } ;
constant float3 grace2_2 = {-0.011141, -0.005090, -0.012231 } ;
constant float3 grace2_1 = {-0.026240, -0.022401, -0.047479 } ;
constant float3 grace20  = {-0.015570, -0.009471, -0.014733 } ;
constant float3 grace21  = { 0.056014,  0.021444,  0.013915 } ;
constant float3 grace22  = { 0.021205, -0.005432, -0.030374 } ;

constant matrix4 gracered = {
  { 0.009098, -0.004780,  0.024033, -0.014947 },
  {-0.004780, -0.009098, -0.011258,  0.020210 },
  { 0.024033, -0.011258, -0.011570, -0.017383 },
  {-0.014947,  0.020210, -0.017383,  0.073787 }
} ;
constant matrix4 gracegreen = {
  {-0.002331, -0.002184,  0.009201, -0.002846 },
  {-0.002184,  0.002331, -0.009611,  0.017903 },
  { 0.009201, -0.009611, -0.007038, -0.009331 },
  {-0.002846,  0.017903, -0.009331,  0.041083 }
} ;
constant matrix4 graceblue = {
  {-0.013032, -0.005248,  0.005970,  0.000483 },
  {-0.005248,  0.013032, -0.020370,  0.030949 },
  { 0.005970, -0.020370, -0.010948, -0.013784 },
  { 0.000483,  0.030949, -0.013784,  0.051648 }
} ;

// Finally, for convenience, we define a shader specialized to the
// Grace Cathedral environment.  We could also simply use irradmat with the
// matrices as parameters instead.  

surface shader float4 grace (float1 albedo, matrix4 viewmat) 
{
  float3 n = invert(affine(viewmat))*N ; // normal in world coords

  return { albedo*irradmat (gracered,   n), 
	   albedo*irradmat (gracegreen, n), 
	   albedo*irradmat (graceblue,  n), 
           1.0 } ;
}

