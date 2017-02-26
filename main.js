"use strict";
/*globals SHADER_LOADER, THREE,
GameController, executeAction, Stats,
window, document, $, requestAnimationFrame */

var camera, scene, renderer, controls, flatScene, flatCamera, stats;

var depthMaterial;
var shaders;
var frameBuffers = {};
var shadow;

const SUPERSAMPLING = 1;
const SSAO_SAMPLES = Math.max(2, 8 / SUPERSAMPLING);
const N_BLOCK_MATERIALS = 10;

(function(){

    SHADER_LOADER.load(function(shaders) {
        init(shaders);
        animate();
    });

})();

function randomByte() {
    return Math.floor(Math.random()*0xff);
}

function randomMaterial() {
    return new THREE.MeshBasicMaterial({
        color:
            randomByte() << 16 |
            randomByte() << 8 |
            randomByte()
    });
}

function planeGeometry(sz) {
    var geometry = new THREE.Geometry();

    const x1 = -sz;
    const y1 = -sz;
    const z = 0;

    const x2 = sz;
    const y2 = sz;

    geometry.vertices.push(
        new THREE.Vector3(x1,y1,z),//vertex0
        new THREE.Vector3(x2,y1,z),//1
        new THREE.Vector3(x2,y2,z),//2
        new THREE.Vector3(x1,y2,z)//3
        );
    geometry.faces.push(
        new THREE.Face3(0,1,2),
        new THREE.Face3(2,3,0)
        );

    geometry.computeFaceNormals();

    return geometry;
}

function gameRenderer(game) {

    const w = game.dims.x;
    const h = game.dims.y;
    const d = game.dims.z;

    const boxSz = 1.0 / Math.max(w, h);

    const centerZ = d * boxSz * 0.33;

    const geometries = {
        box: new THREE.BoxBufferGeometry( boxSz, boxSz, boxSz ),
        plane: planeGeometry(0.5),
        circle: new THREE.CircleGeometry( 3.0, 100 )
    };

    const planeMaterial = new THREE.MeshBasicMaterial({ color: 0xa0a0a0 });
    const circleMaterial = new THREE.MeshBasicMaterial({ color: 0xc0c0c0 });

    return function() {

        let blocks = game.getCementedBlocks();
        blocks = blocks.concat(game.getActiveBlocks());

        const meshes = blocks.map(block => {
            const mesh = new THREE.Mesh( geometries.box, block.material );

            mesh.translateX((block.x - w*0.5 + 0.5)*boxSz);

            // flip Z and Y
            mesh.translateZ((block.y - h*0.5 + 0.5)*boxSz);
            mesh.translateY((block.z+0.5)*boxSz - centerZ);

            return mesh;
        });

        // add plane
        const plane = new THREE.Mesh(geometries.plane, planeMaterial);
        plane.translateY(-centerZ);
        plane.rotateX( - Math.PI / 2);
        plane.doubleSided = true;
        meshes.push(plane);

        // add circle
        const circle = new THREE.Mesh(geometries.circle, circleMaterial);
        circle.translateY(-centerZ - 0.01);
        circle.rotateX( - Math.PI / 2);
        circle.doubleSided = true;
        meshes.push(circle);

        return meshes;
    };
}


function deg2rad(deg) { return Math.PI * deg / 180.0; }

function initShadow() {

    const shadowPitch = deg2rad(50);
    const shadowYaw = deg2rad(30);

    const shadowZ = new THREE.Vector3(
        Math.cos(shadowYaw)*Math.cos(shadowPitch),
        Math.sin(shadowPitch),
        Math.sin(shadowYaw)*Math.cos(shadowPitch)).negate();

    const shadowX = new THREE.Vector3().crossVectors(shadowZ, new THREE.Vector3(0,1,0)).normalize();
    const shadowY = new THREE.Vector3().crossVectors(shadowX, shadowZ);

    const width = 5;
    const height = 5;
    const far = 10.0;

    const pos = shadowZ.clone().multiplyScalar(-2.0);

    const camera = new THREE.OrthographicCamera(
        width / - 2, width / 2, height / 2, height / - 2, 0.0, far );

    camera.matrixAutoUpdate = false;
    camera.matrix
        .makeBasis(shadowX, shadowY, shadowZ.negate())
        .setPosition(pos);
    camera.updateMatrixWorld( true );

    return {
        size: new THREE.Vector2(width, height),
        far: far,
        pos: pos,
        xVec: shadowX,
        yVec: shadowY,
        zVec: shadowZ,
        camera: camera
    };
}

