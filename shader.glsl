uniform float cameraNear;
uniform float cameraFar;

uniform vec2 size;
uniform float aoClamp;
uniform float lumInfluence;
//uniform sampler2D tDiffuse;
uniform sampler2D tDepth;
uniform vec2 projectionXY;

varying vec2 vUv;
varying vec3 vNormal;
varying vec3 vScreenNormal;

//#define DL 2.399963229728653
//#define EULER 2.718281828459045

#define M_PI 3.1415926535897932384626433832795

const int samples = 32;
const float radius = 0.04;
const float farClip = 0.2;

const float intensity = 1.0;

#include <packing>

vec2 rand( const vec2 coord ) {
    float nx = dot ( coord, vec2( 12.9898, 78.233 ) );
    float ny = dot ( coord, vec2( 12.9898, 78.233 ) * 2.0 );
    return clamp( fract ( 43758.5453 * sin( vec2( nx, ny ) ) ), 0.0, 1.0 );
}


float readDepth( const in vec2 coord ) {
    float cameraFarPlusNear = cameraFar + cameraNear;
    float cameraFarMinusNear = cameraFar - cameraNear;

    float z = unpackRGBAToDepth( texture2D( tDepth, coord ) );

    //float z_n = 2.0 * z_b - 1.0;
    //float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));

    z = 2.0* z - 1.0;
    return 2.0 * cameraNear * cameraFar / ( cameraFarPlusNear - z * cameraFarMinusNear );
}

void main() {
    //vec3 color = texture2D( tDiffuse, vUv ).rgb;
    vec3 color = vec3(1.0);

    vec2 noise1 = rand( gl_FragCoord.xy );
    vec2 noise2 = rand( gl_FragCoord.xy * 0.5 );

    vec2 xy0 = gl_FragCoord.xy / size;
    float depth = readDepth(xy0);

    vec2 wh = projectionXY / depth;

    float ao = 0.0;
    float dl = sqrt(M_PI * float(samples));

    vec3 zVec = vScreenNormal;
    vec3 xVec = normalize(cross(zVec, vec3(noise1.x, noise1.y, 0)));
    vec3 yVec = cross(zVec, xVec);

    vec2 minD = 0.5 / size;

    for ( int i = 0; i <= samples; i ++ ) {

        float z = 1. - (2.*float(i+1) - 1.) / float(samples);
        float l = sign(z) * dl*acos(z) + noise2.y * M_PI * 2.;
        float r = sqrt( 1.0 - z*z ) * (sin(float(i)*100.) + 1.0)*0.5;

        float curRadius = (1.0+sin(float(i*100)))*0.5*radius;
        vec3 dp = (cos(l)*r*xVec + sin(l)*r*yVec + (abs(z) + 5. / float(samples)) * zVec)*curRadius;
        if (abs(dp.x * wh.x) > minD.x && abs(dp.y * wh.y) > minD.y)
        {

            float d1 = readDepth(xy0 + vec2(dp.x, dp.y) * wh);
            if (depth - dp.z > d1 + radius*0.1) {
                ao += 1.0 * clamp(1. - abs(depth - d1) / farClip, 0., 1.);
            }
        }
    }

    ao /= float( samples );
    ao = 1.0 - ao * intensity;

    /*vec3 color = texture2D( tDiffuse, vUv ).rgb;

    vec3 lumcoeff = vec3( 0.299, 0.587, 0.114 );
    float lum = dot( color.rgb, lumcoeff );
    vec3 luminance = vec3( lum );
    vec3 final = vec3( color * mix( vec3( ao ), vec3( 1.0 ), luminance * lumInfluence ) );*/

    float diffIntensity = 0.3;
    float diffuse = dot(vNormal, normalize(vec3(-1,5,3))) * diffIntensity + (1.-diffIntensity);

    vec3 final = color * ao * diffuse;

    gl_FragColor = vec4( final, 1.0 );
}
