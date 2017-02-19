uniform vec2 size;

uniform sampler2D tSource;
varying vec2 vUv;

void main() {
    float totalWeight = 0.;
    vec4 sum = vec4(0.,0.,0.,0.);
    for (int i=0; i<SAMPLES; ++i) {
        vec2 coord = vUv;

        float rel = ((float(i) / float(SAMPLES-1))*2. - 1.) * 1.5;
        WHICHCOORD += 1.0 / DIMENSION * rel * float(SIGMA);
        float w = exp(-0.5*rel*rel);
        totalWeight += w;
        sum += texture2D( tSource, coord ) * w;
    }
    gl_FragColor = sum / totalWeight;
}
