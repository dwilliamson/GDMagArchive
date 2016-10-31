#include <iostream.h>
#include <fstream.h>
#include <stdlib.h>

#include "global.h"
#include "point.h"
#include "brdf_lib.h"

static bool doClamping = 1;

void setClamping( bool val ) {
    doClamping = val;
}

Double inline SQR( Double x ) {
    return (x*x);
}

Double gaussianBRDF( Double th_i, Double phi_i, Double th_r, Double phi_r,
		     Double f_d, Double f_s, Double ax, Double ay ) {
    
    // Copy from the bv-1.2p program done by (c) Rusinkiewicz

    if( doClamping && brdf > 5.0 ) brdf = 5.0;

    return brdf;
}

Double gaussian( Double th_i, Double phi_i, Double th_r, Double phi_r ) {
    return gaussianBRDF( th_i, phi_i, th_r, phi_r, 0.33, 0.19, 0.088, 0.13 );
}

// Copied from the bv-1.2p program done by Rusinkiewicz
Double phongBRDF( Double th_i, Double phi_i, Double th_o, Double phi_o,
		  Double diffuse, Double specular, Double n) {
    //Vector  in( sin(th_i)*cos(phi_i), sin(th_i)*sin(phi_i), cos(th_i) );
    Vector  in_r( sin(th_i)*cos(phi_i+M_PI), sin(th_i)*sin(phi_i+M_PI), cos(th_i) );
    Vector out( sin(th_o)*cos(phi_o), sin(th_o)*sin(phi_o), cos(th_o) );
    
    //    in.normalize();
    //    out.normalize();

    Vector N( 0.0, 0.0, 1.0 );
    //    Vector in_r = N*2.0*(N|in) - in;
    Double costheta = in_r|out;

    costheta = MAX(0.0,costheta);  // CLAMP to [0,1]

    // Let's try phong

    /* diffuse + specular <= 1.0 then it is energy conserving
       Double diffuse = 0.3;
       Double specular = 0.7;
       Double n = 6.0;
    */

    Double brdf;

    // phong (modified a la Fortune, to be energy conserving!)
    brdf = diffuse/M_PI + specular*pow(costheta,n)*(n+2.0)/(2*M_PI);

    if( doClamping && brdf > 5.0 ) brdf = 5.0;

    return brdf;
}

Double phong( Double th_i, Double phi_i, Double th_o, Double phi_o ) {
    return phongBRDF( th_i, phi_i, th_o, phi_o, 0.3, 0.7, 6.0 );
}

// working correctly ?
Double nonLinearPhongBRDF( Double th_i, Double phi_i,
			   Double th_o, Double phi_o,
			   Double Cx, Double Cy, Double Cz,
			   Double diffuse, Double specular, Double n ) {

    Vector  in( sin(th_i)*cos(phi_i), sin(th_i)*sin(phi_i), cos(th_i) );
    Vector out( sin(th_o)*cos(phi_o), sin(th_o)*sin(phi_o), cos(th_o) );
    
    in.normalize();
    out.normalize();

    Double cosalpha = Cx*in[0]*out[0] + Cy*in[1]*out[1] + Cz*in[2]*out[2];

    Double brdf = diffuse/M_PI + specular*pow(cosalpha,n);

    if( doClamping && brdf > 5.0 ) brdf = 5.0;

    return brdf;
}

Double cylinderBRDF( Double theta_in, Double phi_in,
		     Double theta_out, Double phi_out,
		     Double d, Double h, Double n,
		     Double rs, Double rd )
{
  // Copy from the bv-1.2p program done by (c) Rusinkiewicz
  
    if( doClamping && val > 5.0 ) val = 5.0;

    if( val < 0.0 ) val = 0.0;

    if( isnan(val) ) val = 0.0;

    return val;
}

Double cylinder( Double theta_in, Double phi_in,
		 Double theta_out, Double phi_out )
{
    return cylinderBRDF( theta_in, phi_in, theta_out, phi_out,
			 2.3, 0, 100, 0.8, 0.2 );
}

const Cardinal nrCuretSamplesRead = 205;
const Cardinal nrCuretSamples = nrCuretSamplesRead*2;

