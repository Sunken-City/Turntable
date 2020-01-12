import * as THREE from "three";

export class TurntableScene {
    public scene = new THREE.Scene();
    private disc = new THREE.Mesh(new THREE.BoxBufferGeometry(1, 1, 1), new THREE.MeshStandardMaterial());
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

        sunLight.lookAt(new THREE.Vector3());

        this.scene.add(sunLight);

        this.scene.add(this.disc);
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
