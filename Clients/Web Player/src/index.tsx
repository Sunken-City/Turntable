import * as React from "react";
import * as ReactDOM from "react-dom";

// this magic prevents right clicking everywhere
document.oncontextmenu = (e) => {
    e.preventDefault();
};

window.onload = () => {

    // remove everything from the barebones loader
    const myNode = document.body;
    while (myNode.firstChild) {
        myNode.removeChild(myNode.firstChild);
    }

    document.body.style.background = "black";
    document.body.style.fontFamily = "sans-serif";
    document.body.style.color = "white";

    // why does body have a margin for some reason?
    document.body.style.margin = "0px";

    // this function just starts React which bootstraps everything else
    const reactDiv = document.createElement("div");

    ReactDOM.render(
        <div>
            Hello World uwu
        </div>,
        document.body.appendChild(reactDiv),
    );
};
