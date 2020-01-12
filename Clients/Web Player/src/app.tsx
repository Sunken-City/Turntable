import styled from "styled-components";
import React from "react";
import IconBackURI from "./images/back.png";
import IconPlayURI from "./images/play.png";
import IconPauseURI from "./images/pause.png";
import IconNextURI from "./images/next.png";
import { Background } from "./background";

const ControlsContainer = styled.div`
    position: fixed;

    left: 0px;
    right: 0px;
    bottom: 0px;
`;

const SongAndPlayContainer = styled.div`
    display: flex;
`;

const SongInfo = styled.div`
    color: red;
    font-weight: bold;
`;

const PlayInfo = styled.div`
    color: red;
    font-weight: bold;
    margin: auto;
    margin-right: 0px;
    margin-bottom: 0px;
`;

const ControlsBar = styled.div`
    display: flex;
    height: 100px;

    background: white;
    color: black;
`;

const ControlIconButton = styled.button`
    outline: 0;
    border-style: none;

    &:active:not([disabled]) {
        background: #cccccc;
        transform: translate(1px, 1px);
    }

    &:disabled {
        opacity: 0.5;
    }
`;

const ControlIconImage = styled.img`

`;

function ControlIcon(props: any): JSX.Element {
    return <ControlIconButton {...props}>
        <ControlIconImage src={props.src} />
    </ControlIconButton>;
}

const Controls = styled.div`
    margin: auto;
`;

class State {
    public playing = false;
    public unloaded = true;
}

export class App extends React.PureComponent<{}, State> {
    public state = new State();

    private audio?: HTMLAudioElement;

    public playAudio = (e: React.ChangeEvent<HTMLInputElement>) => {
        if (!e.target.files) {
            return;
        }

        if (this.audio) {
            this.audio.pause();
            this.audio.currentTime = 0;
            this.audio.src = "";
            this.audio = undefined;
        }

        const objURL = window.URL.createObjectURL(e.target.files[0]);
        this.audio = new Audio(objURL);
        this.audio.play();

        this.setState({ playing: true, unloaded: false });
    }

    public togglePauseAudio = () => {
        if (this.audio?.paused) {
            this.audio.play();
            this.setState({ playing: true });
        }
        else {
            this.audio?.pause();
            this.setState({ playing: false });
        }

    }

    public render(): JSX.Element {
        const playIcon = this.state.playing ? IconPauseURI : IconPlayURI;
        return <>
            <Background />
            <ControlsContainer>
                <SongAndPlayContainer>
                    <SongInfo>
                        SONG ARTIST: No Artist <br />
                        SONG ARTIST: No Artist <br />
                        SONG ARTIST: No Artist <br />
                        SONG ARTIST: No Artist <br />
                        SONG ARTIST: No Artist <br />
                        SONG ARTIST: No Artist <br />
                    </SongInfo>
                    <PlayInfo>
                        PlayCounts: 0 <br />
                        RPM: 0 <br />
                        <input type="file" accept="audio/*" onChange={this.playAudio} />
                    </PlayInfo>
                </SongAndPlayContainer>
                <ControlsBar >
                    <Controls>
                        <ControlIcon src={IconBackURI} />
                        <ControlIcon
                            src={playIcon}
                            onClick={this.togglePauseAudio}
                            disabled={this.state.unloaded}
                        />
                        <ControlIcon src={IconNextURI} />
                    </Controls>
                </ControlsBar>
            </ControlsContainer>
        </>;
    }
}
