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

    export interface IInjectConfig {
        dllPath: string,
        dllPath64: string, 
        helper: string, 
        helper64: string
    }

    export function getTopWindows(): IWindow[];
    export function injectProcess(process: IProcessThread, config: IInjectConfig): IInjectResult;
}