function init(loadedShaders) {
    camera = new THREE.PerspectiveCamera( 70, window.innerWidth / window.innerHeight, 1, 10 );

    shadow = initShadow();

    camera.position.set(-0.3, 3, 1);
    scene = new THREE.Scene();

    const SSAO_BLUR_SAMPLES = 5;
    const SSAO_BLUR_SIGMA = 3.0;

    shaders = {
        ssao: {
            defines: {
                "SAMPLES": SSAO_SAMPLES
            },
            uniforms: {
                "tDepth":       { value: null },
                "size":         { value: new THREE.Vector2( 512, 512 ) },
                "cameraNear":   { value: 1 },
                "cameraFar":    { value: 100 },
                "projectionXY": { value: new THREE.Vector2(1, 1) },
            },

            vertexShader: $('#ssao-vertex-shader').text(),
            fragmentShader: loadedShaders.ssao.fragment
        },

        compose: {
            uniforms: {
                "size":         { value: new THREE.Vector2( 512, 512 ) },

                "cameraMatrix": { value: new THREE.Matrix4() },
                "tDiffuse":     { value: null },
                "tShadow":      { value: null },
                "tAO":          { value: null },
                "shadowFar":    { value: shadow.far },
                "shadowX":      { value: shadow.xVec },
                "shadowY":      { value: shadow.yVec },
                "shadowZ":      { value: shadow.zVec },
                "shadowOrigin": { value: shadow.pos },
                "shadowSize":   { value: shadow.size }
            },

            vertexShader: $('#compose-vertex-shader').text(),
            fragmentShader: loadedShaders.compose.fragment
        },

        blurX: {
            defines: {
                "WHICHCOORD": "coord.x",
                "DIMENSION": "size.x",
                "SAMPLES": SSAO_BLUR_SAMPLES,
                "SIGMA": SSAO_BLUR_SIGMA
            },
            uniforms: {
                "tSource":  { value: null },
                "size":     { value: new THREE.Vector2( 512, 512 ) }
            },
            vertexShader: $('#flat-vertex-shader').text(),
            fragmentShader: loadedShaders.blur.fragment
        },

        blurY: {
            defines: {
                "WHICHCOORD": "coord.y",
                "DIMENSION": "size.y",
                "SAMPLES": SSAO_BLUR_SAMPLES,
                "SIGMA": SSAO_BLUR_SIGMA
            },
            uniforms: {
                "tSource":  { value: null },
                "size":     { value: new THREE.Vector2( 512, 512 ) }
            },
            vertexShader: $('#flat-vertex-shader').text(),
            fragmentShader: loadedShaders.blur.fragment
        },

        downsample: {
            defines: {
                "DIM": SUPERSAMPLING,
            },
            uniforms: {
                "tSource":  { value: null },
                "size":     { value: new THREE.Vector2( 512, 512 ) }
            },
            vertexShader: $('#flat-vertex-shader').text(),
            fragmentShader: loadedShaders.downsample2d.fragment
        }
    };

    for (var v in shaders) {
        shaders[v].material = new THREE.ShaderMaterial(shaders[v]);
    }

    flatCamera = new THREE.OrthographicCamera( - 1, 1, 1, - 1, 0, 1 );

    // must have some material even if always overridden
    const flatQuad = new THREE.Mesh( new THREE.PlaneBufferGeometry( 2, 2 ),
        shaders.blurX.material );

    flatQuad.frustumCulled = false; // Avoid getting clipped
    flatScene = new THREE.Scene();
    flatScene.add( flatQuad );

    const blockMaterials = Array.from(new Array(N_BLOCK_MATERIALS), () => randomMaterial());
    function pieceDecorator() {
        return blockMaterials[Math.floor(Math.random()*blockMaterials.length)];
    }

    const game = new GameController(pieceDecorator);

    const gameRenderFunc = gameRenderer(game.game);
    let prevMeshes = null;

    function generateMeshes() {
        if (prevMeshes !== null) {
            prevMeshes.forEach(mesh => scene.remove(mesh));
        }
        prevMeshes = [];
        gameRenderFunc().forEach(function(mesh) {
            prevMeshes.push(mesh);
            scene.add(mesh);
        });
    }

    function keyControls(e)  {
        const action = executeAction(game.game.controls, e.key);
        if (action !== null) generateMeshes();
    }

    game.changedCallback = generateMeshes;
    game.run();
    generateMeshes();

    const container = document.createElement( 'div' );
    document.body.appendChild( container );

    renderer = new THREE.WebGLRenderer();
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth, window.innerHeight );
    document.body.appendChild( renderer.domElement );
    document.body.addEventListener("keydown", keyControls);

    container.appendChild( renderer.domElement );

    if ($.urlParam("debug")) {
        stats = new Stats();
        stats.domElement.style.position = 'absolute';
        stats.domElement.style.top = '0px';
        container.appendChild( stats.domElement );
    }

    controls = new THREE.OrbitControls( camera, renderer.domElement );
    controls.addEventListener( 'change', render ); // remove when using animation loop
    // enable animation loop when using damping or autorotation
    //controls.enableDamping = true;
    //controls.dampingFactor = 0.25;
    controls.enableZoom = false;
    controls.enablePan = false;

    initPostprocessing();

    window.addEventListener( 'resize', onWindowResize, false );
}

function animate() {
    requestAnimationFrame( animate );
    controls.update();
    render();
    if (stats) stats.update();
}

