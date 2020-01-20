import * as THREE from "three";

export class TurntableScene {
    public scene = new THREE.Scene();
    private material = new THREE.MeshStandardMaterial({

    });

    private fboVertexShader = `
        uniform vec4 size;

        void main() {
            vec2 transformedPos = position.xy * size.xy - vec2(1.0, -1.0) + vec2( size.x ,  -size.y ) + vec2( size.w , - size.z ) * 2.;
	
            gl_Position = vec4( transformedPos , 1.0 , 1.0 );
        }
    `;

    private fboFragmentShader = `
    void main() {
        float gTime = 1.0;

        vec4 earthboundBlue = vec4(0.407, 0.659, 0.847, 1.0);
        vec4 earthboundGreen = vec4(0.500, 0.847, 0.565, 1.0);
        vec2 aspectRatio = vec2(16, 9) * 3.0;
        float timeFactor = gTime / 20.0;

        vec2 uv = (vec2(1.0, 1.0) + vec2(timeFactor, timeFactor));
        uv = uv * aspectRatio; //Multiply by the aspect ratio to make our checkerboard squares square

        int xCoordinate = int(round(uv.x));
        int yCoordinate = int(round(uv.y));
        int squareNumber = xCoordinate + (int(aspectRatio.x) * yCoordinate);

        squareNumber += yCoordinate % 2; //Change the colors every other row
        float interpolator = float(squareNumber % 2);
        gl_FragColor = mix(earthboundBlue, earthboundGreen, interpolator);
    }
    `;

    private fbo = new THREE.Mesh(new THREE.PlaneBufferGeometry(2, 2, 1, 1), new THREE.ShaderMaterial({
        uniforms: {
            size: {
                type: "v4",
                value: new THREE.Vector4(1, 1, 0, 0),
            }
        },

        vertexShader: this.fboVertexShader,

        fragmentShader: this.fboFragmentShader,

        depthWrite: false,
    }));

    private disc = new THREE.Mesh(new THREE.CylinderBufferGeometry(3, 3, 0.1, 50), this.material);
    private camera = new THREE.PerspectiveCamera(50);

    constructor() {
        this.scene.background = new THREE.Color("green");

        this.camera.position.z = 10;
        this.camera.position.y = 5;

        this.camera.lookAt(new THREE.Vector3());

        this.scene.add(new THREE.AmbientLight(0xfffffff, 0.5));

        const sunLight = new THREE.DirectionalLight(0xffffff, 0.7);
        sunLight.position.x = 10;
        sunLight.position.y = 10;
        sunLight.position.z = 10;

        // this.material.color = new THREE.Color();
        this.material.roughness = 0.9;
        this.material.metalness = 0.9;

        sunLight.lookAt(new THREE.Vector3());

        this.scene.add(sunLight);

        this.scene.add(this.disc);
        this.scene.add(this.fbo);
    }

    public onResize(w: number, h: number): void {
        this.camera.aspect = w / h;
        this.camera.updateProjectionMatrix();
    }

    public update(dt: number): void {
        this.disc.rotateY(1 * dt);
    }

    public render(renderer: THREE.WebGLRenderer): void {
        renderer.render(this.scene, this.camera);
    }
}
