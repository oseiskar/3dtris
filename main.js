var camera, scene, renderer, controls;

var depthMaterial;
var shaders;
var meshes;
var frameBuffers = {};
var shadow;

(function(){

    SHADER_LOADER.load(function(shaders) {
        init(shaders);
        animate();
    });

})();

function planeGeometry(sz) {
    var geometry = new THREE.Geometry();

    var x1 = -sz;
    var y1 = -sz;
    var z = 0;

    var x2 = sz;
    var y2 = sz;

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

function generateMeshes() {
    var w = 7;
    var h = 6;
    var d = 8;

    var boxSz = 1.0 / Math.max(w, h);

    function randomByte() {
        return Math.floor(Math.random()*0xff);
    }

    var meshes = [];
    for (var z=0; z<d; ++z) {
        for (var x=0; x<w; ++x) {
            for (var y=0; y<h; ++y) {

                var prob = 1.0 / (z+1);
                if (Math.random() > prob) continue;

        	    var material = new THREE.MeshBasicMaterial({
                    color:
                        randomByte() << 16 |
                        randomByte() << 8 |
                        randomByte()
                });

                var box = new THREE.BoxBufferGeometry( boxSz, boxSz, boxSz );
            	var mesh = new THREE.Mesh( box, material );

                mesh.translateX((x - w*0.5)*boxSz);

                // flip Z and Y
                mesh.translateZ((y - h*0.5)*boxSz);
                mesh.translateY((z+0.5)*boxSz);
                meshes.push(mesh);
            }
        }
    }

    // add plane
    var plane = new THREE.Mesh(
        planeGeometry(2),
        new THREE.MeshBasicMaterial({ color: 0x808080 })
    );
    plane.rotateX( - Math.PI / 2);
    plane.doubleSided = true;
    meshes.push(plane);

    return meshes;
}

function deg2rad(deg) { return Math.PI * deg / 180.0; }

function initShadow() {

    var shadowPitch = deg2rad(40);
    var shadowYaw = deg2rad(30);

    var shadowZ = new THREE.Vector3(
        Math.cos(shadowYaw)*Math.cos(shadowPitch),
        Math.sin(shadowPitch),
        Math.sin(shadowYaw)*Math.cos(shadowPitch)).negate();

    var shadowX = new THREE.Vector3().crossVectors(shadowZ, new THREE.Vector3(0,1,0)).normalize();
    var shadowY = new THREE.Vector3().crossVectors(shadowX, shadowZ);

    var width = 5;
    var height = 5;
    var far = 10.0;

    var pos = shadowZ.clone().multiplyScalar(-2.0);

    var camera = new THREE.OrthographicCamera(
        width / - 2, width / 2, height / 2, height / - 2, 0.0, far );

    camera.matrixAutoUpdate = false;
    camera.matrix
        .makeBasis(shadowX, shadowY, shadowZ.negate())
        .setPosition(pos);
    camera.updateMatrixWorld( true );

    console.log(camera);

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

	camera.position.z = 3;
	scene = new THREE.Scene();

    shaders = {
        ssao: {
    	    uniforms: {
                "cameraMatrix": { value: new THREE.Matrix4() },

    		    "tDepth":       { value: null },
        		"tDiffuse":     { value: null },
    		    "size":         { value: new THREE.Vector2( 512, 512 ) },
    		    "cameraNear":   { value: 1 },
    		    "cameraFar":    { value: 100 },
                "projectionXY": { value: new THREE.Vector2(1, 1) },

                "tShadow":      { value: null },
                "shadowFar":    { value: shadow.far },
                "shadowX":      { value: shadow.xVec },
                "shadowY":      { value: shadow.yVec },
                "shadowZ":      { value: shadow.zVec },
                "shadowOrigin": { value: shadow.pos },
                "shadowSize":   { value: shadow.size }
    	    },

    	    vertexShader: $('#ssao-vertex-shader').text(),
    	    fragmentShader: loadedShaders.ssao.fragment
    	}
    };

    shaders.ssao.material = new THREE.ShaderMaterial(shaders.ssao);

    meshes = generateMeshes();
    meshes.forEach(function(mesh) {
        scene.add(mesh);
    });

	renderer = new THREE.WebGLRenderer();
	renderer.setPixelRatio( window.devicePixelRatio );
	renderer.setSize( window.innerWidth, window.innerHeight );
	document.body.appendChild( renderer.domElement );

	controls = new THREE.OrbitControls( camera, renderer.domElement );
	controls.addEventListener( 'change', render ); // remove when using animation loop
	// enable animation loop when using damping or autorotation
	//controls.enableDamping = true;
	//controls.dampingFactor = 0.25;
	controls.enableZoom = false;

	initPostprocessing();

	window.addEventListener( 'resize', onWindowResize, false );
}

function animate() {
	requestAnimationFrame( animate );
	controls.update();
	render();
}

function onWindowResize() {

	var width = window.innerWidth;
	var height = window.innerHeight;

	camera.aspect = width / height;
	camera.updateProjectionMatrix();
	renderer.setSize( width, height );

	// Resize renderTargets
	//ssaoPass.uniforms[ 'size' ].value.set( width, height );

	var pixelRatio = renderer.getPixelRatio();
	var newWidth  = Math.floor( width / pixelRatio ) || 1;
	var newHeight = Math.floor( height / pixelRatio ) || 1;

    for (var key in frameBuffers) {
        frameBuffers[key].setSize( newWidth, newHeight );
    }

    updateUniforms();
}

function updateUniforms() {
    shaders.ssao.uniforms.size.value.set( window.innerWidth, window.innerHeight );
    shaders.ssao.uniforms.projectionXY.value.set(
        camera.projectionMatrix.elements[0],
        camera.projectionMatrix.elements[5]);
}

function initPostprocessing() {

	// Setup depth pass
	depthMaterial = new THREE.MeshDepthMaterial();
	depthMaterial.depthPacking = THREE.RGBADepthPacking;
	depthMaterial.blending = THREE.NoBlending;

	var pars = { minFilter: THREE.LinearFilter, magFilter: THREE.LinearFilter };
	frameBuffers.depth = new THREE.WebGLRenderTarget( window.innerWidth, window.innerHeight, pars );
    //frameBuffers.ao = new THREE.WebGLRenderTarget( window.innerWidth, window.innerHeight, pars );
    frameBuffers.diffuse = new THREE.WebGLRenderTarget( window.innerWidth, window.innerHeight, pars );
    frameBuffers.shadow = new THREE.WebGLRenderTarget( 3000, 3000, pars );

    shaders.ssao.uniforms.tDiffuse.value = frameBuffers.diffuse.texture;
	shaders.ssao.uniforms.tDepth.value = frameBuffers.depth.texture;
	shaders.ssao.uniforms.cameraNear.value = camera.near;
	shaders.ssao.uniforms.cameraFar.value = camera.far;

    shaders.ssao.uniforms.tShadow.value = frameBuffers.shadow.texture;

    updateUniforms();

}

function render() {

    shaders.ssao.uniforms.cameraMatrix.value.copy(camera.matrixWorld);

    //var camera = shadow.camera;
	// Render depth into frameBuffers.depth
	scene.overrideMaterial = depthMaterial;
	renderer.render( scene, camera, frameBuffers.depth, true );

    renderer.render( scene, shadow.camera, frameBuffers.shadow, true );

	scene.overrideMaterial = null;
    renderer.render( scene, camera, frameBuffers.diffuse );

    scene.overrideMaterial = shaders.ssao.material;
	renderer.render(scene, camera);
}
