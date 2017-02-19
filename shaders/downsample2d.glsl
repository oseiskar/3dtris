uniform vec2 size;

uniform sampler2D tSource;
varying vec2 vUv;

void main() {
    vec4 sum = vec4(0.,0.,0.,0.);

    vec2 d = 1. / (size * float(DIM));

    for (int x=0; x<DIM; ++x) {
        for(int y=0; y<DIM; ++y) {
            vec2 coord = vUv + d*vec2(float(x), float(y));
            sum += texture2D( tSource, coord );
        }
    }
    gl_FragColor = sum / float(DIM*DIM);
}