Double curetOut[nrCuretSamples][2] = {
    // theta_o, phi_o
    { 1.389566, -1.970402 },
    { 0.981748, 0.000000 },
    { 1.463021, -2.763231 },
    { 1.308097, -1.163336 },
    { 0.987375, -0.134267 },
    { 1.215585, -0.811489 },
    { 1.007874, -0.282959 },
    { 1.141555, -0.627520 },
    { 1.050598, -0.417031 },
    { 1.209430, -1.992270 },
    { 0.785399, 0.000000 },
    { 1.272499, -2.496589 },
    { 1.099841, -1.476736 },
    { 0.774850, -0.219877 },
    { 1.348016, -2.815465 },
    { 1.008179, -1.101321 },
    { 0.809816, -0.439945 },
    { 1.421060, -3.015733 },
    { 0.877978, -0.762419 },
    { 1.374447, 3.141593 },
    { 1.031794, -2.032979 },
    { 0.589049, 0.000000 },
    { 1.337086, -3.090099 },
    { 1.101194, -2.432726 },
    { 0.909333, -1.623802 },
    { 0.582273, -0.334581 },
    { 1.128806, -2.736898 },
    { 0.716526, -1.270836 },
    { 1.217332, -2.966775 },
    { 0.581425, -0.799895 },
    { 1.178097, 3.141593 },
    { 0.858886, -2.100699 },
    { 0.392699, 0.000000 },
    { 1.038285, -3.013835 },
    { 0.866142, -2.496546 },
    { 0.627477, -1.794991 },
    { 0.318893, -0.713993 },
    { 0.929649, -2.801295 },
    { 0.426124, -1.398445 },
    { 0.981747, 3.141593 },
    { 0.694806, -2.211453 },
    { 0.196349, 0.000000 },
    { 0.839669, -2.948852 },
    { 0.733813, -2.628785 },
    { 0.445325, -1.955310 },
    { 0.213063, -1.410017 },
    { 0.785398, 3.141593 },
    { 0.548028, -2.395741 },
    { 0.000001, 3.141593 },
    { 0.625671, -2.857075 },
    { 0.274014, -2.335623 },
    { 0.589048, 3.141593 },
    { 0.436759, -2.701422 },
    { 0.196350, 3.141593 },
    { 0.392699, 3.141593 },
    { 1.432405, -2.365895 },
    { 0.589049, 0.000000 },
    { 1.285872, -1.570796 },
    { 0.610885, -0.293831 },
    { 1.122109, -1.226629 },
    { 0.680547, -0.578211 },
    { 0.979211, -1.038908 },
    { 0.785398, -0.785398 },
    { 1.296783, -2.395740 },
    { 0.392700, 0.000000 },
    { 1.431921, -2.864769 },
    { 1.122109, -1.914964 },
    { 0.413656, -0.508053 },
    { 0.955317, -1.570796 },
    { 0.523599, -0.886076 },
    { 0.717004, -1.267733 },
    { 1.167071, -2.447955 },
    { 0.196350, 0.000000 },
    { 1.308997, -2.790714 },
    { 0.979211, -2.102685 },
    { 0.261800, -0.938882 },
    { 1.419463, -3.014362 },
    { 0.717004, -1.873860 },
    { 0.440510, -1.570796 },
    { 1.047198, -2.526113 },
    { 0.000000, 0.000000 },
    { 1.420805, -3.108912 },
    { 1.150262, -2.819842 },
    { 0.785398, -2.356194 },
    { 0.261800, -2.202711 },
    { 1.277953, -3.006341 },
    { 0.523599, -2.255517 },
    { 1.374446, 3.141593 },
    { 0.942289, -2.634483 },
    { 0.196350, 3.141593 },
    { 1.220213, -3.063418 },
    { 1.068004, -2.895807 },
    { 0.680547, -2.563381 },
    { 0.413656, -2.633539 },
    { 1.178097, 3.141593 },
    { 0.858886, -2.776103 },
    { 0.392700, 3.141593 },
    { 1.002667, -3.011773 },
    { 0.610885, -2.847762 },
    { 0.981747, 3.141593 },
    { 0.804433, -2.948923 },
    { 0.589049, 3.141593 },
    { 0.785398, 3.141593 },
    { 0.196350, 0.000000 },
    { 1.308097, -1.978257 },
    { 0.274014, -0.805969 },
    { 1.099841, -1.664856 },
    { 0.445325, -1.186283 },
    { 0.909333, -1.517790 },
    { 0.627477, -1.346601 },
    { 1.423821, -2.776103 },
    { 0.000001, 0.000000 },
    { 1.215585, -2.330104 },
    { 0.213063, -1.731576 },
    { 1.008179, -2.040271 },
    { 0.426124, -1.743148 },
    { 0.716526, -1.870756 },
    { 1.356554, -2.809910 },
    { 0.196349, 3.141593 },
    { 1.141555, -2.514072 },
    { 0.318893, -2.427600 },
    { 0.877978, -2.379174 },
    { 0.581425, -2.341698 },
    { 1.296782, -2.856669 },
    { 0.392699, 3.141593 },
    { 1.464020, -3.065729 },
    { 1.050598, -2.724561 },
    { 0.582273, -2.807012 },
    { 0.809816, -2.701648 },
    { 1.246977, -2.915406 },
    { 0.589049, 3.141593 },
    { 1.422492, -3.075442 },
    { 1.007874, -2.858633 },
    { 0.774850, -2.921715 },
    { 1.209429, -2.984388 },
    { 0.785399, 3.141593 },
    { 1.386121, -3.103699 },
    { 0.987375, -3.007326 },
    { 1.374447, 3.141593 },
    { 1.186043, -3.060959 },
    { 0.981748, 3.141593 },
    { 1.178097, 3.141593 },
    { 0.196349, 3.141593 },
    { 1.370707, -2.376765 },
    { 0.284924, -2.376765 },
    { 1.154239, -2.096192 },
    { 0.472749, -2.096192 },
    { 0.955317, -2.005315 },
    { 0.662145, -2.005315 },
    { 0.392699, 3.141593 },
    { 1.360401, -2.710826 },
    { 0.472749, -2.710826 },
    { 1.150262, -2.456873 },
    { 0.615479, -2.456873 },
    { 0.876815, -2.376909 },
    { 0.589048, 3.141593 },
    { 1.357866, -2.860557 },
    { 0.662145, -2.860557 },
    { 1.130287, -2.738536 },
    { 0.876815, -2.738536 },
    { 0.785398, 3.141593 },
    { 1.357866, -2.988109 },
    { 0.955317, -2.988109 },
    { 1.150262, -2.940235 },
    { 0.981748, 3.141593 },
    { 1.360401, -3.046963 },
    { 1.154239, -3.046963 },
    { 1.178098, 3.141593 },
    { 1.370707, -3.100452 },
    { 1.374447, 3.141593 },
    { 0.589048, 3.141593 },
    { 1.463021, -2.763231 },
    { 0.625671, -2.857075 },
    { 1.272499, -2.496589 },
    { 0.733813, -2.628785 },
    { 1.101194, -2.432726 },
    { 0.866142, -2.496546 },
    { 0.785398, 3.141593 },
    { 0.839669, -2.948852 },
    { 1.348016, -2.815465 },
    { 0.929649, -2.801295 },
    { 1.128806, -2.736898 },
    { 0.981747, 3.141593 },
    { 1.038285, -3.013835 },
    { 1.421060, -3.015733 },
    { 1.217332, -2.966775 },
    { 1.178097, 3.141593 },
    { 1.337086, -3.090099 },
    { 1.374447, 3.141593 },
    { 0.981747, 3.141593 },
    { 1.002667, -3.011773 },
    { 1.431921, -2.864769 },
    { 1.068004, -2.895807 },
    { 1.308997, -2.790714 },
    { 1.150262, -2.819842 },
    { 1.178097, 3.141593 },
    { 1.220213, -3.063418 },
    { 1.277953, -3.006341 },
    { 1.419463, -3.014362 },
    { 1.374446, 3.141593 },
    { 1.420805, -3.108912 },
    { 1.374447, 3.141593 },
    { 1.386121, -3.103699 },
    { 1.422492, -3.075442 },
    { 1.464020, -3.065729 }
};