function onWindowResize() {

    const width = window.innerWidth;
    const height = window.innerHeight;

    camera.aspect = width / height;
    camera.updateProjectionMatrix();
    renderer.setSize( width, height );

    setBufferSizes();

    updateUniforms();
}

function setBufferSizes() {

    const width = window.innerWidth;
    const height = window.innerHeight;

    const pixelRatio = renderer.getPixelRatio();
    const newWidth  = Math.floor( width / pixelRatio ) || 1;
    const newHeight = Math.floor( height / pixelRatio ) || 1;

    for (var key in frameBuffers) {
        if (!frameBuffers[key].constantSize) {
            let szMul = 1;
            if (frameBuffers[key].supersampling) {
                szMul = frameBuffers[key].supersampling;
            }
            frameBuffers[key].setSize( newWidth*szMul, newHeight*szMul );
        }
    }
}

function updateUniforms() {
    const width = window.innerWidth;
    const height = window.innerHeight;

    for (var v in shaders) {
        let szMul = 1;
        if (shaders[v].supersampling) {
            szMul = shaders[v].supersampling;
        }
        shaders[v].uniforms.size.value.set( width*szMul, height*szMul );
    }

    shaders.ssao.uniforms.projectionXY.value.set(
        camera.projectionMatrix.elements[0],
        camera.projectionMatrix.elements[5]);
}

function initPostprocessing() {

    // Setup depth pass
    depthMaterial = new THREE.MeshDepthMaterial();
    depthMaterial.depthPacking = THREE.RGBADepthPacking;
    depthMaterial.blending = THREE.NoBlending;

    const pars = { minFilter: THREE.LinearFilter, magFilter: THREE.LinearFilter };
    frameBuffers.depth = new THREE.WebGLRenderTarget(  1, 1, pars );
    frameBuffers.ao = new THREE.WebGLRenderTarget( 1, 1, pars );
    shaders.ssao.supersampling = SUPERSAMPLING;
    frameBuffers.ao.supersampling = SUPERSAMPLING;
    frameBuffers.diffuse = new THREE.WebGLRenderTarget( 1, 1, pars );
    frameBuffers.diffuse.supersampling = SUPERSAMPLING;
    frameBuffers.tmp = new THREE.WebGLRenderTarget( 1, 1, pars );
    frameBuffers.final = new THREE.WebGLRenderTarget( 1, 1, pars );
    frameBuffers.final.supersampling = SUPERSAMPLING;
    shaders.compose.supersampling = SUPERSAMPLING;
    frameBuffers.shadow = new THREE.WebGLRenderTarget( 3000, 3000, pars );
    frameBuffers.shadow.constantSize = true;

    setBufferSizes();

    shaders.ssao.uniforms.cameraNear.value = camera.near;
    shaders.ssao.uniforms.cameraFar.value = camera.far;
    shaders.ssao.uniforms.tDepth.value = frameBuffers.depth.texture;

    shaders.blurX.uniforms.tSource.value = frameBuffers.ao.texture;
    shaders.blurY.uniforms.tSource.value = frameBuffers.tmp.texture;

    shaders.compose.uniforms.tDiffuse.value = frameBuffers.diffuse.texture;
    shaders.compose.uniforms.tAO.value = frameBuffers.ao.texture;
    shaders.compose.uniforms.tShadow.value = frameBuffers.shadow.texture;

    shaders.downsample.uniforms.tSource.value = frameBuffers.final.texture;

    updateUniforms();
}

function render() {

    shaders.compose.uniforms.cameraMatrix.value.copy(camera.matrixWorld);

    renderer.setClearColor(0x808080, 1);
    //var camera = shadow.camera;
    // Render depth into frameBuffers.depth
    scene.overrideMaterial = depthMaterial;
    renderer.render( scene, camera, frameBuffers.depth, true );

    renderer.render( scene, shadow.camera, frameBuffers.shadow, true );

    scene.overrideMaterial = shaders.ssao.material;
    renderer.setClearColor(0xffffff, 1);
    renderer.render(scene, camera, frameBuffers.ao );
    renderer.setClearColor(0x000000, 0);

    flatScene.overrideMaterial = shaders.blurX.material;
    renderer.render(flatScene, flatCamera, frameBuffers.tmp );
    flatScene.overrideMaterial = shaders.blurY.material;
    renderer.render(flatScene, flatCamera, frameBuffers.ao );

    scene.overrideMaterial = null;
    renderer.render( scene, camera, frameBuffers.diffuse );

    scene.overrideMaterial = shaders.compose.material;

    renderer.setClearColor(0x808080, 1);
    if (SUPERSAMPLING > 1) {
        renderer.render(scene, camera, frameBuffers.final);

        flatScene.overrideMaterial = shaders.downsample.material;
        renderer.render(flatScene, flatCamera );
    } else {
        renderer.render(scene, camera);
    }
    renderer.setClearColor(0x000000, 0);

}
