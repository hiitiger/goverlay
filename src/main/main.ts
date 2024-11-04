import { app as ElectronApp } from "electron";

import "./utils/config";

// import { Application } from "./electron/app-entry"
import { IngameOverlay } from "../";

// const appEntry = new Application()
const appEntry = new IngameOverlay();

ElectronApp.disableHardwareAcceleration();

ElectronApp.on("ready", () => {
    appEntry.start();
});

ElectronApp.on("window-all-closed", () => {
    if (process.platform !== "darwin") {
        ElectronApp.quit();
    }
});

ElectronApp.on("activate", () => {
    appEntry.activate();
});