Double curetIn[nrCuretSamples][2] = {
    // theta_i, phi_i
    { 1.374447, -1.570796 },
    { 1.374447, 0.000000 },
    { 1.370707, -2.376765 },
    { 1.370707, -0.764828 },
    { 1.370707, -0.041141 },
    { 1.360401, -0.430767 },
    { 1.360401, -0.094629 },
    { 1.357866, -0.281035 },
    { 1.357866, -0.153484 },
    { 1.178098, -1.570796 },
    { 1.178098, 0.000000 },
    { 1.154239, -2.096192 },
    { 1.154239, -1.045400 },
    { 1.154239, -0.094629 },
    { 1.150262, -2.456873 },
    { 1.150262, -0.684719 },
    { 1.150262, -0.201358 },
    { 1.130287, -2.738536 },
    { 1.130287, -0.403056 },
    { 0.981748, 3.141593 },
    { 0.981748, -1.570796 },
    { 0.981748, 0.000000 },
    { 0.955317, -2.988109 },
    { 0.955317, -2.005315 },
    { 0.955317, -1.136277 },
    { 0.955317, -0.153484 },
    { 0.876815, -2.376909 },
    { 0.876815, -0.764684 },
    { 0.876815, -2.738536 },
    { 0.876815, -0.403056 },
    { 0.785398, 3.141593 },
    { 0.785398, -1.570796 },
    { 0.785398, 0.000000 },
    { 0.662145, -2.860557 },
    { 0.662145, -2.005315 },
    { 0.662145, -1.136277 },
    { 0.662145, -0.281035 },
    { 0.615479, -2.456873 },
    { 0.615479, -0.684719 },
    { 0.589048, 3.141593 },
    { 0.589048, -1.570796 },
    { 0.589048, 0.000000 },
    { 0.472749, -2.710826 },
    { 0.472749, -2.096192 },
    { 0.472749, -1.045400 },
    { 0.472749, -0.430767 },
    { 0.392699, 3.141593 },
    { 0.392699, -1.570796 },
    { 0.392699, 0.000000 },
    { 0.284924, -2.376765 },
    { 0.284924, -0.764828 },
    { 0.196349, 3.141593 },
    { 0.196349, -1.570796 },
    { 0.196349, 0.000000 },
    { 0.000000, 0.000000 },
    { 1.374447, -1.570796 },
    { 1.374447, 0.000000 },
    { 1.370707, -0.764828 },
    { 1.370707, -0.041141 },
    { 1.360401, -0.430767 },
    { 1.360401, -0.094629 },
    { 1.357866, -0.281035 },
    { 1.357866, -0.153484 },
    { 1.178098, -1.570796 },
    { 1.178098, 0.000000 },
    { 1.154239, -2.096192 },
    { 1.154239, -1.045400 },
    { 1.154239, -0.094629 },
    { 1.150262, -0.684719 },
    { 1.150262, -0.201358 },
    { 1.130287, -0.403056 },
    { 0.981748, -1.570796 },
    { 0.981748, 0.000000 },
    { 0.955317, -2.005315 },
    { 0.955317, -1.136277 },
    { 0.955317, -0.153484 },
    { 0.876815, -2.376909 },
    { 0.876815, -0.764684 },
    { 0.876815, -0.403056 },
    { 0.785398, -1.570796 },
    { 0.785398, 0.000000 },
    { 0.662145, -2.860557 },
    { 0.662145, -2.005315 },
    { 0.662145, -1.136277 },
    { 0.662145, -0.281035 },
    { 0.615479, -2.456873 },
    { 0.615479, -0.684719 },
    { 0.589048, 3.141593 },
    { 0.589048, -1.570796 },
    { 0.589048, 0.000000 },
    { 0.472749, -2.710826 },
    { 0.472749, -2.096192 },
    { 0.472749, -1.045400 },
    { 0.472749, -0.430767 },
    { 0.392699, 3.141593 },
    { 0.392699, -1.570796 },
    { 0.392699, 0.000000 },
    { 0.284924, -2.376765 },
    { 0.284924, -0.764828 },
    { 0.196349, 3.141593 },
    { 0.196349, -1.570796 },
    { 0.196349, 0.000000 },
    { 0.000000, 0.000000 },
    { 1.374447, 0.000000 },
    { 1.370707, -0.764828 },
    { 1.370707, -0.041141 },
    { 1.360401, -0.430767 },
    { 1.360401, -0.094629 },
    { 1.357866, -0.281035 },
    { 1.357866, -0.153484 },
    { 1.178098, -1.570796 },
    { 1.178098, 0.000000 },
    { 1.154239, -1.045400 },
    { 1.154239, -0.094629 },
    { 1.150262, -0.684719 },
    { 1.150262, -0.201358 },
    { 1.130287, -0.403056 },
    { 0.981748, -1.570796 },
    { 0.981748, 0.000000 },
    { 0.955317, -1.136277 },
    { 0.955317, -0.153484 },
    { 0.876815, -0.764684 },
    { 0.876815, -0.403056 },
    { 0.785398, -1.570796 },
    { 0.785398, 0.000000 },
    { 0.662145, -2.005315 },
    { 0.662145, -1.136277 },
    { 0.662145, -0.281035 },
    { 0.615479, -0.684719 },
    { 0.589048, -1.570796 },
    { 0.589048, 0.000000 },
    { 0.472749, -2.096192 },
    { 0.472749, -1.045400 },
    { 0.472749, -0.430767 },
    { 0.392699, -1.570796 },
    { 0.392699, 0.000000 },
    { 0.284924, -2.376765 },
    { 0.284924, -0.764828 },
    { 0.196349, 3.141593 },
    { 0.196349, -1.570796 },
    { 0.196349, 0.000000 },
    { 0.000000, 0.000000 },
    { 1.374447, 0.000000 },
    { 1.370707, -0.764828 },
    { 1.370707, -0.041141 },
    { 1.360401, -0.430767 },
    { 1.360401, -0.094629 },
    { 1.357866, -0.281035 },
    { 1.357866, -0.153484 },
    { 1.178098, 0.000000 },
    { 1.154239, -1.045400 },
    { 1.154239, -0.094629 },
    { 1.150262, -0.684719 },
    { 1.150262, -0.201358 },
    { 1.130287, -0.403056 },
    { 0.981748, 0.000000 },
    { 0.955317, -1.136277 },
    { 0.955317, -0.153484 },
    { 0.876815, -0.764684 },
    { 0.876815, -0.403056 },
    { 0.785398, 0.000000 },
    { 0.662145, -1.136277 },
    { 0.662145, -0.281035 },
    { 0.615479, -0.684719 },
    { 0.589048, 0.000000 },
    { 0.472749, -1.045400 },
    { 0.472749, -0.430767 },
    { 0.392699, 0.000000 },
    { 0.284924, -0.764828 },
    { 0.196349, 0.000000 },
    { 1.374447, 0.000000 },
    { 1.370707, -0.764828 },
    { 1.370707, -0.041141 },
    { 1.360401, -0.430767 },
    { 1.360401, -0.094629 },
    { 1.357866, -0.281035 },
    { 1.357866, -0.153484 },
    { 1.178098, 0.000000 },
    { 1.154239, -0.094629 },
    { 1.150262, -0.684719 },
    { 1.150262, -0.201358 },
    { 1.130287, -0.403056 },
    { 0.981748, 0.000000 },
    { 0.955317, -0.153484 },
    { 0.876815, -0.764684 },
    { 0.876815, -0.403056 },
    { 0.785398, 0.000000 },
    { 0.662145, -0.281035 },
    { 0.589048, 0.000000 },
    { 1.374447, 0.000000 },
    { 1.370707, -0.041141 },
    { 1.360401, -0.430767 },
    { 1.360401, -0.094629 },
    { 1.357866, -0.281035 },
    { 1.357866, -0.153484 },
    { 1.178098, 0.000000 },
    { 1.154239, -0.094629 },
    { 1.150262, -0.201358 },
    { 1.130287, -0.403056 },
    { 0.981748, 0.000000 },
    { 0.955317, -0.153484 },
    { 1.374447, 0.000000 },
    { 1.370707, -0.041141 },
    { 1.360401, -0.094629 },
    { 1.357866, -0.153484 }
};

