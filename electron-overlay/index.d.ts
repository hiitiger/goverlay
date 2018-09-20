/// <reference types="node" />
declare module "electron-overlay" {

    interface IHotkey{
        name: string;
        keyCode: number;
        modifiers?: {
            alt?: boolean;
            ctrl?: boolean;
            shift?: boolean;
            meta?: boolean;
        };
        passthrough?: boolean
    }

    interface IRectangle {
        x: number;
        y: number;
        width: number;
        height: number;
    }
    interface IOverlayWindowDetails{
        name: string;
        transparent: boolean;
        resizable: boolean;
        rect: IRectangle;
        nativeHandle: number;
        caption?: {
            left: number;
            right: number;
            top: number;
            height: number;
        }
    }
 
    export function start(): void;
    export function stop(): void;
    export function setEventCallback(cb: (event: string, ...args: any[]) => void): void;
    export function setHotkeys(hotkeys: IHotkey[]): void;
    export function sendCommand(name: string): void;
    export function addWindow(windowId: number, details: IOverlayWindowDetails): void;
    export function closeWindow(windowId: number): void;
    export function sendWindowBounds(windowId: number, details: {rect: IRectangle}): void;
    export function sendFrameBuffer(windowId: number, buffer: Buffer, width: number, height: number): void;
    export function translateInputEvent(event: {windowId: number, msg: number, wparam: number, lparam: number}): any;
}