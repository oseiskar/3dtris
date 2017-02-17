uniform vec2 size;

uniform sampler2D tSource;
varying vec2 vUv;

const int samples = 5;
const float sigma = 3.0;

void main() {
    float totalWeight = 0.;
    vec4 sum = vec4(0.,0.,0.,0.);
    for (int i=0; i<samples; ++i) {
        vec2 coord = vUv;

        float rel = ((float(i) / float(samples-1))*2. - 1.) * 1.5;
        WHICHCOORD += 1.0 / DIMENSION * rel * sigma;
        float w = exp(-0.5*rel*rel);
        totalWeight += w;
        sum += texture2D( tSource, coord ) * w;
    }
    gl_FragColor = sum / totalWeight;
}