Double curetSamples[nrCuretSamples];
Double curetSamplesRed[nrCuretSamples];
Double curetSamplesGreen[nrCuretSamples];
Double curetSamplesBlue[nrCuretSamples];

bool curetInit( char *name ) {
    if( !name ) name = "curet.data";

    ifstream curin;

    curin.open( name );
    if( !curin ) {
	cerr << "Could not open data file " << name << endl;
	return false;
    }

    Double value;
    curin >> value; // reads the first number, which is the data-index

    Cardinal i;
    for( i = 0; i < nrCuretSamplesRead; i++ ) {
	curin >> value;
	if( value < 0.0 ) value = 0.0;

	curetSamples[i] = value;
    }

    curin.close();

    // And now start rotating to make it anisotropic.

    for( i = 0; i < nrCuretSamplesRead; i++ ) {
	Double phi = curetIn[i][1];

	curetIn[i][1] = curetIn[i][1] - phi;

	curetOut[i][1] = curetOut[i][1] - phi;
    }


    // And now swap it to the other half!

    for( i = 0; i < nrCuretSamplesRead; i++ ) {
	curetIn[i+nrCuretSamplesRead][0] = curetIn[i][0];
	curetIn[i+nrCuretSamplesRead][1] = curetIn[i][1];

	curetOut[i+nrCuretSamplesRead][0] = curetOut[i][0];
	curetOut[i+nrCuretSamplesRead][1] = 360.0-curetOut[i][1];

	curetSamples[i+nrCuretSamplesRead] = curetSamples[i];
    }

    return true;
}

