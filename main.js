var camera, scene, renderer, controls;

var depthMaterial;
var shaders;
var meshes;
var frameBuffers = {};

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
    meshes.push(plane);

    return meshes;
}

function init(loadedShaders) {
	camera = new THREE.PerspectiveCamera( 70, window.innerWidth / window.innerHeight, 1, 10 );
	camera.position.z = 3;
	scene = new THREE.Scene();

    shaders = {
        ssao: {
    	    uniforms: {
    		    "tDepth":       { value: null },
        		"tDiffuse":     { value: null },
    		    "size":         { value: new THREE.Vector2( 512, 512 ) },
    		    "cameraNear":   { value: 1 },
    		    "cameraFar":    { value: 100 },
                "projectionXY": { value: new THREE.Vector2(1, 1) }
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

	// Setup render pass
	var renderPass = new THREE.RenderPass( scene, camera );

	// Setup depth pass
	depthMaterial = new THREE.MeshDepthMaterial();
	depthMaterial.depthPacking = THREE.RGBADepthPacking;
	depthMaterial.blending = THREE.NoBlending;

	var pars = { minFilter: THREE.LinearFilter, magFilter: THREE.LinearFilter };
	frameBuffers.depth = new THREE.WebGLRenderTarget( window.innerWidth, window.innerHeight, pars );
    //frameBuffers.ao = new THREE.WebGLRenderTarget( window.innerWidth, window.innerHeight, pars );
    frameBuffers.diffuse = new THREE.WebGLRenderTarget( window.innerWidth, window.innerHeight, pars );

    shaders.ssao.uniforms.tDiffuse.value = frameBuffers.diffuse.texture;
	shaders.ssao.uniforms.tDepth.value = frameBuffers.depth.texture;
	shaders.ssao.uniforms.cameraNear.value = camera.near;
	shaders.ssao.uniforms.cameraFar.value = camera.far;

    updateUniforms();

}

function render() {
	// Render depth into frameBuffers.depth
	scene.overrideMaterial = depthMaterial;
	renderer.render( scene, camera, frameBuffers.depth, true );

	scene.overrideMaterial = null;
    renderer.render( scene, camera, frameBuffers.diffuse );

    scene.overrideMaterial = shaders.ssao.material;
	renderer.render(scene, camera);
}
