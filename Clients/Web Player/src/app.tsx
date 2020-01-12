import styled from "styled-components";
import React from "react";
import IconBackURI from "./images/back.png";
import IconPlayURI from "./images/play.png";
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

const ControlIcon = styled.img`
    border-radius: 999px;

    &:active {
        background: #cccccc;
        transform: translate(1px, 1px);
    }
`;

const Controls = styled.div`
    margin: auto;
`;

export function App(): JSX.Element {
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
                    RPM: 0
                </PlayInfo>
            </SongAndPlayContainer>
            <ControlsBar >
                <Controls>
                    {/* <input type="file" accept="audio/*" /> */}
                    <ControlIcon src={IconBackURI} />
                    <ControlIcon src={IconPlayURI} />
                    <ControlIcon src={IconNextURI} />
                </Controls>
            </ControlsBar>
        </ControlsContainer>
    </>;
}