bool curetColorInit() {
    ifstream curinr, curing, curinb;
    char *rname = "curetred.data";
    char *gname = "curetgreen.data";
    char *bname = "curetblue.data";

    curinr.open( rname );
    curing.open( gname );
    curinb.open( bname );
    if( !curinr || !curing || !curinb ) {
	cerr << "Could not open data file." << endl;
	curinr.close();
	curing.close();
	curinb.close();
	return false;
    }

    Double value;
    curinr >> value; // reads the first number, which is the data-index
    curing >> value; // reads the first number, which is the data-index
    curinb >> value; // reads the first number, which is the data-index

    Cardinal i;
    for( i = 0; i < nrCuretSamplesRead; i++ ) {
	curinr >> value;
	if( value < 0.0 ) value = 0.0;
	curetSamplesRed[i] = value;

	curing >> value;
	if( value < 0.0 ) value = 0.0;
	curetSamplesGreen[i] = value;

	curinb >> value;
	if( value < 0.0 ) value = 0.0;
	curetSamplesBlue[i] = value;
    }

    // And now start rotating to make it anisotropic.

    for( i = 0; i < nrCuretSamplesRead; i++ ) {
	Double phi = curetIn[i][1];

	curetIn[i][1] = curetIn[i][1] - phi;
	curetOut[i][1] = curetOut[i][1] - phi;
    }


    // And now swap it to the other half!

    for( i = 0; i < nrCuretSamplesRead; i++ ) {
	curetIn[i+nrCuretSamplesRead][0] = curetIn[i][0];
	curetIn[i+nrCuretSamplesRead][1] = curetIn[i][1];

	curetOut[i+nrCuretSamplesRead][0] = curetOut[i][0];
	curetOut[i+nrCuretSamplesRead][1] = 360.0-curetOut[i][1];

	curetSamplesRed[i+nrCuretSamplesRead] = curetSamplesRed[i];
	curetSamplesGreen[i+nrCuretSamplesRead] = curetSamplesGreen[i];
	curetSamplesBlue[i+nrCuretSamplesRead] = curetSamplesBlue[i];
    }


    curinr.close();
    curing.close();
    curinb.close();
    return true;
}

Double distance( Vector &in, Vector &out,
		 Double th_i2, Double phi_i2,
		 Double th_o2, Double phi_o2 ) {
    Double dist;

    Vector  in2( sin(th_i2)*cos(phi_i2), sin(th_i2)*sin(phi_i2), cos(th_i2) );
    Vector out2( sin(th_o2)*cos(phi_o2), sin(th_o2)*sin(phi_o2), cos(th_o2) );

    // dist = (1-(in|in2)) + (1-(out|out2));
    // dist = (in|in2)/2.0 + (out|out2)/2.0;
    dist = (in|in2)/2.0 + (out|out2)/2.0;
    // reverse distance: 1 is closest, 0 is farest

    /*
    dist  = ABS(theta_in2  - theta_in);
    dist += ABS(theta_out2 - theta_out);
    dist += ABS(phi_in2 *sin(theta_in2)  - phi_in *sin(theta_in));
    dist += ABS(phi_out2*sin(theta_out2) - phi_out*sin(theta_out));
    */

    return dist;
}

Double curet( Double theta_in, Double phi_in,
	      Double theta_out, Double phi_out ) {
    Double r;

    // Basically: find k-nearest neighbours (brute force)
    // and then interpolate according to distance.
    // we assume k = 10;

    Int i, l;
    const Cardinal k = 10;

    Cardinal indices[k]; // indices[0]: smallest, indices[1]: next smallest ...
    Double distances[nrCuretSamples+1];
    distances[nrCuretSamples] = -2.0;

    for( l = 0; l < k; l++ ) {
	indices[l] = nrCuretSamples;
    }

    phi_out -= phi_in;
    phi_in = 0.0;

    Vector  in( sin(theta_in)*cos(phi_in), sin(theta_in)*sin(phi_in), cos(theta_in) );
    Vector out( sin(theta_out)*cos(phi_out), sin(theta_out)*sin(phi_out), cos(theta_out) );

    // Fill distances-array
    for( i = 0; i < nrCuretSamples; i++ ) {
	distances[i] = distance( in, out,
				 curetIn[i][0], curetIn[i][1], curetOut[i][0], curetOut[i][1] );
    }

    // Now find k smallest distances (small distance = large value)
    for( i = 0; i < nrCuretSamples; i++ ) {

	// Check if we are at all in the top k 
	// (compare with biggest of the smallest distances).
	if( distances[i] <= distances[indices[k-1]] ) 
	    continue;

	for( l = 0; l < k; l++ ) {
	    if( distances[i] > distances[indices[l]] ) {
		// move the rest of the indices down by one
		for( Int ll = k-2; ll >= l; ll-- ) {
		    indices[ll+1] = indices[ll];
		}
		indices[l] = i;
		
		// Well we're in, so get out of here
		break;
	    }
	}
    }
    Double weight = 0;
    Double d[k];
    for( l = 0; l < k; l++ ) {
	d[l] = pow(distances[indices[l]],100);
	weight += d[l];
    }

    if( weight == 0.0 ) {
	for( l = 0; l < k; l++ ) {
	    d[l] = pow(distances[indices[l]]*100,100);
	    weight += d[l];
	}
    }

    r = 0;
    for( l = 0; l < k; l++ ) {
	r += curetSamples[indices[l]] * (d[l]/weight);
    }

    if( isnan( r ) ) {
	cout << "sdaf" << endl;
	r = 0.0;
    }

    // a bit dark!
    r *= 6.0;

    if( doClamping && r > 5.0 ) r = 5.0;

    return r;
}

