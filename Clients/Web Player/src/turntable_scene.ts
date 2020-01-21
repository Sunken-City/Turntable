import * as THREE from "three";

export class TurntableScene {
    public totalTime = 0;
    public scene = new THREE.Scene();
    private material = new THREE.MeshStandardMaterial({

    });

    private fboVertexShader = `
        uniform vec4 size;
        uniform float iTime;

        void main() {
            vec2 transformedPos = position.xy * size.xy - vec2(1.0, -1.0) + vec2( size.x ,  -size.y ) + vec2( size.w , - size.z ) * 2.;
	
            gl_Position = vec4( transformedPos , 1.0 , 1.0 );
        }
    `;

    private fboFragmentShaderHeader = `
    uniform float iTime;
    uniform vec3 iResolution;
    `;

    private fragmentShader = `

    void mainImage( out vec4 fragColor, in vec2 fragCoord )
    {
      vec2 passUV = fragCoord/iResolution.xy;
      vec4 lightBlue = vec4(0.49, 0.796, 0.745, 1.0);
      vec4 darkBlue = vec4(0.0, 0.224, 0.431, 1.0);
      float timeFactor = iTime / 20.0f;
      vec2 uv = (passUV + vec2(timeFactor, timeFactor)) * vec2(16, 9);
      if(abs(fract(uv.y) - fract(uv.x)) < 0.1f)
      {
        fragColor = lightBlue;
      }
      else
      {
        fragColor = darkBlue;
      }
    }
    `;

    private fboFragmentShaderFooter = `
    void main() {
        mainImage(gl_FragColor, gl_FragCoord.xy);
    }`;

    private fboUniforms = {
        iTime: { value: 0 },
        size: {
            value: new THREE.Vector4(1, 1, 0, 0),
        },
        iResolution: {
            value: new THREE.Vector3(1, 1, 1),
        },
    };

    private fbo = new THREE.Mesh(new THREE.PlaneBufferGeometry(2, 2, 1, 1), new THREE.ShaderMaterial({
        uniforms: this.fboUniforms,

        vertexShader: this.fboVertexShader,

        fragmentShader: this.fboFragmentShaderHeader + this.fragmentShader + this.fboFragmentShaderFooter,

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

        this.fbo.frustumCulled = false;
        this.fbo.renderOrder = -1;

        this.scene.add(this.disc);
        this.scene.add(this.fbo);
    }

    public onResize(w: number, h: number): void {
        this.camera.aspect = w / h;
        this.camera.updateProjectionMatrix();
        this.fboUniforms.size.value.x = w;
        this.fboUniforms.size.value.y = h;
        this.fboUniforms.iResolution.value.set(w, h, 1);
    }

    public update(dt: number): void {
        this.totalTime += dt;
        this.disc.rotateY(1 * dt);
        this.fboUniforms.iTime.value = this.totalTime;
    }

    public render(renderer: THREE.WebGLRenderer): void {
        renderer.render(this.scene, this.camera);
    }
}
