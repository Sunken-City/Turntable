import React from "react";
import styled from "styled-components";
import * as THREE from "three";
import { TurntableScene } from "./turntable_scene";

const Canvas = styled.canvas`
    width: 100vw;
    height: 100vh;
`;

export class Background extends React.PureComponent {
    private canvasRef = React.createRef<HTMLCanvasElement>();
    private renderer?: THREE.WebGLRenderer;
    private turntableScene = new TurntableScene();
    private lastFrame = performance.now();

    private animate = (timeInfo: number) => {
        const dt = (timeInfo - this.lastFrame) / 1000;
        this.lastFrame = timeInfo;

        this.turntableScene.update(dt);
        if (this.renderer) {
            this.turntableScene.render(this.renderer);
        }

        requestAnimationFrame(this.animate);
    }

    public componentDidMount(): void {
        if (this.canvasRef.current) {
            const canvas = this.canvasRef.current;

            this.renderer = new THREE.WebGLRenderer({ canvas });

            this.updateMount(this.canvasRef.current);

            requestAnimationFrame(this.animate);
        }

        window.addEventListener("resize", this.onResize);
    }

    public render(): JSX.Element {
        return <Canvas ref={this.canvasRef} />;
    }

    private updateMount(mount: HTMLCanvasElement): void {
        const w = document.documentElement.clientWidth;
        const h = document.documentElement.clientHeight;
        this.renderer?.setSize(w, h, true);
        this.turntableScene.onResize(w, h);
    }

    private onResize = () => {
        if (this.canvasRef.current) {
            this.updateMount(this.canvasRef.current);
        }
    }
}