void curetColor( Double theta_in, Double phi_in,
		 Double theta_out, Double phi_out,
		 Double *r, Double *g, Double *b ) {
    // Basically: find k-nearest neighbours (brute force)
    // and then interpolate according to distance.
    // we assume k = 10;

    Int i, l;
    const Cardinal k = 10;

    Cardinal indices[k]; // indices[0]: smallest, indices[1]: next smallest ...
    Double distances[nrCuretSamples+1];
    distances[nrCuretSamples] = -2.0;

    for( l = 0; l < k; l++ ) {
	indices[l] = nrCuretSamples;
    }

    phi_out -= phi_in;
    phi_in = 0.0;

    Vector  in( sin(theta_in)*cos(phi_in), sin(theta_in)*sin(phi_in), cos(theta_in) );
    Vector out( sin(theta_out)*cos(phi_out), sin(theta_out)*sin(phi_out), cos(theta_out) );

    // Fill distances-array
    for( i = 0; i < nrCuretSamples; i++ ) {
	distances[i] = distance( in, out,
				 curetIn[i][0], curetIn[i][1], curetOut[i][0], curetOut[i][1] );
    }

    // Now find k smallest distances (small distance = large value)
    for( i = 0; i < nrCuretSamples; i++ ) {

	// Check if we are at all in the top k 
	// (compare with biggest of the smallest distances).
	if( distances[i] <= distances[indices[k-1]] ) 
	    continue;

	for( l = 0; l < k; l++ ) {
	    if( distances[i] > distances[indices[l]] ) {
		// move the rest of the indices down by one
		for( Int ll = k-2; ll >= l; ll-- ) {
		    indices[ll+1] = indices[ll];
		}
		indices[l] = i;
		
		// Well we're in, so get out of here
		break;
	    }
	}
    }

    Double weight = 0;
    Double d[k];
    for( l = 0; l < k; l++ ) {
	d[l] = pow(distances[indices[l]],100);
	weight += d[l];
    }

    if( weight == 0.0 ) {
	for( l = 0; l < k; l++ ) {
	    d[l] = pow(distances[indices[l]]*100,100);
	    weight += d[l];
	}
    }

    *r = *g = *b = 0.0;
    for( l = 0; l < k; l++ ) {
	*r += curetSamplesRed[indices[l]] * (d[l]/weight);
	*g += curetSamplesGreen[indices[l]] * (d[l]/weight);
	*b += curetSamplesBlue[indices[l]] * (d[l]/weight);
    }

    // a bit dark!
    *r *= 6.0 * 0.76;
    *g *= 6.0 * 1.0;
    *b *= 6.0 * 1.7;

    return;
}


const Cardinal maxWardSamples = 10000;
Cardinal nrWardSamples = 0;
Double wardData[maxWardSamples][5];

bool wardInit( char *name ) {
    if( !name ) name = "ward.data";

    ifstream win;

    win.open( name );
    if( !win ) {
	cerr << "Could not open data file " << name << endl;
	return false;
    }

    char line[200];
    win.getline( line, 200 );
    win.getline( line, 200 );
    win.getline( line, 200 );
    win.getline( line, 200 );
    win.getline( line, 200 );
    win.getline( line, 200 );

    nrWardSamples = 0;
    Cardinal i, j;
    for( i = 0; i < maxWardSamples; i++ ) {
	for( j = 0; j < 5; j++ ) {
	    win >> wardData[i][j];

	    if( win.eof() ) goto outside;   // sorry about that

	    if( j < 4 ) {
		wardData[i][j] *= (M_PI/180.0); // convert deg -> rad
	    }
	}
	nrWardSamples++;

	if( win.eof() ) break;
    }

outside:
    win.close();

    return true;
}

Double ward( Double theta_in, Double phi_in,
	     Double theta_out, Double phi_out ) {
    Double r;

    // Basically: find k-nearest neighbours (brute force)
    // and then interpolate according to distance.
    // we assume k = 13;

    Int i, l;
    const Cardinal k = 13;

    Cardinal indices[k]; // indices[0]: smallest, indices[1]: next smallest ...
    Double distances[maxWardSamples+1];
    Cardinal distanceindices[maxWardSamples+1], nrDistances = 0;

    distances[nrWardSamples] = -2.0;

    for( l = 0; l < k; l++ ) {
	indices[l] = nrWardSamples;
    }

    phi_out -= phi_in;
    phi_in = 0.0;
    if( phi_out < 0.0 ) phi_out += 2.0*M_PI;

    Vector  in( sin(theta_in)*cos(phi_in), sin(theta_in)*sin(phi_in), cos(theta_in) );
    Vector out( sin(theta_out)*cos(phi_out), sin(theta_out)*sin(phi_out), cos(theta_out) );

    // Fill distances-array
    for( i = 0; i < nrWardSamples; i++ ) {
	// Check if distance is too large
	if( (ABS( theta_in - wardData[i][0] ) > (9.6*M_PI/180.0)) ) {
	    continue;
	}

	distances[nrDistances] = distance( in, out,
					     wardData[i][0], wardData[i][1], wardData[i][2], wardData[i][3] ); 
	distanceindices[nrDistances++] = i; // store to which ward sample we measured the distance
    }

    if( nrDistances < k ) {
	cout << "Argh: less distances than k. " << distances << endl;
    }

    // Now find k smallest distances (small distance = large value)
    for( i = 0; i < nrDistances; i++ ) {

	// Check if we are at all in the top k 
	// (compare with biggest of the smallest distances).
	if( distances[i] <= distances[indices[k-1]] ) 
	    continue;

	for( l = 0; l < k; l++ ) {
	    if( distances[i] > distances[indices[l]] ) {
		// move the rest of the indices down by one
		for( Int ll = k-2; ll >= l; ll-- ) {
		    indices[ll+1] = indices[ll];
		}
		indices[l] = i;
		
		// Well we're in, so get out of here
		break;
	    }
	}
    }


    // Print what we are looking for and what we are getting:
    /*
    cout << "WANT: th_in: " << theta_in*180.0/M_PI << endl;
    cout << " GET: th_in: " << (wardData[distanceindices[indices[0]]][0]*180.0/M_PI) << endl;
    cout << "WANT: th_out: " << theta_out*180.0/M_PI << endl;
    cout << " GET: th_out: " << (wardData[distanceindices[indices[0]]][2]*180.0/M_PI) << endl;
    cout << "WANT: phi_out: " << phi_out*180.0/M_PI << endl;
    cout << " GET: phi_out: " << (wardData[distanceindices[indices[0]]][3]*180.0/M_PI) << endl;
    */

    Double weight = 0;
    Double d[k];
    for( l = 0; l < k; l++ ) {
	d[l] = pow(distances[indices[l]],200);
	weight += d[l];
    }

    if( weight == 0.0 ) {
	for( l = 0; l < k; l++ ) {
	    d[l] = pow(distances[indices[l]]*100,200);
	    weight += d[l];
	}
    }

    r = 0;
    for( l = 0; l < k; l++ ) {
	r += wardData[distanceindices[indices[l]]][4] * (d[l]/weight);
    }

    if( isnan( r ) ) {
	cout << "sdaf" << endl;
	r = 0.0;
    }
    
    if( r > 6.0 ) {
	cout << r << endl;
	for( l = 0; l < k; l++ ) {
	    cout << "index: " << indices[l] << endl;
	    cout << " distance: " << distances[indices[l]] << endl;
	}
    }

    // r = wardData[distanceindices[indices[0]]][4];

    // a bit dark!
    r *= 6.0;

    if( doClamping && r > 5.0 ) r = 5.0;
    
    return r;
}


