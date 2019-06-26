/// <reference types="node" />
declare module "node-ovhook" {

    export interface IProcessThread {
        processId: number, 
        threadId: number;
    }

    export interface IWindow extends IProcessThread {
        windowId: number;
        title: string;
    }

    export interface IInjectResult {
        injectHelper: string;
        injectDll: string;
        injectSucceed: boolean;
    }

    export function getTopWindows(): IWindow[];
    export function injectProcess(process: IProcessThread): IInjectResult;
}