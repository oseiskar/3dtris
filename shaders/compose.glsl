uniform vec2 size;

uniform sampler2D tDepth;
uniform sampler2D tDiffuse;

uniform sampler2D tShadow;
uniform sampler2D tAO;

uniform float shadowFar;
uniform vec3 shadowX, shadowY, shadowZ;
uniform vec3 shadowOrigin;
uniform vec2 shadowSize;

varying vec3 vNormal;
varying vec3 vPosition;

#include <packing>

float readShadowDepth( const in vec2 coord ) {
    float z = unpackRGBAToDepth( texture2D( tShadow, coord ) );
    return z * shadowFar;
}

void main() {
    vec2 screenCoord = gl_FragCoord.xy / size;

    vec3 color = texture2D( tDiffuse, screenCoord ).rgb;
    float ao = texture2D( tAO, screenCoord ).r;

    vec3 shadowPos = vPosition - shadowOrigin;
    vec2 shadowCoord = vec2(dot(shadowPos, shadowX), dot(shadowPos, shadowY)) / shadowSize + 0.5;
    //shadowCoord = vec2(shadowCoord.x + 0.5, 1.0 - (shadowCoord.y + 0.5));
    float shadowDepth = dot(shadowPos, -shadowZ);
    float refDepth = readShadowDepth(shadowCoord);

    float ambient = 0.4;
    float diffuse = max(dot(vNormal, shadowZ), 0.);

    const float brightness = 1.;

    float shadow = 0.;
    if (shadowDepth > refDepth + 0.01)
        shadow = 1.;

    diffuse = diffuse * (1. - shadow) * (1. - ambient) + ambient;

    //shadow = refDepth;
    //shadow = shadowDepth ;//+ 3.0;

    vec3 final = color*ao*diffuse*brightness;

    gl_FragColor = vec4( final, 1.0 );
}