// -----------------  PhongLobeApprox BRDF --------------

#define MAX_NR_STEPS 100
#define MAX_LOBES 10

Double exponents[MAX_LOBES][MAX_NR_STEPS];
Double intensities[MAX_LOBES][MAX_NR_STEPS];
Double th_offsets[MAX_LOBES][MAX_NR_STEPS];
Double phi_offsets[MAX_LOBES][MAX_NR_STEPS];

Cardinal nrExponents;
Cardinal maxExponent;
Cardinal nrLobes;

bool phongApproxInit( char *filename ) {
    ifstream datafile;

    datafile.open( filename );

    if( !datafile ) {
	cerr << "Couldn't open data-file " << filename << endl;
	return false;
    }

    datafile >> nrExponents >> nrLobes >> maxExponent;
    

    if( nrLobes > MAX_LOBES ) {
	cerr << "Too many lobes in data-file, max = " << MAX_LOBES << endl;
	return false;
    }
    if( nrExponents > MAX_NR_STEPS ) {
	cerr << "Too many steps in data-file, max = " << MAX_NR_STEPS << endl;
	return false;
    }

    for( Cardinal j = 0; j < nrLobes; j++ ) {
	for( Cardinal i = 0; i < nrExponents; i++ ) {
	    Double dummy;
	    
	    datafile >> dummy >> exponents[j][i] >> intensities[j][i] >> th_offsets[j][i] >> phi_offsets[j][i];
	}
    }

    datafile.close();

    return true;
}

Double phongApproxBRDF( Double theta_in, Double phi_in,
			Double theta_out, Double phi_out ) {
    Double nr = theta_in/M_PI_2*(nrExponents-1);
    if( nr > (nrExponents-1) ) nr = nrExponents-1;
    Cardinal nrInt = Cardinal(floor(nr));
    Double prop = nr - floor(nr);

    Cardinal lobe = 0; //  nrLobes-1;  // just lobe zero for now

    // use linear interpolation of the proposed exponents
    Double N = (1-prop)*exponents[lobe][nrInt] + prop*exponents[lobe][nrInt+1];
    Double intensity = (1-prop)*intensities[lobe][nrInt] + prop*intensities[lobe][nrInt+1];

    Double th_offset = (1-prop)*th_offsets[lobe][nrInt] + prop*th_offsets[lobe][nrInt+1];
    Double phi_offset = (1-prop)*phi_offsets[lobe][nrInt] + prop*phi_offsets[lobe][nrInt+1];

    return phongBRDF( theta_in + th_offset, phi_in - phi_offset, theta_out, phi_out,
		      0.0, intensity, N );
}

// -----------------  ShapeLobeApprox BRDF --------------

#define SL_MAX_NR_STEPS 32
#define SL_MAX_LOBES 20

Double sl_th_offsets[SL_MAX_LOBES][SL_MAX_NR_STEPS];
Double sl_phi_offsets[SL_MAX_LOBES][SL_MAX_NR_STEPS];
Double sl_brdfValues[SL_MAX_LOBES][SL_MAX_NR_STEPS][SL_MAX_NR_STEPS];

Cardinal sl_nrSteps, sl_nrPSteps, sl_nrLobes;

bool shapeApproxInit( char *filename ) {
    ifstream datafile;
    Double dummy;

    datafile.open( filename );

    if( !datafile ) {
	cerr << "Couldn't open data-file " << filename << endl;
	return false;
    }

    datafile >> dummy >> sl_nrSteps >> sl_nrPSteps >> sl_nrLobes;

    if( dummy != -1 ) {
	cerr << filename << " not a correct Shape Lobe Approximation file!" << endl;
	return false;
    }    

    if( sl_nrLobes > SL_MAX_LOBES ) {
	cerr << "Too many lobes in data-file, max = " << SL_MAX_LOBES << endl;
	return false;
    }
    if( sl_nrSteps > SL_MAX_NR_STEPS || sl_nrPSteps > SL_MAX_NR_STEPS ) {
	cerr << "Too many steps in data-file, max = " << SL_MAX_NR_STEPS << endl;
	return false;
    }

    for( Cardinal i = 0; i < sl_nrSteps; i++ ) {
	for( Cardinal j = 0; j < sl_nrLobes; j++ ) {
	    datafile >> dummy >> sl_th_offsets[j][i] >> sl_phi_offsets[j][i];

	    for( Cardinal p = 0; p < sl_nrPSteps; p++ ) {
		datafile >> sl_brdfValues[j][i][p];
	    }
	}
    }

    datafile.close();

    return true;
}

bool shape2DApproxInit( char *filename ) {
    ifstream datafile;
    Double dummy;

    datafile.open( filename );

    if( !datafile ) {
	cerr << "Couldn't open data-file " << filename << endl;
	return false;
    }

    datafile >> dummy >> sl_nrSteps >> sl_nrPSteps;
    if( dummy != -2 ) {
	cerr << filename << " not a correct Shape Lobe 2D Approximation file!" << endl;
	return false;
    }
    sl_nrLobes = 1; // always one, no other way!

    if( sl_nrLobes > SL_MAX_LOBES ) {
	cerr << "Too many lobes in data-file, max = " << SL_MAX_LOBES << endl;
	return false;
    }
    if( sl_nrSteps > SL_MAX_NR_STEPS || sl_nrPSteps > SL_MAX_NR_STEPS ) {
	cerr << "Too many steps in data-file, max = " << SL_MAX_NR_STEPS << endl;
	return false;
    }

    Double shapeLobe[sl_nrPSteps];
    Double ks;
    Cardinal i, j, p;

    // read shape lobe first
    for( p = 0; p < sl_nrPSteps; p++ ) {
	datafile >> shapeLobe[p];
    }
    
    // read offsets and generate real shape lobes for each th_i (multiply shapeLobe[p] * ks[i])
    for( i = 0; i < sl_nrSteps; i++ ) {
	datafile >> dummy >> sl_th_offsets[0][i] >> sl_phi_offsets[0][i] >> ks;
	
	for( p = 0; p < sl_nrPSteps; p++ ) {
	    sl_brdfValues[0][i][p] = ks*shapeLobe[p];
	}
    }

    datafile.close();

    return true;
}

#include "VisTransform.hh"
#include "Helper.hh"

Double shapeApproxBRDF( Double theta_in, Double phi_in,
			Double theta_out, Double phi_out ) {
    Cardinal lobe = 0;
    Double result = 0.0;

    Double nr = theta_in/M_PI_2*(sl_nrSteps-1);
    if( nr > (sl_nrSteps-1) ) nr = sl_nrSteps-1;
    Cardinal nrInt = Cardinal(floor(nr));
    Double prop = nr - floor(nr);

    for( lobe = 0; lobe < sl_nrLobes; lobe++ ) {
    //    lobe = 0; {
	Double th_offset = (1-prop)*sl_th_offsets[lobe][nrInt] + prop*sl_th_offsets[lobe][nrInt+1];
	Double phi_offset = (1-prop)*sl_phi_offsets[lobe][nrInt] + prop*sl_phi_offsets[lobe][nrInt+1];

#define TOVEC( t, p ) sin(t)*cos(p), sin(t)*sin(p), cos(t)
	//	Vector peak( TOVEC( theta_in+th_offset, phi_in-phi_offset+M_PI ) );
	Vector peak( TOVEC( theta_in, phi_in+M_PI ) );
	Vector out( TOVEC( theta_out, phi_out ) );

	// now calculate th_p/phi_p relative to peak (i.e. use th_o,phi_o -> convert to -> th_p, phi_p)

	VisTransform trafo;
	Vector n( 0.0, 0.0, 1.0 );

	// now rotate around n for phi!
	trafo = VisTransform::Rotation( n, -phi_offset );
	out = trafo.transformPoint( out );

	// rotate out back around a by -(theta_in+th_offset)
	Vector a = n^peak;
	trafo = VisTransform::Rotation( a, -(theta_in+th_offset) );
	out = trafo.transformPoint( out );
	
	Double th_p, dontcare;

	calcTPFromXY( out[0], out[1], th_p, dontcare );

	// now we have got the th_p (relative to peak)

	// calc proportions and indices to tables

	Double nrP = th_p/M_PI_2*(sl_nrPSteps-1);
	if( nrP > (sl_nrPSteps-1) ) nrP = sl_nrPSteps-1;
	if( nrP < 0 ) nrP = 0;
	Cardinal nrPInt = Cardinal(floor(nrP));
	Double propP = nrP - floor(nrP);

	// trilinear interpolation!

	Double va_lower, va_upper, va;

	va_lower = (1-prop)*sl_brdfValues[lobe][nrInt][nrPInt]+prop*sl_brdfValues[lobe][nrInt+1][nrPInt];
	va_upper = (1-prop)*sl_brdfValues[lobe][nrInt][nrPInt+1]+prop*sl_brdfValues[lobe][nrInt+1][nrPInt+1];

	// use linear interpolation between samples
	va = (1-propP)*va_lower + propP*va_upper;

	// check if the rotated out is underneath horizon
	if( out[2] < 0.0 ) va = 0.0;

	result += va;
    }

    return result > 0.0 ? result : 0.0;   // [0;inf)
}

Double shape2DApproxBRDF( Double theta_in, Double phi_in,
			  Double theta_out, Double phi_out ) {
    return shapeApproxBRDF( theta_in, phi_in, theta_out, phi_out );
}

Cardinal SAgetNumberOfLobes() {
    return sl_nrLobes;
}

void SAsetNumberOfLobes( Cardinal nr ) {
    sl_nrLobes = nr;
}
